// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicAnimInstance.h"
#include "Library/CharacterLibrary.h"
#include "CharacterAnimInstance.generated.h"

UCLASS()
class STALKER_API UCharacterAnimInstance : public UOrganicAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;

	virtual void UpdateMovementInfo(float DeltaSeconds) override;
	virtual void UpdateViewValues(float DeltaSeconds) override;
	virtual void UpdateLayerValues(float DeltaSeconds);
	virtual void UpdateFootIK(float DeltaSeconds);

	virtual void UpdateGroundedValues(float DeltaSeconds) override;

	virtual void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	                            float& CurFootLockAlpha, bool& UseFootLockCurve, FVector& CurFootLockLoc,
	                            FRotator& CurFootLockRot) override;
	virtual void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot) override;
	virtual void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget) override;
	virtual void ResetIKOffsets(float DeltaSeconds) override;
	virtual void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	                            FVector& CurLocationTarget, FVector& CurLocationOffset,
	                            FRotator& CurRotationOffset) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Enable_FootIK_L = "Enable_FootIK_L";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Enable_FootIK_R = "Enable_FootIK_R";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Enable_HandIK_L = "Enable_HandIK_L";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Enable_HandIK_R = "Enable_HandIK_R";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Arm_L = "Layering_Arm_L";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Arm_L_Add = "Layering_Arm_L_Add";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Arm_L_LS = "Layering_Arm_L_LS";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Arm_R = "Layering_Arm_R";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Arm_R_Add = "Layering_Arm_R_Add";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Arm_R_LS = "Layering_Arm_R_LS";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Hand_L = "Layering_Hand_L";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Hand_R = "Layering_Hand_R";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Head_Add = "Layering_Head_Add";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Layering_Spine_Add = "Layering_Spine_Add";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_FootLock_L = "FootLock_L";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_FootLock_R = "FootLock_R";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_VB___foot_target_l = "VB foot_target_l";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_VB___foot_target_r = "VB foot_target_r";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME__ALSCharacterAnimInstance__RotationAmount = "RotationAmount";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootL_BoneName = "ik_foot_l";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootR_BoneName = "ik_foot_r";

public:
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	TObjectPtr<class ABaseCharacter> Character;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (ShowOnlyInnerProperties))
	FAnimOrganicLayerBlending LayerBlendingValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (ShowOnlyInnerProperties))
	FAnimOrganicFootIK FootIKValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FCharacterMovementAction MovementAction = ECharacterMovementAction::None;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FCharacterOverlayState OverlayState = ECharacterOverlayState::Default;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L = nullptr;

private:
	FTimerHandle PlayDynamicTransitionTimer;

	bool bCanPlayDynamicTransition = true;

protected:
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
};
