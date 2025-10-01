// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/CYAIDogCharacter.h"

#include "AI/Controllers/CYAIDogController.h"
#include "AI/Managers/CYGuardDogPoolManager.h"
#include "AI/Managers/CYSplineManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACYAIDogCharacter::ACYAIDogCharacter()
{

	//네트워크 복제 활성화
	bReplicates = true;
	AutoPossessAI= EAutoPossessAI::PlacedInWorldOrSpawned;

	// 기본 이동 속도 설정
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;

	//초기값 설정
	bIsBarking = false;
	AISpeed = 0.0f;
	AIDirection = 0.0f;

	// 회전 설정 개선
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	// 회전 속도 조정
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	
}


void ACYAIDogCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	// 복제 변수 등록
	DOREPLIFETIME(ThisClass, bIsBarking);    // 감지 상태
	DOREPLIFETIME(ThisClass, AISpeed);       // 속도
	DOREPLIFETIME(ThisClass, AIDirection);   // 방향 각도
	DOREPLIFETIME(ThisClass, TargetPatrolPath);//패스

}
//아웃라인 함수 구현
void ACYAIDogCharacter::Multicast_SetTargetOutline_Implementation(AActor* TargetActor, bool bEnable)
{
	//타겟이 유효하지 않을시 리턴
	if (!TargetActor|| !IsValid(TargetActor))return;

	//컴포넌트 가져오기
	UPrimitiveComponent* MeshComponent = TargetActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComponent)
	{
		MeshComponent = TargetActor->FindComponentByClass<UStaticMeshComponent>();
	}
	
	//커스텀 뎁스 렌더링 및 스텐실 값 설정
	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(bEnable);
		if (bEnable)
		{
			MeshComponent->SetCustomDepthStencilValue(1);
		}
	}
}



//풀 매니저 설정
void ACYAIDogCharacter::SetPoolManager(ACYGuardDogPoolManager* InManager)
{
	PoolManager = InManager;
}

//경비견 활성화
void ACYAIDogCharacter::ActivateDog(FVector SpawnLocation, AActor* NewPatrolPath)
{
	if (!HasAuthority()) return;

	//위치 및 순찰 경로 설정
	SetActorLocation(SpawnLocation);
	TargetPatrolPath = NewPatrolPath;
    
	//애니메이션 변수 초기화
	UpdateAIAnimationVariables(0.0f, 0.0f);
    
	// AI 컨트롤러를 가져옵니다.
	ACYAIDogController* DogController = Cast<ACYAIDogController>(GetController());
	if (DogController)
	{
		// 컨트롤러에서 블랙보드 컴포넌트를 가져옵니다.
		UBlackboardComponent* BBComp = DogController->GetBlackboardComponent();
		if (BBComp)
		{
          
			// 기존 추적/공격 상태를 초기화합니다.
			BBComp->SetValueAsObject(FName("Target"), nullptr);
			BBComp->SetValueAsBool(FName("bIsBarking"), false);
            
			// 새로 부여된 순찰 임무를 블랙보드에 설정합니다.
			BBComp->SetValueAsObject(FName("PatrolPathActor"), NewPatrolPath);
          
			// AI가 처음에는 스플라인 위에 있지 않다고 상태를 초기화합니다.
			BBComp->SetValueAsBool(FName("bOnSpline"), false);
          
			// 스플라인 순찰 진행도를 0으로 초기화합니다.
			BBComp->SetValueAsFloat(FName("CurrentSplineDistance"), 0.0f);
		}
		// AI 로직 (행동 트리)을 시작/재시작합니다.
		DogController->StartLogic();
	}

	// 4. 캐릭터 상태 초기화
	SetBarkingState(false);

	// 5. 모든 클라이언트에게 활성화 상태를 알림
	Multicast_OnStateChanged(true);
}


//경비견 비활성화 실제 서버에서만 실행
void ACYAIDogCharacter::DeactivateDog()
{
	//서버인 경우만 실행
	if (HasAuthority())
	{
		DeactivateDog_Internal();
	}
}
//경비견 실제 비활성화 함수
void ACYAIDogCharacter::DeactivateDog_Internal()
{
	//AI 로직 중지
	ACYAIDogController* DogController= Cast<ACYAIDogController>(GetController());
	if (DogController)
	{
		//비활성화 시 타겟의 아웃라인을 끄도록 처리
		if (AActor* CurrentTarget = Cast<AActor>(DogController->GetBlackboardComponent()->GetValueAsObject(FName("Target"))))
		{
			Multicast_SetTargetOutline(CurrentTarget, false);
		}
		
		DogController->StopLogic();

		// 블랙보드 상태 리셋
		UBlackboardComponent* BBComp = DogController->GetBlackboardComponent();
		if (BBComp)
		{
			BBComp->SetValueAsBool(FName("bOnSpline"), false);
			BBComp->SetValueAsFloat(FName("CurrentSplineDistance"), 0.0f);
			BBComp->SetValueAsObject(FName("Target"), nullptr);
			BBComp->SetValueAsBool(FName("bIsBarking"), false);
		}
	}
	//애니메이션 초기화
	UpdateAIAnimationVariables(0.0f, 0.0f);
	//스플라인 반환
	ACYSplineManager* SplineManager=ACYSplineManager::GetInstance(GetWorld());
	if (SplineManager&& TargetPatrolPath)
	{
		//스플라인 매니저에서 반환 및 현재 경비견의 스플라인 nullptr로 설정
		SplineManager->ReleaseSpline(TargetPatrolPath);
		TargetPatrolPath = nullptr;
	}
	if (PoolManager)
	{
		PoolManager->ReturnDogToPool(this);
	}

	Multicast_OnStateChanged(false);
}

//풀링 함수
void ACYAIDogCharacter::Multicast_OnStateChanged_Implementation(bool bIsActive)
{
	// 액터를 보이게/숨기고, 충돌 및 틱을 활성화/비활성화합니다.
	SetActorHiddenInGame(!bIsActive);
	SetActorEnableCollision(bIsActive);
	SetActorTickEnabled(bIsActive);

	// 비활성화 시에는 월드 밖 안전한 장소로 이동시킬 수도 있습니다.
	if (!bIsActive)
	{
		SetActorLocation(FVector(0, 0, -10000)); // 예: 맵 아래
	}
}


void ACYAIDogCharacter::SetBarkingState(bool bNewBarking)
{
	if (HasAuthority())
	{
		if (bIsBarking != bNewBarking)
		{
			bIsBarking = bNewBarking;
			
			//리슨서버 전용
			if(GetNetMode() != NM_DedicatedServer)
			{
				ONRep_IsBarking();
			}
		}
	}
}

void ACYAIDogCharacter::ServerSetBarkingState_Implementation(bool bNewBarking)
{
	SetBarkingState(bNewBarking);
}

void ACYAIDogCharacter::ONRep_IsBarking()
{
	// 데디케이티드 서버에서는 시각적 효과를 실행하지 않음
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
    
	if (bIsBarking)
	{
		OnStartBarkingVisuals();
	}
	else
	{
		OnStopBarkingVisuals();
	}
}

//btt 태스크에서 사용할 속도, 회전 함수
void ACYAIDogCharacter::UpdateAIAnimationVariables(float NewSpeed, float NewDirection)
{
	// 서버에서만 애니메이션 변수 업데이트
	if (HasAuthority())
	{
		AISpeed = NewSpeed;
		AIDirection = NewDirection;
	}
}

void ACYAIDogCharacter::SetSummoner(AActor* InSummoner) { Summoner = InSummoner; }
AActor* ACYAIDogCharacter::GetSummoner() const { return Summoner.Get(); }