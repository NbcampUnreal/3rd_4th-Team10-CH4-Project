#include "CYPlayerStart.h"

#include "Components/CapsuleComponent.h"

ACYPlayerStart::ACYPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	// 에디터에서 팀별로 다른 색상 표시
	if (!IsRunningCommandlet())
	{
		if (GetCapsuleComponent())
		{
			GetCapsuleComponent()->SetCapsuleHalfHeight(92.0f);
			GetCapsuleComponent()->SetCapsuleRadius(40.0f);
			
			GetCapsuleComponent()->SetVisibility(true);
			GetCapsuleComponent()->SetHiddenInGame(true);

			UpdateEditorVisuals();
		}
	}
#endif
}

bool ACYPlayerStart::CanSpawnForTeam(ECYTeamRole TeamRole) const
{
	// Occupied 상태면 사용 불가
	if (bIsOccupied || AllowedTeam == ECYTeamRole::None)
	{
		return false;
	}
	
	//	팀이 일치하는 경우만 사용 가능
	return AllowedTeam == TeamRole;
}

#if WITH_EDITOR
void ACYPlayerStart::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// AllowedTeam이 변경되면 에디터에서 색상 업데이트
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ACYPlayerStart, AllowedTeam))
	{
		UpdateEditorVisuals();
	}
}

void ACYPlayerStart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// // 에디터에서 액터가 생성되거나 이동될 때 색상 업데이트
	UpdateEditorVisuals();
}

void ACYPlayerStart::UpdateEditorVisuals()
{
	if (GetCapsuleComponent())
	{
		// ShapeColor는 디버그용이므로, 실제 렌더링 색상 변경
		switch (AllowedTeam)
		{
		case ECYTeamRole::Cop:
			GetCapsuleComponent()->ShapeColor = FColor::Blue;
			break;
		case ECYTeamRole::Robber:
			GetCapsuleComponent()->ShapeColor = FColor::Red;
			break;
		default:
			GetCapsuleComponent()->ShapeColor = FColor::Green;
			break;
		}
		
		// 캡슐을 강제로 다시 그리도록 설정
		GetCapsuleComponent()->MarkRenderStateDirty();
		GetCapsuleComponent()->UpdateBounds();
		
		// 에디터에서 보이도록 설정
		GetCapsuleComponent()->SetVisibility(true);
		GetCapsuleComponent()->bDrawOnlyIfSelected = false;
		GetCapsuleComponent()->bUseEditorCompositing = true;
	}
}
#endif
