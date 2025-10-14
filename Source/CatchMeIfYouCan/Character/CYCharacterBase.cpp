#include "CYCharacterBase.h"

#include "CYLogChannels.h"
#include "CYPawnData.h"
#include "AbilitySystem/CYAbilitySystemComponent.h"
#include "Player/CYPlayerState.h"
#include "AbilitySystem/CYCombatGameplayTags.h"
#include "AbilitySystem/Attributes/CYCombatAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/Items/CYInventoryComponent.h"
#include "Components/Items/CYItemInteractionComponent.h"
#include "Components/Items/CYWeaponComponent.h"
#include "Physics/CYCollisionChannels.h"
#include "MotionWarpingComponent.h"
#include "Net/UnrealNetwork.h"


ACYCharacterBase::ACYCharacterBase(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// Item 컴포넌트들 생성
	InventoryComponent = CreateDefaultSubobject<UCYInventoryComponent>(TEXT("InventoryComponent"));
	ItemInteractionComponent = CreateDefaultSubobject<UCYItemInteractionComponent>(TEXT("ItemInteractionComponent"));
	WeaponComponent = CreateDefaultSubobject<UCYWeaponComponent>(TEXT("WeaponComponent"));
	
	InteractCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractCapsule"));
	InteractCapsule->SetupAttachment(GetRootComponent());
	InteractCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractCapsule->SetCollisionResponseToChannel(CY_TraceChannel_Interaction, ECR_Block);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh"));
	HelmetMesh->SetupAttachment(GetMesh());
	HelmetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HelmetMesh->SetLeaderPoseComponent(GetMesh());  
	HelmetMesh->bUseBoundsFromLeaderPoseComponent = true;
	HelmetMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	EyewearMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EyewearMesh"));
	EyewearMesh->SetupAttachment(GetMesh());
	EyewearMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EyewearMesh->SetLeaderPoseComponent(GetMesh());
	EyewearMesh->bUseBoundsFromLeaderPoseComponent = true;
	EyewearMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);


	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(GetMesh());
	ChestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ChestMesh->SetLeaderPoseComponent(GetMesh());
	ChestMesh->bUseBoundsFromLeaderPoseComponent = true;
	ChestMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LegsMesh->SetLeaderPoseComponent(GetMesh());
	LegsMesh->bUseBoundsFromLeaderPoseComponent = true;
	LegsMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	FootwearMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FootwearMesh"));
	FootwearMesh->SetupAttachment(GetMesh());
	FootwearMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FootwearMesh->SetLeaderPoseComponent(GetMesh());
	FootwearMesh->bUseBoundsFromLeaderPoseComponent = true;
	FootwearMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

UAbilitySystemComponent* ACYCharacterBase::GetAbilitySystemComponent() const
{
	return CYAbilitySystemComponent.Get();
}

void ACYCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACYCharacterBase, bIsClimbing, COND_SimulatedOnly);
}

void ACYCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACYCharacterBase::SyncInteractCapsuleSizeToRootCapsule() const
{
	if (!InteractCapsule || !GetCapsuleComponent())
	{
		return;
	}
	
	float Radius = 0.f, HalfHeight = 0.f;
	GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
	InteractCapsule->SetCapsuleSize(
		Radius + InteractCapsuleRadiusOffset,
		HalfHeight + InteractCapsuleHalfHeightOffset,
		true
	);
}

void ACYCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAbilitySets();
	Super::EndPlay(EndPlayReason);
}

bool ACYCharacterBase::IsPawnDataReady() const
{
	ACYPlayerState* PS = GetPlayerState<ACYPlayerState>();
	return PS && PS->GetPawnData();
}

void ACYCharacterBase::TryInitializeAbilitySetsWithPawnData()
{
	if (bAbilitySetsInitialized)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	// PawnData 체크
	if (!IsPawnDataReady())
	{
		return;
	}

	// PawnData가 있으면 AbilitySet 초기화
	InitializeAbilitySets();
	
	bAbilitySetsInitialized = true;

	UE_LOG(LogCY, Warning, TEXT("AbilitySets initialized with PawnData"));
}

void ACYCharacterBase::InitializeAbilitySets()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!CYAbilitySystemComponent.IsValid())
	{
		UE_LOG(LogCY, Warning, TEXT("%s Side: InitializeAbilitySets: AbilitySystemComponent is invalid on %s"), *GetClientServerContextString(this), *GetNameSafe(this));
		return;
	}

	ACYPlayerState* CYPS = GetPlayerState<ACYPlayerState>();
	if (!CYPS)
	{
		UE_LOG(LogCY, Warning, TEXT("%s Side: InitializeAbilitySets: PlayerState is invalid on %s"), *GetClientServerContextString(this), *GetNameSafe(this));
		return;
	}

	UCYPawnData* PawnData = CYPS->GetPawnData();
	if (!PawnData)
	{
		UE_LOG(LogCY, Warning, TEXT("%s Side: InitializeAbilitySets: PawnData is invalid on %s"), *GetClientServerContextString(this), *GetNameSafe(this));
		return;
	}
	
	// 기존 Ability 제거
	RemoveAbilitySets();

	UCYAbilitySystemComponent* CYASC = CYAbilitySystemComponent.Get();
	
	// 기본적으로 부여할 AbilitySet 설정
	for (const UCYAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (IsValid(AbilitySet))
		{
			// 해당 시점에서 AbilitySet을 부여하고 부여된 Ability들의 Handle을 GrantedAbilitySetHandles에 저장
			FCYAbilitySet_GrantedHandles& GrantedHandles = GrantedAbilitySetHandles.AddDefaulted_GetRef();
			AbilitySet->GiveToAbilitySystem(CYASC, &GrantedHandles);
		}
	}
}

void ACYCharacterBase::RemoveAbilitySets()
{
	// 서버에서만 제거
	if (!HasAuthority())
	{
		return;
	}
	
	if (!CYAbilitySystemComponent.IsValid())
	{
		return;
	}

	UCYAbilitySystemComponent* CYASC = CYAbilitySystemComponent.Get();

	// 모든 부여된 능력 제거
	for (FCYAbilitySet_GrantedHandles& GrantedHandles : GrantedAbilitySetHandles)
	{
		GrantedHandles.TakeFromAbilitySystem(CYASC);
	}
	GrantedAbilitySetHandles.Empty();
}

void ACYCharacterBase::AddGameplayTag(const FGameplayTag& Tag)
{
	if (CYAbilitySystemComponent.IsValid())
	{
		CYAbilitySystemComponent->AddLooseGameplayTag(Tag);
	}
}
void ACYCharacterBase::RemoveGameplayTag(const FGameplayTag& Tag)
{
	if (CYAbilitySystemComponent.IsValid())
	{
		CYAbilitySystemComponent->RemoveLooseGameplayTag(Tag);
	}
}

bool ACYCharacterBase::HasGameplayTag(const FGameplayTag& Tag) const
{
	if (const UCYAbilitySystemComponent* CYASC = CYAbilitySystemComponent.Get())
	{
		return CYASC->HasMatchingGameplayTag(Tag);
	}
	return false;
}

void ACYCharacterBase::InteractPressed()
{
	if (ItemInteractionComponent)
	{
		ItemInteractionComponent->InteractWithNearbyItem();
	}
}

void ACYCharacterBase::AttackPressed()
{
	if (WeaponComponent && WeaponComponent->CurrentWeapon)
	{
		if (UCYAbilitySystemComponent* ASC = Cast<UCYAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			ASC->TryActivateAbilityByTag(CYGameplayTags::Ability_Combat_WeaponAttack);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No WeaponComponent found"));
	}
}

void ACYCharacterBase::UseInventorySlot(int32 SlotIndex)
{
	if (InventoryComponent)
	{
		InventoryComponent->HoldItem(SlotIndex);
	}
}

void ACYCharacterBase::SetIsClimbing(bool bNewIsClimbing)
{
	bIsClimbing = bNewIsClimbing;
}

void ACYCharacterBase::OnRep_IsClimbing()
{
	UE_LOG(LogCY, Verbose, TEXT("%s: IsClimbing changed to %d"), 
		   *GetName(), bIsClimbing);
}