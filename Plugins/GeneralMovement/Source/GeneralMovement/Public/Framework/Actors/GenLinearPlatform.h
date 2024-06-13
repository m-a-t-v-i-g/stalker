// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GenRollbackActor.h"
#include "GenLinearPlatform.generated.h"

/// Rollback actor that will continuously move back and forth in a straight line between two points at a fixed speed.
UCLASS(ClassGroup = "Movement", BlueprintType, Blueprintable)
class GENERALMOVEMENT_API AGenLinearPlatform : public AGenRollbackActor
{
  GENERATED_BODY()

public:

  AGenLinearPlatform();
  void BeginPlay() override;
  void UpdateState_Implementation(float Time, float DeltaTime, const struct FMove& Move, EActorRollbackContext Context) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.3", UIMin = "0.3"))
  /// For how long the platform should move with the configured velocity before reverting its direction (in seconds).
  float MoveDuration{3.f};
};
