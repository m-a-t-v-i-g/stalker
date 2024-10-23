// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CharacterLocomotionAnimComponent.h"
#include "Library/CharacterLibrary.h"
#include "CharacterAnimInstance.generated.h"

class ABaseCharacter;

UCLASS()
class STALKER_API UCharacterAnimInstance : public UCharacterLocomotionAnimComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Input Information")
	TWeakObjectPtr<ABaseCharacter> Character;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FCharacterMovementAction MovementAction = ECharacterMovementAction::None;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FCharacterOverlayState OverlayState = ECharacterOverlayState::Default;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FAnimOrganicLayerBlending LayerBlendingValues; // Rename to "LayerBlending"

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FAnimOrganicFootIK FootIKValues; // Rename to "FootIK"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L = nullptr;

	virtual void NativeInitializeAnimation() override;

	virtual void UpdateMovementInfo(float DeltaSeconds) override;
	virtual void UpdateViewInfo(float DeltaSeconds) override;
	virtual void UpdateLayerValues(float DeltaSeconds);
	virtual void UpdateFootIK(float DeltaSeconds);

	virtual void UpdateGroundedValues(float DeltaSeconds) override;

	virtual void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	                            float& CurFootLockAlpha, bool& UseFootLockCurve, FVector& CurFootLockLoc,
	                            FRotator& CurFootLockRot);
	virtual void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot);
	virtual void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);
	virtual void ResetIKOffsets(float DeltaSeconds);
	virtual void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	                            FVector& CurLocationTarget, FVector& CurLocationOffset,
	                            FRotator& CurRotationOffset);

	void DynamicTransitionCheck();
	
	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayTransition(const FOrganicDynamicMontage& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayTransitionChecked(const FOrganicDynamicMontage& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FOrganicDynamicMontage Parameters);
	
	void PlayDynamicTransitionDelay();

	UFUNCTION(BlueprintCallable, Category = "Organic|Grounded")
	bool CanDynamicTransition() const;
	
private:
	FTimerHandle PlayDynamicTransitionTimer;

	bool bCanPlayDynamicTransition = true;
};
