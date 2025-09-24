// Fill out your copyright notice in the Description page of Project Settings.


#include "CYPlayerController.h"

#include "CYLogChannels.h"
#include "CYPlayerState.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/CYVitalSet.h"
#include "Character/CYPawnData.h"
#include "Character/CYPlayerCharacter.h"
#include "GameModes/InGame/CYInGameState.h"
#include "UI/HUD/CYHUD.h"

ACYPlayerController::ACYPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ACYPlayerController::BeginPlay()
{
	Super::BeginPlay();
    
	// 로컬 컨트롤러만 HUD 초기화 대기
	if (IsLocalController())
	{
		UE_LOG(LogCY, Log, TEXT("PlayerController BeginPlay - Starting HUD initialization check"));
        
	    // GameState 변경 감지 등록
	    if (UWorld* World = GetWorld())
	    {
	        World->GameStateSetEvent.AddUObject(this, &ACYPlayerController::OnGameStateSet);
	    }
        
	    // 첫 초기화 체크
	    CheckClientInitialization();
	}
}

ACYPlayerState* ACYPlayerController::GetCYPlayerState() const
{
	return CastChecked<ACYPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UCYAbilitySystemComponent* ACYPlayerController::GetCYAbilitySystemComponent() const
{
	const ACYPlayerState* CYPS = GetCYPlayerState();
	return (CYPS ? CYPS->GetCYAbilitySystemComponent() : nullptr);
}

void ACYPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UCYAbilitySystemComponent* CYASC = GetCYAbilitySystemComponent())
	{
		CYASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ACYPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    if (IsLocalController())
    {
        UE_LOG(LogCY, Verbose, TEXT("OnRep_PlayerState - Checking initialization"));
        CheckClientInitialization();
    }
}

void ACYPlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();
    
    if (IsLocalController())
    {
        UE_LOG(LogCY, Warning, TEXT("OnRep_Pawn - Checking initialization"));
        CheckClientInitialization();
    }
}

void ACYPlayerController::OnGameStateSet(AGameStateBase* NewGameState)
{
    if (IsLocalController())
    {
        UE_LOG(LogCY, Warning, TEXT("GameState set - Checking initialization"));
        CheckClientInitialization();
    }
}

void ACYPlayerController::OnPawnDataReady()
{
    if (!IsLocalController())
    {
        return;
    }
    
    CheckClientInitialization();
}

void ACYPlayerController::CheckClientInitialization()
{
    // 이미 초기화 완료
    if (bClientInitialized)
    {
        return;
    }
    
    // 로컬 컨트롤러만
    if (!IsLocalController())
    {
        return;
    }

    // 초기화 가능 체크
    if (CanInitializeClient())
    {
        InitializeClient();
    }
    else
    {
        // 재시도 타이머 설정
        if (!GetWorld()->GetTimerManager().IsTimerActive(InitCheckTimer))
        {
            GetWorld()->GetTimerManager().SetTimer(
                InitCheckTimer,
                this,
                &ACYPlayerController::CheckClientInitialization,
                0.1f,  // 100ms 후 재시도
                false
            );
        }
    }
}

bool ACYPlayerController::CanInitializeClient() const
{
    // 1. PlayerState 및 PawnData 체크 
    ACYPlayerState* PS = GetCYPlayerState();
    if (!PS)
    {
        return false;
    }
    
    const UCYPawnData* PawnData = PS->GetPawnData();
    if (!PawnData)
    {
        return false;
    }
    
    // PawnData에 UI 클래스가 설정되어 있는지 체크 (추후 사용)
    // if (!PawnData->OverlayWidgetClass || !PawnData->OverlayWidgetControllerClass)
    // {
    //     UE_LOG(LogCY, Warning, TEXT("PawnData has no UI classes set"));
    //     return false;
    // }
    
    // 2. Pawn 체크
    ACYPlayerCharacter* CYCharacter = Cast<ACYPlayerCharacter>(GetPawn());
    if (!CYCharacter)
    {
        return false;
    }
    
    // 3. ASC 체크 (초기화 완료 여부)
    UCYAbilitySystemComponent* ASC = GetCYAbilitySystemComponent();
    if (!ASC || !ASC->AbilityActorInfo.IsValid())
    {
        return false;
    }
    
    // 4. VitalSet 체크
    if (!PS->GetVitalSet())
    {
        return false;
    }
    
    // 5. GameState 체크
    if (!GetWorld()->GetGameState<ACYInGameState>())
    {
        return false;
    }
    
    // 6. HUD 체크
    if (!GetHUD())
    {
        return false;
    }
    
    // 모든 조건 충족
    return true;
}

void ACYPlayerController::InitializeClient()
{
    ACYPlayerState* PS = GetCYPlayerState();
    const UCYPawnData* PawnData = PS->GetPawnData();
    
    // PawnData 변경 체크 (팀 변경 등)
    if (CachedPawnData && CachedPawnData != PawnData)
    {
        UE_LOG(LogCY, Warning, TEXT("PawnData changed! Recreating HUD"));
        // TODO: 기존 HUD 정리 로직
    }
    
    // HUD 생성
    CreateTeamSpecificHUD(PawnData);
    
    // 상태 업데이트
    CachedPawnData = PawnData;
    bClientInitialized = true;

    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(InitCheckTimer);
}

void ACYPlayerController::CreateTeamSpecificHUD(const UCYPawnData* PawnData)
{
    if (!PawnData)
    {
        UE_LOG(LogCY, Error, TEXT("CreateTeamSpecificHUD: Invalid PawnData"));
        return;
    }
    
    ACYHUD* HUD = Cast<ACYHUD>(GetHUD());
    if (!HUD)
    {
        UE_LOG(LogCY, Error, TEXT("CreateTeamSpecificHUD: No HUD"));
        return;
    }
    
    // 필요한 모든 데이터 수집
    ACYPlayerState* PS = GetCYPlayerState();
    UCYAbilitySystemComponent* ASC = GetCYAbilitySystemComponent();

    // TODO : VitalSet을 바로 넘겨주는 대신 ASC에 스폰된 Attribute에 따라 UI 컨트롤러에서 바인딩하도록 설계 고려
    UCYVitalSet* VitalSet = PS->GetVitalSet();
    
    ACYInGameState* GameState = GetWorld()->GetGameState<ACYInGameState>();
    
    // 기본 HUD 초기화 (추후 PawnData에서 위젯 클래스 가져와서 사용)
    // TSubclassOf<UCYUserWidget> WidgetClass = PawnData->OverlayWidgetClass;
    // TSubclassOf<UCYOverlayWidgetController> ControllerClass = PawnData->OverlayWidgetControllerClass;
    
    // 현재는 기본 HUD 사용
    HUD->InitOverlay(this, PS, ASC, VitalSet, GameState);

}