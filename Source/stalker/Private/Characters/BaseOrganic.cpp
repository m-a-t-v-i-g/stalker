// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BaseOrganic.h"
#include "GenCapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"

FName ABaseOrganic::CapsuleName {"Collision Cylinder"};
FName ABaseOrganic::OrganicMovementName {"CharMoveComp"};
FName ABaseOrganic::MeshName {"CharacterMesh0"};

ABaseOrganic::ABaseOrganic()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CapsuleComponent = CreateDefaultSubobject<UGenCapsuleComponent>(CapsuleName);
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = FName(TEXT("Characters"));
		ArrowComponent->SpriteInfo.DisplayName = FText(NSLOCTEXT("SpriteCategory", "Characters", "Characters"));
		ArrowComponent->SetupAttachment(CapsuleComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif

	OrganicMovement = CreateDefaultSubobject<UOrganicMovementComponent>(OrganicMovementName);
	if (OrganicMovement)
	{
		OrganicMovement->UpdatedComponent = CapsuleComponent;
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
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SetReplicatingMovement(false);
}

void ABaseOrganic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABaseOrganic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Mesh)
	{
		if (Mesh->PrimaryComponentTick.bCanEverTick && OrganicMovement)
		{
			Mesh->PrimaryComponentTick.AddPrerequisite(OrganicMovement, OrganicMovement->PrimaryComponentTick);
		}
	}
	if (OrganicMovement && CapsuleComponent)
	{
		OrganicMovement->UpdateNavAgent(*CapsuleComponent);
	}
}

void ABaseOrganic::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseOrganic::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	OrganicTick(DeltaSeconds);
}

void ABaseOrganic::OrganicTick(float DeltaTime)
{
	Acceleration = OrganicMovement->GetTransientAcceleration();
	ViewRotation = OrganicMovement->GetViewRotation();
	
	const FVector Velocity = OrganicMovement->GetVelocity();
	
	//const FVector NewAcceleration = (Velocity - PreviousVelocity) / DeltaTime; TODO: Old acceleration
	//Acceleration = NewAcceleration.IsNearlyZero() ? Acceleration / 2 : NewAcceleration;
	
	Speed = Velocity.Size2D();
	bIsMoving = Speed > 1.0f;
	if (bIsMoving)
	{
		LastVelocityRotation = Velocity.ToOrientationRotator();
	}

	float InputAcceleration = GetOrganicMovement()->GetInputAcceleration();
	MovementInputAmount = Acceleration.Size() / InputAcceleration;
	
	bHasMovementInput = MovementInputAmount > 0.0f;
	if (bHasMovementInput)
	{
		LastInputRotation = Acceleration.ToOrientationRotator();
	}
	ViewYawRate = FMath::Abs((ViewRotation.Yaw - PreviousViewYaw) / DeltaTime);
	
	if (MovementState == EOrganicMovementState::Ground)
	{
		
	}
	else if (MovementState == EOrganicMovementState::Airborne)
	{
		
	}
	else if (MovementState == EOrganicMovementState::Ragdoll)
	{
		
	}
	
	PreviousVelocity = GetVelocity();
	PreviousViewYaw = ViewRotation.Yaw;
}

void ABaseOrganic::SetOrganicMovementState(const EOrganicMovementState NewState, bool bForce)
{
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void ABaseOrganic::SetOrganicStance(const EOrganicStance NewStance, bool bForce)
{
	
}

void ABaseOrganic::SetOrganicGait(const EOrganicGait NewGait, bool bForce)
{
	
}

void ABaseOrganic::OnMovementModeChanged()
{
	if (OrganicMovement->IsMovingOnGround())
	{
		SetOrganicMovementState(EOrganicMovementState::Ground);
	}
	else if (OrganicMovement->IsAirborne())
	{
		SetOrganicMovementState(EOrganicMovementState::Airborne);
	}
}

void ABaseOrganic::OnMovementStateChanged(const EOrganicMovementState PreviousState)
{
	if (MovementState == EOrganicMovementState::Airborne)
	{
		/*
		if (MovementAction == EWMovementAction::None)
		{
			AirborneRotation = OrganicMovement->GetRootCollisionRotation();
			if (Stance == EWStanceType::Crouching)
			{
				//OrganicMovement->OnUncrouch(true); TODO: For character
			}
		}
		else if (MovementAction == EWMovementAction::Rolling)
		{
			//RagdollStart(); TODO: For character
		}
		*/
	}
}

void ABaseOrganic::UpdateGroundRotation(float DeltaTime)
{
	
}

void ABaseOrganic::UpdateAirborneRotation(float DeltaTime)
{
	
}
