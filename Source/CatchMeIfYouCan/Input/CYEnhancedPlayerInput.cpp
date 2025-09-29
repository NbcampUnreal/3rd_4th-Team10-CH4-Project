// Fill out your copyright notice in the Description page of Project Settings.


#include "CYEnhancedPlayerInput.h"

#include "EnhancedActionKeyMapping.h"

UCYEnhancedPlayerInput::UCYEnhancedPlayerInput()
{
    
}

void UCYEnhancedPlayerInput::FlushPressedInput(UInputAction* InputAction)
{
	const TArray<FEnhancedActionKeyMapping>& KeyMappings = GetEnhancedActionMappings();
	for (const FEnhancedActionKeyMapping& KeyMapping : KeyMappings)
	{
		if (KeyMapping.Action == InputAction)
		{
			APlayerController* PlayerController = GetOuterAPlayerController();
			ULocalPlayer* LocalPlayer = PlayerController ? Cast<ULocalPlayer>(PlayerController->Player) : nullptr;
			UGameViewportClient* GameViewportClient = LocalPlayer ? LocalPlayer->ViewportClient : nullptr;
			FViewport* Viewport = GameViewportClient ? GameViewportClient->Viewport : nullptr;
			if (LocalPlayer)
			{
				if (FKeyState* KeyState = GetKeyStateMap().Find(KeyMapping.Key))
				{
					if (KeyState->bDown)
					{
						FInputKeyEventArgs Params = FInputKeyEventArgs::CreateSimulated(
							KeyMapping.Key,
							IE_Released,
							0.f,
							1,
							FInputDeviceId(),
							false,
							Viewport
							);
						InputKey(Params);
					}
				}
			}
			
			UWorld* World = GetWorld();
			check(World);
			float TimeSeconds = World->GetRealTimeSeconds();

			// KeyState 수동 리셋
			if (FKeyState* KeyState = GetKeyStateMap().Find(KeyMapping.Key))
			{
				KeyState->RawValue = FVector(0.f, 0.f, 0.f);
				KeyState->bDown = false;
				KeyState->bDownPrevious = false;
				KeyState->LastUpDownTransitionTime = TimeSeconds;
			}
			
			bIsFlushingInputThisFrame = true;
		}
	}
}

FKey UCYEnhancedPlayerInput::GetKeyForAction(UInputAction* InputAction) const
{
	const TArray<FEnhancedActionKeyMapping>& KeyMappings = GetEnhancedActionMappings();
	for (const FEnhancedActionKeyMapping& KeyMapping : KeyMappings)
	{
		if (KeyMapping.Action == InputAction)
			return KeyMapping.Key;
	}

	return FKey();
}