// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenLinearPlatform.h"
#include "GenMovementReplicationComponent.h"
#define GMC_ROLLBACK_ACTOR_LOG
#include "GMC_LOG.h"

AGenLinearPlatform::AGenLinearPlatform()
{
  Velocity = {200.f, 0.f, 0.f};
}

void AGenLinearPlatform::BeginPlay()
{
  Super::BeginPlay();

  // No angular velocity for this actor.
  AngularVelocity = FRotator{0};
  BaseAngularVelocity = FRotator{0};
}

void AGenLinearPlatform::UpdateState_Implementation(float Time, float DeltaTime, const FMove& Move, EActorRollbackContext Context)
{
  const auto RootComp = GetRootComponent();
  if (!RootComp || RootComp->IsSimulatingPhysics() || DeltaTime < UGenMovementReplicationComponent::MIN_DELTA_TIME)
  {
    return;
  }

  // @attention Do not check the velocity for zero here.
  if (MoveDuration <= 0.f)
  {
    return;
  }

  const auto GetLocationFromTime = [this](float Time, float CycleDuration) {
    const float CycleDurationRemainder = fmod(Time, CycleDuration);
    const bool bIsReturning = CycleDurationRemainder > MoveDuration;
    const float MoveDurationRemainder = bIsReturning ? CycleDuration - CycleDurationRemainder : CycleDurationRemainder;
    return BaseTransform.GetLocation() + BaseVelocity * MoveDurationRemainder;
  };

  // @attention Keep in mind that pawns that have an organic movement component will "acquire" the velocity of the rollback actor if they
  // are based on it (in addition to their own velocity).

  const float CycleDuration = 2.f * MoveDuration;
  const float RemainderCurrentLocation = fmod(Time, CycleDuration);
  const float RemainderPreviousLocation = fmod(Time - DeltaTime, CycleDuration);

  // The velocity around the half-way point needs special consideration.
  if (RemainderCurrentLocation > MoveDuration && RemainderPreviousLocation < MoveDuration)
  {
    // The platform just crossed the half-way point.
    const float DiffCurrentLocation = RemainderCurrentLocation - MoveDuration;
    const float DiffPreviousLocation = MoveDuration - RemainderPreviousLocation;
    if (FMath::IsNearlyEqual(DiffCurrentLocation, DiffPreviousLocation, KINDA_SMALL_NUMBER))
    {
      // The platform effectively did not move at all.
      SetVelocity(FVector::ZeroVector);
      SetActorLocation(GetLocationFromTime(Time, CycleDuration));
      return;
    }
    if (DiffCurrentLocation > DiffPreviousLocation)
    {
      // The platform is further towards the base location now than before it turned around. That means the pawn must be moved against the
      // direction of the base velocity.
      const FVector AbsMoveDistance = (DiffCurrentLocation - DiffPreviousLocation) * BaseVelocity;
      const FVector SignVector = {FMath::Sign(-BaseVelocity.X), FMath::Sign(-BaseVelocity.Y), FMath::Sign(-BaseVelocity.Z)};
      SetVelocity(SignVector * AbsMoveDistance / DeltaTime);
      SetActorLocation(GetLocationFromTime(Time, CycleDuration));
      return;
    }
    checkGMC(DiffCurrentLocation < DiffPreviousLocation)
    {
      // The platform is further away from the base location now than before it turned around. That means the pawn must still be moved in
      // the direction of the base velocity.
      const FVector AbsMoveDistance = (DiffPreviousLocation - DiffCurrentLocation) * BaseVelocity;
      const FVector SignVector = {FMath::Sign(BaseVelocity.X), FMath::Sign(BaseVelocity.Y), FMath::Sign(BaseVelocity.Z)};
      SetVelocity(SignVector * AbsMoveDistance / DeltaTime);
      SetActorLocation(GetLocationFromTime(Time, CycleDuration));
      return;
    }
  }

  // The velocity around the starting point needs special consideration.
  // @attention The threshold is an arbitrary small value that must not be greater than the duration of a full cycle.
  constexpr float Threshold = 0.5f;
  checkGMC(CycleDuration >= Threshold)
  if (RemainderCurrentLocation < Threshold && RemainderPreviousLocation > CycleDuration - Threshold)
  {
    // The platform just crossed the starting point.
    const float DiffCurrentLocation = RemainderCurrentLocation;
    const float DiffPreviousLocation = CycleDuration - RemainderPreviousLocation;
    if (FMath::IsNearlyEqual(DiffCurrentLocation, DiffPreviousLocation, KINDA_SMALL_NUMBER))
    {
      // The platform effectively did not move at all.
      SetVelocity(FVector::ZeroVector);
      SetActorLocation(GetLocationFromTime(Time, CycleDuration));
      return;
    }
    if (DiffCurrentLocation > DiffPreviousLocation)
    {
      // The platform is further away from the base location now than before it turned around. That means the pawn must be moved with the
      // direction of the base velocity.
      const FVector AbsMoveDistance = (DiffCurrentLocation - DiffPreviousLocation) * BaseVelocity;
      const FVector SignVector = {FMath::Sign(BaseVelocity.X), FMath::Sign(BaseVelocity.Y), FMath::Sign(BaseVelocity.Z)};
      SetVelocity(SignVector * AbsMoveDistance / DeltaTime);
      SetActorLocation(GetLocationFromTime(Time, CycleDuration));
      return;
    }
    checkGMC(DiffCurrentLocation < DiffPreviousLocation)
    {
      // The platform is closer to the base location now than before it turned around. That means the pawn must still be moved against the
      // direction of the base velocity.
      const FVector AbsMoveDistance = (DiffPreviousLocation - DiffCurrentLocation) * BaseVelocity;
      const FVector SignVector = {FMath::Sign(-BaseVelocity.X), FMath::Sign(-BaseVelocity.Y), FMath::Sign(-BaseVelocity.Z)};
      SetVelocity(SignVector * AbsMoveDistance / DeltaTime);
      SetActorLocation(GetLocationFromTime(Time, CycleDuration));
      return;
    }
  }

  // No special considerations regarding the velocity must be made.
  SetActorLocation(GetLocationFromTime(Time, CycleDuration));
  bool bIsReturning = RemainderCurrentLocation > MoveDuration;
  SetVelocity(bIsReturning ? -BaseVelocity : BaseVelocity);
}
