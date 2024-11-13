// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "Components/ActorComponent.h"
#include "CharacterStateComponent.generated.h"

struct FOnAttributeChangeData;
class UHealthAttributeSet;
class UOrganicAbilityComponent;
class UStalkerCharacterMovementComponent;
class UCharacterWeaponComponent;
class UMovementModelConfig;
class AStalkerCharacter;

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetupStateComponent();

	void SetMovementAction(ECharacterMovementAction NewAction, bool bForce = false);
	FORCEINLINE ECharacterMovementAction GetMovementAction() const { return MovementAction; }
	
	virtual void OnMovementActionChanged(ECharacterMovementAction PreviousAction);

	void SetOverlayState(ECharacterOverlayState NewState, bool bForce = false);
	FORCEINLINE ECharacterOverlayState GetOverlayState() const { return OverlayState; }

	virtual void OnOverlayStateChanged(ECharacterOverlayState PreviousState);
	
	void SetHealthState(ECharacterHealthState NewState, bool bForce = false);
	FORCEINLINE ECharacterHealthState GetHealthState() const { return HealthState; }

	virtual void OnHealthStateChanged(ECharacterHealthState PreviousState);
	
	void SetCombatState(ECharacterCombatState NewState, bool bForce = false);
	FORCEINLINE ECharacterCombatState GetCombatState() const { return CombatState; }

	virtual void OnCombatStateChanged(ECharacterCombatState PreviousState);
	
	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;
	
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Character State")
	ECharacterMovementAction MovementAction = ECharacterMovementAction::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Character State")
	ECharacterOverlayState OverlayState = ECharacterOverlayState::Default;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_HealthState", Category = "Character State")
	ECharacterHealthState HealthState = ECharacterHealthState::Normal;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_CombatState", Category = "Character State")
	ECharacterCombatState CombatState = ECharacterCombatState::Relaxed;

	UPROPERTY(EditDefaultsOnly, Category = "Character State")
	float StateTransitionTime = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character State|Movement Models")
	TObjectPtr<const UMovementModelConfig> NormalMovementModel;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character State|Movement Models")
	TObjectPtr<const UMovementModelConfig> InjuredMovementModel;

	void SetupHealth();
	void OnHealthChange(const FOnAttributeChangeData& HealthChangeData);
	
	void OnFireStart();
	void OnFireStop();
	void OnAimingStart();
	void OnAimingStop();
	void OnOverlayChanged(ECharacterOverlayState NewOverlay);

	void SetRelaxTimer();

	UFUNCTION()
	void OnRep_HealthState(ECharacterHealthState PrevHealthState);

	UFUNCTION()
	void OnRep_CombatState(ECharacterCombatState PrevCombatState);

private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;

	TObjectPtr<UOrganicAbilityComponent> AbilityComponentRef;
	TObjectPtr<UStalkerCharacterMovementComponent> MovementComponentRef;
	TObjectPtr<UCharacterWeaponComponent> WeaponComponentRef;

	TObjectPtr<const UHealthAttributeSet> HealthAttributeSet;
	
	bool bFiring = false;
	bool bAiming = false;
	
	FTimerHandle CombatStateTimer;
};
