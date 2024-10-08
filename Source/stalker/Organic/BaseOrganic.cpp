// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/BaseOrganic.h"
#include "GenCapsuleComponent.h"
#include "AbilitySystem/Attributes/OrganicAttributeSet.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"
#include "Components/Weapons/WeaponComponent.h"
#include "Net/UnrealNetwork.h"

FName ABaseOrganic::MeshName {"OrganicMesh0"};
FName ABaseOrganic::OrganicMovementName {"OrganicMoveComp"};
FName ABaseOrganic::CapsuleName {"OrganicCollision"};
FName ABaseOrganic::AbilitySystemComponentName {"AbilityComp"};
FName ABaseOrganic::InventoryComponentName {"InventoryComp"};
FName ABaseOrganic::WeaponComponentName {"WeaponComp"};

const FName NAME_RotationAmount("RotationAmount");
const FName NAME_YawOffset("YawOffset");

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
	
	OrganicMovement = CreateDefaultSubobject<UOrganicMovementComponent>(OrganicMovementName);
	if (OrganicMovement)
	{
		OrganicMovement->UpdatedComponent = CapsuleComponent;
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UStalkerAbilityComponent>(AbilitySystemComponentName);
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	OrganicAttributeSet = CreateDefaultSubobject<UOrganicAttributeSet>("OrganicAttributeSet");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(WeaponComponentName);
	
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
	
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	SetReplicatingMovement(false);
}

void ABaseOrganic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetMesh())
	{
		GetMesh()->AddTickPrerequisiteActor(this);
		
		if (GetOrganicMovement() && GetMesh()->PrimaryComponentTick.bCanEverTick)
		{
			GetMesh()->PrimaryComponentTick.AddPrerequisite(GetOrganicMovement(), GetOrganicMovement()->PrimaryComponentTick);
		}
	}
	
	if (GetOrganicMovement() && GetCapsuleComponent())
	{
		GetOrganicMovement()->UpdateNavAgent(*CapsuleComponent);
	}

	if (DefaultMovementModel)
	{
		GetOrganicMovement()->SetMovementModel(DefaultMovementModel);
	}
}

void ABaseOrganic::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseOrganic::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (GetAbilitySystemComponent<UStalkerAbilityComponent>())
	{
		GetAbilitySystemComponent<UStalkerAbilityComponent>()->InitAbilitySystem(NewController, this);
	}
}

void ABaseOrganic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseOrganic, RotationMode, COND_SimulatedOnly);
}

UAbilitySystemComponent* ABaseOrganic::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void ABaseOrganic::OnMovementModeChanged()
{
	
}

void ABaseOrganic::SetMovementModel()
{
	
}

void ABaseOrganic::SetMovementState(const EOrganicMovementState NewMovementState, bool bForce)
{
	
}

void ABaseOrganic::SetStance(const EOrganicStance NewStance, bool bForce)
{
	
}

void ABaseOrganic::OnStanceChanged(EOrganicStance PreviousStance)
{
	
}

void ABaseOrganic::SetInputGait(EOrganicGait NewInputGait)
{
	
}

void ABaseOrganic::SetGait(const EOrganicGait NewGait, bool bForce)
{
	
}

void ABaseOrganic::OnGaitChanged(EOrganicGait PreviousGait)
{
	
}

void ABaseOrganic::SetOverlayOverrideState(int32 NewState)
{
	OverlayOverrideState = NewState;
}

void ABaseOrganic::ForceUpdateCharacterState()
{
	
}

void ABaseOrganic::OnMovementStateChanged(const EOrganicMovementState PreviousState)
{
	
}

void ABaseOrganic::SetInputRotationMode(EOrganicRotationMode NewInputRotationMode)
{
	
}

void ABaseOrganic::Server_SetInputRotationMode_Implementation(EOrganicRotationMode NewInputRotationMode)
{
	
}

void ABaseOrganic::SetRotationMode(EOrganicRotationMode NewRotationMode, bool bForce)
{
	
}

void ABaseOrganic::Server_SetRotationMode_Implementation(EOrganicRotationMode NewRotationMode, bool bForce)
{

}

void ABaseOrganic::OnRotationModeChanged(EOrganicRotationMode PrevRotationMode)
{
	
}

void ABaseOrganic::SetInputStance(EOrganicStance NewInputStance)
{
	
}

void ABaseOrganic::Server_SetInputStance_Implementation(EOrganicStance NewInputStance)
{
	
}

void ABaseOrganic::OnSprint(bool bEnabled)
{
	// bEnabled ? SetInputGait(EOrganicGait::Fast) : SetInputGait(EOrganicGait::Medium);
}

void ABaseOrganic::OnCrouch()
{
	// SetStance(EOrganicStance::Crouching);
}

void ABaseOrganic::OnUnCrouch()
{
	// SetStance(EOrganicStance::Standing);
}

void ABaseOrganic::OnJump()
{
	
	/*
	if (MovementState == EOrganicMovementState::Ground)
	{
		if (Stance == EOrganicStance::Standing)
		{
			OrganicMovement->OnJump();
			OnJumpedDelegate.Broadcast();
		}
		else if (Stance == EOrganicStance::Crouching)
		{
			OrganicMovement->OnUnCrouch();
		}
	}
	*/
	
}

bool ABaseOrganic::IsSprinting() const
{
	return GetGait() == EOrganicGait::Fast;
}

bool ABaseOrganic::IsAirborne() const
{
	return GetMovementState() == EOrganicMovementState::Airborne;
}
