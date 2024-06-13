// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GenRollbackActor.h"
#include "GenRotationalPlatform.generated.h"

/// Rollback actor that will continuously rotate at a fixed angular speed.
UCLASS(ClassGroup = "Movement", BlueprintType, Blueprintable)
class GENERALMOVEMENT_API AGenRotationalPlatform : public AGenRollbackActor
{
  GENERATED_BODY()

public:

  AGenRotationalPlatform();
  void BeginPlay() override;
  void UpdateState_Implementation(float Time, float DeltaTime, const FMove& Move, EActorRollbackContext Context) override;
};
