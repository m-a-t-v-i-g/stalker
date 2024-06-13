// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenGoldSrcMovementComponent.h"
#define GMC_MOVEMENT_COMPONENT_LOG
#include "GMC_LOG.h"
#include "GenGoldSrcMovementComponent_DBG.h"

namespace GMCCVars
{
#if ALLOW_CONSOLE

  int32 FlyMode = 0;
  FAutoConsoleVariableRef CVarFlyMode(
    TEXT("gmc.FlyMode"),
    FlyMode,
    TEXT("Cheat/Server only: Fly mode removes the effects of gravity on the pawn. 0: Disable, 1: Enable"),
    ECVF_Cheat
  );

  int32 NoClipMode = 0;
  FAutoConsoleVariableRef CVarNoClipMode(
    TEXT("gmc.NoClipMode"),
    NoClipMode,
    TEXT("Cheat/Server only: Noclip mode removes all collision from the pawn and allows for free movement in all directions. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Cheat
  );

#endif
}

UGenGoldSrcMovementComponent::UGenGoldSrcMovementComponent()
{
  // Set the actor rotation directly from the view direction.
  bOrientToInputDirection = false;
  bOrientToControlRotationDirection = true;
  RotationRate = 0.f;

  // Attuned to @see WaterControl.
  FluidMinExitSpeed = 320.f;

  // No buoyancy by default, we have @see SinkSpeed instead.
  Buoyancy = 0.f;

  // Set for consistent default values.
  MaxDesiredSpeed = MaxSpeed;

#if WITH_EDITORONLY_DATA && WITH_EDITOR

  bHideProperties = true;

  // Hide the @see GroundFriction property (we use @see FloorFriction instead).
  HideProperty(StaticClass(), "GroundFriction");

  // Hide the @see FallControl property (not applicable).
  HideProperty(StaticClass(), "FallControl");

#endif
}

void UGenGoldSrcMovementComponent::BeginPlay()
{
  // Enables us to use @see DefaultSpeed (set in parent function).
  MaxDesiredSpeed = MaxSpeed;

  Super::BeginPlay();

  if (!PawnOwner || !GetGenPawnOwner())
  {
    return;
  }

  GetGenPawnOwner()->SetInputMode(EInputMode::AllAbsolute);

  DefaultHalfHeight = GetRootCollisionHalfHeight();

  if (const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent()))
  {
    DefaultCollisionType = RootComponent->GetCollisionEnabled();
  }
}

void UGenGoldSrcMovementComponent::AdjustVelocityFromHitAirborne_Implementation(const FHitResult& Hit, float DeltaSeconds)
{
  // Disable the super implementation, we want to be able to gain upward velocity from impacts.
  AdjustVelocityFromHit(Hit, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::HaltMovement()
{
  Super::HaltMovement();
  bIsSprinting = false;
  bCanJump = false;
  bJustJumped = false;
  bJustLandedPrePhysics = false;
  bJustLandedPostPhysics = false;
  bJustLandedPostPhysicsLock = false;
  bJustLanded = false;
}

FVector UGenGoldSrcMovementComponent::ComputeSlideVector(
  const FVector& Delta,
  float Time,
  const FVector& Normal,
  const FHitResult& Hit
) const
{
  return UMovementComponent::ComputeSlideVector(Delta, Time, Normal, Hit);
}

float UGenGoldSrcMovementComponent::SlideAlongSurface(
  const FVector& Delta,
  float Time,
  const FVector& Normal,
  FHitResult& Hit,
  bool bHandleImpact
)
{
  return UMovementComponent::SlideAlongSurface(Delta, Time, Normal, Hit, bHandleImpact);
}

void UGenGoldSrcMovementComponent::TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const
{
  UMovementComponent::TwoWallAdjust(Delta, Hit, OldHitNormal);
}

float UGenGoldSrcMovementComponent::CalculatePathFollowingBrakingDistance_Implementation(float InMaxSpeed, float DeltaSeconds) const
{
  return InMaxSpeed;
}

void UGenGoldSrcMovementComponent::ClampToValidValues()
{
  Super::ClampToValidValues();

  MinAnalogWalkSpeed = FMath::Min(MinAnalogWalkSpeed, StopSpeed);
  CrouchedHalfHeight = FMath::Clamp(CrouchedHalfHeight, KINDA_SMALL_NUMBER, DefaultHalfHeight);
  EdgeFrictionMinRequiredHeight =
    FMath::Clamp(EdgeFrictionMinRequiredHeight, MAX_DISTANCE_TO_FLOOR + KINDA_SMALL_NUMBER, FloorTraceLength - KINDA_SMALL_NUMBER);
}

void UGenGoldSrcMovementComponent::BindReplicationData_Implementation()
{
  Super::BindReplicationData_Implementation();

  BindInputFlag(ID_Sprint, bWantsToSprint, false, false);
  BindInputFlag(ID_Crouch, bWantsToCrouch, false, false);
  BindInputFlag(ID_Jump, bWantsToJump, false, false);
  BindBool(bCanJump, true, false, false);
  BindBool(bJustLandedPostPhysics, true, false, false);
  BindBoolWithAccessor(bJustJumped, JustJumpedAccessor, false, true, true);
  BindBoolWithAccessor(bJustLanded, JustLandedAccessor, false, true, true);
  BindBoolWithAccessor(bIsSprinting, IsSprintingAccessor, true, true, true);
  BindFloatWithAccessor(MaxSpeed, MaxSpeedAccessor, true, false, false);
}

void UGenGoldSrcMovementComponent::PreMovementUpdate_Implementation(float DeltaSeconds)
{
  Super::PreMovementUpdate_Implementation(DeltaSeconds);

  if (GetIterationNumber() == 1)
  {
    bJustJumped = false;
    bJustLanded = false;
  }

  // Reset this before just before updating the movement mode (which might call @see OnLanded).
  bJustLandedPrePhysics = false;

  // This may be set by @see ProcessLanded.
  bJustLandedPostPhysicsLock = false;

#if ALLOW_CONSOLE

  const bool bJustDisabledNoClipMode = IsUsingNoClipMode() && GMCCVars::NoClipMode == 0;
  const bool bJustDisabledFlyMode = IsUsingFlyMode() && GMCCVars::FlyMode == 0;
  if (bJustDisabledNoClipMode || bJustDisabledFlyMode)
  {
    // Set to airborne as default movement mode, it will be adjusted afterwards if another cheat movement mode is still active (e.g. noclip
    // may have been deactivated but flying could still be active).
    SetMovementMode(EGenMovementMode::Airborne);
  }

  // Cheat modes are only available on the server.
  if (IsLocallyControlledServerPawn())
  {
    if (GMCCVars::NoClipMode != 0)
    {
      // Noclip mode takes precedence over flying mode.
      SetMovementMode(MovementModeNoClip());
    }
    else if (GMCCVars::FlyMode != 0)
    {
      SetMovementMode(MovementModeFlying());
    }
  }

#endif
}

void UGenGoldSrcMovementComponent::OnMovementModeChanged_Implementation(EGenMovementMode PreviousMovementMode)
{
  if (IsUsingNoClipMode())
  {
    // Zero velocity when switching to noclip.
    HaltMovement();
  }

  Super::OnMovementModeChanged_Implementation(PreviousMovementMode);
}

void UGenGoldSrcMovementComponent::MovementUpdate_Implementation(float DeltaSeconds)
{
  Super::MovementUpdate_Implementation(DeltaSeconds);

  // @attention The order in which these movement abilities are called matters.
  Sprint(bWantsToSprint, DeltaSeconds);
  Crouch(bWantsToCrouch, DeltaSeconds);
  Jump(bWantsToJump, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::Sprint_Implementation(bool bRequestedSprint, float DeltaSeconds)
{
  if (bRequestedSprint && CanSprint())
  {
    MaxSpeed = SprintSpeed;
    bIsSprinting = true;
  }
  else
  {
    MaxSpeed = DefaultSpeed;
    bIsSprinting = false;
  }
}

bool UGenGoldSrcMovementComponent::CanSprint_Implementation() const
{
  return IsMovingOnGround() && !IsFullyCrouched();
}

void UGenGoldSrcMovementComponent::Crouch_Implementation(bool bRequestedCrouch, float DeltaSeconds)
{
  float CurrentHalfHeight = GetRootCollisionHalfHeight();
  if (bRequestedCrouch && CanCrouch())
  {
    if (CurrentHalfHeight > CrouchedHalfHeight)
    {
      LerpRootCollisionHalfHeight(CrouchedHalfHeight, CrouchInterpSpeed, 0.99f, DeltaSeconds, IsMovingOnGround(), EAdjustDirection::Down);
      CurrentHalfHeight = GetRootCollisionHalfHeight();
    }
    if (FMath::IsNearlyEqual(CurrentHalfHeight, CrouchedHalfHeight, 0.1f))
    {
      if (IsMovingOnGround())
      {
        MaxSpeed = CrouchWalkSpeed;
        bIsSprinting = false;
      }
    }
  }
  else
  {
    if (CurrentHalfHeight < DefaultHalfHeight)
    {
      LerpRootCollisionHalfHeight(DefaultHalfHeight, CrouchInterpSpeed, 0.99f, DeltaSeconds, IsMovingOnGround(), EAdjustDirection::Up);
    }
  }
}

bool UGenGoldSrcMovementComponent::CanCrouch_Implementation() const
{
  return IsMovingOnGround() || IsAirborne() || IsUsingFlyMode();
}

void UGenGoldSrcMovementComponent::Jump_Implementation(bool bRequestedJump, float DeltaSeconds)
{
  if (IsSwimming())
  {
    bCanJump = false;
    if (bRequestedJump)
    {
      WaterJump(DeltaSeconds);
    }
    return;
  }

  if (bRequestedJump)
  {
    if (CanJump())
    {
      AddImpulse({0.f, 0.f, JumpForce}, !bConsiderMassOnJump);

      if (bApplyJumpSpeedBoost)
      {
        ApplyJumpSpeedBoost(DeltaSeconds);
      }

      MaxSpeed = DefaultSpeed;
      bJustJumped = true;

      if (JumpMode == EJumpMode::SemiAuto)
      {
        bCanJump = false;
      }

      OnJumped();
    }

    if (JumpMode != EJumpMode::SemiAuto)
    {
      bCanJump = JumpMode == EJumpMode::FullAuto;
    }

    return;
  }

  bCanJump = true;
}

bool UGenGoldSrcMovementComponent::CanJump_Implementation() const
{
  if (!IsMovingOnGround())
  {
    return false;
  }

  return bCanJump;
}

void UGenGoldSrcMovementComponent::WaterJump_Implementation(float DeltaSeconds)
{
  if (GetCurrentImmersionDepth() < 1.f)
  {
    const float SpeedXY = GetSpeedXY();
    const float AddWaterJumpPush = WaterJumpPush - SpeedXY;
    const FVector ForwardDirection = PawnOwner->GetControlRotation().Vector().GetSafeNormal2D();
    const FVector AddSpeedXY = ForwardDirection * (AddWaterJumpPush > 0.f ? AddWaterJumpPush : 0.f);
    const FVector AddSpeedZ = {0.f, 0.f, WaterJumpForce - Velocity.Z};
    if (AddSpeedZ.Z > 0.f)
    {
      AddImpulse(AddSpeedXY + AddSpeedZ, true);
    }
  }
}

void UGenGoldSrcMovementComponent::ApplyJumpSpeedBoost_Implementation(float DeltaSeconds)
{
  const bool bApplyLargeSpeedBoost = !IsSprinting() && !IsFullyCrouched();
  const float SpeedBoost = bApplyLargeSpeedBoost ? LargeJumpSpeedBoostFactor : SmallJumpSpeedBoostFactor;
  const float ForwardMove = GetMoveInputVector().X;
  float SpeedAddition = FMath::Abs(ForwardMove) * MaxSpeed * SpeedBoost;
  const float MaxSpeedBoosted = MaxSpeed + SpeedAddition;
  const float NewSpeed = Velocity.Size2D() + SpeedAddition;
  checkGMC(NewSpeed >= 0.f)

  if (NewSpeed > MaxSpeedBoosted)
  {
    SpeedAddition -= NewSpeed - MaxSpeedBoosted;
  }

  if (ForwardMove < 0.f)
  {
    SpeedAddition *= -1.f;
  }

  const FVector ForwardDirection = PawnOwner->GetControlRotation().Vector().GetSafeNormal2D();
  AddImpulse(ForwardDirection * SpeedAddition, !bConsiderMassOnJump);
}

void UGenGoldSrcMovementComponent::ProcessLanded(const FHitResult& Hit, float DeltaSeconds, bool bUpdateFloor)
{
  Super::ProcessLanded(Hit, DeltaSeconds, bUpdateFloor);

  bJustLandedPostPhysicsLock = true;
}

void UGenGoldSrcMovementComponent::OnLanded_Implementation()
{
  Super::OnLanded_Implementation();

  bJustLandedPrePhysics = bJustLandedPostPhysics = true;
}

void UGenGoldSrcMovementComponent::PostMovementUpdate_Implementation(float DeltaSeconds)
{
  Super::PostMovementUpdate_Implementation(DeltaSeconds);

  // We combine @see bJustLandedPrePhysics and @see bJustLandedPostPhysics into one variable which is used for animations and replicated to
  // simulated proxies.
  // @attention Do not overwrite a "true"-value on sub-stepped iterations.
  bJustLanded |= HasJustLanded_Internal();
}

bool UGenGoldSrcMovementComponent::UpdateMovementModeDynamic_Implementation(const FFloorParams& Floor, float DeltaSeconds)
{
  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  check(RootComponent)

  if (IsUsingNoClipMode())
  {
    RootComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return true;
  }

  RootComponent->SetCollisionEnabled(DefaultCollisionType);

  if (IsUsingFlyMode())
  {
    return true;
  }

  return Super::UpdateMovementModeDynamic_Implementation(Floor, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::PostPhysicsUpdate_Implementation(float DeltaSeconds)
{
  Slide(MaxGroundedVelocityZ + KINDA_SMALL_NUMBER, DeltaSeconds);

  Super::PostPhysicsUpdate_Implementation(DeltaSeconds);
}

void UGenGoldSrcMovementComponent::Slide(float MinRequiredZ, float DeltaSeconds)
{
  if (MinRequiredZ < 0.f || !IsMovingOnGround() || !CurrentFloor.HasValidShapeData())
  {
    return;
  }

  const FHitResult FloorHit = CurrentFloor.ShapeHit();
  const FVector FloorNormal = FloorHit.ImpactNormal;
  const FVector ContactNormal = FloorHit.Normal;
  if (
    HitWalkableFloor(FloorHit)
    && FloorNormal.Z > KINDA_SMALL_NUMBER
    && FloorNormal.Z < (1.f - KINDA_SMALL_NUMBER)
    && ContactNormal.Z > KINDA_SMALL_NUMBER
  )
  {
    checkGMC(Velocity.Z == 0.f)

    // Project the velocity onto the floor but keep the speed the same.
    const FVector ProjectedVelocity =
      FVector(Velocity.X, Velocity.Y, -(FloorNormal | Velocity) / FloorNormal.Z).GetSafeNormal() * Velocity.Size();

    if (ProjectedVelocity.Z >= MinRequiredZ)
    {
      // The threshold is being exceeded, change to airborne movement to slide up the ramp.
      FHitResult Hit;
      SafeMoveUpdatedComponent({0.f, 0.f, MAX_DISTANCE_TO_FLOOR}, UpdatedComponent->GetComponentQuat(), true, Hit);
      UpdateVelocity(ProjectedVelocity, DeltaSeconds);
      SetMovementMode(EGenMovementMode::Airborne);
    }
  }
}

FVector UGenGoldSrcMovementComponent::PreProcessInputVector_Implementation(FVector RawInputVector)
{
  FVector TransformedInputVector = RawInputVector;
  const bool bInWater = IsSwimming();

  // Bots using acceleration already receive the move input in their controller's local space.
  if (!IsServerBot())
  {
    if (bInWater && bUseAltSwimControls)
    {
      // Transform the absolute input vector into an "AllRelative" vector for swimming.
      const FRotator ControlRotation = PawnOwner->GetControlRotation();
      TransformedInputVector = ControlRotation.RotateVector(RawInputVector);
    }
    else
    {
      // Transform the absolute input vector into an "AbsoluteZ" vector.
      FRotator ControlRotationNoPitch = PawnOwner->GetControlRotation();
      ControlRotationNoPitch.Pitch = 0.f;
      TransformedInputVector = ControlRotationNoPitch.RotateVector(RawInputVector);
    }
  }

  if (bInWater)
  {
    // If we continue to receive upward input when close to the water line we set the Z input to a small value to prevent the pawn from
    // repeatedly bouncing in and out of the water.
    const bool bThrottleUpwardInput = CurrentImmersionDepth <= BuoyantStateMinImmersion + IMMERSION_DEPTH_TOLERANCE;
    if (bThrottleUpwardInput)
    {
      TransformedInputVector.Z = FMath::Min(TransformedInputVector.Z, KINDA_SMALL_NUMBER);
    }
  }
  else if (IsMovingOnGround() || IsAirborne())
  {
    // No vertical input while grounded or airborne.
    TransformedInputVector.Z = 0.f;
  }

  return bIgnoreInputModifier ? TransformedInputVector.GetSafeNormal() : TransformedInputVector;
}

void UGenGoldSrcMovementComponent::PhysicsCustom_Implementation(float DeltaSeconds)
{
  Super::PhysicsCustom_Implementation(DeltaSeconds);

  if (IsUsingNoClipMode())
  {
    CalculateVelocity(DeltaSeconds);
    MoveUpdatedComponent(GetVelocity() * DeltaSeconds, UpdatedComponent->GetComponentRotation(), false);
    return;
  }

  if (IsUsingFlyMode())
  {
    PhysicsAirborne(DeltaSeconds);
    return;
  }
}

void UGenGoldSrcMovementComponent::CalculateVelocity(float DeltaSeconds)
{
  Super::CalculateVelocity(DeltaSeconds);

  // Enforce the speed cap if desired.
  UpdateVelocity(SpeedHardCap > 0.f ? Velocity.GetClampedToSize(0.f, SpeedHardCap) : Velocity);

  if (!bJustLandedPostPhysicsLock)
  {
    // Reset this before just before applying the actual location change (which might call @see OnLanded as well if currently airborne).
    bJustLandedPostPhysics = false;
  }
  else
  {
    // If we just landed the grounded physics are running with the remaining delta time and @see CalculateVelocity is called again, but we
    // don't want to reset @see bJustLandedPostPhysics in that case.
    checkGMC(IsMovingOnGround())
  }
}

void UGenGoldSrcMovementComponent::ApplyDeceleration(float DeltaSeconds)
{
  if (Velocity.IsZero() || HasAnimRootMotion())
  {
    return;
  }

  // This override is only for applying the slowdown when partially immersed in water.
  if (IsSwimming() || PartialImmersionSlowDown == 0.f)
  {
    return;
  }

  FVector Deceleration{0};
  if (CurrentImmersionDepth >= PartialImmersionThreshold)
  {
    // We only apply the slowdown in the XY plane so the pawn can still jump to the full Z height.
    checkGMC(PartialImmersionThreshold <= BuoyantStateMinImmersion - IMMERSION_DEPTH_TOLERANCE)
    Deceleration += PartialImmersionSlowDown * -FVector{Velocity.X, Velocity.Y, 0.f};
  }

  const FVector StartVelocity = Velocity;
  if (!Deceleration.IsZero())
  {
    Deceleration = ClampToMinDeceleration(Deceleration);
    AddAcceleration(Deceleration, DeltaSeconds);
    const FVector VelocityXY = FVector(Velocity.X, Velocity.Y, 0.f);
    const FVector VelocityZ = FVector(0.f, 0.f, Velocity.Z);
    if (DirectionsDifferXY(StartVelocity, Velocity) || VelocityXY.SizeSquared() <= FMath::Square(BRAKE_TO_STOP_VELOCITY))
    {
      UpdateVelocity(VelocityZ, DeltaSeconds);
    }
  }
}

void UGenGoldSrcMovementComponent::LimitSpeed(float DeltaSeconds)
{
  // Override to disable the super implementation, speed limiting is handled by @see PhysicsGoldSrc.
}

float UGenGoldSrcMovementComponent::GetInputAcceleration() const
{
  FLog(Warning, "<InputAcceleration> is not in use for this movement component.")
  return 0.f;
}

float UGenGoldSrcMovementComponent::GetBrakingDeceleration() const
{
  FLog(Warning, "<BrakingDeceleration> is not in use for this movement component.")
  return 0.f;
}

float UGenGoldSrcMovementComponent::GetOverMaxSpeedDeceleration() const
{
  FLog(Warning, "<OverMaxSpeedDeceleration> is not in use for this movement component.")
  return 0.f;
}

float UGenGoldSrcMovementComponent::GetMaxSpeed() const
{
  checkGMC(MaxSpeed >= 0.f)

  if (IsServerBot())
  {
    // Get max speed for path following.
    if (bRequestedMoveWithMaxSpeed || UseAccelerationForPathFollowing())
    {
      return MaxSpeed;
    }
    return bUseRequestedVelocityMaxSpeed ? RequestedVelocity.Size() : MaxSpeed;
  }

  return MaxSpeed;
}

void UGenGoldSrcMovementComponent::ApplyInputVelocity(float DeltaSeconds)
{
  if (HasAnimRootMotion()) return;

  const FVector WishMove = GetProcessedInputVector();
  FVector WishVelocity = WishMove * MaxSpeed;

  // Enforce min walk speed if moving.
  if (WishVelocity.SizeSquared() > 0.f)
  {
    checkGMC(MaxSpeed >= MinAnalogWalkSpeed)
    WishVelocity = WishVelocity.GetClampedToSize(MinAnalogWalkSpeed, BIG_NUMBER);
  }

  PhysicsGoldSrc(WishMove, WishVelocity, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::PhysicsGoldSrc(const FVector& WishMove, FVector WishVelocity, float DeltaSeconds)
{
  const bool bInWater = IsSwimming();

  if (bInWater && WishMove.IsZero())
  {
    // When there's no input drift downwards.
    WishVelocity.Z -= SinkSpeed;
  }

  const bool bIsUsingCheatModes = IsUsingNoClipMode() || IsUsingFlyMode();

  if (!(bIsUsingCheatModes || bInWater))
  {
    WishVelocity.Z = 0.f;
  }

  const FVector WishDirection = WishVelocity.GetSafeNormal();
  float WishSpeed = WishVelocity.Size();
  if (WishSpeed > MaxSpeed)
  {
    WishVelocity *= MaxSpeed / WishSpeed;
    WishSpeed = MaxSpeed;
  }

  if (bIsUsingCheatModes)
  {
    Velocity = WishVelocity;
    return;
  }

  if (bInWater)
  {
    WaterMove(WishDirection, WishSpeed, DeltaSeconds);
    return;
  }

  const bool bOnGround = IsMovingOnGround();
  if (IsAirborne() || bOnGround && HasJustLanded_Internal())
  {
    AirMove(WishDirection, WishSpeed, DeltaSeconds);
    return;
  }

  if (bOnGround)
  {
    WalkMove(WishDirection, WishSpeed, DeltaSeconds);
    return;
  }
}

void UGenGoldSrcMovementComponent::WalkMove(FVector WishDirection, float WishSpeed, float DeltaSeconds)
{
  ApplyFloorFriction(DeltaSeconds);
  Accelerate(WishDirection, WishSpeed, Velocity | WishDirection, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::AirMove(FVector WishDirection, float WishSpeed, float DeltaSeconds)
{
  WishSpeed = FMath::Clamp(WishSpeed, 0.f, MaxSpeed * AirControl);
  Accelerate(WishDirection, WishSpeed, Velocity | WishDirection, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::WaterMove(FVector WishDirection, float WishSpeed, float DeltaSeconds)
{
  WishSpeed *= WaterControl;
  const float NewSpeed = ApplyWaterFriction(DeltaSeconds);
  Accelerate(WishDirection, WishSpeed, NewSpeed, DeltaSeconds);
}

void UGenGoldSrcMovementComponent::Accelerate(FVector WishDirection, float WishSpeed, float CurrentSpeed, float DeltaSeconds)
{
  float AddSpeed = WishSpeed - CurrentSpeed;
  if (AddSpeed <= 0)
  {
    return;
  }

  float AccelSpeed = (IsAirborne() ? AirAcceleration : Acceleration) * WishSpeed * DeltaSeconds;
  if (AccelSpeed > AddSpeed)
  {
    AccelSpeed = AddSpeed;
  }

  Velocity += AccelSpeed * WishDirection;
}

float UGenGoldSrcMovementComponent::ApplyFloorFriction(float DeltaSeconds)
{
  checkGMC(Velocity.Z == 0.f)

  float CurrentSpeed = GetSpeedXY();

  if (CurrentSpeed == 0.f)
  {
    return 0.f;
  }

  const bool bBelowMinWalkSpeed = CurrentSpeed < MinAnalogWalkSpeed;
  const bool bInStopRange = CurrentSpeed < StopSpeed && !bBelowMinWalkSpeed;
  const bool bNoInput = GetProcessedInputVector().IsZero();
  const float Control = bInStopRange || bBelowMinWalkSpeed && (bNoInput || bIgnoreInputModifier) ? StopSpeed : CurrentSpeed;
  float NewSpeed = CurrentSpeed - DeltaSeconds * Control * GetFloorFriction();
  if (NewSpeed <= BRAKE_TO_STOP_VELOCITY)
  {
    NewSpeed = 0;
  }
  else if (NewSpeed < MinAnalogWalkSpeed && !bIgnoreInputModifier && !bNoInput)
  {
    const float VelocityDotInput = Velocity.GetSafeNormal() | GetProcessedInputVector().GetSafeNormal();
    if (FMath::IsNearlyEqual(VelocityDotInput, 1.f, KINDA_SMALL_NUMBER))
    {
      NewSpeed = MinAnalogWalkSpeed;
    }
  }
  NewSpeed /= CurrentSpeed;

  Velocity *= NewSpeed;
  return NewSpeed;
}

float UGenGoldSrcMovementComponent::GetFloorFriction() const
{
  if (EdgeFrictionScale == 1.f || Velocity.IsNearlyZero(KINDA_SMALL_NUMBER))
  {
    return FloorFriction;
  }

  if (!bUseAltEdgeFriction)
  {
    const FVector VelocityXYDirection = Velocity.GetSafeNormal2D();
    const FVector LeadingEdge = VelocityXYDirection * ComputeDistanceToRootCollisionBoundaryXY(VelocityXYDirection);
    if (LeadingEdge.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
      return FloorFriction;
    }

    FHitResult TraceHit;
    const FVector TraceStart = GetLowerBound() + LeadingEdge;
    const FVector TraceEnd = TraceStart + FVector::DownVector * EdgeFrictionMinRequiredHeight;
    FCollisionQueryParams CollisionQueryParams(FName(__func__), false, GetOwner());
    const auto& CollisionResponseParams = UpdatedComponent->GetCollisionResponseToChannels();
    if (const auto World = GetWorld())
    {
      World->LineTraceSingleByChannel(
        TraceHit,
        TraceStart,
        TraceEnd,
        UpdatedComponent->GetCollisionObjectType(),
        CollisionQueryParams,
        CollisionResponseParams
      );
    }

    if (!TraceHit.bBlockingHit)
    {
      // The leading edge is over a drop-off, increase the friction.
      return FloorFriction * EdgeFrictionScale;
    }
    return FloorFriction;
  }

  // Alternative behaviour: edge friction is only applied if the pawn is already halfway past the edge.
  const auto& TraceHit = CurrentFloor.LineHit();
  checkGMC(FloorTraceLength > EdgeFrictionMinRequiredHeight)
  if (!TraceHit.bStartPenetrating && TraceHit.Distance > EdgeFrictionMinRequiredHeight)
  {
    return FloorFriction * EdgeFrictionScale;
  }
  return FloorFriction;
}

float UGenGoldSrcMovementComponent::ApplyWaterFriction(float DeltaSeconds)
{
  const float CurrentSpeed = Velocity.Size();

  if (CurrentSpeed == 0.f)
  {
    return 0.f;
  }

  float FluidFriction{0.f};
  if (const auto PhysicsVolume = GetPhysicsVolume())
  {
    FluidFriction = PhysicsVolume->FluidFriction * FLUID_FRICTION_SCALE;
  }
  float NewSpeed = CurrentSpeed - FluidFriction * CurrentSpeed * DeltaSeconds;
  if (NewSpeed <= BRAKE_TO_STOP_VELOCITY)
  {
    NewSpeed = 0;
  }

  Velocity *= NewSpeed / CurrentSpeed;

  // Not actually the new speed of the pawn, this is intentional.
  return NewSpeed;
}
