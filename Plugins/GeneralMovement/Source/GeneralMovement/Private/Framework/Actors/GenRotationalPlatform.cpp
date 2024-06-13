// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenRotationalPlatform.h"
#include "GenMovementReplicationComponent.h"
#define GMC_ROLLBACK_ACTOR_LOG
#include "GMC_LOG.h"

AGenRotationalPlatform::AGenRotationalPlatform()
{
  AngularVelocity = {0.f, 45.f, 0.f};
}

void AGenRotationalPlatform::BeginPlay()
{
  Super::BeginPlay();

  // No linear velocity for this actor.
  Velocity = FVector{0};
  BaseVelocity = FVector(0);
}

void AGenRotationalPlatform::UpdateState_Implementation(float Time, float DeltaTime, const FMove& Move, EActorRollbackContext Context)
{
  const auto RootComp = GetRootComponent();
  if (!RootComp || RootComp->IsSimulatingPhysics())
  {
    return;
  }

  // @attention Do not check for zero with @see FRotator::IsZero here (because the angular velocity is not an orientation).
  if (AngularVelocity.Roll == 0.f && AngularVelocity.Pitch == 0.f && AngularVelocity.Yaw == 0.f)
  {
    return;
  }

  const float DegreesRemainder = fmod(Time, 360.f);
  const FRotator NewRotation = (DegreesRemainder * AngularVelocity).GetNormalized();
  SetActorRotation(BaseTransform.GetRotation().Rotator() + NewRotation);
}
