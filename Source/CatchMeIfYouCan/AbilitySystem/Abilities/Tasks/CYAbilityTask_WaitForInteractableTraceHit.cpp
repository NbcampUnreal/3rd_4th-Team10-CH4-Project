#include "CYAbilityTask_WaitForInteractableTraceHit.h"

#include "AbilitySystemComponent.h"
#include "Interaction/CYInteractable.h"

UCYAbilityTask_WaitForInteractableTraceHit::UCYAbilityTask_WaitForInteractableTraceHit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UCYAbilityTask_WaitForInteractableTraceHit* UCYAbilityTask_WaitForInteractableTraceHit::WaitForInteractableTraceHit(UGameplayAbility* OwningAbility, FCYInteractionQuery InteractionQuery, ECollisionChannel TraceChannel, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionTraceRange, float InteractionTraceRate, bool bShowDebug)
{
	UCYAbilityTask_WaitForInteractableTraceHit* Task = NewAbilityTask<UCYAbilityTask_WaitForInteractableTraceHit>(OwningAbility);
	Task->InteractionTraceRange = InteractionTraceRange;
	Task->InteractionTraceRate = InteractionTraceRate;
	Task->StartLocation = StartLocation;
	Task->InteractionQuery = InteractionQuery;
	Task->TraceChannel = TraceChannel;
	Task->bShowDebug = bShowDebug;
	return Task;
}

void UCYAbilityTask_WaitForInteractableTraceHit::Activate()
{
	Super::Activate();

	// Avatar 액터가 유효할 때까지 대기 상태로 설정
	SetWaitingOnAvatar();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TraceTimerHandle, this, &ThisClass::PerformTrace, InteractionTraceRate, true);
	}
}

void UCYAbilityTask_WaitForInteractableTraceHit::OnDestroy(bool bInOwnerFinished)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UCYAbilityTask_WaitForInteractableTraceHit::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (AvatarActor == nullptr)
	{
		return;
	}
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	AvatarActor->GetAttachedActors(ActorsToIgnore, false, true);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CYAbilityTask_WaitForInteractableTraceHit), false);
	Params.AddIgnoredActors(ActorsToIgnore);

	// 레이캐스트 시작점 (플레이어 위치)
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;

	// 플레이어 컨트롤러의 카메라 시점을 기반으로 레이캐스트 종료점 계산
	AimWithPlayerController(AvatarActor, Params, TraceStart, InteractionTraceRange, TraceEnd);

	FHitResult HitResult;
	LineTrace(TraceStart, TraceEnd, Params, HitResult);

	TArray<TScriptInterface<ICYInteractable>> Interactables;

	// 히트된 액터가 상호작용 가능한지 확인
	TScriptInterface<ICYInteractable> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		Interactables.AddUnique(InteractableActor);
	}

	// 히트된 컴포넌트가 상호작용 가능한지 확인
	TScriptInterface<ICYInteractable> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		Interactables.AddUnique(InteractableComponent);
	}

	// 상호작용 정보 업데이트
	UpdateInteractionInfos(InteractionQuery, Interactables);

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		FColor DebugColor = HitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, DebugColor, false, InteractionTraceRate);
			DrawDebugSphere(GetWorld(), HitResult.Location, 5.f, 16, DebugColor, false, InteractionTraceRate);
		}
		else
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, InteractionTraceRate);
		}
	}
#endif
}

void UCYAbilityTask_WaitForInteractableTraceHit::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch) const
{
	if (Ability == nullptr)
	{
		return;
	}
	
	APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (PlayerController == nullptr)
	{
		return;
	}
	
	FVector CameraStart;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraStart, CameraRotation);

	const FVector CameraDirection = CameraRotation.Vector();
	FVector CameraEnd = CameraStart + (CameraDirection * MaxRange);

	// 카메라 방향의 Ray를 플레이어 위치 기준의 인터렉션 가능 범위(Sphere) 이내로 제한한다
	ClipCameraRayToAbilityRange(CameraStart, CameraDirection, TraceStart, MaxRange, CameraEnd);

	// 카메라에서 클리핑된 종료점까지 레이캐스트 수행
	FHitResult HitResult;
	LineTrace(CameraStart, CameraEnd, Params, HitResult);

	// 히트 결과에 따른 최종 타겟 지점 결정
	// 1. Hit된 물체가 인터렉션 가능 범위(Sphere) 이내라면, Hit 위치를 AdjustedEnd로 정한다.
	// 2. Hit된 물체가 없거나 Hit된 물체가 인터렉션 가능 범위(Sphere)를 벗어 났다면, Hit 위치를 무시하고 Clip된 CameraEnd를 AdjustedEnd로 정한다.
	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));
	const FVector AdjustedEnd = bUseTraceResult ? HitResult.Location : CameraEnd;

	// 플레이어에서 조정된 종료점으로의 방향 벡터 계산
	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		// 방향 벡터가 0이면 카메라 방향을 사용
		AdjustedAimDir = CameraDirection;
	}

	// 플레이어에서 AdjustedAimDir 방향으로 최대 인터렉션 가능 범위(Sphere의 표면)까지 확장한 위치를 TraceEnd로 사용한다.
	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UCYAbilityTask_WaitForInteractableTraceHit::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& OutClippedPosition) const
{
	// 카메라에서 어빌리티 중심점(플레이어)으로의 벡터
	FVector CameraToCenter = AbilityCenter - CameraLocation;

	// 카메라 방향으로의 투영 거리 계산
	float DistanceCameraToDot = FVector::DotProduct(CameraToCenter, CameraDirection);
	
	// 카메라가 어빌리티 중심점을 향하고 있는 경우에만 처리
	if (DistanceCameraToDot >= 0)
	{
		// 카메라에서 어빌리티 중심점까지의 수직 거리 제곱 계산
		float DistanceSquared = CameraToCenter.SizeSquared() - (DistanceCameraToDot * DistanceCameraToDot);
		float RadiusSquared = (AbilityRange * AbilityRange);

		// 카메라 레이가 어빌리티 범위(구체)와 교차하는지 확인
		if (DistanceSquared <= RadiusSquared)
		{
			// 교차점까지의 거리 계산 (피타고라스 정리 응용)
			float DistanceDotToSphere = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceCameraToSphere = DistanceCameraToDot + DistanceDotToSphere;

			// 클리핑된 위치 계산 (카메라에서 구체 표면까지)
			OutClippedPosition = CameraLocation + (DistanceCameraToSphere * CameraDirection);
			return true;
		}
	}
	return false;
}

void UCYAbilityTask_WaitForInteractableTraceHit::LineTrace(const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHitResult) const
{
	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, TraceChannel, Params);
	
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
	else
	{
		OutHitResult = FHitResult();
		OutHitResult.TraceStart = Start;
		OutHitResult.TraceEnd = End;
	}
}

void UCYAbilityTask_WaitForInteractableTraceHit::UpdateInteractionInfos(const FCYInteractionQuery& InteractQuery, const TArray<TScriptInterface<ICYInteractable>>& Interactables)
{
	TArray<FCYInteractionInfo> NewInteractionInfos;

	// 각 상호작용 가능한 객체에서 상호작용 정보 수집
	for (const TScriptInterface<ICYInteractable>& Interactable : Interactables)
	{
		TArray<FCYInteractionInfo> TempInteractionInfos;
		FCYInteractionInfoBuilder InteractionInfoBuilder(Interactable, TempInteractionInfos);

		// 상호작용 객체에서 정보 수집 (Durtaion 적용 포함)
		Interactable->GatherPostInteractionInfos(InteractQuery, InteractionInfoBuilder);

		for (FCYInteractionInfo& InteractionInfo : TempInteractionInfos)
		{
			if (InteractionInfo.AbilityToGrant)
			{
				// 해당 어빌리티가 플레이어에게 있는지 확인
				FGameplayAbilitySpec* InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InteractionInfo.AbilityToGrant);
				if (InteractionAbilitySpec)
				{
					// 상호작용 가능 여부와 어빌리티 활성화 가능 여부 확인
					if (Interactable->CanInteraction(InteractionQuery) && InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
					{
						NewInteractionInfos.Add(InteractionInfo);
					}
				}
			}
		}
	}

	// 이전 정보와 비교하여 변화 감지
	bool bInfosChanged = false;
	if (NewInteractionInfos.Num() == CurrentInteractionInfos.Num())
	{
		// 정보의 개수가 같으면 내용 비교
		NewInteractionInfos.Sort();

		for (int InfoIndex = 0; InfoIndex < NewInteractionInfos.Num(); InfoIndex++)
		{
			const FCYInteractionInfo& NewInfo = NewInteractionInfos[InfoIndex];
			const FCYInteractionInfo& CurrentInfo = CurrentInteractionInfos[InfoIndex];

			if (NewInfo != CurrentInfo)
			{
				bInfosChanged = true;
				break;
			}
		}
	}
	else
	{
		bInfosChanged = true;
	}

	if (bInfosChanged)
	{
		// 이전 객체들의 하이라이트 해제
		HighlightInteractables(CurrentInteractionInfos, false);
		// 새로운 정보로 업데이트
		CurrentInteractionInfos = NewInteractionInfos;
		// 새로운 객체들에 하이라이트 적용
		HighlightInteractables(CurrentInteractionInfos, true);
		
		// 변화 알림 델리게이트 호출
		InteractableChanged.Broadcast(CurrentInteractionInfos);
	}
}

void UCYAbilityTask_WaitForInteractableTraceHit::HighlightInteractables(const TArray<FCYInteractionInfo>& InteractionInfos, bool bShouldHighlight)
{
	TArray<UMeshComponent*> MeshComponents;
	for (const FCYInteractionInfo& InteractionInfo : InteractionInfos)
	{
		if (ICYInteractable* Interactable = InteractionInfo.Interactable.GetInterface())
		{
			Interactable->GetMeshComponents(MeshComponents);
		}
	}

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		MeshComponent->SetRenderCustomDepth(bShouldHighlight);
	}
}
