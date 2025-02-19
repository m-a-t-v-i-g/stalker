// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenOrganicMovementComponent.h"
#include "GenPawn.h"
#include "FlatCapsuleComponent.h"
#define GMC_MOVEMENT_COMPONENT_LOG
#include "GMC_LOG.h"
#include "GenOrganicMovementComponent_DBG.h"

DECLARE_CYCLE_STAT(TEXT("Gen Replicated Tick"), STAT_GenReplicatedTick, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Gen Simulated Tick"), STAT_GenSimulatedTick, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Tick Pose"), STAT_TickPose, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("On Immediate State Loaded"), STAT_OnImmediateStateLoaded, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("On Simulated State Loaded"), STAT_OnSimulatedStateLoaded, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("On Movement Mode Changed"), STAT_OnMovementModeChanged, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Perform Movement"), STAT_PerformMovement, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Update Movement Mode"), STAT_UpdateMovementMode, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Pre Movement Update"), STAT_PreMovementUpdate, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Run Physics"), STAT_RunPhysics, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Physics Grounded"), STAT_PhysicsGrounded, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Physics Airborne"), STAT_PhysicsAirborne, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Physics Buoyant"), STAT_PhysicsBuoyant, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Physics Custom"), STAT_PhysicsCustom, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Move Along Floor"), STAT_MoveAlongFloor, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Move Through Air"), STAT_MoveThroughAir, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Move Through Fluid"), STAT_MoveThroughFluid, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Calculate Velocity"), STAT_CalculateVelocity, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Slide Along Surface"), STAT_SlideAlongSurface, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Two Wall Adjust"), STAT_TwoWallAdjust, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Handle Impact"), STAT_HandleImpact, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Step Up"), STAT_StepUp, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Maintain Distance To Floor"), STAT_MaintainDistanceToFloor, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Move With Base"), STAT_MoveWithBase, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Process Landed"), STAT_ProcessLanded, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Swim"), STAT_Swim, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Find Water Line"), STAT_FindWaterLine, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Check Leave Fluid"), STAT_CheckLeaveFluid, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Movement Update"), STAT_MovementUpdate, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Movement Update Simulated"), STAT_MovementUpdateSimulated, STATGROUP_GMCOrganicMovementComp)
DECLARE_CYCLE_STAT(TEXT("Physics Interaction"), STAT_PhysicsInteraction, STATGROUP_GMCOrganicMovementComp)

namespace GMCCVars
{
#if ALLOW_CONSOLE && !NO_LOGGING

  int32 StatOrganicMovementValues = 0;
  FAutoConsoleVariableRef CVarStatOrganicMovementValues(
    TEXT("gmc.StatOrganicMovementValues"),
    StatOrganicMovementValues,
    TEXT("Display realtime motion values from the organic movement component. 0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogOrganicMovementValues = 0;
  FAutoConsoleVariableRef CVarLogOrganicMovementValues(
    TEXT("gmc.LogOrganicMovementValues"),
    LogOrganicMovementValues,
    TEXT("Log realtime motion values from the organic movement component. 0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 ShowFloorSweep = 0;
  FAutoConsoleVariableRef CVarShowFloorSweep(
    TEXT("gmc.ShowFloorSweep"),
    ShowFloorSweep,
    TEXT("Visualize the result of the floor sweep. 0: Disable, 1: Enable"),
    ECVF_Default
  );

#endif
}

UGenOrganicMovementComponent::UGenOrganicMovementComponent()
{
  SetWalkableFloorAngle(WalkableFloorAngle);

  // Avoidance defaults.
  AvoidanceGroup.bGroup0 = true;
  GroupsToAvoid.Packed = 0xFFFF'FFFF;
  GroupsToIgnore.Packed = 0;

  // Nav-movement defaults.
  GetNavAgentPropertiesRef().bCanCrouch = false;
  GetNavAgentPropertiesRef().bCanJump = false;
  GetNavAgentPropertiesRef().bCanWalk = true;
  GetNavAgentPropertiesRef().bCanSwim = false;
  GetNavAgentPropertiesRef().bCanFly = false;

  // Replication defaults.
  LoadServerStateReplicationPreset(ROLE_AutonomousProxy);
  LoadServerStateReplicationPreset(ROLE_SimulatedProxy);
}

#if WITH_EDITOR

void UGenOrganicMovementComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);

  if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UGenOrganicMovementComponent, WalkableFloorAngle))
  {
    SetWalkableFloorAngle(WalkableFloorAngle);
  }
}

#endif

void UGenOrganicMovementComponent::LoadServerStateReplicationPreset(ENetRole RecipientRole)
{
  switch (RecipientRole)
  {
    case ROLE_AutonomousProxy:
    {
      ServerState_AutonomousProxy_Default.bSerializeTimestamp = true;
      ServerState_AutonomousProxy_Default.bSerializeMoveValidation = true;
      ServerState_AutonomousProxy_Default.bSerializeLocation = true;
      ServerState_AutonomousProxy_Default.bSerializeVelocity = true;
      ServerState_AutonomousProxy_Default.bSerializeRotationRoll = false;
      ServerState_AutonomousProxy_Default.bSerializeRotationPitch = false;
      ServerState_AutonomousProxy_Default.bSerializeRotationYaw = true;
      ServerState_AutonomousProxy_Default.bSerializeControlRotationRoll = false;
      ServerState_AutonomousProxy_Default.bSerializeControlRotationPitch = false;
      ServerState_AutonomousProxy_Default.bSerializeControlRotationYaw = false;
      ServerState_AutonomousProxy_Default.bSerializeInputMode = true;
      ServerState_AutonomousProxy_Default.bSerializeBoundData = true;
      return;
    }
    case ROLE_SimulatedProxy:
    {
      ServerState_SimulatedProxy_Default.bSerializeTimestamp = true;
      ServerState_SimulatedProxy_Default.bSerializeMoveValidation = false; // Not used by simulated proxies.
      ServerState_SimulatedProxy_Default.bSerializeLocation = true;
      ServerState_SimulatedProxy_Default.bSerializeVelocity = true;
      ServerState_SimulatedProxy_Default.bSerializeRotationRoll = false;
      ServerState_SimulatedProxy_Default.bSerializeRotationPitch = false;
      ServerState_SimulatedProxy_Default.bSerializeRotationYaw = true;
      ServerState_SimulatedProxy_Default.bSerializeControlRotationRoll = false;
      ServerState_SimulatedProxy_Default.bSerializeControlRotationPitch = true;
      ServerState_SimulatedProxy_Default.bSerializeControlRotationYaw = true;
      ServerState_SimulatedProxy_Default.bSerializeInputMode = true;
      ServerState_SimulatedProxy_Default.bSerializeBoundData = true;
      return;
    }
    default: checkNoEntry();
  }
}

void UGenOrganicMovementComponent::BindReplicationData_Implementation()
{
  Super::BindReplicationData_Implementation();
  BindHalfByteWithAccessor(CurrentRootCollisionShape, CurrentRootCollisionShapeAccessor, REPLICATE_COLLISION, REPLICATE_COLLISION, false);
  BindVectorWithAccessor(CurrentRootCollisionExtent, CurrentRootCollisionExtentAccessor, REPLICATE_COLLISION, REPLICATE_COLLISION, false);
  BindHalfByteWithAccessor(MovementMode, MovementModeAccessor, true, true, false);
  BindBoolWithAccessor(bReceivedUpwardForce, ReceivedUpwardForceAccessor, true, false, false);
  BindBoolWithAccessor(bHasAnimRootMotion, HasAnimRootMotionAccessor, true, false, false);
}

void UGenOrganicMovementComponent::OnServerAuthPhysicsSimulationToggled_Implementation(bool bEnabled, FServerAuthPhysicsSettings Settings)
{
  Super::OnServerAuthPhysicsSimulationToggled_Implementation(bEnabled, Settings);

  // Movement simulation happens only the server while physics are active.
  if (!IsServerPawn())
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  if (bEnabled)
  {
    SetMovementMode(EGenMovementMode::Grounded);
    if (const auto BodyInstance = RootComponent->GetBodyInstance())
    {
      // Set the physics velocity to the current kinematic velocity.
      BodyInstance->SetLinearVelocity(GetVelocity(), false);
    }
  }
  else
  {
    // Reset any rotations around the roll and pitch axis since these are not supported by the movement implementation.
    SetRootCollisionRotation({0.f, GetRootCollisionRotation().Yaw, 0.f});
    SetMovementMode(EGenMovementMode::Airborne);
  }
}

void UGenOrganicMovementComponent::OnClientAuthPhysicsSimulationToggled_Implementation(bool bEnabled, FClientAuthPhysicsSettings Settings)
{
  Super::OnClientAuthPhysicsSimulationToggled_Implementation(bEnabled, Settings);

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  if (bEnabled)
  {
    SetMovementMode(EGenMovementMode::Grounded);
    if (const auto BodyInstance = RootComponent->GetBodyInstance())
    {
      // Set the physics velocity to the current kinematic velocity.
      BodyInstance->SetLinearVelocity(GetVelocity(), false);
    }
  }
  else
  {
    // Reset any rotations around the roll and pitch axis since these are not supported by the movement implementation.
    SetRootCollisionRotation({0.f, GetRootCollisionRotation().Yaw, 0.f});
    SetMovementMode(EGenMovementMode::Airborne);
  }
}

void UGenOrganicMovementComponent::OnGMCDisabled_Implementation()
{
  HaltMovement();
  // Set the movement mode to grounded as default (we don't want to call @see OnMovementModeChanged here).
  MovementMode = static_cast<uint8>(EGenMovementMode::Grounded);
}

void UGenOrganicMovementComponent::BeginPlay()
{
  if (!PawnOwner || !GetGenPawnOwner())
  {
    Super::BeginPlay();
    return;
  }

  // Set a reference to the skeletal mesh of the owning pawn (if present).
  const auto Mesh = PawnOwner->FindComponentByClass(USkeletalMeshComponent::StaticClass());
  if (Mesh)
  {
    SetSkeletalMeshReference(Cast<USkeletalMeshComponent>(Mesh));
    checkGMC(SkeletalMesh == Mesh)
  }

  // Set the root collision parameters.
  CurrentRootCollisionShape = static_cast<uint8>(Super::GetRootCollisionShape());
  CurrentRootCollisionExtent = Super::GetRootCollisionExtent();

  // Save the default speed.
  DefaultSpeed = MaxDesiredSpeed;

  // Set the start movement mode directly (preventing a call to @see OnMovementModeChanged).
  MovementMode = static_cast<uint8>(EGenMovementMode::Grounded);

  // The intended input mode for this component is "AbsoluteZ".
  GetGenPawnOwner()->SetInputMode(EInputMode::AbsoluteZ);

  Super::BeginPlay();
}

void UGenOrganicMovementComponent::GenReplicatedTick_Implementation(float DeltaTime)
{
  SCOPE_CYCLE_COUNTER(STAT_GenReplicatedTick)

  if (ShouldSkipUpdate(DeltaTime))
  {
    return;
  }

  checkGMC(UpdatedComponent)
  if (UpdatedComponent->IsSimulatingPhysics())
  {
    PhysicsSimulationUpdate(DeltaTime);
    return;
  }

  if (IsServerBot())
  {
    CheckAvoidance();
  }

  if (ShouldComputeAvoidance())
  {
    AvoidanceLockTimer = FMath::Clamp(AvoidanceLockTimer - DeltaTime, 0.f, BIG_NUMBER);
  }

  if (bResolveExternalPenetrations)
  {
    AutoResolvePenetration();
  }

  PerformMovement(DeltaTime);

  if (bEnablePhysicsInteraction)
  {
    SCOPE_CYCLE_COUNTER(STAT_PhysicsInteraction)
    ApplyDownwardForce(DeltaTime);
    ApplyRepulsionForce(DeltaTime);
  }

  if (ShouldComputeAvoidance())
  {
    UpdateAvoidance();
  }
}

void UGenOrganicMovementComponent::GenSimulatedTick_Implementation(float DeltaTime)
{
  SCOPE_CYCLE_COUNTER(STAT_GenSimulatedTick)

  if (!CanMove())
  {
    BlockSkeletalMeshPoseTick();
    return;
  }

  {
    SCOPE_CYCLE_COUNTER(STAT_MovementUpdateSimulated)

    // Preferred entry point for subclasses to implement their own logic for simulated pawns.
    MovementUpdateSimulated(DeltaTime);
  }

  if (PreviousMovementModeSimulated != GetMovementMode())
  {
    OnMovementModeChangedSimulated(PreviousMovementModeSimulated);
    if (PreviousMovementModeSimulated == EGenMovementMode::Airborne && GetMovementMode() == EGenMovementMode::Grounded)
    {
      OnLandedSimulated();
    }
  }
  PreviousMovementModeSimulated = GetMovementMode();

  if (ShouldTickPose())
  {
    SkeletalMesh->TickPose(DeltaTime, false);
  }

  // It is safer to handle montage instances on a per tick basis for simulated pawns as well especially since listen server pawns run both
  // the movement and simulation logic.
  ResetMontages(SkeletalMesh, StepMontagePolicy);
}

void UGenOrganicMovementComponent::OnImmediateStateLoaded_Implementation(EImmediateContext Context)
{
  SCOPE_CYCLE_COUNTER(STAT_OnImmediateStateLoaded)

  if constexpr (REPLICATE_COLLISION)
  {
    MaintainRootCollisionCoherency();
    return;
  }
}

void UGenOrganicMovementComponent::OnSimulatedStateLoaded_Implementation(
  const FState& SmoothState,
  const FState& StartState,
  const FState& TargetState,
  ESimulatedContext Context
)
{
  SCOPE_CYCLE_COUNTER(STAT_OnSimulatedStateLoaded)

  if constexpr (REPLICATE_COLLISION)
  {
    if (InterpolationMethod == EInterpolationMethod::None)
    {
      MaintainRootCollisionCoherency();
      return;
    }
    MaintainRootCollisionCoherencySimulated(SmoothState, StartState, TargetState);
  }
}

FMove UGenOrganicMovementComponent::PreSimulateMovementForExtrapolation_Implementation(
  const FMove& Move,
  float ExtrapolationTime,
  const FState& StartState,
  const FState& TargetState,
  const FState& ExtrapolationState,
  const TArray<int32>& SkippedStateIndices
)
{
  // @note Since we currently do not get the input vector without a dedicated binding we use the last known velocity direction as an
  // approximation.
  FMove ProcessedMove{Move};
  ProcessedMove.InputVector = GetValidVector(TargetState.Velocity, FVector{0}).GetSafeNormal();
  return ProcessedMove;
}

EGenCollisionShape UGenOrganicMovementComponent::GetRootCollisionShape() const
{
  checkGMC(
    HasValidRootCollision() && PawnOwner->HasActorBegunPlay()
      ? CurrentRootCollisionShape == static_cast<uint8>(Super::GetRootCollisionShape())
      : true
  )
  return static_cast<EGenCollisionShape>(CurrentRootCollisionShape);
}

FVector UGenOrganicMovementComponent::GetRootCollisionExtent() const
{
  checkGMC(
    HasValidRootCollision() && PawnOwner->HasActorBegunPlay()
      ? CurrentRootCollisionExtent.Equals(Super::GetRootCollisionExtent(), 0.01f)
      : true
  )
  return CurrentRootCollisionExtent;
}

USceneComponent* UGenOrganicMovementComponent::SetRootCollisionShape(
  EGenCollisionShape NewCollisionShape,
  const FVector& Extent,
  FName Name
)
{
  if (
    NewCollisionShape >= EGenCollisionShape::Invalid
    || NewCollisionShape == static_cast<EGenCollisionShape>(Super::GetRootCollisionShape())
    && GetValidExtent(NewCollisionShape, Extent).Equals(Super::GetRootCollisionExtent(), 0.01f)
  )
  {
    checkGMC(PawnOwner)
    if (!PawnOwner) return nullptr;
    const auto OriginalRootComponent = PawnOwner->GetRootComponent();
    checkGMC(OriginalRootComponent)
    return OriginalRootComponent;
  }

  // Build a unique name with the appropriate prefix for the new object.
  FName DynamicName = [this](EGenCollisionShape CollisionShape) {
    switch (CollisionShape)
    {
      case EGenCollisionShape::VerticalCapsule:
        return MakeUniqueObjectName(PawnOwner, UCapsuleComponent::StaticClass(), ROOT_NAME_DYNAMIC_CAPSULE);
      case EGenCollisionShape::HorizontalCapsule:
        return MakeUniqueObjectName(PawnOwner, UFlatCapsuleComponent::StaticClass(), ROOT_NAME_DYNAMIC_FLAT_CAPSULE);
      case EGenCollisionShape::Box:
        return MakeUniqueObjectName(PawnOwner, UBoxComponent::StaticClass(), ROOT_NAME_DYNAMIC_BOX);
      case EGenCollisionShape::Sphere:
        return MakeUniqueObjectName(PawnOwner, USphereComponent::StaticClass(), ROOT_NAME_DYNAMIC_SPHERE);
      case EGenCollisionShape::Invalid:
      case EGenCollisionShape::MAX:
      default: checkNoEntryGMC();
    }
    checkNoEntryGMC()
    return FName{};
  }(NewCollisionShape);

  const auto NewRootComponent = Super::SetRootCollisionShape(NewCollisionShape, Extent, DynamicName);
  CurrentRootCollisionShape = static_cast<uint8>(NewCollisionShape);
  // "Extent" could have been altered if it is not a valid extent vector so we query the actual root collision extent directly through the
  // parent function.
  CurrentRootCollisionExtent = Super::GetRootCollisionExtent();
  return NewRootComponent;
}

void UGenOrganicMovementComponent::SetRootCollisionExtent(const FVector& NewExtent, bool bUpdateOverlaps)
{
  checkGMC(HasValidRootCollision())
  Super::SetRootCollisionExtent(NewExtent, bUpdateOverlaps);
  // "NewExtent" could have been altered if it is not a valid extent vector so we query the actual root collision extent directly through
  // the parent function.
  CurrentRootCollisionExtent = Super::GetRootCollisionExtent();
}

void UGenOrganicMovementComponent::MaintainRootCollisionCoherency()
{
  // @attention The parent versions of @see GetRootCollisionShape and @see GetRootCollisionExtent must be used here (otherwise we could fail
  // a check macro).
  const auto CurrentCollisionShape = static_cast<EGenCollisionShape>(CurrentRootCollisionShape);
  if (Super::GetRootCollisionShape() != CurrentCollisionShape)
  {
    SetRootCollisionShape(CurrentCollisionShape, CurrentRootCollisionExtent);
    return;
  }
  if (!Super::GetRootCollisionExtent().Equals(CurrentRootCollisionExtent, 0.01f))
  {
    SetRootCollisionExtent(CurrentRootCollisionExtent);
  }
}

void UGenOrganicMovementComponent::MaintainRootCollisionCoherencySimulated(
  const FState& SmoothState,
  const FState& StartState,
  const FState& TargetState
)
{
  checkGMC(SmoothState.IsValid())
  checkGMC(StartState.IsValid())
  checkGMC(TargetState.IsValid())
  checkGMC(InterpolationMethod > EInterpolationMethod::None)
  checkGMC(HasValidRootCollision())

  if (bInterpolateCollisionExtent)
  {
    // If we interpolate the collision extent we use the shape that is already set from the state we are closest to in time at this point.
    // @attention Extrapolating the collision extent will almost never produce a desirable result so we clamp the alpha to 1.
    const FVector StartRootCollisionExtent = GetPreReplicatedVectorFromState(StartState, CurrentRootCollisionExtentAccessor);
    const FVector TargetRootCollisionExtent = GetPreReplicatedVectorFromState(TargetState, CurrentRootCollisionExtentAccessor);
    const float Alpha = FMath::Clamp(
      (SmoothState.Timestamp - StartState.Timestamp) / FMath::Max(TargetState.Timestamp - StartState.Timestamp, MIN_DELTA_TIME), 0.f, 1.f
    );
    CurrentRootCollisionExtent = FMath::LerpStable(StartRootCollisionExtent, TargetRootCollisionExtent, Alpha);
  }
  else
  {
    // Set both properties directly from the target state.
    CurrentRootCollisionShape = GetPreReplicatedHalfByteFromState(TargetState, CurrentRootCollisionShapeAccessor);
    CurrentRootCollisionExtent = GetPreReplicatedVectorFromState(TargetState, CurrentRootCollisionExtentAccessor);
  }
  MaintainRootCollisionCoherency();
}

void UGenOrganicMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
  if (!NewUpdatedComponent)
  {
    Super::SetUpdatedComponent(NewUpdatedComponent);
    return;
  }

  const auto NewGenPawnOwner = Cast<AGenPawn>(NewUpdatedComponent->GetOwner());
  if (!NewGenPawnOwner)
  {
    Super::SetUpdatedComponent(NewUpdatedComponent);
    return;
  }

  if (
    !(Cast<UFlatCapsuleComponent>(NewUpdatedComponent)
    || Cast<UCapsuleComponent>(NewUpdatedComponent)
    || Cast<UBoxComponent>(NewUpdatedComponent)
    || Cast<USphereComponent>(NewUpdatedComponent))
  )
  {
    CFLog(GetWorld() && GetWorld()->IsGameWorld(), Error, "New updated component must have a supported collision shape.")
    Super::SetUpdatedComponent(NewUpdatedComponent);
    return;
  }

  if (IsValid(UpdatedPrimitive) && UpdatedPrimitive->OnComponentBeginOverlap.IsBound())
  {
    UpdatedPrimitive->OnComponentBeginOverlap.RemoveDynamic(this, &UGenOrganicMovementComponent::RootCollisionTouched);
  }

  Super::SetUpdatedComponent(NewUpdatedComponent);
  checkGMC(IsValid(UpdatedComponent))
  checkGMC(IsValid(UpdatedPrimitive))
  checkGMC(GetGenPawnOwner())

  if (!IsValid(UpdatedComponent) || !IsValid(UpdatedPrimitive))
  {
    FLog(Error, "New updated component is not valid.")
    DisableMovement();
    return;
  }

  if (PawnOwner->GetRootComponent() != UpdatedComponent)
  {
    FLog(Warning, "New updated component must be the root component.")
    PawnOwner->SetRootComponent(UpdatedComponent);
  }

  if (bEnablePhysicsInteraction)
  {
    UpdatedPrimitive->OnComponentBeginOverlap.AddUniqueDynamic(this, &UGenOrganicMovementComponent::RootCollisionTouched);
  }
}

void UGenOrganicMovementComponent::PerformMovement(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_PerformMovement)

  PreMovementUpdate(DeltaSeconds);

  if (!CanMove())
  {
    BlockSkeletalMeshPoseTick();
    HaltMovement();
    return;
  }

  {
    SCOPE_CYCLE_COUNTER(STAT_UpdateMovementMode)

    EGenMovementMode PreviousMovementMode = GetMovementMode();

    if (!UpdateMovementModeDynamic(CurrentFloor, DeltaSeconds))
    {
      UpdateMovementModeStatic(CurrentFloor, DeltaSeconds);
    }

    OnMovementModeUpdated(PreviousMovementMode);
  }

  // @note Often the input vector is processed with regard to the current movement mode so we call this after the movement mode was updated.
  ProcessedInputVector = PreProcessInputVector(GetMoveInputVector());

  RunPhysics(DeltaSeconds);

  const FVector VelocityBeforeMovementUpdate = GetVelocity();

  {
    SCOPE_CYCLE_COUNTER(STAT_MovementUpdate)

    // Preferred entry point for subclasses to implement their own movement logic.
    MovementUpdate(DeltaSeconds);
  }

  bHasAnimRootMotion = false;
  bool bSimulatePoseTick{false};
  if (ShouldTickPose(&bSimulatePoseTick))
  {
    TickPose(DeltaSeconds, bSimulatePoseTick);
  }

  CheckReceivedUpwardForce(VelocityBeforeMovementUpdate);

  PostMovementUpdate(DeltaSeconds);

  // Clear montages according to @see StepMontagePolicy after they have been ticked and the post-movement update has run to ensure that no
  // unwanted montage data is carried over between ticks by the anim instance.
  // @attention It is important that montages are only cleared after the post-movement update so the event can be used to query the updated
  // montage data.
  ResetMontages(SkeletalMesh, StepMontagePolicy);

  DEBUG_STAT_AND_LOG_ORGANIC_MOVEMENT_VALUES
  DEBUG_LOG_NAN_DIAGNOSTIC
}

void UGenOrganicMovementComponent::PreMovementUpdate_Implementation(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_PreMovementUpdate)

  ClampToValidValues();

  // This is also a good place to (re)set all values that need to be frame-independent to guarantee accurate client replay.
  SetPhysDeltaTime(DeltaSeconds, true);
  ProcessedInputVector = FVector::ZeroVector;
  LedgeFallOffDirection = FVector::ZeroVector;
  UpdateFloor(CurrentFloor, FloorTraceLength);
  UpdateImmersionDepth();
  RootMotionParams = FRootMotionMovementParams();
  bStuckInGeometry = false;
}

void UGenOrganicMovementComponent::ClampToValidValues()
{
  SetWalkableFloorAngle(WalkableFloorAngle);
  Mass = FMath::Clamp(Mass, KINDA_SMALL_NUMBER, BIG_NUMBER);
  MaxDesiredSpeed = FMath::Clamp(MaxDesiredSpeed, 0.f, BIG_NUMBER);
  MinAnalogWalkSpeed = FMath::Clamp(MinAnalogWalkSpeed, BRAKE_TO_STOP_VELOCITY + KINDA_SMALL_NUMBER, BIG_NUMBER);
  MinAnalogWalkSpeed = FMath::Min(MinAnalogWalkSpeed, MaxDesiredSpeed);
  RotationRate = FMath::Clamp(RotationRate, 0.f, BIG_NUMBER);
  MaxStepUpHeight = FMath::Clamp(MaxStepUpHeight, 0.f, BIG_NUMBER);
  MaxStepDownHeight = FMath::Clamp(MaxStepDownHeight, 0.f, BIG_NUMBER);
  LedgeFallOffThreshold = FMath::Clamp(LedgeFallOffThreshold, 0.f, 1.f);
  BuoyantStateMinImmersion = FMath::Clamp(BuoyantStateMinImmersion, KINDA_SMALL_NUMBER, 1.f);
  PartialImmersionThreshold = FMath::Clamp(PartialImmersionThreshold, KINDA_SMALL_NUMBER, 1.f);
  PartialImmersionThreshold = FMath::Min(PartialImmersionThreshold, BuoyantStateMinImmersion - IMMERSION_DEPTH_TOLERANCE);
  FluidMinExitSpeed = FMath::Clamp(FluidMinExitSpeed, 0.f, BIG_NUMBER);
  GravityScale = FMath::Clamp(GravityScale, 0.f, BIG_NUMBER);
  FloorTraceLength = FMath::Clamp(FloorTraceLength, MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR + KINDA_SMALL_NUMBER, BIG_NUMBER);
  MaxDepenetrationWithGeometry = FMath::Clamp(MaxDepenetrationWithGeometry, 0.f, BIG_NUMBER);
  MaxDepenetrationWithPawn = FMath::Clamp(MaxDepenetrationWithPawn, 0.f, BIG_NUMBER);
}

void UGenOrganicMovementComponent::RunPhysics(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_RunPhysics)

  PrePhysicsUpdate(DeltaSeconds);

  switch (GetMovementMode())
  {
    case EGenMovementMode::None:
    {
      checkGMC(false)
      return;
    }
    case EGenMovementMode::Grounded:
    {
      PhysicsGrounded(DeltaSeconds);
      break;
    }
    case EGenMovementMode::Airborne:
    {
      PhysicsAirborne(DeltaSeconds);
      break;
    }
    case EGenMovementMode::Buoyant:
    {
      PhysicsBuoyant(DeltaSeconds);
      break;
    }
    default:
    {
      SCOPE_CYCLE_COUNTER(STAT_PhysicsCustom)
      PhysicsCustom(DeltaSeconds);
    }
  }

  PostPhysicsUpdate(DeltaSeconds);
}

void UGenOrganicMovementComponent::PostPhysicsUpdate_Implementation(float DeltaSeconds)
{
  // The flag is mainly intended to be set externally by the user and should have been processed at this point, so this is a good place to reset it for the next
  // movement update.
  bReceivedUpwardForce = false;

  // Set the physics delta time again in case it was altered by custom physics.
  SetPhysDeltaTime(DeltaSeconds, false);
}

bool UGenOrganicMovementComponent::UpdateMovementModeDynamic_Implementation(const FFloorParams& Floor, float DeltaSeconds)
{
  const bool bIsInWater = CurrentImmersionDepth >= BuoyantStateMinImmersion;
  if (bIsInWater)
  {
    return false;
  }

  if (IsAirborne())
  {
    if (bReceivedUpwardForce)
    {
      // Remain airborne as long as we are receiving upward velocity.
      return true;
    }
  }

  if (IsMovingOnGround())
  {
    if (!IsAffectedByGravity())
    {
      // Without gravity we don't stick to or fall off any surfaces.
      return false;
    }

    // If our movement mode is currently grounded we would continue to stick to the ground even if we received upward velocity externally.
    if (bReceivedUpwardForce || IsExceedingMaxGroundedVelocityZ())
    {
      FHitResult Hit;
      SafeMoveUpdatedComponent({0.f, 0.f, MAX_DISTANCE_TO_FLOOR}, UpdatedComponent->GetComponentQuat(), true, Hit);
      SetMovementMode(EGenMovementMode::Airborne);
      return true;
    }

    // We should continue to stick to the ground if we did not receive external upward force and other constrains are not violated (e.g.
    // to not lose contact with a dynamic base we are standing on that is moving downwards).
    const bool bShapeHitWithinStepDownHeight =
      Floor.HasValidShapeData() && Floor.GetShapeDistanceToFloor() <= MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR;
    const bool bLineHitWithinStepDownHeight =
      Floor.HasValidLineData() && Floor.GetLineDistanceToFloor() <= MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR;
    const bool bShouldFallOffLedge =
      !bLineHitWithinStepDownHeight
      && Floor.HasValidShapeData()
      && IsExceedingFallOffThreshold(Floor.ShapeHit().ImpactPoint, GetLowerBound(), UpdatedComponent->GetComponentLocation());
    if (bShapeHitWithinStepDownHeight && !bShouldFallOffLedge && HitWalkableFloor(Floor.ShapeHit()))
    {
      checkGMC(IsMovingOnGround())
      return true;
    }
  }

  if (IsSwimming())
  {
    // Since we are not in water anymore the pawn must have just left the fluid volume.
    return CheckLeaveFluid(CurrentFloor, EXIT_FLUID_IMMERSION_TOLERANCE, FluidMinExitSpeed, 0.f);
  }

  return false;
}

void UGenOrganicMovementComponent::UpdateMovementModeStatic_Implementation(const FFloorParams& Floor, float DeltaSeconds)
{
  checkGMC(BuoyantStateMinImmersion >= KINDA_SMALL_NUMBER)

  // @attention When swimming or flying (no gravity) we are never based on anything i.e. nothing is considered a walkable surface.

  if (CurrentImmersionDepth >= BuoyantStateMinImmersion)
  {
    SetMovementMode(EGenMovementMode::Buoyant);
    FLog(VeryVerbose, "New movement mode is %s (immersion depth = %f).", *DebugGetMovementModeAsString(MovementMode), CurrentImmersionDepth)
    return;
  }

  if (!IsAffectedByGravity())
  {
    SetMovementMode(EGenMovementMode::Airborne);
    return;
  }

  checkGMC(GetGravityZ() < 0.f)
  checkGMC(LedgeFallOffDirection.IsZero())
  bool bShouldFallOffLedge = false;
  if (Floor.HasValidShapeData())
  {
    const bool bShapeHitWithinFloorRange = Floor.GetShapeDistanceToFloor() <= MAX_DISTANCE_TO_FLOOR;
    const bool bLineHitWithinFloorRange = Floor.HasValidLineData() && Floor.GetLineDistanceToFloor() <= MAX_DISTANCE_TO_FLOOR;
    const bool bHitFloorIsWalkable =
      bShapeHitWithinFloorRange && HitWalkableFloor(Floor.ShapeHit()) || bLineHitWithinFloorRange && HitWalkableFloor(Floor.LineHit());
    const bool bVelocityPointsAwayFromGround = (Floor.ShapeHit().ImpactNormal | GetVelocity().GetSafeNormal()) > DOT_PRODUCT_75;
    const bool bHasSignificantUpwardVelocity = GetVelocity().Z > 100.f;
    const bool bShouldRemainAirborne = IsAirborne() && (bHasSignificantUpwardVelocity && bVelocityPointsAwayFromGround || IsExceedingMaxGroundedVelocityZ());
    const bool bLineHitWithinStepDownHeight =
      Floor.HasValidLineData() && Floor.GetLineDistanceToFloor() <= MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR;
    bShouldFallOffLedge =
      !bLineHitWithinStepDownHeight
      && IsExceedingFallOffThreshold(Floor.ShapeHit().ImpactPoint, GetLowerBound(), UpdatedComponent->GetComponentLocation());
    if (
      (bShapeHitWithinFloorRange || bLineHitWithinFloorRange)
      && bHitFloorIsWalkable
      && !bShouldRemainAirborne
      && !bShouldFallOffLedge
    )
    {
      if (IsAirborne())
      {
        FLog(VeryVerbose, "Pawn landing on ground due to being in range of a walkable floor.")

        const FHitResult& Hit = Floor.ShapeHit().IsValidBlockingHit() ? Floor.ShapeHit() : Floor.LineHit();
        checkGMC(Hit.IsValidBlockingHit())

        // Switching to grounded movement like this should trigger a hit notification as well.
        if (const auto UpdatedComponentPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent))
        {
          PawnOwner->DispatchBlockingHit(UpdatedComponentPrimitive, Hit.GetComponent(), true, Hit);
        }

        // Process landing and set the movement mode to grounded.
        ProcessLanded(Hit, DeltaSeconds, false);

        return;
      }

      // The pawn was already on the ground.
      checkGMC(IsMovingOnGround())

      return;
    }
  }

  SetMovementMode(EGenMovementMode::Airborne);

  if (bShouldFallOffLedge)
  {
    const FVector ImpactToCenter = (UpdatedComponent->GetComponentLocation() - Floor.ShapeHit().ImpactPoint);
    LedgeFallOffDirection = FVector(ImpactToCenter.X, ImpactToCenter.Y, 0.f).GetSafeNormal();
  }
}

void UGenOrganicMovementComponent::OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode)
{
  if (IsMovingOnGround())
  {
    MaintainDistanceToFloor(CurrentFloor);
  }
}

bool UGenOrganicMovementComponent::MaintainDistanceToFloor(FFloorParams& Floor)
{
  SCOPE_CYCLE_COUNTER(STAT_MaintainDistanceToFloor)

  if (!Floor.HasValidShapeData() || !HitWalkableFloor(Floor.ShapeHit()))
  {
    FLog(
      VeryVerbose,
      "Returning: Floor.HasValidShapeData() = %s, HitWalkableFloor(Floor.ShapeHit()) = %s.",
      Floor.HasValidShapeData() ? TEXT("true") : TEXT("false"),
      HitWalkableFloor(Floor.ShapeHit()) ? TEXT("true") : TEXT("false")
    )
    // We need a valid shape trace that hit a walkable surface to adjust.
    return false;
  }

  float CurrentDistanceToFloor = Floor.GetShapeDistanceToFloor();
  if (CurrentDistanceToFloor >= MIN_DISTANCE_TO_FLOOR && CurrentDistanceToFloor <= MAX_DISTANCE_TO_FLOOR)
  {
    FLog(
      VeryVerbose,
      "Returning: Pawn already within set limits (MIN_DISTANCE_TO_FLOOR = %f, MAX_DISTANCE_TO_FLOOR = %f).",
      MIN_DISTANCE_TO_FLOOR,
      MAX_DISTANCE_TO_FLOOR
    )
    // The pawn is already within the target area.
    return true;
  }

  const FVector StartLocation = UpdatedComponent->GetComponentLocation();
  const float AvgDistanceToFloor = 0.5f * (MIN_DISTANCE_TO_FLOOR + MAX_DISTANCE_TO_FLOOR);
  const float DeltaZ = AvgDistanceToFloor - CurrentDistanceToFloor;

  // Here we check for undesirable behaviour that can happen with rounded collision shapes. The pawn could unintentionally get pushed up a
  // barrier because the underside of the collision "peeks" over a ledge (i.e. the impact point of the shape hit is higher than the impact
  // point of the line hit).
  const bool bHasValidLineData = Floor.HasValidLineData();
  if (
    DeltaZ > 0.f
    && bHasValidLineData
    && Floor.ShapeHit().ImpactPoint.Z > Floor.LineHit().ImpactPoint.Z
    && Floor.GetLineDistanceToFloor() >= MIN_DISTANCE_TO_FLOOR
    && Floor.GetLineDistanceToFloor() <= MAX_DISTANCE_TO_FLOOR
  )
  {
    return false;
  }

  // Try to maintain the average distance from the floor.
  FHitResult HitResult;
  {
    const FVector AdjustDelta = FVector(0.f, 0.f, DeltaZ);
    SafeMoveUpdatedComponent(AdjustDelta, UpdatedComponent->GetComponentQuat(), true, HitResult);
    // We refrain from running the floor traces again here and just update the saved distances. Some aspects of the saved shape trace may
    // not reflect the current state completely accurately anymore but the deviations are most likely not significant enough to justify
    // retracing.
    const float MovedDeltaZ = DeltaZ * HitResult.Time;
    Floor.SetShapeDistanceToFloor(Floor.GetShapeDistanceToFloor() + MovedDeltaZ);
    if (bHasValidLineData) Floor.SetLineDistanceToFloor(Floor.GetLineDistanceToFloor() + MovedDeltaZ);
    FLog(
      VeryVerbose,
      "Pawn moved %s by %f (HitResult.bBlockingHit = %s).",
      DeltaZ > 0.f ? TEXT("up") : TEXT("down"),
      DeltaZ * HitResult.Time,
      HitResult.bBlockingHit ? TEXT("true") : TEXT("false")
    )

    if (MovedDeltaZ == 0.f)
    {
      FLog(
        VeryVerbose,
        "Could not maintain distance to floor. Current values are "
        "%f (shape trace) and %f (line trace) (set limits: MIN_DISTANCE_TO_FLOOR = %f, MAX_DISTANCE_TO_FLOOR = %f).",
        Floor.GetShapeDistanceToFloor(),
        bHasValidLineData ? Floor.GetLineDistanceToFloor() : -1.f,
        MIN_DISTANCE_TO_FLOOR,
        MAX_DISTANCE_TO_FLOOR
      )
      return false;
    }
  }

  CurrentDistanceToFloor = Floor.GetShapeDistanceToFloor();
  if (CurrentDistanceToFloor >= MIN_DISTANCE_TO_FLOOR && CurrentDistanceToFloor <= MAX_DISTANCE_TO_FLOOR)
  {
    FLog(
      VeryVerbose,
      "Adjustment successful, new values are %f (shape trace) and %f (line trace) "
      "(set limits: MIN_DISTANCE_TO_FLOOR = %f, MAX_DISTANCE_TO_FLOOR = %f).",
      Floor.GetShapeDistanceToFloor(),
      bHasValidLineData ? Floor.GetLineDistanceToFloor() : -1.f,
      MIN_DISTANCE_TO_FLOOR,
      MAX_DISTANCE_TO_FLOOR
    )
    return true;
  }

  FLog(
    VeryVerbose,
    "Adjustment not successful, new values are %f (shape trace) and %f (line trace) "
    "(set limits: MIN_DISTANCE_TO_FLOOR = %f, MAX_DISTANCE_TO_FLOOR = %f).",
    Floor.GetShapeDistanceToFloor(),
    bHasValidLineData ? Floor.GetLineDistanceToFloor() : -1.f,
    MIN_DISTANCE_TO_FLOOR,
    MAX_DISTANCE_TO_FLOOR
  )
  return false;
}

void UGenOrganicMovementComponent::PhysicsGrounded(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_PhysicsGrounded)

  CalculateVelocity(DeltaSeconds);

  // The base velocity should be calculated before moving along the floor but applied afterwards.
  const FVector BaseVelocity = bMoveWithBase ? ComputeBaseVelocity(CurrentFloor.ShapeHit().GetComponent()) : FVector{0};
  const FVector LocationDelta = GetVelocity() * DeltaSeconds;
  MoveAlongFloor(LocationDelta, CurrentFloor, DeltaSeconds);
  if (bMoveWithBase) MoveWithBase(BaseVelocity, CurrentFloor, DeltaSeconds);
}

void UGenOrganicMovementComponent::PhysicsAirborne(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_PhysicsAirborne)

  const FVector OldVelocity = GetVelocity();
  CalculateVelocity(DeltaSeconds);

  // Use midpoint integration to compute the location delta.
  const FVector LocationDelta = 0.5f * (OldVelocity + GetVelocity()) * DeltaSeconds;
  MoveThroughAir(LocationDelta, DeltaSeconds);
}

void UGenOrganicMovementComponent::PhysicsBuoyant(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_PhysicsBuoyant)

  CalculateVelocity(DeltaSeconds);

  const FVector LocationDelta = GetVelocity() * DeltaSeconds;
  MoveThroughFluid(LocationDelta, DeltaSeconds);
}

float UGenOrganicMovementComponent::GetInputAcceleration() const
{
  switch (GetMovementMode())
  {
    case EGenMovementMode::None: return 0.f;
    case EGenMovementMode::Grounded: return InputAccelerationGrounded;
    case EGenMovementMode::Airborne: return InputAccelerationAirborne;
    case EGenMovementMode::Buoyant: return InputAccelerationBuoyant;
    default: return GetInputAccelerationCustom();
  }
}

float UGenOrganicMovementComponent::GetBrakingDeceleration() const
{
  switch (GetMovementMode())
  {
    case EGenMovementMode::None: return 0.f;
    case EGenMovementMode::Grounded: return BrakingDecelerationGrounded;
    case EGenMovementMode::Airborne: return BrakingDecelerationAirborne;
    case EGenMovementMode::Buoyant: return BrakingDecelerationBuoyant;
    default: return GetBrakingDecelerationCustom();
  }
}

float UGenOrganicMovementComponent::GetOverMaxSpeedDeceleration() const
{
  switch (GetMovementMode())
  {
    case EGenMovementMode::None: return 0.f;
    case EGenMovementMode::Grounded: return OverMaxSpeedDecelerationGrounded;
    case EGenMovementMode::Airborne: return OverMaxSpeedDecelerationAirborne;
    case EGenMovementMode::Buoyant: return OverMaxSpeedDecelerationBuoyant;
    default: return GetOverMaxSpeedDecelerationCustom();
  }
}

void UGenOrganicMovementComponent::CalculateVelocity(float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_CalculateVelocity)

  if (IsMovingOnGround())
  {
    UpdateVelocity({GetVelocity().X, GetVelocity().Y, 0.f}, DeltaSeconds);
  }

  const bool bCanPerformDirectMove = IsServerBot() && !UseAccelerationForPathFollowing();
  if (bCanPerformDirectMove && ShouldPerformDirectMove())
  {
    BotDirectMove(RequestedVelocity, DeltaSeconds);
  }
  else
  {
    ApplyInputVelocity(DeltaSeconds);

    ApplyRotation(false, DeltaSeconds);

    ApplyExternalForces(DeltaSeconds);

    CalculateVelocityCustom(DeltaSeconds);

    ApplyDeceleration(DeltaSeconds);

    LimitSpeed(DeltaSeconds);
  }

  if (ShouldComputeAvoidance())
  {
    CalculateAvoidanceVelocity(DeltaSeconds);
  }

  PostProcessPawnVelocity();
}

bool UGenOrganicMovementComponent::ShouldPerformDirectMove_Implementation() const
{
  return !RequestedVelocity.IsNearlyZero() && !HasAnimRootMotion();
}

void UGenOrganicMovementComponent::BotDirectMove_Implementation(const FVector& PathVelocity, float DeltaSeconds)
{
  FVector PathDirection = PathVelocity.GetSafeNormal();
  const float CurrentMaxSpeed = GetMaxSpeed();

  FVector NewVelocity{0};
  if (IsMovingOnGround())
  {
    PathDirection.Z = Velocity.Z = 0.f;
    NewVelocity = PathDirection * CurrentMaxSpeed;
  }
  else if (IsAirborne() && IsAffectedByGravity() && PathDirection.Z == 0.f)
  {
    // Apply gravity if no Z velocity was requested.
    const FVector NewVelocityXY = PathDirection * CurrentMaxSpeed;
    const FVector NewVelocityZ = {0.f, 0.f, FMath::Clamp(Velocity.Z, -BIG_NUMBER, 0.f) + GetGravityZ() * DeltaSeconds};
    NewVelocity = NewVelocityXY + NewVelocityZ;
  }
  else
  {
    NewVelocity = PathDirection * CurrentMaxSpeed;
  }

  UpdateVelocity(NewVelocity);

  ApplyRotation(true, DeltaSeconds);
}

void UGenOrganicMovementComponent::ApplyInputVelocity(float DeltaSeconds)
{
  if (!HasMoveInputEnabled() || HasAnimRootMotion()) return;

  FVector InputAcceleration = ComputeInputAcceleration();
  if (InputAcceleration.IsNearlyZero())
  {
    return;
  }

  if (IsMovingOnGround())
  {
    // Maintain horizontal velocity while grounded.
    InputAcceleration.Z = 0.f;
    // The ground friction affects our ability to accelerate and change direction. For the input acceleration we clamp this to a mulitplier
    // of 1 but for deceleration it can be larger to come to a halt faster.
    InputAcceleration *= FMath::Min(GroundFriction, 1.f);
  }

  const bool bShouldApplyFallControl = FallControl > KINDA_SMALL_NUMBER && IsAirborne() && IsAffectedByGravity();
  if (bShouldApplyFallControl)
  {
    // If we use fall control the input will be added in a different way.
    ApplyFallControl(FallControl, InputAcceleration, DeltaSeconds);
    return;
  }

  AddAcceleration(InputAcceleration, DeltaSeconds);
}

void UGenOrganicMovementComponent::ApplyRotation(bool bIsDirectBotMove, float DeltaSeconds)
{
  if (HasAnimRootMotion())
  {
    return;
  }

  // If both options are set "bOrientToControlRotationDirection" takes precedence.
  if (bOrientToControlRotationDirection)
  {
    if (const auto Controller = PawnOwner->GetController())
    {
      RotateYawTowardsDirectionSafe(Controller->GetControlRotation().Vector(), RotationRate, DeltaSeconds);
    }
    return;
  }

  if (bOrientToInputDirection && HasMoveInputEnabled())
  {
    // For bots with direct movement the input direction is the requested velocity direction.
    RotateYawTowardsDirectionSafe(bIsDirectBotMove ? GetVelocity() : GetProcessedInputVector(), RotationRate, DeltaSeconds);
    return;
  }
}

void UGenOrganicMovementComponent::ApplyExternalForces(float DeltaSeconds)
{
  checkGMC(!LedgeFallOffDirection.IsZero() ? IsAirborne() : true)

  if (IsAirborne())
  {
    // Add gravity.
    AddAcceleration(GetGravity(), DeltaSeconds);

    // Make the pawn fall off a ledge if required.
    ApplyLedgeFallOffAcceleration(DeltaSeconds);

    return;
  }

  if (IsSwimming())
  {
    // Add buoyancy. Not applied if we are just at the water line, otherwise we would constantly switch in and out of the buoyant state.
    if (CurrentImmersionDepth > BuoyantStateMinImmersion + IMMERSION_DEPTH_TOLERANCE)
    {
      const float BuoyancyMagnitude = ComputeBuoyantForce(Mass, GetGravityZ(), Buoyancy);
      AddForce({0.f, 0.f, BuoyancyMagnitude}, DeltaSeconds);
    }
    return;
  }
}

void UGenOrganicMovementComponent::ApplyLedgeFallOffAcceleration(float DeltaSeconds)
{
  if (LedgeFallOffDirection.IsZero())
  {
    return;
  }

  const bool bNeedsPush = GetSpeedXY() <= MIN_LEDGE_FALL_OFF_VELOCITY || (LedgeFallOffDirection | GetVelocity()) <= DOT_PRODUCT_15;
  if (bNeedsPush)
  {
    checkGMC(LedgeFallOffDirection.IsNormalized())
    AddAcceleration(LedgeFallOffDirection * MIN_LEDGE_FALL_OFF_VELOCITY, DeltaSeconds);
  }
}

void UGenOrganicMovementComponent::ApplyDeceleration(float DeltaSeconds)
{
  if (Velocity.IsZero() || HasAnimRootMotion())
  {
    return;
  }

  FVector Deceleration{0};
  if (IsSwimming())
  {
    if (const auto PhysicsVolume = GetPhysicsVolume())
    {
      // When in water the fluid friction slows us down in all directions.
      Deceleration += PhysicsVolume->FluidFriction * FLUID_FRICTION_SCALE * -Velocity;
    }
  }
  else
  {
    // When not in water we decelerate only in the direction we are currently not accelerating in, meaning there is no deceleration if the
    // velocity direction aligns with the acceleration direction.
    const float DecelerationScale = FMath::Clamp(1.f - (GetTransientAcceleration().GetSafeNormal() | Velocity.GetSafeNormal()), 0.f, 1.f);
    Deceleration = GetBrakingDeceleration() * DecelerationScale * -Velocity.GetSafeNormal();
    if (const bool bIsFlying = IsFlying(); IsMovingOnGround() || bIsFlying)
    {
      // Scale deceleration with the ground friction.
      Deceleration *= bIsFlying ? 1.f : GroundFriction;
      if (CurrentImmersionDepth >= PartialImmersionThreshold)
      {
        // We move slower while partially immersed in water.
        checkGMC(PartialImmersionThreshold <= BuoyantStateMinImmersion - IMMERSION_DEPTH_TOLERANCE)
        Deceleration += PartialImmersionSlowDown * -Velocity;
      }
    }
    else if (IsAirborne())
    {
      // Do not decelerate against gravity.
      const bool bIsFallingDown = GetGravityZ() < 0.f && GetVelocity().Z < 0.f;
      // Do not decelerate against input in positive Z-direction.
      const bool bIsReceivingUpwardInput = ComputeInputAcceleration().Z > KINDA_SMALL_NUMBER;
      if (bIsFallingDown || bIsReceivingUpwardInput)
      {
        Deceleration.Z = 0.f;
      }
    }
  }

  const FVector StartVelocity = Velocity;
  if (!Deceleration.IsZero())
  {
    Deceleration = ClampToMinDeceleration(Deceleration);
    AddAcceleration(Deceleration, DeltaSeconds);
    // Check whether we need to set the velocity to zero (the whole vector or specific components) because we either reversed direction or
    // reached the brake-to-stop velocity.
    const FVector VelocityXY = FVector(Velocity.X, Velocity.Y, 0.f);
    const FVector VelocityZ = FVector(0.f, 0.f, Velocity.Z);
    if (DirectionsDiffer(StartVelocity, Velocity) || Velocity.SizeSquared() <= FMath::Square(BRAKE_TO_STOP_VELOCITY))
    {
      UpdateVelocity(FVector::ZeroVector, DeltaSeconds);
    }
    else
    {
      if (DirectionsDifferXY(StartVelocity, Velocity) || VelocityXY.SizeSquared() <= FMath::Square(BRAKE_TO_STOP_VELOCITY))
      {
        UpdateVelocity(VelocityZ, DeltaSeconds);
      }
      if (DirectionsDifferZ(StartVelocity, Velocity) || FMath::Abs(Velocity.Z) <= BRAKE_TO_STOP_VELOCITY)
      {
        UpdateVelocity(VelocityXY, DeltaSeconds);
      }
    }
  }
}

void UGenOrganicMovementComponent::LimitSpeed(float DeltaSeconds)
{
  if (Velocity.IsZero())
  {
    return;
  }

  if (IsAirborne() && GetGravityZ() < 0.f)
  {
    const bool bIsFalling = GetVelocity().Z < 0.f;
    if (bIsFalling)
    {
      // If falling, don't let the Z component in the direction of gravity exceed terminal velocity.
      LimitAirborneSpeedTerminalVelocity(DeltaSeconds);
    }

    // We still want to limit the speed of the pawn to terminal velocity when falling with root motion.
    if (HasAnimRootMotion())
    {
      return;
    }

    // XY-velocity components will still be limited by the max desired speed. We don't enforce a limit on the upward Z-velocity when
    // airborne, it is only slowed down by gravity.
    LimitSpeedXY(DeltaSeconds);
    return;
  }

  if (HasAnimRootMotion())
  {
    return;
  }

  LimitSpeedXYZ(DeltaSeconds);
}

void UGenOrganicMovementComponent::LimitSpeedXY(float DeltaSeconds)
{
  if (const float MaxSpeed = GetMaxSpeed(); FVector(Velocity.X, Velocity.Y, 0.f).SizeSquared() > FMath::Square(MaxSpeed))
  {
    // Slow down XY components to max speed.
    const FVector StartVelocityXY = FVector(Velocity.X, Velocity.Y, 0.f);
    AddAcceleration(-StartVelocityXY.GetSafeNormal() * GetOverMaxSpeedDeceleration(), DeltaSeconds);
    const FVector NewVelocityXY = FVector(Velocity.X, Velocity.Y, 0.f);
    if (DirectionsDifferXY(StartVelocityXY, NewVelocityXY) || NewVelocityXY.SizeSquared() < FMath::Square(MaxSpeed))
    {
      // Don't decelerate the pawn below max speed.
      UpdateVelocity(FVector(0.f, 0.f, Velocity.Z) + StartVelocityXY.GetSafeNormal() * MaxSpeed, DeltaSeconds);
    }
  }
}

void UGenOrganicMovementComponent::LimitSpeedXYZ(float DeltaSeconds)
{
  if (const float MaxSpeed = GetMaxSpeed(); IsExceedingMaxSpeed(MaxSpeed))
  {
    // Slow down to max speed. The max speed may be affected by an analog input modifier.
    const FVector StartVelocity = Velocity;
    AddAcceleration(-Velocity.GetSafeNormal() * GetOverMaxSpeedDeceleration(), DeltaSeconds);
    if (DirectionsDiffer(StartVelocity, Velocity) || Velocity.SizeSquared() < FMath::Square(MaxSpeed))
    {
      // Don't decelerate the pawn below max speed.
      UpdateVelocity(StartVelocity.GetSafeNormal() * MaxSpeed, DeltaSeconds);
    }
  }
}

void UGenOrganicMovementComponent::LimitAirborneSpeedTerminalVelocity(float DeltaSeconds)
{
  const bool bIsFalling = IsAirborne() && GetGravityZ() < 0.f && GetVelocity().Z < 0.f;
  if (!bIsFalling)
  {
    return;
  }

  if (const auto PhysicsVolume = GetPhysicsVolume())
  {
    const float TerminalVelocity = PhysicsVolume->TerminalVelocity;
    if (FMath::Abs(Velocity.Z) > TerminalVelocity)
    {
      // Slow down Z component to terminal velocity.
      const FVector StartVelocityZ = FVector(0.f, 0.f, Velocity.Z);
      AddAcceleration(-StartVelocityZ.GetSafeNormal() * GetOverMaxSpeedDeceleration(), DeltaSeconds);
      if (DirectionsDifferZ(StartVelocityZ, Velocity) || FMath::Abs(Velocity.Z) < TerminalVelocity)
      {
        // Don't decelerate the pawn below max speed.
        UpdateVelocity(FVector(Velocity.X, Velocity.Y, 0.f) + StartVelocityZ.GetSafeNormal() * TerminalVelocity, DeltaSeconds);
      }
    }
  }
}

float UGenOrganicMovementComponent::ComputeInputModifier() const
{
  FVector Input = GetProcessedInputVector();
  // For grounded movement we disregard a negative Z component because we can't move into the ground.
  if (IsMovingOnGround()) Input.Z = FMath::Clamp(Input.Z, 0.f, 1.f);
  const float InputModifier = FMath::Clamp(Input.Size(), 0.f, 1.f);
  return InputModifier;
}

void UGenOrganicMovementComponent::UpdateImmersionDepth()
{
  CurrentImmersionDepth = ComputeImmersionDepth();
}

float UGenOrganicMovementComponent::ComputeBuoyantForce(float PawnMass, float GravityZ, float BuoyancyCoefficient) const
{
  return Mass * GravityZ * -BuoyancyCoefficient;
}

void UGenOrganicMovementComponent::CheckReceivedUpwardForce_Implementation(const FVector& PreviousVelocity)
{
  if (bReceivedUpwardForce)
  {
    // The flag has already been set externally.
    return;
  }

  const bool bReceivedUpwardVelocity = GetVelocity().Z >= PreviousVelocity.Z + 0.01f;
  if (bReceivedUpwardVelocity)
  {
    FLog(VeryVerbose, "Pawn received upward velocity: VelocityZ (%f) > PreviousVelocityZ (%f)", GetVelocity().Z, PreviousVelocity.Z)
    bReceivedUpwardForce = true;
    return;
  }

  if (HasAnimRootMotion())
  {
    // If we have root motion the input vector usually doesn't matter.
    return;
  }

  const EInputMode CurrentInputMode = GetGenPawnOwner()->GetInputMode();
  if (CurrentInputMode == EInputMode::AllAbsolute || CurrentInputMode == EInputMode::None)
  {
    // It is probably better to not make assumptions about the Z-component for absolute input since we cannot generally know what it is
    // supposed to represent.
    return;
  }

  checkGMC(CurrentInputMode == EInputMode::AbsoluteZ || CurrentInputMode == EInputMode::AllRelative)

  const bool bReceivedUpwardInput = GetProcessedInputVector().Z > KINDA_SMALL_NUMBER;
  if (bReceivedUpwardInput)
  {
    FLog(VeryVerbose, "Pawn received upward input: ProcessedInputVectorZ = %f", GetProcessedInputVector().Z)
    bReceivedUpwardForce = true;
    return;
  }
}

bool UGenOrganicMovementComponent::CheckLeaveFluid(
  const FFloorParams& Floor,
  float ImmersionTolerance,
  float SpeedZTolerance,
  float DeltaSeconds
)
{
  SCOPE_CYCLE_COUNTER(STAT_CheckLeaveFluid)

  checkGMC(IsSwimming())
  checkGMC(ImmersionTolerance >= 0.f)
  checkGMC(BuoyantStateMinImmersion >= KINDA_SMALL_NUMBER)
  checkGMC(BuoyantStateMinImmersion <= 1.f)

  if (SpeedZTolerance <= KINDA_SMALL_NUMBER)
  {
    return false;
  }

  if (CurrentImmersionDepth == 0.f)
  {
    // The pawn may have left the water volume from the side in which case this check does not apply.
    return false;
  }

  const bool bShapeHitWithinFloorRange = Floor.HasValidShapeData() && Floor.GetShapeDistanceToFloor() <= MAX_DISTANCE_TO_FLOOR;
  const bool bHitWalkableFloor = bShapeHitWithinFloorRange && HitWalkableFloor(Floor.ShapeHit());
  if (
    CurrentImmersionDepth + ImmersionTolerance > BuoyantStateMinImmersion
    && GetSpeedZ() < SpeedZTolerance
    && !bHitWalkableFloor
  )
  {
    UpdateVelocity({GetVelocity().X, GetVelocity().Y, FMath::Min(GetSpeedZ(), 0.f)}, DeltaSeconds);
    const float DeltaZ = 2.f * GetRootCollisionHalfHeight() * (BuoyantStateMinImmersion - CurrentImmersionDepth);
    if (DeltaZ > 0.f)
    {
      // Move the pawn back into the fluid volume.
      MoveUpdatedComponent({0.f, 0.f, -(DeltaZ + 2.f * UU_MILLIMETER)}, UpdatedComponent->GetComponentQuat(), true);
      UpdateImmersionDepth();
    }
    return true;
  }
  return false;
}

FVector UGenOrganicMovementComponent::ComputeInputAcceleration_Implementation() const
{
  return GetProcessedInputVector().GetClampedToMaxSize(1.f) * GetInputAcceleration();
}

FVector UGenOrganicMovementComponent::ClampToMinDeceleration(const FVector& Deceleration) const
{
  const FVector DecelerationXY = FVector(Deceleration.X, Deceleration.Y, 0.f);
  const FVector DecelerationZ = FVector(0.f, 0.f, Deceleration.Z);
  const float DecelerationXYSizeSquared = DecelerationXY.SizeSquared();
  const float DecelerationZSize = FMath::Abs(Deceleration.Z);
  FVector ClampedDeceleration = Deceleration;
  if (DecelerationXYSizeSquared > 0.f && DecelerationXYSizeSquared < FMath::Square(MIN_DECELERATION))
  {
    ClampedDeceleration = DecelerationXY.GetClampedToSize(MIN_DECELERATION, BIG_NUMBER) + DecelerationZ;
  }
  if (DecelerationZSize > 0.f && DecelerationZSize < MIN_DECELERATION)
  {
    ClampedDeceleration.Z = DecelerationZ.Z < 0.f ? -MIN_DECELERATION : MIN_DECELERATION;
  }
  return ClampedDeceleration;
}

void UGenOrganicMovementComponent::MoveWithBase(const FVector& BaseVelocity, FFloorParams& Floor, float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_MoveWithBase)

  checkGMC(IsMovingOnGround())
  checkGMC(bMoveWithBase)
  if (BaseVelocity.IsNearlyZero()) return;
  const FVector LocationDelta = BaseVelocity * DeltaSeconds;
  // Do not sweep here because the base may have moved inside the pawn.
  MoveUpdatedComponent(LocationDelta, UpdatedComponent->GetComponentQuat(), false);
  UpdateFloor(Floor, FloorTraceLength);
  MaintainDistanceToFloor(Floor);
}

FVector UGenOrganicMovementComponent::ComputeBaseVelocity(UPrimitiveComponent* MovementBase)
{
  if (!MovementBase) return FVector{0};

  FVector BaseVelocity{0};
  if (IsMovable(MovementBase))
  {
    if (const auto PawnMovementBase = Cast<APawn>(MovementBase->GetOwner()))
    {
      // The current movement base is another pawn.
      BaseVelocity = GetPawnBaseVelocity(PawnMovementBase);
    }
    else
    {
      // The current movement base is some form of geometry.
      BaseVelocity = GetLinearVelocity(MovementBase);
      const FVector BaseTangentialVelocity = ComputeTangentialVelocity(GetLowerBound(), MovementBase);
      BaseVelocity += BaseTangentialVelocity;
    }
  }

  BaseVelocity = PostProcessBaseVelocity(MovementBase, BaseVelocity);
  CFLog(!BaseVelocity.IsZero(), VeryVerbose, "Calculated base velocity is %s.", *BaseVelocity.ToString())
  return BaseVelocity;
}

FVector UGenOrganicMovementComponent::GetPawnBaseVelocity(APawn* PawnMovementBase) const
{
  if (!PawnMovementBase) return FVector{0};

  const auto GenPawnMovementBase = Cast<AGenPawn>(PawnMovementBase);
  const auto GenMovementComponentOther = Cast<UGenMovementComponent>(PawnMovementBase->GetMovementComponent());

  if (GenPawnMovementBase && GenMovementComponentOther)
  {
    // Since angular velocity is not synchronised we only consider the linear velocity of the base pawn.
    return GenMovementComponentOther->GetVelocity();
  }

  // If the above case does not apply just use whatever the implementation provides. For standalone games we could also calculate the base
  // velocity as per usual (combined linear and angular velocity) but it is probably better not to have different behaviour for different
  // net modes.
  return PawnMovementBase->GetVelocity();
}

FVector UGenOrganicMovementComponent::MoveAlongFloor(const FVector& LocationDelta, FFloorParams& Floor, float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_MoveAlongFloor)

  checkGMC(IsMovingOnGround())
  FLog(
    VeryVerbose,
    "Starting to move along floor (LocationDelta = %s, Velocity = %s).",
    *LocationDelta.ToString(),
    *GetVelocity().ToString()
  )

  // When moving along the floor we ignore the Z-component of the location delta.
  const FVector MoveDelta = FVector(LocationDelta.X, LocationDelta.Y, 0.f);
  if (MoveDelta.IsNearlyZero())
  {
    FLog(VeryVerbose, "MoveDelta is zero, returning.")
    return FVector::ZeroVector;
  }

  FScopedMovementUpdate ScopedMovement(UpdatedComponent, EScopedUpdate::DeferredUpdates);

  FFloorParams InitialFloor = Floor;
  FHitResult MoveHitResult;
  const float MaxSpeedXY = FVector(GetVelocity().X, GetVelocity().Y, 0.f).Size();
  const FVector StartLocation = UpdatedComponent->GetComponentLocation();
  const bool bStartedWithinLineMaxStepDownHeight =
    Floor.HasValidLineData() && Floor.GetLineDistanceToFloor() <= MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR;
  FVector RampVector = ComputeRampVector(MoveDelta, Floor.ShapeHit());
  SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, MoveHitResult);
  const float MoveHitTime = MoveHitResult.Time;
  const float MoveHitTimeRemaining = 1.f - MoveHitTime;
  const FVector MoveDeltaRemaining = MoveDelta * MoveHitTimeRemaining;
  float RemainingDeltaSeconds = DeltaSeconds * MoveHitTimeRemaining;
  if (MoveHitResult.bStartPenetrating)
  {
    FLog(VeryVerbose, "MoveUpdatedComponent produced invalid blocking hit.")
    HandleImpact(MoveHitResult, DeltaSeconds, RampVector);
    // Try again by sliding along the surface with the full location delta.
    SlideAlongSurface(MoveDelta, 1.f, MoveHitResult.Normal, MoveHitResult, true);
  }
  else if (MoveHitResult.IsValidBlockingHit())
  {
    FLog(VeryVerbose, "MoveUpdatedComponent produced valid blocking hit.")
    if (HitWalkableFloor(MoveHitResult) && MoveHitTime > 0.f && MoveHitResult.Normal.Z > KINDA_SMALL_NUMBER)
    {
      FLog(VeryVerbose, "Moving up walkable surface.")
      // We hit another walkable surface.
      RampVector = ComputeRampVector(MoveDeltaRemaining, MoveHitResult);
      SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, MoveHitResult);
    }
    else
    {
      FLog(VeryVerbose, "Pawn hit a barrier.")
      bool bDidStepUp = false;
      const FVector HitStartLocation = UpdatedComponent->GetComponentLocation();
      // The floor sweep shape may have hit the same unwalkable surface as the move hit result but the floor line trace should still hit a
      // walkable floor for us to try a step-up.
      if (CanStepUp(MoveHitResult) && HitWalkableFloor(Floor.LineHit()))
      {
        FLog(VeryVerbose, "Trying to step up barrier.")
        // Try to step up onto the barrier.
        FHitResult StepUpHit;
        if (!StepUp(MoveDeltaRemaining, MoveHitResult, Floor, &StepUpHit))
        {
          FLog(VeryVerbose, "Step up was not successful, sliding along surface.")
          // The step-up was not successful, handle the impact and slide along the surface instead.
          HandleImpact(MoveHitResult, DeltaSeconds, RampVector);
          SlideAlongSurface(MoveDelta, MoveHitTimeRemaining, MoveHitResult.Normal, MoveHitResult, true);
          RemainingDeltaSeconds *= 1.f - MoveHitResult.Time;
        }
        else
        {
          FLog(VeryVerbose, "Step up was successful.")
          bDidStepUp = true;
          // The step-up was successful.
          // @note We can also arrive at this point when hitting a wall (i.e. we didn't actually step up onto anything). This is correct
          // behaviour since it would be difficult to determine beforehand how high the barrier actually is. Sliding along the surface is
          // handled within the step-up function in this case (while processing the forward hit). Recalculation of the velocity from the
          // moved distance is therefore also required in this case.
        }
      }
      else
      {
        FLog(VeryVerbose, "Cannot step up this barrier, sliding along surface.")
        // We cannot step up onto this barrier so just slide along the hit surface.
        HandleImpact(MoveHitResult, DeltaSeconds, RampVector);
        SlideAlongSurface(MoveDelta, MoveHitTimeRemaining, MoveHitResult.Normal, MoveHitResult, true);
        RemainingDeltaSeconds *= 1.f - MoveHitResult.Time;
      }
      // If we collided with something we need to adjust the velocity.
      const float AppliedDeltaSeconds = DeltaSeconds - RemainingDeltaSeconds;
      const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
      if (!bDidStepUp && AppliedDeltaSeconds >= MIN_DELTA_TIME)
      {
        FLog(VeryVerbose, "Recalculating velocity from moved distance after barrier hit.")
        UpdateVelocity((CurrentLocation - GetStartLocation()) / AppliedDeltaSeconds, AppliedDeltaSeconds);
      }
      else if (AppliedDeltaSeconds < MIN_DELTA_TIME && CurrentLocation.Equals(HitStartLocation, KINDA_SMALL_NUMBER))
      {
        // We are most likely stuck in a corner.
        FLog(VeryVerbose, "Zeroing velocity after barrier hit.")
        UpdateVelocity(FVector::ZeroVector, DeltaSeconds);
      }
      else if (bDidStepUp && CurrentLocation.Z <= HitStartLocation.Z + KINDA_SMALL_NUMBER)
      {
        // We did not actually step up onto anything.
        FLog(VeryVerbose, "Adjusting velocity from impact normal after barrier hit.")
        AdjustVelocityFromHit(MoveHitResult, DeltaSeconds);
      }
      // The new velocity should never exceed the velocity that was used as the basis for the calculation of the location delta.
      UpdateVelocity(FVector(GetVelocity().X, GetVelocity().Y, 0.f).GetClampedToMaxSize(MaxSpeedXY), AppliedDeltaSeconds);
    }
  }
  else if (!MoveHitResult.bBlockingHit)
  {
    FLog(VeryVerbose, "MoveUpdatedComponent produced no blocking hit.")
  }

  FVector AppliedDelta = UpdatedComponent->GetComponentLocation() - StartLocation;
  if (!AppliedDelta.IsNearlyZero())
  {
    FLog(VeryVerbose, "Updating floor after move.")
    UpdateFloor(Floor, FloorTraceLength);
    const bool bStillWithinLineMaxStepDownHeight =
      Floor.HasValidLineData() && Floor.GetLineDistanceToFloor() <= MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR;
    if (HitWalkableFloor(Floor.ShapeHit()) && bStillWithinLineMaxStepDownHeight)
    {
      FLog(VeryVerbose, "Updated floor is still walkable, maintaining distance to floor.")
      // This will also make the pawn stick to the floor again if we exceed the max step down height after moving.
      MaintainDistanceToFloor(Floor);
    }
    else
    {
      FLog(VeryVerbose, "Updated floor is not walkable.")
      if (!bCanWalkOffLedges && bStartedWithinLineMaxStepDownHeight && !bStillWithinLineMaxStepDownHeight)
      {
        FLog(VeryVerbose, "\"bCanWalkOffLedges\" is false: reverting move and zeroing velocity.")
        ScopedMovement.RevertMove();
        AppliedDelta = FVector::ZeroVector;
        UpdateVelocity(FVector::ZeroVector, DeltaSeconds);
        Floor = InitialFloor;
      }
      else
      {
        // @attention The movement mode must not be changed here.
      }
    }
  }
  else
  {
    FLog(VeryVerbose, "No movement occured.")
  }
  FLog(
    VeryVerbose,
    "Finished moving along floor. New velocity is %s, applied location delta is %s.",
    *GetVelocity().ToString(),
    *AppliedDelta.ToString()
  )
  return AppliedDelta;
}

FVector UGenOrganicMovementComponent::ComputeRampVector(const FVector& LocationDelta, const FHitResult& RampHit) const
{
  checkfGMC(LocationDelta.Z == 0.f, TEXT("The movement delta should be horizontal."))
  const FVector FloorNormal = RampHit.ImpactNormal;
  const FVector ContactNormal = RampHit.Normal;
  if (
    HitWalkableFloor(RampHit)
    && FloorNormal.Z > KINDA_SMALL_NUMBER
    && FloorNormal.Z < (1.f - KINDA_SMALL_NUMBER)
    && ContactNormal.Z > KINDA_SMALL_NUMBER
  )
  {
    // Compute a vector that moves parallel to the surface by projecting the horizontal movement onto the ramp.
    const FVector RampVector = FVector(LocationDelta.X, LocationDelta.Y, -(FloorNormal | LocationDelta) / FloorNormal.Z);
    return bRescaleSlopeDelta ? RampVector.GetSafeNormal() * LocationDelta.Size() : RampVector;
  }
  return LocationDelta;
}

bool UGenOrganicMovementComponent::StepUp(
  const FVector& LocationDelta,
  const FHitResult& BarrierHit,
  FFloorParams& Floor,
  FHitResult* OutForwardHit
)
{
  SCOPE_CYCLE_COUNTER(STAT_StepUp)

  checkGMC(IsMovingOnGround())
  checkGMC(CanStepUp(BarrierHit))
  checkGMC(Floor.HasValidLineData())
  checkGMC(HitWalkableFloor(Floor.LineHit()))

  const FVector StepLocationDelta = FVector(LocationDelta.X, LocationDelta.Y, 0.f);
  if (MaxStepUpHeight <= 0.f || StepLocationDelta.IsNearlyZero())
  {
    return false;
  }

  FLog(VeryVerbose, "Starting step-up...")
  const FVector StartLocation = UpdatedComponent->GetComponentLocation();
  float PawnHalfHeight{};
  float UpperBoundZ{};
  FVector Extent = GetRootCollisionExtent();
  switch (GetRootCollisionShape())
  {
    case EGenCollisionShape::VerticalCapsule:
      PawnHalfHeight = Extent.Z;
      UpperBoundZ = StartLocation.Z + (PawnHalfHeight - Extent.X);
      break;
    case EGenCollisionShape::Box:
      PawnHalfHeight = Extent.Z;
      UpperBoundZ = StartLocation.Z + PawnHalfHeight;
      break;
    case EGenCollisionShape::HorizontalCapsule:
    case EGenCollisionShape::Sphere:
      PawnHalfHeight = Extent.X;
      UpperBoundZ = StartLocation.Z;
      break;
    default: checkNoEntry();
  }
  const float BarrierImpactPointZ = BarrierHit.ImpactPoint.Z;
  if (BarrierImpactPointZ > UpperBoundZ)
  {
    // The top of the collision is hitting something so we cannot travel upward at all.
    FLog(VeryVerbose, "Aborting step-up, pawn is blocked from traveling upward.")
    return false;
  }

  float TravelUpHeight = MaxStepUpHeight;
  float TravelDownHeight = MaxStepUpHeight;
  float StartFloorPointZ = StartLocation.Z - PawnHalfHeight;
  float StartDistanceToFloor = 0.f;
  if (Floor.HasValidShapeData() && HitWalkableFloor(Floor.ShapeHit()))
  {
    StartDistanceToFloor = Floor.GetShapeDistanceToFloor();
    TravelDownHeight = MaxStepUpHeight + 2.f * MAX_DISTANCE_TO_FLOOR;
  }
  else if (HitWalkableFloor(Floor.LineHit()) && StartDistanceToFloor <= MAX_DISTANCE_TO_FLOOR)
  {
    StartDistanceToFloor = Floor.GetLineDistanceToFloor();
    TravelDownHeight = MaxStepUpHeight + 2.f * MAX_DISTANCE_TO_FLOOR;
  }
  // We float a variable distance above the floor so we want to calculate the max step-up height based on that distance if possible.
  TravelUpHeight = FMath::Max(MaxStepUpHeight - StartDistanceToFloor, 0.f);
  StartFloorPointZ -= StartDistanceToFloor;

  if (BarrierImpactPointZ <= StartFloorPointZ)
  {
    // The impact point is under the lower bound of the collision.
    FLog(VeryVerbose, "Aborting step-up, barrier impact point is located below the pawn's base.")
    return false;
  }

  FScopedMovementUpdate ScopedMovement(UpdatedComponent, EScopedUpdate::DeferredUpdates);

  // Step up.
  // @attention Barriers are always treated as vertical walls. The procedure is sweeping upward by "TravelUpHeight" (along positive Z),
  // sweeping forward by "StepLocationDelta" (horizontally) and sweeping downward by "TravelDownHeight" (along negative Z).
  FHitResult UpHit;
  const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
  MoveUpdatedComponent(FVector(0.f, 0.f, TravelUpHeight), PawnRotation, true, &UpHit);
  if (UpHit.bStartPenetrating)
  {
    // Invalid move, abort the step-up.
    FLog(VeryVerbose, "Aborting step-up, upward sweep started in penetration.")
    ScopedMovement.RevertMove();
    return false;
  }
  FLog(VeryVerbose, "Upward sweep has %s blocking hit.", UpHit.bBlockingHit ? TEXT("valid") : TEXT("no"))

  // Step forward.
  FVector ClampedStepLocationDelta = StepLocationDelta;
  if (StepLocationDelta.Size() < MIN_STEP_UP_DELTA)
  {
    // We need to enforce a minimal location delta for the forward sweep, otherwise we could fail a step-up that should actually be feasible
    // when hitting a non-walkable surface (because the forward step would have been very short).
    ClampedStepLocationDelta = StepLocationDelta.GetClampedToSize(MIN_STEP_UP_DELTA, BIG_NUMBER);
  }
  FHitResult ForwardHit;
  MoveUpdatedComponent(ClampedStepLocationDelta, PawnRotation, true, &ForwardHit);
  if (OutForwardHit) *OutForwardHit = ForwardHit;
  if (ForwardHit.bBlockingHit)
  {
    if (ForwardHit.bStartPenetrating)
    {
      // Invalid move, abort the step-up.
      FLog(VeryVerbose, "Aborting step-up, forward sweep started in penetration.")
      ScopedMovement.RevertMove();
      return false;
    }
    FLog(VeryVerbose, "Forward sweep has valid blocking hit.")

    // If we hit something above us and also something ahead of us we should notify about the upward hit as well. If we hit something above
    // us but not in front of us we don't notify about any hit since we are not blocked from moving (in which case we don't enter this
    // branch in the first place).
    if (UpHit.bBlockingHit)
    {
      // Notify upward hit.
      HandleImpact(UpHit);
    }
    // Notify forward hit.
    HandleImpact(ForwardHit);

    // Slide along the hit surface, but do so based on the original step location delta.
    const float ForwardHitTime = ForwardHit.Time;
    const float SlideTime = SlideAlongSurface(StepLocationDelta, 1.f - ForwardHit.Time, ForwardHit.Normal, ForwardHit, true);
    if (ForwardHitTime == 0.f && SlideTime == 0.f)
    {
      // No movement occurred, abort the step-up.
      FLog(VeryVerbose, "Aborting step-up, pawn was stuck after forward sweep.")
      ScopedMovement.RevertMove();
      return false;
    }
  }
  else
  {
    FLog(VeryVerbose, "Forward sweep has no blocking hit.")
  }

  // Step down.
  FHitResult DownHit;
  MoveUpdatedComponent(FVector(0.f, 0.f, -TravelDownHeight), UpdatedComponent->GetComponentQuat(), true, &DownHit);
  if (DownHit.bStartPenetrating)
  {
    // Invalid move, abort the step-up.
    FLog(VeryVerbose, "Aborting step-up, downward sweep started in penetration.")
    ScopedMovement.RevertMove();
    return false;
  }
  if (DownHit.bBlockingHit)
  {
    // We do not notify about hits from the downward sweep since its only purpose is to find a valid end location for the step up, it does
    // not go in the direction of actual movement.
    FLog(VeryVerbose, "Downward sweep has valid blocking hit.")
    const float DeltaZ = DownHit.ImpactPoint.Z - StartFloorPointZ;
    if (DeltaZ > MaxStepUpHeight)
    {
      // This step sequence would have allowed us to travel higher than the max step-up height.
      FLog(
        VeryVerbose,
        "Aborting step-up, MaxStepUpHeight (%f) would have been exceeded by actual step-up height (%f).",
        MaxStepUpHeight,
        DeltaZ
      )
      ScopedMovement.RevertMove();
      return false;
    }
    // Reject unwalkable surface normals.
    if (!HitWalkableFloor(DownHit))
    {
      FLog(VeryVerbose, "Downward sweep hit unwalkable surface.")
      if ((StepLocationDelta | DownHit.ImpactNormal) < 0.f)
      {
        // The normal points towards the pawn i.e. opposes the current movement direction.
        FLog(
          VeryVerbose,
          "Aborting step-up, unwalkable impact normal (%s) opposes movement direction (%s).",
          *DownHit.ImpactNormal.ToString(),
          *StepLocationDelta.GetSafeNormal().ToString()
        )
        ScopedMovement.RevertMove();
        return false;
      }
      if (DownHit.Location.Z > StartLocation.Z)
      {
        // Do not step up to an unwalkable normal above us. It is fine to step down onto an unwalkable normal below us though (we will slide
        // off) since rejecting those moves would prevent us from being able to walk off edges.
        FLog(
          VeryVerbose,
          "Aborting step-up, new unwalkable location (%s) would have been higher than the start location (%s).",
          *DownHit.Location.ToString(),
          *StartLocation.ToString()
        )
        ScopedMovement.RevertMove();
        return false;
      }
    }
    CFLog(HitWalkableFloor(DownHit), VeryVerbose, "Downward sweep hit walkable surface.")

    // Reject moves where the downward sweep hit something very close to the edge of the collision. Does not apply to box shapes.
    if (
      GetRootCollisionShape() != EGenCollisionShape::Box
      && !IsWithinEdgeTolerance(DownHit.Location, DownHit.ImpactPoint, EDGE_TOLERANCE)
    )
    {
      FLog(VeryVerbose, "Aborting step-up, impact point is not within edge-tolerance.")
      ScopedMovement.RevertMove();
      return false;
    }
    // Don't step up onto surfaces that are not a valid base.
    if (DeltaZ > 0.f && !CanStepUp(DownHit))
    {
      FLog(VeryVerbose, "Aborting step-up, pawn would have stepped up onto invalid surface.")
      ScopedMovement.RevertMove();
      return false;
    }
  }
  else
  {
    FLog(VeryVerbose, "Downward sweep has no blocking hit.")
  }
  FLog(VeryVerbose, "Finished step-up successfully.")
  return true;
}

bool UGenOrganicMovementComponent::MoveThroughAir(const FVector& LocationDelta, float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_MoveThroughAir)

  checkGMC(IsAirborne())
  FLog(VeryVerbose, "Starting to move through air (LocationDelta = %s).", *LocationDelta.ToString())

  if (LocationDelta.IsNearlyZero())
  {
    FLog(VeryVerbose, "MoveDelta is zero, returning.")
    return false;
  }

  FHitResult Hit;
  const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
  SafeMoveUpdatedComponent(LocationDelta, PawnRotation, true, Hit);

  float LastMoveTimeSlice = DeltaSeconds;
  float DeltaSecondsRemaining = DeltaSeconds * (1.f - Hit.Time);
  float DeltaSecondsApplied = DeltaSeconds - DeltaSecondsRemaining;

  if (Hit.bBlockingHit)
  {
    if (IsValidLandingSpot(Hit))
    {
      // The pawn has landed on a walkable surface.
      FLog(VeryVerbose, "Pawn landing on ground after first blocking hit.")
      ProcessLanded(Hit, DeltaSecondsRemaining, true);
      PhysicsGrounded(DeltaSecondsRemaining);
      return true;
    }
    else
    {
      // We hit a wall.
      HandleImpact(Hit, LastMoveTimeSlice, LocationDelta);
      AdjustVelocityFromHitAirborne(Hit, DeltaSecondsApplied);

      const FVector OldHitNormal = Hit.Normal;
      const FVector OldHitImpactNormal = Hit.ImpactNormal;
      const FVector SavedDelta = GetVelocity() * DeltaSecondsRemaining;
      FVector Delta = ComputeSlideVector(LocationDelta, 1.f - Hit.Time, OldHitNormal, Hit);
      if (DeltaSecondsRemaining >= MIN_DELTA_TIME && !DirectionsDiffer(Delta, SavedDelta))
      {
        // It is okay to move with the deflection vector since that will not change our direction.
        SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

        if (Hit.bBlockingHit)
        {
          // We hit another wall.
          LastMoveTimeSlice = DeltaSecondsRemaining;
          DeltaSecondsRemaining *= 1.f - Hit.Time;
          DeltaSecondsApplied = DeltaSeconds - DeltaSecondsRemaining;
          if (IsValidLandingSpot(Hit))
          {
            FLog(VeryVerbose, "Pawn landing on ground after second blocking hit.")
            ProcessLanded(Hit, DeltaSecondsRemaining, true);
            PhysicsGrounded(DeltaSecondsRemaining);
            return true;
          }
          HandleImpact(Hit, LastMoveTimeSlice, Delta);
          AdjustVelocityFromHitAirborne(Hit, DeltaSecondsApplied);

          FVector PreTwoWallDelta = Delta;
          TwoWallAdjust(Delta, Hit, OldHitNormal);
          SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

          // When "bDitch" is true the pawn is straddling two slopes, neither of which are walkable.
          bool bDitch =
            OldHitImpactNormal.Z > 0.f
            && Hit.ImpactNormal.Z > 0.f
            && FMath::Abs(Delta.Z) <= KINDA_SMALL_NUMBER
            && (Hit.ImpactNormal | OldHitImpactNormal) < 0.f;

          if (Hit.Time == 0.f)
          {
            // If we are stuck try to side step.
            FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();
            if (SideDelta.IsNearlyZero())
            {
              SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0.f).GetSafeNormal();
            }
            SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
          }

          if (IsAffectedByGravity() && (bDitch || IsValidLandingSpot(Hit) || Hit.Time == 0.f))
          {
            FLog(VeryVerbose, "Pawn landing on ground after two-wall-adjustment.")
            ProcessLanded(Hit, DeltaSecondsRemaining, true);
            PhysicsGrounded(DeltaSecondsRemaining);
            return true;
          }
        }
      }
    }
  }
  return false;
}

void UGenOrganicMovementComponent::AdjustVelocityFromHitAirborne_Implementation(const FHitResult& Hit, float DeltaSeconds)
{
  if (IsAffectedByGravity())
  {
    const float SavedVelocityZ = GetVelocity().Z;
    AdjustVelocityFromHit(Hit, DeltaSeconds);
    UpdateVelocity({GetVelocity().X, GetVelocity().Y, FMath::Clamp(GetVelocity().Z, -BIG_NUMBER, SavedVelocityZ)}, DeltaSeconds);
    return;
  }

  AdjustVelocityFromHit(Hit, DeltaSeconds);
}

bool UGenOrganicMovementComponent::IsValidLandingSpot(const FHitResult& Hit)
{
  checkGMC(IsAirborne())

  if (!IsAffectedByGravity())
  {
    return false;
  }

  if (!Hit.bBlockingHit)
  {
    return false;
  }

  if (Hit.bStartPenetrating)
  {
    ResolvePenetration(GetPenetrationAdjustment(Hit), Hit, UpdatedComponent->GetComponentQuat());
    return false;
  }

  if (IsExceedingMaxGroundedVelocityZ())
  {
    return false;
  }

  if (!HitWalkableFloor(Hit))
  {
    return false;
  }

  FHitResult StepDownHeightTest;
  const FVector LineTraceStart = Hit.Location;
  const FVector LineTraceEnd = LineTraceStart + FVector::DownVector * (MaxStepDownHeight + MAX_DISTANCE_TO_FLOOR);
  FCollisionQueryParams CollisionQueryParams(FName(__func__), false, GetOwner());
  const auto& CollisionResponseParams = UpdatedComponent->GetCollisionResponseToChannels();
  if (const auto World = GetWorld())
  {
    World->LineTraceSingleByChannel(
      StepDownHeightTest,
      LineTraceStart,
      LineTraceEnd,
      UpdatedComponent->GetCollisionObjectType(),
      CollisionQueryParams,
      CollisionResponseParams
    );
  }
  const bool bCanLandOnLedge =
    StepDownHeightTest.IsValidBlockingHit()
    || !IsExceedingFallOffThreshold(Hit.ImpactPoint, Hit.Location - FVector(0.f, 0.f, GetRootCollisionHalfHeight()), Hit.Location);
  if (!bCanLandOnLedge)
  {
    return false;
  }

  float HitLocationLowerBoundZ{};
  const float HitLocationZ = Hit.Location.Z;
  const FVector Extent = GetRootCollisionExtent();
  switch (GetRootCollisionShape())
  {
    case EGenCollisionShape::VerticalCapsule:
      HitLocationLowerBoundZ = HitLocationZ - (Extent.Z/*HalfHeight*/ - Extent.X/*Radius*/);
      break;
    case EGenCollisionShape::Box:
      HitLocationLowerBoundZ = HitLocationZ - Extent.Z;
      break;
    case EGenCollisionShape::HorizontalCapsule:
    case EGenCollisionShape::Sphere:
      HitLocationLowerBoundZ = HitLocationZ;
      break;
    default: checkNoEntry();
  }
  if (Hit.ImpactPoint.Z >= HitLocationLowerBoundZ)
  {
    // The impact point is on the vertical side of the collision shape or above it.
    return false;
  }

  if (!IsWithinEdgeTolerance(Hit.Location, Hit.ImpactPoint, EDGE_TOLERANCE))
  {
    // We reject hits that are barely on the cusp of the outer side of the collision.
    return false;
  }

  // The pawn hit a valid landing spot.
  return true;
}

void UGenOrganicMovementComponent::ProcessLanded(const FHitResult& Hit, float DeltaSeconds, bool bUpdateFloor)
{
  SCOPE_CYCLE_COUNTER(STAT_ProcessLanded)

  checkGMC(IsAirborne())

  LedgeFallOffDirection = FVector::ZeroVector;

  if (bEnablePhysicsInteraction)
  {
    ApplyImpactPhysicsForces(Hit, GetTransientAcceleration(), GetVelocity());
  }

  AdjustVelocityFromHit(Hit, DeltaSeconds);

  if (bUpdateFloor)
  {
    UpdateFloor(CurrentFloor, FloorTraceLength);
  }

  SetMovementMode(EGenMovementMode::Grounded);
}

bool UGenOrganicMovementComponent::MoveThroughFluid(const FVector& LocationDelta, float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_MoveThroughFluid)

  checkGMC(IsSwimming())
  FLog(VeryVerbose, "Starting to move through fluid (LocationDelta = %s).", *LocationDelta.ToString())

  if (LocationDelta.IsNearlyZero())
  {
    FLog(VeryVerbose, "MoveDelta is zero, returning.")
    return false;
  }

  FHitResult Hit;
  const float MaxSpeed = GetVelocity().Size();
  const float TimeOutOfWater = Swim(LocationDelta, Hit, DeltaSeconds);
  float DeltaSecondsRemaining = DeltaSeconds * TimeOutOfWater;
  const float TimeInWater = DeltaSeconds - DeltaSecondsRemaining;

  if (TimeOutOfWater > 0.f)
  {
    if (CheckLeaveFluid(CurrentFloor, EXIT_FLUID_IMMERSION_TOLERANCE, FluidMinExitSpeed, DeltaSecondsRemaining))
    {
      UpdateVelocityFromMovedDistance(TimeInWater, MaxSpeed);
      return false;
    }

    // Pawn has left the water volume.
    if (TimeInWater >= KINDA_SMALL_NUMBER)
    {
      // Don't slow down the Z-velocity so we can still jump out of the water.
      const float PreviousVelocityZ = GetVelocity().Z;
      UpdateVelocityFromMovedDistance(TimeInWater, MaxSpeed);
      UpdateVelocity({GetVelocity().X, GetVelocity().Y, PreviousVelocityZ}, TimeInWater);
    }
    ProcessLeftFluid(DeltaSecondsRemaining);
    PhysicsAirborne(DeltaSecondsRemaining);
    return true;
  }

  if (Hit.IsValidBlockingHit())
  {
    // Pawn did not leave the water volume, but hit a wall underwater.
    HandleImpact(Hit, DeltaSeconds, LocationDelta);
    AdjustVelocityFromHit(Hit, DeltaSeconds - DeltaSecondsRemaining);
    SlideAlongSurface(LocationDelta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    DeltaSecondsRemaining *= 1.f - Hit.Time;
    AdjustVelocityFromHit(Hit, DeltaSeconds - DeltaSecondsRemaining);
  }

  return false;
}

void UGenOrganicMovementComponent::ProcessLeftFluid(float DeltaSeconds)
{
  checkGMC(IsSwimming())

  SetMovementMode(EGenMovementMode::Airborne);
}

float UGenOrganicMovementComponent::Swim(const FVector& LocationDelta, FHitResult& OutHit, float DeltaSeconds)
{
  SCOPE_CYCLE_COUNTER(STAT_Swim)

  checkGMC(BuoyantStateMinImmersion >= KINDA_SMALL_NUMBER)
  const FVector StartLocation = UpdatedComponent->GetComponentLocation();
  float TimeOutOfWater = 0.f;
  SafeMoveUpdatedComponent(LocationDelta, UpdatedComponent->GetComponentQuat(), true, OutHit);
  UpdateImmersionDepth();
  if (CurrentImmersionDepth < BuoyantStateMinImmersion)
  {
    // Pawn has left the water volume.
    const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
    FVector WaterLine = FindWaterLine(StartLocation, CurrentLocation, CurrentImmersionDepth);

    // Adjust the water line according to the configured min immersion depth.
    if (!FMath::IsNearlyEqual(BuoyantStateMinImmersion, 0.5f, KINDA_SMALL_NUMBER))
    {
      const FVector TraveledInWater = WaterLine - StartLocation;
      WaterLine = StartLocation + (1.5f - BuoyantStateMinImmersion) * TraveledInWater;
    }

    const float DesiredDistance = LocationDelta.Size();
    if (!WaterLine.Equals(CurrentLocation) && DesiredDistance > KINDA_SMALL_NUMBER)
    {
      const float DistanceOutOfWater = (CurrentLocation - WaterLine).Size();
      // For the rare case that the pawn leaves the water volume from the side the water line may not have been calculated correctly.
      if (DesiredDistance > DistanceOutOfWater)
      {
        TimeOutOfWater = DistanceOutOfWater / DesiredDistance;
        // Move us back to the water line. We shouldn't hit anything on the way back because the was no collision before either.
        MoveUpdatedComponent(WaterLine - CurrentLocation, UpdatedComponent->GetComponentQuat(), false);
      }
    }
  }
  return TimeOutOfWater;
}

FVector UGenOrganicMovementComponent::FindWaterLine(
  const FVector& LocationInWater,
  const FVector& LocationOutOfWater,
  float ImmersionDepth
) const
{
  SCOPE_CYCLE_COUNTER(STAT_FindWaterLine)

  const auto World = GetWorld();
  if (!World) return FVector{0};

  FVector Result = LocationOutOfWater;

  // Do a line trace that goes into the body of water.
  TArray<FHitResult> Hits;
  World->LineTraceMultiByChannel(
    Hits,
    LocationOutOfWater,
    LocationInWater,
    UpdatedComponent->GetCollisionObjectType(),
    FCollisionQueryParams(SCENE_QUERY_STAT(__func__), true, GetOwner())
  );

  for (int32 Index = 0; Index < Hits.Num(); ++Index)
  {
    const FHitResult& Hit = Hits[Index];
    if (!PawnOwner->IsOwnedBy(Hit.GetActor()) && !Hit.GetComponent()->IsWorldGeometry())
    {
      APhysicsVolume* Volume = Cast<APhysicsVolume>(Hit.GetActor());
      if (Volume && Volume->bWaterVolume)
      {
        // This is the water volume the pawn came out of so the water line is at the impact location of the trace.
        Result = Hit.Location;

        // We want to remain in the same state that we are currently in so we adjust the result slightly in the appropriate direction.
        const FVector IntoWaterDirection = (LocationInWater - LocationOutOfWater).GetSafeNormal();
        if (ImmersionDepth >= BuoyantStateMinImmersion)
        {
          // We are currently in the buoyant movement state i.e. considered to be inside the water.
          Result += 0.1f * IntoWaterDirection;
        }
        else
        {
          // We are currently not considered to be inside the water.
          Result -= 0.1f * IntoWaterDirection;
        }
        break;
      }
    }
  }

  return Result;
}

void UGenOrganicMovementComponent::ApplyFallControl(float Control, const FVector& InputAcceleration, float DeltaSeconds)
{
  checkGMC(Control > KINDA_SMALL_NUMBER && IsAirborne() && IsAffectedByGravity())

  FVector Velocity2D = {GetVelocity().X, GetVelocity().Y, 0.f};
  const FVector InputAcceleration2D = {InputAcceleration.X, InputAcceleration.Y, 0.f};
  if (InputAcceleration2D.IsNearlyZero())
  {
    return;
  }

  FVector VelocityToAdd = InputAcceleration2D * Control * DeltaSeconds;

  const FVector ProposedVelocity = Velocity2D + VelocityToAdd;
  const float MaxSpeed = GetMaxSpeed();
  if (ProposedVelocity.Size() <= MaxSpeed)
  {
    // We won't exceed the max speed, add the full input velocity.
    AddVelocity(VelocityToAdd, DeltaSeconds);
    return;
  }

  // We are either already above max speed or will be after adding the full input velocity. In this case we only want to apply the direction
  // change from the user input but don't want to gain any additional speed after being at or above max speed.
  const float Speed2D = Velocity2D.Size();
  const FVector NewVelocity2D = ProposedVelocity.GetClampedToMaxSize(FMath::Max(Speed2D, MaxSpeed));

  // Set the new 2D velocity but leave the Z velocity unchanged.
  UpdateVelocity({NewVelocity2D.X, NewVelocity2D.Y, GetVelocity().Z}, DeltaSeconds);
}

bool UGenOrganicMovementComponent::IsExceedingFallOffThreshold(
  const FVector& ImpactPoint,
  const FVector& PawnLowerBound,
  const FVector& PawnCenter
) const
{
  checkGMC(LedgeFallOffThreshold >= 0.f)
  checkGMC(LedgeFallOffThreshold <= 1.f)

  // If the pawn cannot walk off ledges we always use the center of the collision as fall-off threshold, otherwise the pawn could partially
  // land on a ledge (from an airborne state) and still walk off afterwards.
  const float TestThreshold = bCanWalkOffLedges ? LedgeFallOffThreshold : 0.f;

  if (TestThreshold == 1.f)
  {
    // A threshold of 1 means we do not want to fall off at all.
    return false;
  }

  const FVector CenterToImpactXY = FVector(ImpactPoint.X, ImpactPoint.Y, 0.f) - FVector(PawnCenter.X, PawnCenter.Y, 0.f);
  const FVector ImpactDirectionXY = CenterToImpactXY.GetSafeNormal();
  if (ImpactDirectionXY.IsNearlyZero())
  {
    // The impact point is at the center.
    return false;
  }

  const float DistanceToImpactXY = CenterToImpactXY.Size();
  const float DistanceToBoundaryXY = ComputeDistanceToRootCollisionBoundaryXY(ImpactDirectionXY);
  checkGMC(DistanceToBoundaryXY > 0.f)
  // @attention The relatively large tolerance of one millimeter is imperative here.
  const bool bImpactWithinZConstraints =
    ImpactPoint.Z >= (PawnLowerBound.Z - MAX_DISTANCE_TO_FLOOR - UU_MILLIMETER) && ImpactPoint.Z < (PawnCenter.Z + UU_MILLIMETER);
  const bool bImpactWithinXYConstraints = (DistanceToBoundaryXY + UU_MILLIMETER) > DistanceToImpactXY;
  if (!bImpactWithinZConstraints || !bImpactWithinXYConstraints)
  {
    // The impact point is not on the lower side of the collision shape.
    return false;
  }

  if (GetRootCollisionShape() == EGenCollisionShape::Box)
  {
    // Due to the way Unreal handles box collisions we assume the threshold to be either just 1 (if it is actually >= 0.5) or 0 (if it is
    // actually < 0.5). Any values in between are not reliable because the impact point is not guaranteed to be exactly on the edge of the
    // ledge.
    // @attention Do not check this any earlier than here, the constraints for the impact point must be met.
    return TestThreshold < 0.5f;
  }

  return (DistanceToImpactXY / DistanceToBoundaryXY) > TestThreshold;
}

FHitResult UGenOrganicMovementComponent::AutoResolvePenetration()
{
  checkGMC(!bStuckInGeometry)

  DEBUG_LOG_AUTO_RESOLVE_PENETRATION_START
  FHitResult Hit = Super::AutoResolvePenetration();

  if (Hit.bStartPenetrating)
  {
    const bool bHitPawn = static_cast<bool>(Cast<APawn>(Hit.GetActor()));
    if (const FVector AdjustmentDirection = GetPenetrationAdjustment(Hit).GetSafeNormal(); AdjustmentDirection != FVector::ZeroVector)
    {
      const FVector InitialLocation = UpdatedComponent->GetComponentLocation();
      const float MaxDepenetration = bHitPawn ? MaxDepenetrationWithPawn : MaxDepenetrationWithGeometry;
      // Subdivide the max depenetration distance into smaller substeps and incrementally adjust the pawn position.
      // @attention This can lead to an overall greater adjustment than the set max depenetration distance.
      constexpr int32 Resolution = 10;
      const float AdjustmentStepDistance = MaxDepenetration / Resolution;
      for (int NumRetry = 1; NumRetry <= Resolution; ++NumRetry)
      {
        UpdatedComponent->SetWorldLocation(InitialLocation + AdjustmentDirection * NumRetry * AdjustmentStepDistance);
        if (Super::AutoResolvePenetration().bStartPenetrating)
        {
          // If we are still stuck after adjusting undo the location change.
          UpdatedComponent->SetWorldLocation(InitialLocation);
        }
        else
        {
          // The penetration was resolved. We still return the original downward hit.
          DEBUG_LOG_AUTO_RESOLVE_PENETRATION_END
          return Hit;
        }
      }
    }
    FLog(Warning, "Pawn stuck in %s.", bHitPawn ? TEXT("other pawn") : TEXT("geometry"))
    bStuckInGeometry = true;
    OnStuckInGeometry();
  }

  DEBUG_LOG_AUTO_RESOLVE_PENETRATION_END
  return Hit;
}

float UGenOrganicMovementComponent::SlideAlongSurface(
  const FVector& Delta,
  float Time,
  const FVector& Normal,
  FHitResult& Hit,
  bool bHandleImpact
)
{
  SCOPE_CYCLE_COUNTER(STAT_SlideAlongSurface)

  if (!Hit.bBlockingHit)
  {
    return 0.f;
  }
  FVector NewNormal = Normal;
  if (IsMovingOnGround())
  {
    if (Normal.Z > 0.f)
    {
      // Do not push the pawn up an unwalkable surface.
      if (!HitWalkableFloor(Hit))
      {
        NewNormal = NewNormal.GetSafeNormal2D();
      }
    }
    else if (Normal.Z < -KINDA_SMALL_NUMBER)
    {
      if (CurrentFloor.HasValidShapeData() && CurrentFloor.GetShapeDistanceToFloor() < MIN_DISTANCE_TO_FLOOR)
      {
        // Do not push the pawn down further into the floor when the impact point is on the top part of the collision shape.
        const FVector FloorNormal = CurrentFloor.ShapeHit().Normal;
        const bool bFloorOpposedToMovement = (Delta | FloorNormal) < 0.f && (FloorNormal.Z < 1.f - DELTA);
        if (bFloorOpposedToMovement)
        {
          NewNormal = FloorNormal;
        }
        NewNormal = NewNormal.GetSafeNormal2D();
      }
    }
  }
  return Super::SlideAlongSurface(Delta, Time, NewNormal, Hit, bHandleImpact);
}

void UGenOrganicMovementComponent::TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const
{
  SCOPE_CYCLE_COUNTER(STAT_TwoWallAdjust)

  const FVector InDelta = Delta;
  Super::TwoWallAdjust(Delta, Hit, OldHitNormal);

  if (IsMovingOnGround())
  {
    // Only allow the pawn to slide up walkable surfaces. Unwalkable surfaces are treated as vertical walls.
    if (Delta.Z > 0.f)
    {
      if ((Hit.Normal.Z >= WalkableFloorZ || HitWalkableFloor(Hit)) && Hit.Normal.Z > KINDA_SMALL_NUMBER)
      {
        // Maintain horizontal movement.
        const float Time = (1.f - Hit.Time);
        const FVector ScaledDelta = Delta.GetSafeNormal() * InDelta.Size();
        Delta = FVector(InDelta.X, InDelta.Y, ScaledDelta.Z / Hit.Normal.Z) * Time;
        // The delta Z should never exceed the max allowed step up height so we rescale if necessary. This should be rare (the hit normal Z
        // divisor would have to be very small) but we'd rather lose horizontal movement than go too high.
        if (Delta.Z > MaxStepUpHeight)
        {
          const float Rescale = MaxStepUpHeight / Delta.Z;
          Delta *= Rescale;
        }
      }
      else
      {
        Delta.Z = 0.f;
      }
    }
    else if (Delta.Z < 0.f)
    {
      if (CurrentFloor.HasValidShapeData() && CurrentFloor.GetShapeDistanceToFloor() < MIN_DISTANCE_TO_FLOOR)
      {
        // Do not push the pawn further down into the floor.
        Delta.Z = 0.f;
      }
    }
  }
}

FVector UGenOrganicMovementComponent::ComputeSlideVector(
  const FVector& Delta,
  float Time,
  const FVector& Normal,
  const FHitResult& Hit
) const
{
  FVector Result = Super::ComputeSlideVector(Delta, Time, Normal, Hit);
  if (IsAirborne() && IsAffectedByGravity())
  {
    // Prevent boosting up slopes.
    Result = HandleSlopeBoosting(Result, Delta, Time, Normal, Hit);
  }
  return Result;
}

FVector UGenOrganicMovementComponent::HandleSlopeBoosting(
  const FVector& SlideResult,
  const FVector& Delta,
  float Time,
  const FVector& Normal,
  const FHitResult& Hit
) const
{
  FVector Result = SlideResult;
  if (Result.Z > 0.f)
  {
    // Don't move any higher than we originally intended.
    const float ZLimit = Delta.Z * Time;
    if (Result.Z - ZLimit > KINDA_SMALL_NUMBER)
    {
      if (ZLimit > 0.f)
      {
        // Rescale the entire vector (not just the Z-component), otherwise we would change the direction and most likely head right back
        // into the impact.
        const float UpPercent = ZLimit / Result.Z;
        Result *= UpPercent;
      }
      else
      {
        // We were heading downwards but would have deflected upwards, make the deflection horizontal instead.
        Result = FVector::ZeroVector;
      }
      // Adjust the remaining portion of the original slide result to be horizontal and parallel to impact normal.
      const FVector RemainderXY = (SlideResult - Result) * FVector(1.f, 1.f, 0.f);
      const FVector NormalXY = Normal.GetSafeNormal2D();
      const FVector Adjust = Super::ComputeSlideVector(RemainderXY, 1.f, NormalXY, Hit);
      Result += Adjust;
    }
  }
  return Result;
}

void UGenOrganicMovementComponent::SetMovementMode(EMovementMode NewMovementMode)
{
  switch (NewMovementMode)
  {
    case MOVE_None:
      SetMovementMode(EGenMovementMode::None);
      return;
    case MOVE_Walking:
    case MOVE_NavWalking:
      SetMovementMode(EGenMovementMode::Grounded);
      return;
    case MOVE_Swimming:
      SetMovementMode(EGenMovementMode::Buoyant);
      return;
    case MOVE_Flying:
    case MOVE_Falling:
      SetMovementMode(EGenMovementMode::Airborne);
      return;
    case MOVE_Custom:
    case MOVE_MAX:
    default: return;
  }
}

void UGenOrganicMovementComponent::SetMovementMode(uint8 NewMovementMode)
{
  checkGMC(NewMovementMode >= 0 && NewMovementMode <= 255)
  if (NewMovementMode != MovementMode)
  {
    uint8 PreviousMovementMode = MovementMode;
    MovementMode = NewMovementMode;
    OnMovementModeChanged(static_cast<EGenMovementMode>(PreviousMovementMode));
  }
}

void UGenOrganicMovementComponent::SetMovementMode(EGenMovementMode NewMovementMode)
{
  SetMovementMode(static_cast<uint8>(NewMovementMode));
}

UPrimitiveComponent* UGenOrganicMovementComponent::GetMovementBase() const
{
  return IsMovingOnGround() && CurrentFloor.HasValidShapeData() ? CurrentFloor.ShapeHit().GetComponent() : nullptr;
}

AActor* UGenOrganicMovementComponent::GetMovementBaseActor() const
{
  return IsMovingOnGround() && CurrentFloor.HasValidShapeData() ? CurrentFloor.ShapeHit().GetActor() : nullptr;
}

float UGenOrganicMovementComponent::GetMaxSpeed() const
{
  checkGMC(MaxDesiredSpeed >= 0.f)

  if (IsServerBot())
  {
    // Get max speed for path following.
    if (bRequestedMoveWithMaxSpeed || UseAccelerationForPathFollowing())
    {
      return MaxDesiredSpeed;
    }
    return bUseRequestedVelocityMaxSpeed ? RequestedVelocity.Size() : MaxDesiredSpeed;
  }

  if (GetProcessedInputVector().IsZero() || bIgnoreInputModifier)
  {
    // No modifier is being applied.
    return MaxDesiredSpeed;
  }

  // Calculate the modified max speed.
  checkGMC(MaxDesiredSpeed >= MinAnalogWalkSpeed)
  return FMath::Clamp(ComputeInputModifier() * MaxDesiredSpeed, MinAnalogWalkSpeed, BIG_NUMBER);
}

bool UGenOrganicMovementComponent::HitWalkableFloor(const FHitResult& Hit) const
{
  if (!Hit.IsValidBlockingHit())
  {
    return false;
  }
  if (!CanStepUp(Hit))
  {
    return false;
  }
  // Check if the impact normal Z exceeds the walkable Z value.
  float WalkableZ = WalkableFloorZ;
  const UPrimitiveComponent* HitComponent = Hit.GetComponent();
  if (HitComponent)
  {
    // Hit component may override the walkable floor Z.
    const FWalkableSlopeOverride& SlopeOverride = HitComponent->GetWalkableSlopeOverride();
    WalkableZ = SlopeOverride.ModifyWalkableFloorZ(WalkableZ);
  }
  if (Hit.ImpactNormal.Z + KINDA_SMALL_NUMBER < WalkableZ)
  {
    if (bLandOnEdges && IsAirborne())
    {
      // For rounded collision shapes more impacts that are on hard edges will be recognized as walkable with this check.
      if (Hit.Normal.Z + KINDA_SMALL_NUMBER < WalkableZ)
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  return true;
}

void UGenOrganicMovementComponent::HandleImpact(const FHitResult& Impact, float TimeSlice, const FVector& MoveDelta)
{
  SCOPE_CYCLE_COUNTER(STAT_HandleImpact)

  if (!Impact.bBlockingHit)
  {
    checkfGMC(false, TEXT("HandleImpact should only be called for blocking hits."))
    return;
  }

  if (const auto PFAgent = GetPathFollowingAgent())
  {
    PFAgent->OnMoveBlockedBy(Impact);
  }

  if (const auto OtherPawn = Cast<APawn>(Impact.GetActor()))
  {
    NotifyBumpedPawn(OtherPawn);
  }

  if (bEnablePhysicsInteraction)
  {
    ApplyImpactPhysicsForces(Impact, GetTransientAcceleration(), GetVelocity());
  }
}

void UGenOrganicMovementComponent::SetWalkableFloorAngle(float NewWalkableFloorAngle)
{
  WalkableFloorAngle = FMath::Clamp(NewWalkableFloorAngle, 0.f, 90.0f);
  WalkableFloorZ = FMath::Cos(FMath::DegreesToRadians(WalkableFloorAngle));
}

void UGenOrganicMovementComponent::SetWalkableFloorZ(float NewWalkableFloorZ)
{
  WalkableFloorZ = FMath::Clamp(NewWalkableFloorZ, 0.f, 1.f);
  WalkableFloorAngle = FMath::RadiansToDegrees(FMath::Acos(WalkableFloorZ));
}

bool UGenOrganicMovementComponent::IsExceedingMaxSpeed(float MaxSpeed) const
{
  return Velocity.SizeSquared() > FMath::Square(FMath::Max(MaxSpeed, 0.f));
}

bool UGenOrganicMovementComponent::CanMove() const
{
  return Super::CanMove() && MovementMode != static_cast<uint8>(EGenMovementMode::None) && !bStuckInGeometry;
}

bool UGenOrganicMovementComponent::HasMoveInputEnabled() const
{
  return Super::HasMoveInputEnabled() && MovementMode != static_cast<uint8>(EGenMovementMode::None);
}

void UGenOrganicMovementComponent::OnMovementModeChanged_Implementation(EGenMovementMode PreviousMovementMode)
{
  SCOPE_CYCLE_COUNTER(STAT_OnMovementModeChanged)

  FLog(
    VeryVerbose,
    "Movement mode changed from %s to %s.",
    *DebugGetMovementModeAsString(static_cast<uint8>(PreviousMovementMode)),
    *DebugGetMovementModeAsString(MovementMode)
  )

  if (IsMovingOnGround() && PreviousMovementMode == EGenMovementMode::Airborne)
  {
    OnLanded();
    if (const auto PFAgent = GetPathFollowingAgent()) PFAgent->OnLanded();
  }
  else if (IsAirborne() && PreviousMovementMode == EGenMovementMode::Grounded)
  {
    // @attention Do not use @see GetMovementBase here since we are already in the air and do not have a movement base anymore.
    if (CurrentFloor.HasValidShapeData())
    {
      const auto PreviousMovementBase = CurrentFloor.ShapeHit().GetComponent();
      checkGMC(PreviousMovementBase)
      if (ShouldImpartVelocityFromBase(PreviousMovementBase))
      {
        FVector VelocityToImpart{0};
        if (const auto PawnMovementBase = Cast<APawn>(PreviousMovementBase->GetOwner()))
        {
          if (bImpartLinearBaseVelocity)
          {
            VelocityToImpart += GetPawnBaseVelocity(PawnMovementBase);
          }
        }
        else
        {
          if (bImpartLinearBaseVelocity)
          {
            VelocityToImpart += GetLinearVelocity(PreviousMovementBase);
          }
          if (bImpartAngularBaseVelocity)
          {
            VelocityToImpart += ComputeTangentialVelocity(GetLowerBound(), PreviousMovementBase);
          }
        }
        AddImpulse(PostProcessVelocityToImpart(PreviousMovementBase, VelocityToImpart), !bConsiderMassOnImpartVelocity);
      }
    }
  }
}

bool UGenOrganicMovementComponent::ShouldImpartVelocityFromBase(UPrimitiveComponent* MovementBase) const
{
  checkGMC(MovementBase)
  if (!IsMovable(MovementBase))
  {
    return false;
  }
  if (!bMoveWithBase)
  {
    return false;
  }
  if (MovementBase->IsSimulatingPhysics())
  {
    // Do not impart the velocity of the movement base if it is simulating physics (can cause the pawn to receive a very large/unrealistic
    // impulse, especially from small objects).
    return false;
  }
  return true;
}

FVector UGenOrganicMovementComponent::GetPenetrationAdjustment(const FHitResult& Hit) const
{
  FVector Result = Super::GetPenetrationAdjustment(Hit);
  float MaxDistance = MaxDepenetrationWithGeometry;
  if (Cast<APawn>(Hit.GetActor())) MaxDistance = MaxDepenetrationWithPawn;
  Result = Result.GetClampedToMaxSize(MaxDistance);
  return Result;
}

float UGenOrganicMovementComponent::GetGravityZ() const
{
  checkGMC(GravityScale >= 0.f)
  const float GravityZ = FMath::Clamp(UMovementComponent::GetGravityZ(), -BIG_NUMBER, 0.f) * GravityScale;
  return GravityZ;
}

FVector UGenOrganicMovementComponent::GetGravity() const
{
  return {0.f, 0.f, GetGravityZ()};
}

void UGenOrganicMovementComponent::HaltMovement()
{
  Super::HaltMovement();
  bReceivedUpwardForce = false;
  bHasAnimRootMotion = false;
  RequestedVelocity = FVector::ZeroVector;
  bRequestedMoveWithMaxSpeed = false;
  bIsUsingAvoidanceInternal = false;
  ResetAvoidanceData();
}

void UGenOrganicMovementComponent::DisableMovement()
{
  HaltMovement();
  SetMovementMode(EGenMovementMode::None);
}

void UGenOrganicMovementComponent::RootCollisionTouched(
  UPrimitiveComponent* OverlappedComponent,
  AActor* OtherActor,
  UPrimitiveComponent* OtherComponent,
  int32 OtherBodyIndex,
  bool bFromSweep,
  const FHitResult& SweepResult
)
{
  checkGMC(bEnablePhysicsInteraction)

  if (!OtherComponent || !OtherComponent->IsAnySimulatingPhysics())
  {
    return;
  }

  FName BoneName = NAME_None;
  if (OtherBodyIndex != INDEX_NONE)
  {
    const auto OtherSkinnedMeshComponent = Cast<USkinnedMeshComponent>(OtherComponent);
    check(OtherSkinnedMeshComponent)
    BoneName = OtherSkinnedMeshComponent->GetBoneName(OtherBodyIndex);
  }

  float TouchForceFactorModified = TouchForceScale;
  if (bScaleTouchForceToMass)
  {
    const auto BodyInstance = OtherComponent->GetBodyInstance(BoneName);
    TouchForceFactorModified *= BodyInstance ? BodyInstance->GetBodyMass() : 1.f;
  }

  float ImpulseStrength = FMath::Clamp(
    GetVelocity().Size2D() * TouchForceFactorModified,
    MinTouchForce > 0.f ? MinTouchForce : -FLT_MAX,
    MaxTouchForce > 0.f ? MaxTouchForce : FLT_MAX
  );

  const FVector OtherComponentLocation = OtherComponent->GetComponentLocation();
  const FVector ShapeLocation = UpdatedComponent->GetComponentLocation();
  FVector ImpulseDirection =
    FVector(OtherComponentLocation.X - ShapeLocation.X, OtherComponentLocation.Y - ShapeLocation.Y, 0.25f).GetSafeNormal();
  ImpulseDirection = (ImpulseDirection + GetVelocity().GetSafeNormal2D()) * 0.5f;
  ImpulseDirection.Normalize();

  OtherComponent->AddImpulse(ImpulseDirection * ImpulseStrength, BoneName);
}

void UGenOrganicMovementComponent::ApplyImpactPhysicsForces(
  const FHitResult& Impact,
  const FVector& ImpactAcceleration,
  const FVector& ImpactVelocity
)
{
  checkGMC(bEnablePhysicsInteraction)

  if (!Impact.IsValidBlockingHit())
  {
    return;
  }

  const auto ImpactComponent = Impact.GetComponent();
  if (!ImpactComponent)
  {
    return;
  }

  const auto BodyInstance = ImpactComponent->GetBodyInstance(Impact.BoneName);
  if (!BodyInstance || !BodyInstance->IsInstanceSimulatingPhysics())
  {
    return;
  }

  FVector ForceLocation = Impact.ImpactPoint;
  if (bUsePushForceZOffset)
  {
    FVector Center, Extents;
    BodyInstance->GetBodyBounds().GetCenterAndExtents(Center, Extents);
    if (!Extents.IsNearlyZero())
    {
      ForceLocation.Z = Center.Z + Extents.Z * PushForceZOffsetFactor;
    }
  }

  float PushForceModifier = 1.f;
  const FVector ComponentVelocity = ImpactComponent->GetPhysicsLinearVelocity();
  const FVector VirtualVelocity = ImpactAcceleration.IsZero() ? ImpactVelocity : ImpactAcceleration.GetSafeNormal() * GetMaxSpeed();

  if (bScalePushForceToVelocity && !ComponentVelocity.IsNearlyZero())
  {
    const float DotProduct = ComponentVelocity | VirtualVelocity;
    if (DotProduct > 0.f && DotProduct < 1.f)
    {
      PushForceModifier *= DotProduct;
    }
  }

  const float BodyMass = FMath::Max(BodyInstance->GetBodyMass(), 1.f);
  if (bScalePushForceToMass)
  {
    PushForceModifier *= BodyMass;
  }

  FVector ForceToApply = Impact.ImpactNormal * -1.0f;
  ForceToApply *= PushForceModifier;
  if (ComponentVelocity.IsNearlyZero(1.f))
  {
    ForceToApply *= InitialPushForceScale;
    ImpactComponent->AddImpulseAtLocation(ForceToApply, ForceLocation, Impact.BoneName);
  }
  else
  {
    ForceToApply *= PushForceScale;
    ImpactComponent->AddForceAtLocation(ForceToApply, ForceLocation, Impact.BoneName);
  }
}

void UGenOrganicMovementComponent::ApplyDownwardForce(float DeltaSeconds)
{
  checkGMC(bEnablePhysicsInteraction)

  if (!CurrentFloor.HasValidShapeData() || DownwardForceScale == 0.f)
  {
    return;
  }

  if (const auto MovementBase = CurrentFloor.ShapeHit().GetComponent())
  {
    const FVector Gravity = GetGravity();
    if (!Gravity.IsZero() && MovementBase->IsAnySimulatingPhysics())
    {
      MovementBase->AddForceAtLocation(
        Gravity * Mass * DownwardForceScale,
        CurrentFloor.ShapeHit().ImpactPoint,
        CurrentFloor.ShapeHit().BoneName
      );
    }
  }
}

void UGenOrganicMovementComponent::ApplyRepulsionForce(float DeltaSeconds)
{
  checkGMC(bEnablePhysicsInteraction)

  if (!PawnOwner || !UpdatedPrimitive || RepulsionForce <= 0.f)
  {
    return;
  }

  const TArray<FOverlapInfo>& Overlaps = UpdatedPrimitive->GetOverlapInfos();
  if (Overlaps.Num() == 0)
  {
    return;
  }

  for (int32 Index = 0; Index < Overlaps.Num(); ++Index)
  {
    const FOverlapInfo& Overlap = Overlaps[Index];

    const auto OverlapComponent = Overlap.OverlapInfo.GetComponent();
    if (!OverlapComponent || OverlapComponent->Mobility < EComponentMobility::Movable)
    {
      continue;
    }

    // Use the body instead of the component for cases where multi-body overlaps are enabled.
    FBodyInstance* OverlapBody = nullptr;
    const int32 OverlapBodyIndex = Overlap.GetBodyIndex();
    const auto SkeletalMeshForBody = OverlapBodyIndex != INDEX_NONE ? Cast<USkeletalMeshComponent>(OverlapComponent) : nullptr;
    if (SkeletalMeshForBody)
    {
      OverlapBody = SkeletalMeshForBody->Bodies.IsValidIndex(OverlapBodyIndex) ? SkeletalMeshForBody->Bodies[OverlapBodyIndex] : nullptr;
    }
    else
    {
      OverlapBody = OverlapComponent->GetBodyInstance();
    }

    if (!OverlapBody)
    {
      FLog(Verbose, "%s could not find overlap-body for body index %d.", *GetName(), OverlapBodyIndex)
      continue;
    }

    if (!OverlapBody->IsInstanceSimulatingPhysics())
    {
      continue;
    }

    // Trace to get the hit location on the collision.
    FHitResult Hit;
    FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(ApplyRepulsionForce));
    CollisionQueryParams.bReturnFaceIndex = false;
    CollisionQueryParams.bReturnPhysicalMaterial = false;
    const FVector ShapeLocation = UpdatedPrimitive->GetComponentLocation();
    const FVector BodyLocation = OverlapBody->GetUnrealWorldTransform().GetLocation();
    bool bHasHit = UpdatedPrimitive->LineTraceComponent(
      Hit,
      BodyLocation,
      {ShapeLocation.X, ShapeLocation.Y, BodyLocation.Z},
      CollisionQueryParams
    );

    constexpr float StopBodyDistance = 2.5f;
    bool bIsPenetrating = Hit.bStartPenetrating || Hit.PenetrationDepth > StopBodyDistance;
    FVector HitLocation = Hit.ImpactPoint;

    // If we did not hit the collision, we are inside the shape.
    if (!bHasHit)
    {
      HitLocation = BodyLocation;
      bIsPenetrating = true;
    }

    const FVector BodyVelocity = OverlapBody->GetUnrealWorldVelocity();
    const float DistanceNow = (HitLocation - BodyLocation).SizeSquared2D();
    const float DistanceLater = (HitLocation - (BodyLocation + BodyVelocity * DeltaSeconds)).SizeSquared2D();

    if (bHasHit && DistanceNow < StopBodyDistance && !bIsPenetrating)
    {
      OverlapBody->SetLinearVelocity(FVector::ZeroVector, false);
    }
    else if (DistanceLater <= DistanceNow || bIsPenetrating)
    {
      FVector ForceCenter = ShapeLocation;

      if (bHasHit)
      {
        ForceCenter.Z = HitLocation.Z;
      }
      else
      {
        float ShapeHalfHeight = GetRootCollisionHalfHeight();
        ForceCenter.Z = FMath::Clamp(BodyLocation.Z, ShapeLocation.Z - ShapeHalfHeight, ShapeLocation.Z + ShapeHalfHeight);
      }

      constexpr float ToleranceFactor = 1.2f;
      const float RepulsionForceRadius = ComputeDistanceToRootCollisionBoundaryXY(BodyLocation - ShapeLocation) * ToleranceFactor;
      OverlapBody->AddRadialForceToBody(ForceCenter, RepulsionForceRadius, RepulsionForce * Mass, ERadialImpulseFalloff::RIF_Constant);
    }
  }
}

void UGenOrganicMovementComponent::SetSkeletalMeshReference(USkeletalMeshComponent* Mesh)
{
  if (SkeletalMesh && SkeletalMesh != Mesh)
  {
    SkeletalMesh->RemoveTickPrerequisiteComponent(this);
  }

  SkeletalMesh = Mesh;

  if (SkeletalMesh)
  {
    SkeletalMesh->AddTickPrerequisiteComponent(this);
  }
}

void UGenOrganicMovementComponent::BlockSkeletalMeshPoseTick() const
{
  if (!SkeletalMesh) return;
  SkeletalMesh->bIsAutonomousTickPose = false;
  SkeletalMesh->bOnlyAllowAutonomousTickPose = true;
}

bool UGenOrganicMovementComponent::ShouldTickPose(bool* bOutSimulate) const
{
  // @attention The @see bHasAnimRootMotion flag should be false at this time regardless of whether we are currently playing root motion or
  // not. If this function returns true the flag will be set in @see TickPose if required.
  checkGMC(!HasAnimRootMotion())

  if (bOutSimulate)
  {
    *bOutSimulate = false;
  }

  if (!SkeletalMesh)
  {
    return false;
  }

  // Enforce that these properties are not set.
  SkeletalMesh->bOnlyAllowAutonomousTickPose = false;
  SkeletalMesh->bIsAutonomousTickPose = false;

  const bool bIsPlayingMontage = IsPlayingMontage(SkeletalMesh);
  if (!bIsPlayingMontage)
  {
    // The manual pose tick is only relevant when playing montages.
    return false;
  }

  // Depending on the setting of @see StepMontagePolicy we want to use the manual or automatic mesh pose tick.
  // @attention Implementing all animation work through frame independent montages (with StepMontagePolicy == EStepMontagePolicy::All)
  // yields the most accurate network synchronisation possible regarding the mesh pose. Driving animations from an animation Blueprint
  // usually works fine as well and is recommended for most games (if the collision of the skeletal mesh does not affect gameplay). However,
  // some subtle issues may arise in this case on smoothed listen servers and clients (e.g. when executing combined moves or replays) due to
  // the fact that the animated state of the pawn is usually not tied to the moves being executed when using an animation Blueprint.
  if (StepMontagePolicy == EStepMontagePolicy::None)
  {
    if (!IsNetMode(NM_Standalone))
    {
      // Montage stepping policy "None" should only be used for standalone games.
      return false;
    }
    const bool bIsPlayingRootMotionMontage = static_cast<bool>(GetRootMotionMontageInstance(SkeletalMesh));
    if (!bIsPlayingRootMotionMontage)
    {
      // In standalone the manual pose tick is only used for applying root motion from animations, but there's no root motion montage
      // currently playing.
      return false;
    }
  }
  else if (StepMontagePolicy == EStepMontagePolicy::RootMotionOnly)
  {
    const bool bIsPlayingRootMotionMontage = static_cast<bool>(GetRootMotionMontageInstance(SkeletalMesh));
    if (!bIsPlayingRootMotionMontage)
    {
      // We are stepping only root motion montages but none is currently playing.
      return false;
    }
  }
  checkGMC(
    bIsPlayingMontage
    && (
         StepMontagePolicy == EStepMontagePolicy::All
      || StepMontagePolicy == EStepMontagePolicy::RootMotionOnly && GetRootMotionMontageInstance(SkeletalMesh)
      || StepMontagePolicy == EStepMontagePolicy::None && IsNetMode(NM_Standalone) && GetRootMotionMontageInstance(SkeletalMesh)
    )
  )

  if (const bool bSimulate = IsReplaying() || IsExecutingRemoteMoves() || IsSubSteppedIteration())
  {
    // Marking this as autonomous pose tick allows us to tick multiple times per frame. Montage advancement will only be simulated in this
    // case meaning @see USkeletalMeshComponent::LastPoseTickFrame will not be updated and another (regular) pose tick can still happen
    // afterwards.
    SkeletalMesh->bIsAutonomousTickPose = true;
    const bool bShouldTickPose = SkeletalMesh->ShouldTickPose();
    SkeletalMesh->bIsAutonomousTickPose = false;

    if (bOutSimulate && bShouldTickPose)
    {
      *bOutSimulate = true;
    }

    return bShouldTickPose;
  }

  return SkeletalMesh->ShouldTickPose();
}

void UGenOrganicMovementComponent::TickPose(float DeltaSeconds, bool bSimulate)
{
  SCOPE_CYCLE_COUNTER(STAT_TickPose)

  check(SkeletalMesh)
  checkGMC(!SkeletalMesh->bOnlyAllowAutonomousTickPose)
  checkGMC(!SkeletalMesh->bIsAutonomousTickPose)

  // During the pose tick some of the montage instance data (which we still need afterwards) can get cleared.
  UAnimMontage* RootMotionMontage{nullptr};
  float RootMotionMontagePosition{-1.f};
  if (const auto RootMotionMontageInstance = GetRootMotionMontageInstance(SkeletalMesh))
  {
    RootMotionMontage = RootMotionMontageInstance->Montage;
    RootMotionMontagePosition = RootMotionMontageInstance->GetPosition();
  }

  if (bSimulate)
  {
    if (const auto AnimInstance = SkeletalMesh->GetAnimInstance())
    {
      for (auto MontageInstance : AnimInstance->MontageInstances)
      {
        float MontagePosition = MontageInstance->GetPosition();
        MontageInstance->UpdateWeight(DeltaSeconds);
        MontageInstance->SimulateAdvance(DeltaSeconds, MontagePosition, RootMotionParams/*only modified if the montage has root motion*/);
        MontageInstance->SetPosition(MontagePosition);
        auto& MontageBlend = MontageInstance->*get(FAnimMontageInstance_Blend());
        if (MontageInstance->IsStopped() && MontageBlend.IsComplete())
        {
          // During the pose tick most data will be cleared on the montage instance when it terminates. However, when we use
          // @see FAnimMontageInstance::SimulateAdvance no instance data is touched so we need to reproduce this behaviour manually to get
          // consistent results across all execution modes.
          AnimInstance->ClearMontageInstanceReferences(*MontageInstance);
          MontageBlend.SetCustomCurve(NULL);
          MontageBlend.SetBlendOption(EAlphaBlendOption::Linear);
          MontageInstance->Montage = nullptr;
        }
      }
    }
  }
  else
  {
    if (IsAutonomousProxy())
    {
      // We do not want to combine moves if a montage is currently blending in or out.
      // @attention This must be called before the pose tick.
      Client_CheckMontageBlend();
    }

    const bool bWasPlayingRootMotion = IsPlayingRootMotion(SkeletalMesh);
    SkeletalMesh->TickPose(DeltaSeconds, true);
    const bool bIsPlayingRootMotion = IsPlayingRootMotion(SkeletalMesh);
    if (bWasPlayingRootMotion || bIsPlayingRootMotion)
    {
      RootMotionParams = SkeletalMesh->ConsumeRootMotion();
    }
  }

  checkGMC(!bHasAnimRootMotion)
  if (RootMotionParams.bHasRootMotion)
  {
    if (ShouldDiscardRootMotion(RootMotionMontage, RootMotionMontagePosition))
    {
      RootMotionParams = FRootMotionMovementParams();
      return;
    }
    bHasAnimRootMotion = true;

    // The root motion translation can be scaled by the user.
    RootMotionParams.ScaleRootMotionTranslation(GetAnimRootMotionTranslationScale());
    // Save the root motion transform in world space.
    RootMotionParams.Set(SkeletalMesh->ConvertLocalRootMotionToWorld(RootMotionParams.GetRootMotionTransform()));
    // Calculate the root motion velocity from the world space root motion translation.
    CalculateAnimRootMotionVelocity(DeltaSeconds);
    // Apply the root motion rotation now. Translation is applied with the next update from the calculated velocity. Splitting root motion
    // translation and rotation up like this may not be optimal but the alternative is to replicate the rotation for replay which is far
    // more undesirable.
    ApplyAnimRootMotionRotation(DeltaSeconds);
  }
}

void UGenOrganicMovementComponent::Client_CheckMontageBlend()
{
  checkGMC(IsAutonomousProxy())

  const auto AnimInstance = SkeletalMesh->GetAnimInstance();
  if (!AnimInstance)
  {
    return;
  }

  for (auto MontageInstance : AnimInstance->MontageInstances)
  {
    auto Montage = MontageInstance->Montage;
    if (!Montage)
    {
      continue;
    }

    const float MontageLength = Montage->GetPlayLength();
    const float CurrentPosition = MontageInstance->GetPosition();

    const bool bIsBlendingIn = CurrentPosition <= Montage->BlendIn.GetBlendTime();
    if (bIsBlendingIn)
    {
      DoNotCombineNextMove();
      return;
    }

    const bool bIsBlendingOut = CurrentPosition >= MontageLength - Montage->BlendOut.GetBlendTime();
    if (bIsBlendingOut)
    {
      DoNotCombineNextMove();
      return;
    }
  }
}

bool UGenOrganicMovementComponent::ShouldDiscardRootMotion(UAnimMontage* RootMotionMontage, float RootMotionMontagePosition) const
{
  if (!RootMotionMontage || RootMotionMontagePosition < 0.f)
  {
    return true;
  }

  const float MontageLength = RootMotionMontage->GetPlayLength();
  if (RootMotionMontagePosition >= MontageLength)
  {
    return true;
  }

  if (!bApplyRootMotionDuringBlendIn)
  {
    if (RootMotionMontagePosition <= RootMotionMontage->BlendIn.GetBlendTime())
    {
      return true;
    }
  }

  if (!bApplyRootMotionDuringBlendOut)
  {
    if (RootMotionMontagePosition >= MontageLength - RootMotionMontage->BlendOut.GetBlendTime())
    {
      return true;
    }
  }

  return false;
}

void UGenOrganicMovementComponent::ResetMontages(USkeletalMeshComponent* Mesh, EStepMontagePolicy Policy) const
{
  if (Policy == EStepMontagePolicy::None || !Mesh)
  {
    return;
  }

  if (Policy == EStepMontagePolicy::All)
  {
    ResetAllMontages(Mesh);
    return;
  }

  checkGMC(Policy == EStepMontagePolicy::RootMotionOnly)
  {
    ResetRootMotionMontage(Mesh);
    return;
  }
}

void UGenOrganicMovementComponent::CalculateAnimRootMotionVelocity(float DeltaSeconds)
{
  checkGMC(HasAnimRootMotion())
  checkGMC(SkeletalMesh)

  FVector RootMotionDelta = RootMotionParams.GetRootMotionTransform().GetTranslation();

  // Ignore components with very small delta values.
  RootMotionDelta.X = FMath::IsNearlyEqual(RootMotionDelta.X, 0.f, 0.01f) ? 0.f : RootMotionDelta.X;
  RootMotionDelta.Y = FMath::IsNearlyEqual(RootMotionDelta.Y, 0.f, 0.01f) ? 0.f : RootMotionDelta.Y;
  RootMotionDelta.Z = FMath::IsNearlyEqual(RootMotionDelta.Z, 0.f, 0.01f) ? 0.f : RootMotionDelta.Z;

  FVector RootMotionVelocity = RootMotionDelta / DeltaSeconds;
  UpdateVelocity(PostProcessAnimRootMotionVelocity(RootMotionVelocity, DeltaSeconds));
}

FVector UGenOrganicMovementComponent::PostProcessAnimRootMotionVelocity_Implementation(
  const FVector& RootMotionVelocity,
  float DeltaSeconds
)
{
  FVector FinalVelocity = RootMotionVelocity;

  if (IsAirborne())
  {
    // Ignore changes to the Z-velocity when airborne.
    FinalVelocity.Z = GetVelocity().Z;
  }

  return FinalVelocity;
}

void UGenOrganicMovementComponent::ApplyAnimRootMotionRotation(float DeltaSeconds)
{
  const FQuat RootMotionQuat = RootMotionParams.GetRootMotionTransform().GetRotation();
  if (RootMotionQuat.IsIdentity(KINDA_SMALL_NUMBER))
  {
    return;
  }

  const FRotator RootMotionRotator = RootMotionQuat.Rotator();
  const FRotator RootMotionRotatorXY = FRotator(RootMotionRotator.Pitch, 0.f, RootMotionRotator.Roll);
  const FRotator RootMotionRotatorZ = FRotator(0.f, RootMotionRotator.Yaw, 0.f);

  if (!RootMotionRotatorZ.IsNearlyZero(0.01f))
  {
    const FRotator NewRotationZ = UpdatedComponent->GetComponentRotation() + RootMotionRotatorZ;
    // Passing a rate of 0 will set the target rotation directly while still adjusting for collisions.
    RotateYawTowardsDirectionSafe(NewRotationZ.Vector(), 0.f, DeltaSeconds);
  }
  if (!RootMotionRotatorXY.IsNearlyZero(0.01f))
  {
    const FRotator NewRotationXY = UpdatedComponent->GetComponentRotation() + RootMotionRotatorXY;
    // We set the roll- and pitch-rotations in a safe way but we won't get any adjustment in case of collisions.
    SetRootCollisionRotationSafe(NewRotationXY);
  }
}

float UGenOrganicMovementComponent::GetMontagePositionByPolicy(USkeletalMeshComponent* Mesh, UAnimMontage* Montage) const
{
  if (StepMontagePolicy == EStepMontagePolicy::None || !Mesh)
  {
    return -1.f;
  }

  const auto RootMotionMontageInstance = GetRootMotionMontageInstance(Mesh);
  const bool bIsOnlySteppingRootMotionMontages = StepMontagePolicy == EStepMontagePolicy::RootMotionOnly;
  if (bIsOnlySteppingRootMotionMontages && !RootMotionMontageInstance)
  {
    return -1.f;
  }

  const auto AnimInstance = Mesh->GetAnimInstance();
  if (!AnimInstance) return -1.f;

  if (Montage)
  {
    const auto& MontageInstances = AnimInstance->MontageInstances;
    for (int32 Index = MontageInstances.Num() - 1; Index >= 0; --Index)
    {
      const auto MontageInstance = MontageInstances[Index];
      checkGMC(MontageInstance)
      if (MontageInstance && Montage == MontageInstance->Montage)
      {
        if (bIsOnlySteppingRootMotionMontages && Montage != RootMotionMontageInstance->Montage)
        {
          return -1.f;
        }
        return MontageInstance->GetPosition();
      }
    }
    return -1.f;
  }

  checkGMC(!Montage)

  const auto& MontageInstances = AnimInstance->MontageInstances;
  for (int32 Index = MontageInstances.Num() - 1; Index >= 0; --Index)
  {
    const auto MontageInstance = MontageInstances[Index];
    checkGMC(MontageInstance)
    if (MontageInstance)
    {
      if (bIsOnlySteppingRootMotionMontages && MontageInstance != RootMotionMontageInstance)
      {
        return -1.f;
      }
      return MontageInstance->GetPosition();
    }
  }

  return -1.f;
}

float UGenOrganicMovementComponent::GetMontageWeightByPolicy(USkeletalMeshComponent* Mesh, UAnimMontage* Montage) const
{
  if (StepMontagePolicy == EStepMontagePolicy::None || !Mesh)
  {
    return -1.f;
  }

  const auto RootMotionMontageInstance = GetRootMotionMontageInstance(Mesh);
  const bool bIsOnlySteppingRootMotionMontages = StepMontagePolicy == EStepMontagePolicy::RootMotionOnly;
  if (bIsOnlySteppingRootMotionMontages && !RootMotionMontageInstance)
  {
    return -1.f;
  }

  const auto AnimInstance = Mesh->GetAnimInstance();
  if (!AnimInstance) return -1.f;

  if (Montage)
  {
    const auto& MontageInstances = AnimInstance->MontageInstances;
    for (int32 Index = MontageInstances.Num() - 1; Index >= 0; --Index)
    {
      const auto MontageInstance = MontageInstances[Index];
      checkGMC(MontageInstance)
      if (MontageInstance && Montage == MontageInstance->Montage)
      {
        if (bIsOnlySteppingRootMotionMontages && Montage != RootMotionMontageInstance->Montage)
        {
          return -1.f;
        }
        return MontageInstance->GetWeight();
      }
    }
    return -1.f;
  }

  checkGMC(!Montage)

  const auto& MontageInstances = AnimInstance->MontageInstances;
  for (int32 Index = MontageInstances.Num() - 1; Index >= 0; --Index)
  {
    const auto MontageInstance = MontageInstances[Index];
    checkGMC(MontageInstance)
    if (MontageInstance)
    {
      if (bIsOnlySteppingRootMotionMontages && MontageInstance != RootMotionMontageInstance)
      {
        return -1.f;
      }
      return MontageInstance->GetWeight();
    }
  }

  return -1.f;
}

bool UGenOrganicMovementComponent::StepMontage(
  USkeletalMeshComponent* Mesh,
  UAnimMontage* Montage,
  float Position,
  float Weight,
  float PlayRate
)
{
  CFLog(StepMontagePolicy == EStepMontagePolicy::None, Warning, "<StepMontagePolicy> is \"None\".")
  CFLog(StepMontagePolicy == EStepMontagePolicy::RootMotionOnly && Montage && !Montage->HasRootMotion(), Warning,
    "<StepMontagePolicy> is \"RootMotionOnly\" but the passed montage (%s) has no root motion.", *Montage->GetName())
  return Super::StepMontage(Mesh, Montage, Position, Weight, PlayRate);
}

void UGenOrganicMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
  RequestedVelocity = NavAgentPropsAdjustment(MoveVelocity);
  bRequestedMoveWithMaxSpeed = bForceMaxSpeed;
}

void UGenOrganicMovementComponent::RequestPathMove(const FVector& MoveInput)
{
  FVector AdjustedMoveInput = NavAgentPropsAdjustment(MoveInput);

  Super::RequestPathMove(AdjustedMoveInput);
}

FVector UGenOrganicMovementComponent::NavAgentPropsAdjustment_Implementation(const FVector& NavOutput)
{
  FVector AdjustedOutput = NavOutput;

  if (!CanEverFly())
  {
    if (IsMovingOnGround() || IsAirborne()) AdjustedOutput.Z = 0.f;
  }

  if (!CanEverSwim())
  {
    if (IsSwimming()) AdjustedOutput = FVector::ZeroVector;
  }

  if (!CanEverMoveOnGround())
  {
    if (IsMovingOnGround()) AdjustedOutput = FVector::ZeroVector;
  }
  else
  {
    if (IsMovingOnGround()) AdjustedOutput = NavOutput.GetSafeNormal2D() * NavOutput.Size();
  }

  return AdjustedOutput;
}

bool UGenOrganicMovementComponent::CanStartPathFollowing() const
{
  if (!CanMove() || HasAnimRootMotion() || !UpdatedComponent || UpdatedComponent->IsSimulatingPhysics())
  {
    checkGMC(UpdatedComponent == PawnOwner->GetRootComponent())
    return false;
  }

  return Super::CanStartPathFollowing();
}

bool UGenOrganicMovementComponent::CanStopPathFollowing() const
{
  return Super::CanStopPathFollowing() && !IsAirborne();
}

float UGenOrganicMovementComponent::GetPathFollowingBrakingDistance(float MaxSpeed) const
{
  if (bUseFixedBrakingDistanceForPaths)
  {
    return FixedPathBrakingDistance;
  }

  return CalculatePathFollowingBrakingDistance(MaxSpeed, GetMoveDeltaTime());
}

float UGenOrganicMovementComponent::CalculatePathFollowingBrakingDistance_Implementation(float MaxSpeed, float DeltaSeconds) const
{
  const float BrakingDeceleration = FMath::Max(GetBrakingDeceleration(), KINDA_SMALL_NUMBER);
  return FMath::Square(MaxSpeed) / (2.f * BrakingDeceleration);
}

void UGenOrganicMovementComponent::NotifyBumpedPawn(APawn* BumpedPawn)
{
  Super::NotifyBumpedPawn(BumpedPawn);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
  if (const auto World = GetWorld())
  {
    const auto Avoidance = World->GetAvoidanceManager();
    if (Avoidance && Avoidance->IsDebugEnabled(AvoidanceUID))
    {
      DrawDebugLine(
        World,
        GetActorFeetLocation(),
        GetActorFeetLocation() + FVector(0.f, 0.f, 500.f),
        AvoidanceLockTimer > 0.f ? FColor(255, 64, 64) : FColor(64, 64, 255),
        false,
        2.f,
        SDPG_MAX,
        20.f
      );
    }
  }
#endif

  // Unlock avoidance move. This mostly happens when two pawns that are locked into avoidance moves collide with each other.
  AvoidanceLockTimer = 0.f;
}

void UGenOrganicMovementComponent::CalculateAvoidanceVelocity(float DeltaTime)
{
  checkGMC(ShouldComputeAvoidance())
  const auto World = GetWorld();
  if (!World) return;
  const auto AvoidanceManager = World->GetAvoidanceManager();
  if (!AvoidanceManager || AvoidanceWeight >= 1.0f) return;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
  const bool bShowDebug = AvoidanceManager->IsDebugEnabled(AvoidanceUID);
#else
  const bool bShowDebug = false;
#endif

  // Currently we adjust velocity only if we are grounded.
  if (!GetVelocity().IsZero() && IsMovingOnGround())
  {
    // If we are doing a locked avoidance move already skip testing.
    if (AvoidanceLockTimer > 0.f)
    {
      UpdateVelocity(AvoidanceLockVelocity, DeltaTime);
      if (bShowDebug)
      {
        DrawDebugLine(
          World,
          GetActorFeetLocation(),
          GetActorFeetLocation() + GetVelocity(),
          FColor::Blue, false,
          0.5f,
          SDPG_MAX
        );
      }
    }
    else
    {
      FVector NewVelocity = AvoidanceManager->GetAvoidanceVelocityForComponent(this);

      // User-defined post-processing.
      PostProcessAvoidanceVelocity(NewVelocity);

      if (!NewVelocity.Equals(GetVelocity()))
      {
        // We had to divert course so we lock this avoidance move in for a short time. This will make us a VO so other unlocked pawns will
        // know that we should be avoided.
        UpdateVelocity(NewVelocity, DeltaTime);
        SetAvoidanceVelocityLock(AvoidanceManager, AvoidanceManager->LockTimeAfterAvoid);
        if (bShowDebug)
        {
          DrawDebugLine(
            World,
            GetActorFeetLocation(),
            GetActorFeetLocation() + GetVelocity(),
            FColor::Red,
            false,
            0.05f,
            SDPG_MAX,
            10.f
          );
        }
      }
      else
      {
        // Although we did not have to divert course, our velocity for this frame is decided. We will not reciprocate anything further so we
        // should be treated as a VO for the remainder of this frame.
        SetAvoidanceVelocityLock(AvoidanceManager, AvoidanceManager->LockTimeAfterClean);
        if (bShowDebug)
        {
          DrawDebugLine(
            World,
            GetActorFeetLocation(),
            GetActorFeetLocation() + GetVelocity(),
            FColor::Green,
            false,
            0.05f,
            SDPG_MAX,
            10.f
          );
        }
      }
    }
    AvoidanceManager->UpdateRVO(this);
    bWasAvoidanceUpdated = true;
  }
  else if (bShowDebug)
  {
    DrawDebugLine(
      World,
      GetActorFeetLocation(),
      GetActorFeetLocation() + GetVelocity(),
      FColor::Yellow,
      false,
      0.05f,
      SDPG_MAX
    );
  }

  if (bShowDebug)
  {
    const FVector UpLine = {0.f, 0.f, 500.f};
    DrawDebugLine(
      World,
      GetActorFeetLocation(),
      GetActorFeetLocation() + UpLine,
      AvoidanceLockTimer > 0.01f ? FColor::Red : FColor::Blue,
      false,
      0.05f,
      SDPG_MAX,
      5.f
    );
  }
}

void UGenOrganicMovementComponent::UpdateAvoidance()
{
  checkGMC(ShouldComputeAvoidance())

  if (const auto World = GetWorld())
  {
    const auto AvoidanceManager = World->GetAvoidanceManager();
    if (AvoidanceManager && !bWasAvoidanceUpdated)
    {
      AvoidanceManager->UpdateRVO(this);

      // We consider this a clean move because we didn't even try to avoid.
      SetAvoidanceVelocityLock(AvoidanceManager, AvoidanceManager->LockTimeAfterClean);
    }
  }
  // Reset for next iteration.
  bWasAvoidanceUpdated = false;
}

void UGenOrganicMovementComponent::CheckAvoidance()
{
  checkfGMC(IsServerBot(), TEXT("Avoidance should only be computed for AI controlled pawns on the server."))

  if (bIsUsingAvoidanceInternal != bUseAvoidance)
  {
    bIsUsingAvoidanceInternal = bUseAvoidance;
    if (bIsUsingAvoidanceInternal)
    {
      EnableRVOAvoidance();
    }
    else
    {
      ResetAvoidanceData();
    }
  }
}

void UGenOrganicMovementComponent::EnableRVOAvoidance()
{
  checkGMC(bIsUsingAvoidanceInternal)

  ResetAvoidanceData();

  if (const auto World = GetWorld())
  {
    const auto AvoidanceManager = World->GetAvoidanceManager();
    if (AvoidanceManager)
    {
      // Initializes update timers in the avoidance manager.
      verifyGMC(AvoidanceManager->RegisterMovementComponent(this, AvoidanceWeight));
    }
  }
}

void UGenOrganicMovementComponent::ResetAvoidanceData()
{
  AvoidanceUID = 0;
  bWasAvoidanceUpdated = false;
  AvoidanceLockTimer = 0.f;
  AvoidanceLockVelocity = FVector::ZeroVector;
}

bool UGenOrganicMovementComponent::ShouldComputeAvoidance()
{
  // Avoidance calculations only need to run on the server.
  return bIsUsingAvoidanceInternal && IsServerBot();
}

void UGenOrganicMovementComponent::SetAvoidanceVelocityLock(UAvoidanceManager* Avoidance, float Duration)
{
  checkGMC(bIsUsingAvoidanceInternal)

  Avoidance->OverrideToMaxWeight(AvoidanceUID, Duration);
  AvoidanceLockVelocity = GetVelocity();
  AvoidanceLockTimer = Duration;
}

void UGenOrganicMovementComponent::SetRVOAvoidanceUID(int32 UID)
{
  AvoidanceUID = UID;
}

int32 UGenOrganicMovementComponent::GetRVOAvoidanceUID()
{
  return AvoidanceUID;
}

void UGenOrganicMovementComponent::SetRVOAvoidanceWeight(float Weight)
{
  AvoidanceWeight = Weight;
}

float UGenOrganicMovementComponent::GetRVOAvoidanceWeight()
{
  return AvoidanceWeight;
}

FVector UGenOrganicMovementComponent::GetRVOAvoidanceOrigin()
{
  return GetActorFeetLocation();
}

float UGenOrganicMovementComponent::GetRVOAvoidanceRadius()
{
  // Use the largest extent that would be required to enclose the root component in a 2D circle.
  return GetMaxDistanceToRootCollisionBoundaryXY();
}

float UGenOrganicMovementComponent::GetRVOAvoidanceConsiderationRadius()
{
  return AvoidanceConsiderationRadius;
}

float UGenOrganicMovementComponent::GetRVOAvoidanceHeight()
{
  return GetRootCollisionHalfHeight();
}

FVector UGenOrganicMovementComponent::GetVelocityForRVOConsideration()
{
  return GetVelocity();
}

int32 UGenOrganicMovementComponent::GetAvoidanceGroupMask()
{
  return AvoidanceGroup.Packed;
}

int32 UGenOrganicMovementComponent::GetGroupsToAvoidMask()
{
  return GroupsToAvoid.Packed;
}

int32 UGenOrganicMovementComponent::GetGroupsToIgnoreMask()
{
  return GroupsToIgnore.Packed;
}

#if UE_VERSION_OLDER_THAN(4, 26, 0)

void UGenOrganicMovementComponent::SetAvoidanceGroup(int32 GroupFlags)
{
  AvoidanceGroup.SetFlagsDirectly(GroupFlags);
}

void UGenOrganicMovementComponent::SetGroupsToAvoid(int32 GroupFlags)
{
  GroupsToAvoid.SetFlagsDirectly(GroupFlags);
}

void UGenOrganicMovementComponent::SetGroupsToIgnore(int32 GroupFlags)
{
  GroupsToIgnore.SetFlagsDirectly(GroupFlags);
}

void UGenOrganicMovementComponent::SetAvoidanceGroupMask(const FNavAvoidanceMask& GroupMask)
{
  AvoidanceGroup.SetFlagsDirectly(GroupMask.Packed);
}

void UGenOrganicMovementComponent::SetGroupsToAvoidMask(const FNavAvoidanceMask& GroupMask)
{
  GroupsToAvoid.SetFlagsDirectly(GroupMask.Packed);
}

void UGenOrganicMovementComponent::SetGroupsToIgnoreMask(const FNavAvoidanceMask& GroupMask)
{
  GroupsToIgnore.SetFlagsDirectly(GroupMask.Packed);
}

#else

void UGenOrganicMovementComponent::SetAvoidanceGroupMask(int32 GroupFlags)
{
  AvoidanceGroup.SetFlagsDirectly(GroupFlags);
}

void UGenOrganicMovementComponent::SetGroupsToAvoidMask(int32 GroupFlags)
{
  GroupsToAvoid.SetFlagsDirectly(GroupFlags);
}

void UGenOrganicMovementComponent::SetGroupsToIgnoreMask(int32 GroupFlags)
{
  GroupsToIgnore.SetFlagsDirectly(GroupFlags);
}

void UGenOrganicMovementComponent::SetAvoidanceGroupMask(const FNavAvoidanceMask& GroupMask)
{
  SetAvoidanceGroupMask(GroupMask.Packed);
}

void UGenOrganicMovementComponent::SetGroupsToAvoidMask(const FNavAvoidanceMask& GroupMask)
{
  SetGroupsToAvoidMask(GroupMask.Packed);
}

void UGenOrganicMovementComponent::SetGroupsToIgnoreMask(const FNavAvoidanceMask& GroupMask)
{
  SetGroupsToIgnoreMask(GroupMask.Packed);
}

#endif
