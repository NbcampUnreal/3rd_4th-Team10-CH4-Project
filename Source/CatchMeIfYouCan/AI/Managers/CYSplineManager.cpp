// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Managers/CYSplineManager.h"

#include "EngineUtils.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

// 싱글톤 인스턴스 초기화
ACYSplineManager* ACYSplineManager::Instance = nullptr;

ACYSplineManager::ACYSplineManager()
{
	//틱 사용 X
	PrimaryActorTick.bCanEverTick = false;

	//싱글톤 인스턴스 설정
	Instance = this;

}

void ACYSplineManager::BeginPlay()
{
	Super::BeginPlay();

	//레벨에서 스플라인들을 찾아 자동으로 등록
	AutoRegisterSplinesFromLevel();

	UE_LOG(LogTemp, Warning, TEXT("SplineManager: 총 %d개의 스플라인이 등록되었습니다."),AvailableSplines.Num());
	
}

void ACYSplineManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//모든 스플라인 해제
	ReleaseAllSplines();

	//싱글톤 인스턴스 정리
	if (Instance == this)
	{
		Instance=nullptr;
	}

	Super::EndPlay(EndPlayReason);
}


//스플라인 찾아오기
void ACYSplineManager::RegisterSplines(const TArray<AActor*>& Splines)
{
	int32 AddedCount = 0;

	for (AActor* SplineActor :Splines)
	{
		
		// 유효성 검사
		if (!IsValidSpline(SplineActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineManager: 유효하지 않은 스플라인 액터 %s"), SplineActor ? *SplineActor->GetName() : TEXT("NULL"));
			continue;
		}
		
		// 중복 등록 방지
		if (IsAlreadyRegistered(SplineActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineManager: 스플라인 %s는 이미 등록되어 있습니다"), *SplineActor->GetName());
			continue;
		}

		//사용 가능한 목록에 등록
		AvailableSplines.Add(SplineActor);
		AddedCount++;
		
		UE_LOG(LogTemp, Log, TEXT("SplineManager: 스플라인 %s를 등록했습니다"), *SplineActor->GetName());

	}
	
	UE_LOG(LogTemp, Log, TEXT("SplineManager: %d개의 새로운 스플라인을 등록했습니다. 총 %d개"), AddedCount, AvailableSplines.Num());

}

//소환 컴포넌트에서 호출될 스플라인 할당 함수
AActor* ACYSplineManager::AssignAvailableSpline()
{
	
	// 유효하지 않은 스플라인들 정리
	CleanupInvalidSplines();
	
	// 사용 가능한 스플라인이 있는지 확인
	if (AvailableSplines.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineManager: 사용 가능한 스플라인이 없습니다"));
		return nullptr;
	}
	
	// 첫 번째 사용 가능한 스플라인 할당
	AActor* AssignedSpline = AvailableSplines[0];
	AvailableSplines.RemoveAt(0);
	OccupiedSplines.Add(AssignedSpline);
	
	UE_LOG(LogTemp, Log, TEXT("SplineManager: 스플라인 %s 할당 완료. 남은 스플라인: %d개, 사용중: %d개"), *AssignedSpline->GetName(), AvailableSplines.Num(), OccupiedSplines.Num());
	
	return AssignedSpline;
}

void ACYSplineManager::ReleaseSpline(AActor* SplineActor)
{

	
	if (!SplineActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineManager: null 스플라인을 해제하려고 시도했습니다"));
		return;
	}
	
	int32 RemovedCount = OccupiedSplines.Remove(SplineActor);

	//만약 제거된 개수가 1 이상이라면
	if (RemovedCount > 0)
	{
		// 사용 가능한 스플라인 목록에 다시 추가 (유효한 경우에만)
		if (IsValidSpline(SplineActor))
		{
			AvailableSplines.Add(SplineActor);
			UE_LOG(LogTemp, Log, TEXT("SplineManager: 스플라인 %s 해제 완료. 사용 가능: %d개, 사용중: %d개"), *SplineActor->GetName(), AvailableSplines.Num(), OccupiedSplines.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineManager: 유효하지 않은 스플라인 %s를 해제했습니다"), *SplineActor->GetName());		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineManager: 스플라인 %s는 사용중이 아닙니다"), *SplineActor->GetName());
	}
}

//사용중인 모든 스플라인 반환
void ACYSplineManager::ReleaseAllSplines()
{

	//사용 가능한 배열에 추가
	for (AActor* SplineActor : OccupiedSplines)
	{
		if (IsValidSpline(SplineActor))
		{
			AvailableSplines.Add(SplineActor);
		}
	}
	
	//사용중인 액터 배열 비우기
	OccupiedSplines.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("SplineManager: 전체 스플라인 해제 완료. 사용 가능한 스플라인: %d개"), AvailableSplines.Num());

}

//사용가능한 배열 개수 가져오기
int32 ACYSplineManager::GetAvailableSplineCount() const
{
	return AvailableSplines.Num();
}

//스플라인 매니저 가져오기
ACYSplineManager* ACYSplineManager::GetInstance(UWorld* World)
{
	if (!Instance && World)
	{
		// 월드에서 SplineManager 찾기
		for (TActorIterator<ACYSplineManager> ActorItr(World); ActorItr; ++ActorItr)
		{
			Instance = *ActorItr;
			break;
		}
	}
	return Instance;
}

//시작시 월드에 존재하는 스플라인 찾아오기
void ACYSplineManager::AutoRegisterSplinesFromLevel(const FName& SplineTag)
{
	UWorld* World= GetWorld();
	if (!World)
	{
		return;
	}

	//스플라인 태그를 가진 모든 액터 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, SplineTag, FoundActors);
	
	// 스플라인 컴포넌트가 있는 액터들만 필터링
	TArray<AActor*> ValidSplines;
	for (AActor* Actor : FoundActors)
	{
		if (IsValidSpline(Actor))
		{
			ValidSplines.Add(Actor);
		}
	}
	
	// 찾은 스플라인들 등록
	if (ValidSplines.Num() > 0)
	{
		RegisterSplines(ValidSplines);
		UE_LOG(LogTemp, Log, TEXT("SplineManager: '%s' 태그로 %d개의 스플라인을 자동 등록했습니다"), *SplineTag.ToString(), ValidSplines.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SplineManager: '%s' 태그가 있는 유효한 스플라인을 찾을 수 없습니다"), *SplineTag.ToString());
	}
	
	
}



//액터가 이미 배열에 있는지 확인
bool ACYSplineManager::IsAlreadyRegistered(AActor* SplineActor) const
{
	return AvailableSplines.Contains(SplineActor) || OccupiedSplines.Contains(SplineActor);
}

//스플라인 액터 유효성 확인
bool ACYSplineManager::IsValidSpline(AActor* SplineActor) const
{
	if (!SplineActor || !IsValid(SplineActor))
	{
		return false;
	}
	
	// 액터에 SplineComponent가 있는지 확인
	USplineComponent* SplineComp = SplineActor->FindComponentByClass<USplineComponent>();
	return SplineComp != nullptr;
}


void ACYSplineManager::CleanupInvalidSplines()
{
	// 사용 가능한 스플라인 목록 정리
	for (int32 i = AvailableSplines.Num() - 1; i >= 0; i--)
	{
		if (!IsValidSpline(AvailableSplines[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineManager: 유효하지 않은 스플라인을 제거합니다 (인덱스: %d)"), i);
			AvailableSplines.RemoveAt(i);
		}
	}
	
	// 사용중인 스플라인 목록 정리
	for (int32 i = OccupiedSplines.Num() - 1; i >= 0; i--)
	{
		if (!IsValidSpline(OccupiedSplines[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("SplineManager: 유효하지 않은 사용중 스플라인을 제거합니다 (인덱스: %d)"), i);
			OccupiedSplines.RemoveAt(i);
		}
	}
}