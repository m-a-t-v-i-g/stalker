// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/BaseOrganic.h"
#include "AbilitySet.h"
#include "EnhancedInputSubsystems.h"
#include "GenCapsuleComponent.h"
#include "StalkerGameplayTags.h"
#include "StalkerInputComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/ResistanceAttributeSet.h"
#include "Components/ArrowComponent.h"
#include "Components/HitScanComponent.h"
#include "Components/OrganicAbilityComponent.h"

FName ABaseOrganic::MeshName					{"Mesh0"};
FName ABaseOrganic::CapsuleName					{"Capsule Collision"};
FName ABaseOrganic::AbilitySystemComponentName	{"Ability Component"};
FName ABaseOrganic::HitScanComponentName		{"Hit Scan Component"};

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
		Mesh->SetGenerateOverlapEvents(true);
		Mesh->SetCanEverAffectNavigation(false);
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UOrganicAbilityComponent>(AbilitySystemComponentName);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>("Health Attribute Set");
		ResistanceAttributeSet = CreateDefaultSubobject<UResistanceAttributeSet>("Resistance Attribute Set");
	}

	HitScanComponent = CreateDefaultSubobject<UHitScanComponent>(HitScanComponentName);
	
	bReplicates = true;
	SetReplicatingMovement(false);
	
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
}

void ABaseOrganic::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitEssentialComponents();
	
	if (IsLocallyControlled())
	{
		InitLocalData();
	}
}

void ABaseOrganic::OnRep_Controller()
{
	Super::OnRep_Controller();

	InitEssentialComponents();
	
	if (IsLocallyControlled())
	{
		InitLocalData();
	}
}

UAbilitySystemComponent* ABaseOrganic::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void ABaseOrganic::PostInitializeComponents()
{
	if (HasAuthority())
	{
		InitEssentialData();
	}
	
	Super::PostInitializeComponents();
}

void ABaseOrganic::BindDirectionalInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Move, ETriggerEvent::Triggered,
										   this, &ThisClass::Moving);
	}
}

void ABaseOrganic::BindViewInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_View, ETriggerEvent::Triggered,
										   this, &ThisClass::Viewing);
	}
}

void ABaseOrganic::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		TArray<uint32> BindHandles;
		StalkerInputComp->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,
											 &ThisClass::Input_AbilityInputTagReleased, BindHandles);
	}
}

void ABaseOrganic::InitEssentialComponents()
{
	if (GetController())
	{
		if (auto ASC = GetAbilitySystemComponent<UOrganicAbilityComponent>())
		{
			ASC->SetupAbilitySystem(GetController(), this);
		}
	}
}

void ABaseOrganic::InitEssentialData()
{
	auto AbilityComp = GetAbilitySystemComponent<UOrganicAbilityComponent>();
	if (!AbilityComp)
	{
		return;
	}
	
	if (AbilitySet)
	{
		TArray<FGameplayAbilitySpecHandle> OutHandles;
		AbilitySet->GiveToAbilitySystem(AbilityComp, OutHandles);
	}
}

void ABaseOrganic::InitLocalData()
{
}

void ABaseOrganic::Moving(const FInputActionValue& Value)
{
	Super::MoveForward(Value.Get<FVector2D>().X);
	Super::MoveRight(Value.Get<FVector2D>().Y);
}

void ABaseOrganic::Viewing(const FInputActionValue& Value)
{
	Super::TurnView(Value.Get<FVector2D>().X);
	Super::PitchView(Value.Get<FVector2D>().Y);
}

void ABaseOrganic::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UOrganicAbilityComponent* AbilityComponent = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		AbilityComponent->AbilityInputTagPressed(InputTag);
	}
}

void ABaseOrganic::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UOrganicAbilityComponent* AbilityComponent = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		AbilityComponent->AbilityInputTagReleased(InputTag);
	}
}
