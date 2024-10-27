// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/BaseOrganic.h"
#include "GenCapsuleComponent.h"
#include "AbilitySystem/Attributes/OrganicAttributeSet.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/Movement/StalkerCharacterMovementComponent.h"

FName ABaseOrganic::MeshName {"OrganicMesh0"};
FName ABaseOrganic::CapsuleName {"OrganicCollision"};
FName ABaseOrganic::AbilitySystemComponentName {"AbilityComp"};

ABaseOrganic::ABaseOrganic(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.Get())
{
	struct FConstructorStatics
	{
		FName ID_Characters;
		FText NAME_Characters;
		FConstructorStatics()
			: ID_Characters(TEXT("Characters"))
			, NAME_Characters(NSLOCTEXT("SpriteCategory", "Characters", "Characters"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	CapsuleComponent = CreateDefaultSubobject<UGenCapsuleComponent>(CapsuleName);
	if (CapsuleComponent)
	{
		CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
		CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		CapsuleComponent->CanCharacterStepUpOn = ECB_No;
		CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
		CapsuleComponent->SetCanEverAffectNavigation(false);
		CapsuleComponent->bDynamicObstacle = true;
		SetRootComponent(CapsuleComponent);
	}
	
	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(MeshName);
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(CapsuleComponent);
		Mesh->SetCollisionProfileName("CharacterMesh");
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UOrganicAbilityComponent>(AbilitySystemComponentName);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		/* DEPRECATED */
		OrganicAttributeSet = CreateDefaultSubobject<UOrganicAttributeSet>("OrganicAttributeSet");
	}
	
#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>("Arrow");
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		ArrowComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		ArrowComponent->SetupAttachment(CapsuleComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif
	
	bReplicates = true;
	SetReplicatingMovement(false);
}

void ABaseOrganic::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (auto ASC = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		ASC->InitAbilitySystem(NewController, this);
	}
}

UAbilitySystemComponent* ABaseOrganic::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}
