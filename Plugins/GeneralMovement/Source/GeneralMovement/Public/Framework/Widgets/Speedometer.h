// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Speedometer.generated.h"

/// Provides the bindings for a widget Blueprint which displays the current speed of the pawn.
UCLASS()
class GENERALMOVEMENT_API USpeedometer : public UUserWidget
{
  GENERATED_BODY()

public:

  bool Initialize() override;

  void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

  virtual void SetNumberFormattingOptions();

  FNumberFormattingOptions NumberFormat;

  UFUNCTION(BlueprintCallable, Category = "UI")
  virtual UPawnMovementComponent* GetLocalPawnMovementComponent() const;

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  UTextBlock* Text_Speedometer{nullptr};

  UPROPERTY(BlueprintReadOnly, Category = "UI")
  float Value_Speedometer{0.f};

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
  bool bIncludeVelocityZ{false};
};
