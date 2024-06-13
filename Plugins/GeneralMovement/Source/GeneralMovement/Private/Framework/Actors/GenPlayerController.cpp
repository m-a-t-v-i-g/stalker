// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenPlayerController.h"
#include "GenPawn.h"
#include "GenMovementReplicationComponent.h"
#include "GenGameState.h"
#include "GenGameStateBase.h"
#define GMC_CONTROLLER_LOG
#include "GMC_LOG.h"
#include "GenPlayerController_DBG.h"

GENERALMOVEMENT_API DEFINE_LOG_CATEGORY(LogGMCController);

namespace GMCCVars
{
#if ALLOW_CONSOLE && !NO_LOGGING

  int32 StatPing = 0;
  FAutoConsoleVariableRef CVarStatPing(
    TEXT("gmc.StatPing"),
    StatPing,
    TEXT("Display the current ping of the local machine to the server. Only applicable on clients. 0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogNetWorldTime = 0;
  FAutoConsoleVariableRef CVarLogNetWorldTime(
    TEXT("gmc.LogNetWorldTime"),
    LogNetWorldTime,
    TEXT("Log the current world time on server and client with a UTC-timestamp. 0: Disable, 1: Enable"),
    ECVF_Default
  );

#endif
}

AGenPlayerController::AGenPlayerController()
{
  bAllowTickBeforeBeginPlay = false;
  bReplicates = true;
}

void AGenPlayerController::BeginPlay()
{
  Super::BeginPlay();

  // @deprecated The old time sync process is only used when the game state is not of type @see AGenGameState or @see AGenGameStateBase.
  if (const auto World = GetWorld())
  {
    const auto GameState = World->GetGameState();
    if (!Cast<AGenGameState>(GameState) && !Cast<AGenGameStateBase>(GameState))
    {
      GMC_LOG(
        Warning,
        TEXT("Please use <GenGameState> or <GenGameStateBase> as game state class. ")
        TEXT("Support for the default game state class is deprecated and will be removed in a future release.")
      )

      if (GetLocalRole() == ROLE_AutonomousProxy)
      {
        World->GetTimerManager().SetTimer(
          Client_TimeSyncHandle,
          this,
          &AGenPlayerController::Client_SyncWithServerTime_DEPRECATED,
          TimeSyncInterval,
          true,
          InitialTimeSyncDelay
        );
      }
    }
  }
}

void AGenPlayerController::PlayerTick(float DeltaTime)
{
  Super::PlayerTick(DeltaTime);

  SaveRotationInput();

  if (GetLocalRole() == ROLE_AutonomousProxy)
  {
    Client_UpdateWorldTime(DeltaTime);
  }

  DEBUG_STAT_PING
  DEBUG_LOG_NET_WORLD_TIME
}

void AGenPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);

  // Clear timers when this object gets destroyed.
  if (GetLocalRole() == ROLE_AutonomousProxy)
  {
    if (const auto World = GetWorld()) World->GetTimerManager().ClearTimer(Client_TimeSyncHandle);
  }
}

void AGenPlayerController::AddRollInput(float Val)
{
  Super::AddRollInput(Val);
  if (FMove{}.RotationInputQuantize < ESizeQuantization::None)
  {
    constexpr float MaxRotationInput = (float)UGenMovementReplicationComponent::MAX_ROTATION_INPUT;
    RotationInput.Roll = FMath::Clamp(RotationInput.Roll, -MaxRotationInput, MaxRotationInput);
  }
}

void AGenPlayerController::AddPitchInput(float Val)
{
  Super::AddPitchInput(Val);
  if (FMove{}.RotationInputQuantize < ESizeQuantization::None)
  {
    constexpr float MaxRotationInput = (float)UGenMovementReplicationComponent::MAX_ROTATION_INPUT;
    RotationInput.Pitch = FMath::Clamp(RotationInput.Pitch, -MaxRotationInput, MaxRotationInput);
  }
}

void AGenPlayerController::AddYawInput(float Val)
{
  Super::AddYawInput(Val);
  if (FMove{}.RotationInputQuantize < ESizeQuantization::None)
  {
    constexpr float MaxRotationInput = (float)UGenMovementReplicationComponent::MAX_ROTATION_INPUT;
    RotationInput.Yaw = FMath::Clamp(RotationInput.Yaw, -MaxRotationInput, MaxRotationInput);
  }
}

void AGenPlayerController::UpdateCameraManager(float DeltaSeconds)
{
  bDeferredAutonomousProxyCameraManagerUpdate = false;

  if (GetLocalRole() != ROLE_AutonomousProxy)
  {
    Super::UpdateCameraManager(DeltaSeconds);
    return;
  }

  const auto GenPawn = Cast<AGenPawn>(GetPawn());
  if (!GenPawn)
  {
    Super::UpdateCameraManager(DeltaSeconds);
    return;
  }

  const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent());
  if (!ReplicationComponent)
  {
    Super::UpdateCameraManager(DeltaSeconds);
    return;
  }

  if (
    !ReplicationComponent->Client_ShouldUseSmoothCorrections()
    || !ReplicationComponent->Client_CorrectionBuffer.HasData()
    || !ReplicationComponent->bOnWorldTickStartExecuted
  )
  {
    Super::UpdateCameraManager(DeltaSeconds);
    return;
  }

  bDeferredAutonomousProxyCameraManagerUpdate = true;
}

void AGenPlayerController::Client_UpdateCameraForCorrection_Implementation(float DeltaTime)
{
  checkGMC(GetLocalRole() == ROLE_AutonomousProxy)

  const auto ControlledPawn = GetPawn();
  if (!ControlledPawn)
  {
    return;
  }

  if (const auto SpringArm = ControlledPawn->FindComponentByClass<USpringArmComponent>())
  {
    // If we have a spring arm component we need to update the arm location before running the camera manager update to get the correct
    // camera view. "TickComponent" just calls @see USpringArmComponent::UpdateDesiredArmLocation which is a protected function so using the
    // public tick function the is the easiest way to update the spring arm.
    SpringArm->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All/*not used*/, nullptr/*not used*/);
  }

  // The camera view needs to be updated to the new location and rotation. Usually the camera manager update runs before the post actor
  // tick event but in this case it should have been deferred for the autonomous proxy (so that we can run it now).
  // @attention The @see UpdateCameraManager function is overridden to defer the camera manager update so make sure that the correct super
  // implementation is called here.
  checkGMC(WasAutonomousProxyCameraManagerUpdateDeferred())
  Super::UpdateCameraManager(DeltaTime);
}

bool AGenPlayerController::WasAutonomousProxyCameraManagerUpdateDeferred() const
{
  return bDeferredAutonomousProxyCameraManagerUpdate;
}

FRotator AGenPlayerController::GetRotationInput() const
{
  return SavedRotationInput;
}

void AGenPlayerController::SaveRotationInput()
{
  // The rotation input gets cleared for the next frame after the player tick so we need to save the value for the replication component
  // which only ticks after the player controller.
  SavedRotationInput = RotationInput;
}

float AGenPlayerController::Client_GetSyncedWorldTimeSeconds() const
{
  checkGMC(GetLocalRole() < ROLE_Authority)
  return Client_SyncedWorldTime;
}

float AGenPlayerController::Client_GetTimeSyncAdjustment() const
{
  checkGMC(GetLocalRole() == ROLE_AutonomousProxy)

  if (const auto World = GetWorld())
  {
    // The in/out parameters can be adjusted as desired.
    int32 constexpr MinFrameRate = 30;
    int32 constexpr MaxFrameRate = 120;
    float constexpr MinAdjustment = 0.1f;
    float constexpr MaxAdjustment = 0.5f;

    // Static asserts for safety.
    static_assert(MinFrameRate > 0, "MinFrameRate <= 0");
    static_assert(MaxFrameRate > 0, "MaxFrameRate <= 0");
    static_assert(MinAdjustment > 0.f, "MinAdjustment <= 0");
    static_assert(MaxAdjustment > 0.f, "MaxAdjustment <= 0");
    static_assert(MinFrameRate != MaxFrameRate, "MinFrameRate == MaxFrameRate");
    static_assert(MinAdjustment != MaxAdjustment, "MinAdjustment == MaxAdjustment");

    // Calculate the adjustment for this frame.
    float constexpr MinFrameTime = 1.f / MinFrameRate;
    float constexpr MaxFrameTime = 1.f / MaxFrameRate;
    float constexpr Slope = (MaxAdjustment - MinAdjustment) / (MaxFrameTime - MinFrameTime);
    return FMath::Clamp(MinAdjustment + Slope * (World->GetDeltaSeconds() - MinFrameTime), MinAdjustment, MaxAdjustment);
  }

  return 0.f;
}

void AGenPlayerController::Client_UpdateWorldTime(float DeltaTime)
{
  checkGMC(GetLocalRole() == ROLE_AutonomousProxy)

  if (Client_bDoNotUpdateWorldTime)
  {
    // Do not add to the world time this frame because we already set the updated value from the server.
    Client_bDoNotUpdateWorldTime = false;
  }
  else if (Client_bSlowWorldTime)
  {
    // Only add a fraction of the delta time to bring the client time closer to the server time. This will effectively speed up movement
    // for one frame (which is usually imperceptible with low adjustment values).
    const float Adjustment = Client_GetTimeSyncAdjustment();
    Client_SyncedWorldTime += DeltaTime * (1.f - Adjustment);
    Client_bSlowWorldTime = false;
    GMC_LOG(VeryVerbose, TEXT("Client world time was slowed down for this frame (adjustment = %f)."), Adjustment)
  }
  else if (Client_bSpeedUpWorldTime)
  {
    // Add a higher delta time value to bring the client time closer to the server time. This will effectively slow down movement for one
    // frame (which is usually imperceptible with low adjustment values).
    const float Adjustment = Client_GetTimeSyncAdjustment();
    Client_SyncedWorldTime += DeltaTime * (1.f + Adjustment);
    Client_bSpeedUpWorldTime = false;
    GMC_LOG(VeryVerbose, TEXT("Client world time was sped up for this frame (adjustment = %f)."), Adjustment)
  }
  else
  {
    Client_SyncedWorldTime += DeltaTime;
  }

  checkGMC(!Client_bDoNotUpdateWorldTime)
  checkGMC(!Client_bSlowWorldTime)
  checkGMC(!Client_bSpeedUpWorldTime)

  GMC_CLOG(
    Client_SyncedWorldTime <= Client_SyncedTimeLastFrame,
    Verbose,
    TEXT("Client has world time inconsistency: Timestamp current frame (%9.3f) <= Timestamp last frame (%9.3f)"),
    Client_SyncedWorldTime,
    Client_SyncedTimeLastFrame
  )

  Client_SyncedTimeLastFrame = Client_SyncedWorldTime;
}

void AGenPlayerController::Client_SyncWithServerTime()
{
  checkGMC(GetLocalRole() == ROLE_AutonomousProxy)

  // Always reset these in the beginning in case this function is called more than once during one frame.
  Client_bDoNotUpdateWorldTime = false;
  Client_bSlowWorldTime = false;
  Client_bSpeedUpWorldTime = false;

  const auto World = GetWorld();
  if (!World)
  {
    GMC_LOG(Warning, TEXT("Client time could not not be synced with the server."))
    checkGMC(false)
    return;
  }

  const auto GameState = World->GetGameState();
  const auto GenGameState = Cast<AGenGameState>(GameState);
  const auto GenGameStateBase = Cast<AGenGameStateBase>(GameState);
  if (!GenGameState && !GenGameStateBase)
  {
    // The game state must be either of type @see AGenGameState or @see AGenGameStateBase.
    return;
  }

  const auto Connection = GetNetConnection();
  const float ServerWorldTime =
    GenGameState ? GenGameState->GetLastReplicatedServerWorldTimeSeconds() : GenGameStateBase->GetLastReplicatedServerWorldTimeSeconds();

  // The client ping (round-trip time) in milliseconds.
  float AvgRTTInMs = 0.f;
  if (USE_PLAYER_STATE_PING)
  {
    AvgRTTInMs = GetPingInMilliseconds();
  }
  else if (Connection)
  {
    AvgRTTInMs = Connection->AvgLag * 1000.f;
  }
  GMC_CLOG(AvgRTTInMs <= 0.f, Verbose, TEXT("Client ping could not be retrieved."))

  const float EstimatedLatency = FMath::Min(AvgRTTInMs / 1000.f / 2.f, MaxExpectedPing / 2.f);
  const float NewWorldTime = ServerWorldTime + EstimatedLatency;
  const float DeltaTime = World->GetDeltaSeconds();
  const float SignedTimeDiscrepancy = NewWorldTime - (Client_SyncedWorldTime + DeltaTime/*the time will be updated afterwards*/);
  const float TimeDiscrepancy = FMath::Abs(SignedTimeDiscrepancy);
  if (TimeDiscrepancy > MaxClientTimeDifferenceHardLimit)
  {
    // If the client time deviates more than the set limit we sync the local time with the server time directly.
    GMC_LOG(
      VeryVerbose,
      TEXT("Synced client world time with server, time discrepancy was %f s (max allowed is %f s): ")
      TEXT("new world time = %f s | old world time = %f s | ping = %.0f ms | jitter = %.0f ms"),
      TimeDiscrepancy,
      MaxClientTimeDifferenceHardLimit,
      NewWorldTime,
      Client_SyncedWorldTime,
      AvgRTTInMs,
      Connection ? Connection->GetAverageJitterInMS() : 0.f
    )
    Client_SyncedWorldTime = NewWorldTime;

    // Do not add to the time locally this frame, the received server world time is already the updated time for this frame. The flag is
    // processed and reset in @see Client_UpdateWorldTime.
    Client_bDoNotUpdateWorldTime = true;

    GMC_LOG(
      Verbose,
      TEXT("Corrected client time discrepancy of %f seconds."),
      TimeDiscrepancy
    )
  }
  else
  {
    GMC_LOG(
      VeryVerbose,
      TEXT("Queried server time, client is %f seconds %s."),
      TimeDiscrepancy,
      SignedTimeDiscrepancy > 0.f ? TEXT("behind") : TEXT("ahead")
    )

    // We can never assure more accurate time synchronisation than the current delta time value because the packet may arrive at any point
    // during the processing of a frame.
    float constexpr DeltaTimeTolerance = 0.001f;
    const bool bIsExceedingSoftLimit = TimeDiscrepancy > FMath::Max(DeltaTime + DeltaTimeTolerance, MaxClientTimeDifferenceSoftLimit);
    if (SignedTimeDiscrepancy < 0.f && bIsExceedingSoftLimit)
    {
      // The client is ahead of the server, set the flag to slow time down (processed and reset in @see Client_UpdateWorldTime).
      Client_bSlowWorldTime = true;
    }
    else if (SignedTimeDiscrepancy > 0.f && bIsExceedingSoftLimit)
    {
      // The client is lagging behind the server, set the flag to speed up time (processed and reset in @see Client_UpdateWorldTime).
      Client_bSpeedUpWorldTime = true;
    }
  }
}

void AGenPlayerController::Client_SyncWithServerTime_DEPRECATED()
{
  // @deprecated Old time syncing process, only called if the game mode is not of type @see AGenGameState or @see AGenGameStateBase.

  checkGMC(GetLocalRole() == ROLE_AutonomousProxy)

  Client_bDoNotUpdateWorldTime = false;
  Client_bSlowWorldTime = false;
  Client_bSpeedUpWorldTime = false;

  if (const auto World = GetWorld())
  {
    if (const auto GameState = World->GetGameState())
    {
      const float ServerWorldTime = GameState->GetServerWorldTimeSeconds();
      if (const auto Connection = GetNetConnection())
      {
        const float AvgRTT = Connection->AvgLag;
        const float Ping = FMath::Min(AvgRTT / 2.f, MaxExpectedPing);
        const float NewWorldTime = ServerWorldTime + Ping;
        const float SignedTimeDiscrepancy = NewWorldTime - Client_SyncedWorldTime;
        const float TimeDiscrepancy = FMath::Abs(SignedTimeDiscrepancy);
        if (TimeDiscrepancy > MaxClientTimeDifferenceHardLimit)
        {
          // If the client time deviates more than the set limit we sync the world time with the server.
          GMC_LOG(
            VeryVerbose,
            TEXT("Synced client world time with server, time discrepancy was %f s (max allowed is %f s): ")
            TEXT("new world time = %f s | old world time = %f s | ping = %.0f ms | jitter = %.0f ms"),
            TimeDiscrepancy,
            MaxClientTimeDifferenceHardLimit,
            NewWorldTime,
            Client_SyncedWorldTime,
            Ping * 1000.f/*convert to ms*/,
            Connection->GetAverageJitterInMS()
          )
          Client_SyncedWorldTime = NewWorldTime;

          GMC_LOG(
            Verbose,
            TEXT("Corrected client time discrepancy of %f seconds."),
            TimeDiscrepancy
          )
        }
        else
        {
          GMC_LOG(
            VeryVerbose,
            TEXT("Queried server time, client is %f seconds %s."),
            TimeDiscrepancy,
            SignedTimeDiscrepancy > 0.f ? TEXT("behind") : TEXT("ahead")
          )

          if (SignedTimeDiscrepancy < 0.f)
          {
            // The client is ahead of the server, set the flag to slow time down (processed and reset in @see Client_UpdateWorldTime).
            Client_bSlowWorldTime = true;
          }
        }
      }
      return;
    }
  }
  GMC_LOG(Warning, TEXT("Client time could not not be synced with the server."))
  checkGMC(false)
}

float AGenPlayerController::GetPingInMilliseconds() const
{
  if (!PlayerState)
  {
    return 0.f;
  }

#if UE_VERSION_OLDER_THAN(5, 0, 0)

  const float ExactPing = PlayerState->ExactPing;
  if (ExactPing > 0.0f)
  {
    // Prefer the exact ping if set (only on the server or for the local player).
    return ExactPing;
  }
  // Otherwise use the replicated compressed ping.
  return PlayerState->GetPing() * 4.f;

#else

  return PlayerState->GetPingInMilliseconds();

#endif
}

void AGenPlayerController::Client_SendParametersForAdaptiveSimulationDelay_Implementation(
  UGenMovementReplicationComponent* TargetComponent,
  float NextDelayValue,
  float NextSyncTime
) const
{
  // @attention The target component may not exist on the client yet (or it may have been destroyed already).
  if (TargetComponent)
  {
    TargetComponent->Client_SADSendParameters(NextDelayValue, NextSyncTime);
  }
}

void AGenPlayerController::SetInterpolationDelay(float Delay) const
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenPawn::StaticClass(), Actors);
  for (const auto Actor : Actors)
  {
    if (const auto GenPawn = Cast<AGenPawn>(Actor))
    {
      if (!GenPawn->IsLocallyControlled())
      {
        if (const auto MovementComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent()))
        {
          MovementComponent->SimulationDelay = Delay;
        }
      }
    }
  }
}

void AGenPlayerController::SetInterpolationMethod(int32 Method) const
{
  if (!(Method >= 0 && Method < static_cast<uint8>(EInterpolationMethod::MAX)))
  {
    return;
  }

  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenPawn::StaticClass(), Actors);
  for (const auto Actor : Actors)
  {
    if (const auto GenPawn = Cast<AGenPawn>(Actor))
    {
      if (!GenPawn->IsLocallyControlled())
      {
        if (const auto MovementComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent()))
        {
          MovementComponent->SetInterpolationMethod(static_cast<EInterpolationMethod>(Method));
        }
      }
    }
  }
}

void AGenPlayerController::SetExtrapolationAllowed(bool bAllowed) const
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenPawn::StaticClass(), Actors);
  for (const auto Actor : Actors)
  {
    if (const auto GenPawn = Cast<AGenPawn>(Actor))
    {
      if (!GenPawn->IsLocallyControlled())
      {
        if (const auto MovementComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent()))
        {
          MovementComponent->bAllowExtrapolation = bAllowed;
        }
      }
    }
  }
}

void AGenPlayerController::SetSmoothCollision(bool bSmoothLocation, bool bSmoothRotation) const
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenPawn::StaticClass(), Actors);
  for (const auto Actor : Actors)
  {
    if (const auto GenPawn = Cast<AGenPawn>(Actor))
    {
      if (!GenPawn->IsLocallyControlled())
      {
        if (const auto MovementComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent()))
        {
          MovementComponent->bSmoothCollisionLocation = bSmoothLocation;
          MovementComponent->bSmoothCollisionRotation = bSmoothRotation;
        }
      }
    }
  }
}
