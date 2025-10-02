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
    
	// 로컬 컨트롤러만 클라이언트 side 리소스 초기화 요청 및 네트워크 타이머 동기화 체크 요청
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

void ACYPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    
    Super::EndPlay(EndPlayReason);
}

void ACYPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();
    
    if (IsLocalController())
    {
        CheckNetworkTimerSync();
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
    // 입력 모드를 게임 전용(Game Only)으로 설정
    // 이 작업은 클라이언트에서만 수행되어야 함 (PlayerState::NotifyControllerPawnDataReady에서 이미 확인)
    
    // 1. 입력 모드 설정 -> 게임전용 (Game Only) 설정
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    // 2. 마우스 커서 숨기기 (Game Only 모드의 기본 동작을 따름)
    bShowMouseCursor = false;
    
    // 3. Pawn에 입력 활성화: GetPawn()이 유효한지 확인하고 입력 활성화를 명시적으로 호출할 수 있습니다.
    // Enhanced Input System을 사용한다면 Pawn/Character에 Input Mapping Context를 추가하는 로직이 필요합니다.
    if (APawn* MyPawn = GetPawn())
    {
        // 일반적으로 Character/Pawn에서 AutoReceiveInput=Player0으로 설정되어 있으면 별도의 EnableInput은 필요 없으나,
        // 명시적으로 설정해주거나, Pawn의 Input Component에 Enhanced Input Mapping Context를 추가하는 로직을 호출해야 합니다.
        // Enhanced Input을 사용한다고 가정하고, Pawn/Character가 입력을 받을 준비를 하도록 합니다.
        
        // 예시: 캐릭터에 Enhanced Input Mapping Context를 추가하는 함수를 Character 클래스에 만들고 여기서 호출합니다.
        // todo: Input Data 한 곳으로 옮길 예정
        // if (ACYCharacterBase* Character = Cast<ACYCharacterBase>(MyPawn))
        // {
        //     Character->SetupPlayerInput(); // Character 클래스에서 구현
        // }
    }
    
    // 디버그 출력 (선택 사항)
    UE_LOG(LogTemp, Warning, TEXT("Local PlayerController: Input Mode set to Game Only."));
    
    if (IsLocalController())
    {
        CheckClientInitialization();
    }
    
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

    // if (++InitializationRetryCount > MaxRetryCount)
    // {
    //     UE_LOG(LogCY, Error, TEXT("Failed to initialize client after %d attempts"), MaxRetryCount);
    //     return; // 포기
    // }
    
    // 초기화 가능 체크
    if (CanInitializeClient())
    {
        // 초기화
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
                0.2f,  // 200ms 후 재시도
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
    InitializationRetryCount = 0;

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

void ACYPlayerController::CheckNetworkTimerSync()
{
    if (HasAuthority())
    {
        ClientServerDeltaTime = 0.f;
        return;
    }
    
    ServerRequestServerTime(GetWorld()->GetTimeSeconds());

    if (!GetWorld()->GetTimerManager().IsTimerActive(ResyncTimerHandle))
    {
        TWeakObjectPtr<ACYPlayerController> WeakThis(this);
        GetWorld()->GetTimerManager().SetTimer(
           ResyncTimerHandle,
           [WeakThis]()
           {
               if (WeakThis.IsValid())
               {
                   const float LocalTime = WeakThis->GetWorld()->GetTimeSeconds();
                   WeakThis->ServerRequestServerTime(LocalTime);
               }
           },
           ResyncInterval, true
       );
    }
}

void ACYPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
    float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
    ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ACYPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
    float TimeServerReceivedClientRequest)
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
    SingleTripTime = 0.5f * RoundTripTime;
    float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime; // 예측한 현재 서버의 시간
    ClientServerDeltaTime = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ACYPlayerController::GetServerTime()
{
    if (HasAuthority())
    {
        return GetWorld()->GetTimeSeconds();
    }
    
    return GetWorld()->GetTimeSeconds() + ClientServerDeltaTime;
}
