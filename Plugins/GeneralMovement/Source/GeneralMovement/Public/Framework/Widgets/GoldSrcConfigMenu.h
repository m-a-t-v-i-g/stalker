// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoldSrcConfigMenu.generated.h"

/// Provides the bindings for a widget Blueprint which allows for the runtime configuration of @see UGenGoldSrcMovementComponent.
UCLASS()
class GENERALMOVEMENT_API UGoldSrcConfigMenu : public UUserWidget
{
  GENERATED_BODY()

public:

  bool Initialize() override;

  UFUNCTION(BlueprintCallable, Category = "UI")
  virtual void UpdateInterfaceValues() const;

  UFUNCTION(BlueprintCallable, Category = "UI")
  virtual void UpdateArchetypeValues() const;

  UFUNCTION(BlueprintCallable, Category = "UI")
  virtual UGenGoldSrcMovementComponent* GetLocalGoldSrcMovementComponent() const;

  /// Update Archetype

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_UpdateArchetype{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UButton* Button_UpdateArchetype{nullptr};
  UFUNCTION()
  virtual void OnClicked_UpdateArchetype();

  /// Category: On Ground

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_Acceleration{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  class USpinBox* Value_Acceleration{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_Acceleration(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_MaxSpeed{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_MaxSpeed{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_MaxSpeed(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_StopSpeed{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_StopSpeed{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_StopSpeed(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_SprintSpeed{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_SprintSpeed{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_SprintSpeed(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_CrouchWalkSpeed{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_CrouchWalkSpeed{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_CrouchWalkSpeed(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_CrouchInterpSpeed{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_CrouchInterpSpeed{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_CrouchInterpSpeed(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_CrouchedHalfHeight{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_CrouchedHalfHeight{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_CrouchedHalfHeight(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_FloorFriction{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_FloorFriction{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_FloorFriction(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_EdgeFrictionScale{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_EdgeFrictionScale{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_EdgeFrictionScale(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_EdgeFrictionMinRequiredHeight{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_EdgeFrictionMinRequiredHeight{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_EdgeFrictionMinRequiredHeight(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_bUseAltEdgeFriction{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  class UCheckBox* Value_bUseAltEdgeFriction{nullptr};
  UFUNCTION()
  virtual void OnCheckStateChanged_bUseAltEdgeFriction(bool bIsChecked);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_JumpMode{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  class UComboBoxString* Value_JumpMode{nullptr};
  UFUNCTION()
  virtual void OnSelectionChanged_JumpMode(FString SelectedItem, ESelectInfo::Type SelectionType);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_JumpForce{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_JumpForce{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_JumpForce(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_MaxGroundedVelocityZ{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_MaxGroundedVelocityZ{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_MaxGroundedVelocityZ(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_bApplyJumpSpeedBoost{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UCheckBox* Value_bApplyJumpSpeedBoost{nullptr};
  UFUNCTION()
  virtual void OnCheckStateChanged_bApplyJumpSpeedBoost(bool bIsChecked);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_SmallJumpSpeedBoostFactor{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_SmallJumpSpeedBoostFactor{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_SmallJumpSpeedBoostFactor(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_LargeJumpSpeedBoostFactor{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_LargeJumpSpeedBoostFactor{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_LargeJumpSpeedBoostFactor(float InValue);

  /// Category: In Air

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_AirAcceleration{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_AirAcceleration{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_AirAcceleration(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_AirControl{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_AirControl{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_AirControl(float InValue);

  /// Category: In Water

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_WaterControl{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_WaterControl{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_WaterControl(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_SinkSpeed{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_SinkSpeed{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_SinkSpeed(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_WaterJumpForce{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_WaterJumpForce{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_WaterJumpForce(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_WaterJumpPush{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_WaterJumpPush{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_WaterJumpPush(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_bUseAltSwimControls{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UCheckBox* Value_bUseAltSwimControls{nullptr};
  UFUNCTION()
  virtual void OnCheckStateChanged_bUseAltSwimControls(bool bIsChecked);

  /// Category: Advanced

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_SpeedHardCap{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_SpeedHardCap{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_SpeedHardCap(float InValue);

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_bConsiderMassOnJump{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UCheckBox* Value_bConsiderMassOnJump{nullptr};
  UFUNCTION()
  virtual void OnCheckStateChanged_bConsiderMassOnJump(bool bIsChecked);

  /// Other

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_GravityScale{nullptr};
  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  USpinBox* Value_GravityScale{nullptr};
  UFUNCTION()
  virtual void OnValueChanged_GravityScale(float InValue);
};
