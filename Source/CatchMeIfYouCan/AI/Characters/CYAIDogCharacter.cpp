// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/CYAIDogCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


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

	//회전 설정
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	
}

void ACYAIDogCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	// 복제 변수 등록
	DOREPLIFETIME(ThisClass, bIsBarking);    // 감지 상태
	DOREPLIFETIME(ThisClass, AISpeed);       // 속도
	DOREPLIFETIME(ThisClass, AIDirection);   // 방향 각도
}

void ACYAIDogCharacter::SetBarkingState(bool bNewBarking)
{
	// 서버에서만 상태 변경 가능
	if (HasAuthority())
	{
		if (bIsBarking != bNewBarking)
		{
			bIsBarking = bNewBarking;
		}
	}
}

//추후 필요시 추가 아니면 최종 버전에서 삭제
//void ACYAIDogCharacter::ServerSetBarkingState_Implementation(bool bNewBarking)
//{
	// 클라이언트에서 서버로의 RPC 요청 처리
//	SetBarkingState(bNewBarking);
//}

void ACYAIDogCharacter::ONRep_IsBarking()
{
	// 데디케이티드 서버에서 이벤트 실행 x
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
    
	// 클라이언트에서 상태에 따른 시각적 효과 실행
	if (bIsBarking)
	{
		OnStartBarkingVisuals();  // 블루프린트 이벤트 호출

		//추후 사용할 위치 전달용 함수 =============================================================
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController && PlayerController->GetPawn())
		{
			APawn* MyPawn = PlayerController->GetPawn();
			if (MyPawn->ActorHasTag(FName("Police")))
			{
				FVector DogLocation = GetActorLocation();
			}

		}
		//==========================================================================================
	}
	else
	{
		OnStopBarkingVisuals();   // 블루프린트 이벤트 호출
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