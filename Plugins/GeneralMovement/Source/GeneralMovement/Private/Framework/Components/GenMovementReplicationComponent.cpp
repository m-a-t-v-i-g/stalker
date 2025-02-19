// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenMovementReplicationComponent.h"
#include "GenPawn.h"
#include "GenPlayerController.h"
#include "FlatCapsuleComponent.h"
#include "GenRollbackActor.h"
#define GMC_REPLICATION_COMPONENT_LOG
#include "GMC_LOG.h"
#include "GenMovementReplicationComponent_DBG.h"

GENERALMOVEMENT_API DEFINE_LOG_CATEGORY(LogGMCReplication);

DECLARE_CYCLE_STAT(TEXT("Tick Component"), STAT_TickComponent, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("On World Tick Start"), STAT_OnWorldTickStart, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("On World Tick End"), STAT_OnWorldTickEnd, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Update Locally Controlled Server Pawn"), STAT_UpdateLocallyControlledServerPawn, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Update Autonomous Proxy Pawn"), STAT_UpdateAutonomousProxyPawn, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Update Remotely Controlled Server Pawn"), STAT_UpdateRemotelyControlledServerPawn, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Simulate Pawn"), STAT_SimulatePawn, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Refresh Local Move"), STAT_RefreshLocalMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Manage Prerequisite Ticks"), STAT_ManagePrerequisiteTicks, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Manage Adaptive Simulation Delay"), STAT_ManageAdaptiveSimulationDelay, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Maintain Serialization Map"), STAT_Server_MaintainSerializationMap, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Unpack Client Move"), STAT_Server_UnpackClientMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Verify Timestamps"), STAT_Server_VerifyTimestamps, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Process Proxy Move"), STAT_Server_ProcessProxyMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Process Client Moves"), STAT_Server_ProcessClientMoves, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Execute Client Moves"), STAT_Server_ExecuteClientMoves, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Set Client Move Input"), STAT_Server_SetClientMoveInput, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Ensure Valid Move Data"), STAT_Server_EnsureValidMoveData, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Save Bound Data To Move"), STAT_SaveBoundDataToMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Swap State Buffer"), STAT_Server_SwapStateBuffer, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Resolve Client Discrepancy"), STAT_Server_ResolveClientDiscrepancy, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Quantize Pawn State From"), STAT_Server_QuantizePawnStateFrom, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Save Server State"), STAT_Server_SaveServerState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Save Bound Data To Server State"), STAT_Server_SaveBoundDataToServerState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Server Should Force Net Update"), STAT_Server_ShouldForceNetUpdate, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client On Rep Server State Autonomous Proxy"), STAT_Client_OnRepServerState_AutonomousProxy, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client On Rep Server State SimulatedProxy"), STAT_Client_OnRepServerState_SimulatedProxy, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Quantize Input Values"), STAT_Client_QuantizeInputValues, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Quantize Pawn State From"), STAT_Client_QuantizePawnStateFrom, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Buffer Local State"), STAT_Client_BufferLocalState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Apply Buffered Local State"), STAT_Client_ApplyBufferedLocalState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Maintain Move Queue"), STAT_Client_MaintainMoveQueue, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Should Enqueue Move"), STAT_Client_ShouldEnqueueMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Add To Move Queue"), STAT_Client_AddToMoveQueue, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Execute Move"), STAT_Client_ExecuteMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Unpack Replication Update"), STAT_Client_UnpackReplicationUpdate, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Clear Acknowledged Moves"), STAT_Client_ClearAcknowledgedMoves, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Should Replay"), STAT_Client_ShouldReplay, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Adopt Server State For Replay"), STAT_Client_AdoptServerStateForReplay, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Replay Moves"), STAT_Client_ReplayMoves, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Save Correction Transform"), STAT_Client_SaveCorrectionTransform, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Swap Correction Transform"), STAT_Client_SwapCorrectionTransform, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Client Interpolate Correction Transform"), STAT_Client_InterpolateCorrectionTransform, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Get Time"), STAT_GetTime, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Add To State Queue"), STAT_AddToStateQueue, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Execute Move"), STAT_ExecuteMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Load Move"), STAT_LoadMove, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Fill Move With Data"), STAT_FillMoveWithData, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Save Local Pawn State"), STAT_SaveLocalPawnState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Set Local Pawn State"), STAT_SetLocalPawnState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Set Replicated Pawn State"), STAT_SetReplicatedPawnState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Set Pawn State"), STAT_SetPawnState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Smooth Movement"), STAT_SmoothMovement, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Compute Smoothing Input"), STAT_ComputeSmoothingInput, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Create Initialization State"), STAT_CreateInitializationState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Compute Interpolated State"), STAT_ComputeInterpolatedState, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Extrapolate Simulated"), STAT_ExtrapolateSimulated, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Execute Move Simulated"), STAT_ExecuteMoveSimulated, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Determine Skipped States"), STAT_DetermineSkippedStates, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("SML Save Timestamps"), STAT_SMLSaveTimestamps, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Gather Rollback Pawns"), STAT_GatherRollbackPawns, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Should Be Rolled Back"), STAT_ShouldBeRolledBack, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Save Local State Before Rollback"), STAT_SaveLocalStateBeforeRollback, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Rollback Pawns"), STAT_RollbackPawns, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Compute Rollback Input"), STAT_ComputeRollbackInput, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Restore Rolled Back Pawns"), STAT_RestoreRolledBackPawns, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Restore Original State After Rollback"), STAT_RestoreOriginalStateAfterRollback, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Tick Generic Rollback Actors"), STAT_TickGenericRollbackActors, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Gather Generic Rollback Actors"), STAT_GatherGenericRollbackActors, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Rollback Generic Actors"), STAT_RollbackGenericActors, STATGROUP_GMCReplicationComp)
DECLARE_CYCLE_STAT(TEXT("Restore Rolled Back Generic Actors"), STAT_RestoreRolledBackGenericActors, STATGROUP_GMCReplicationComp)

namespace GMCCVars
{
#if ALLOW_CONSOLE && !NO_LOGGING

  int32 StatNetMovementValues = 0;
  FAutoConsoleVariableRef CVarStatNetMovementValues(
    TEXT("gmc.StatNetMovementValues"),
    StatNetMovementValues,
    TEXT("Display realtime values of movement variables for pawns with the specified role on the screen. ")
    TEXT("0: Disable, 1: ROLE_Authority, 2: ROLE_AutonomousProxy, 3: ROLE_SimulatedProxy"),
    ECVF_Default
  );

  int32 StatNetContextValues = 0;
  FAutoConsoleVariableRef CVarStatNetContextValues(
    TEXT("gmc.StatNetContextValues"),
    StatNetContextValues,
    TEXT("Display realtime values of network related context variables for pawns with the specified role on the screen. ")
    TEXT("0: Disable, 1: ROLE_Authority, 2: ROLE_AutonomousProxy, 3: ROLE_SimulatedProxy"),
    ECVF_Default
  );

  int32 ShowNetRole = 0;
  FAutoConsoleVariableRef CVarShowNetRole(
    TEXT("gmc.ShowNetRole"),
    ShowNetRole,
    TEXT("Display the net roles of pawns as floating text above them. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 ShowSimulationDelay = 0;
  FAutoConsoleVariableRef CVarShowSimulationDelay(
    TEXT("gmc.ShowSimulationDelay"),
    ShowSimulationDelay,
    TEXT("Display each pawn's current simulation delay value as floating text above them. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 ShowClientLocationCorrections = 0;
  FAutoConsoleVariableRef CVarShowClientLocationCorrections(
    TEXT("gmc.ShowClientLocationCorrections"),
    ShowClientLocationCorrections,
    TEXT("Show corrections of the client location on the client. The original location is displayed in red, the corrected in green. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 ShowClientLocationErrors = 0;
  FAutoConsoleVariableRef CVarShowClientLocationErrors(
    TEXT("gmc.ShowClientLocationErrors"),
    ShowClientLocationErrors,
    TEXT("Visualize deviations in location between client (red) and server (green) on the server. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogMoveExecution = 0;
  FAutoConsoleVariableRef CVarLogMoveExecution(
    TEXT("gmc.LogMoveExecution"),
    LogMoveExecution,
    TEXT("Log data for the execution of moves for all actors (excluding client replay iterations). ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogClientReplay = 0;
  FAutoConsoleVariableRef CVarLogClientReplay(
    TEXT("gmc.LogClientReplay"),
    LogClientReplay,
    TEXT("Log all data relating to moves replayed by the autonomous proxy. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogSmoothing = 0;
  FAutoConsoleVariableRef CVarLogSmoothing(
    TEXT("gmc.LogSmoothing"),
    LogSmoothing,
    TEXT("Log all data relating to the smoothing of remotely controlled pawns. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogUsingExtrapolationData = 0;
  FAutoConsoleVariableRef CVarLogUsingExtrapolationData(
    TEXT("gmc.LogUsingExtrapolationData"),
    LogUsingExtrapolationData,
    TEXT("Log when extrapolated data is being used for smoothing remotely controlled pawns. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogStateQueueData = 0;
  FAutoConsoleVariableRef CVarLogStateQueueData(
    TEXT("gmc.LogStateQueueData"),
    LogStateQueueData,
    TEXT("Log the state queue contents of remotely controlled pawns. ")
    TEXT("0: Disable, 1: ROLE_Authority, 2: ROLE_SimulatedProxy"),
    ECVF_Default
  );

  int32 LogSimulatedTickData = 0;
  FAutoConsoleVariableRef CVarLogSimulatedTickData(
    TEXT("gmc.LogSimulatedTickData"),
    LogSimulatedTickData,
    TEXT("Log the states passed to the simulated tick function of remotely controlled pawns. ")
    TEXT("0: Disable, 1: ROLE_Authority, 2: ROLE_SimulatedProxy"),
    ECVF_Default
  );

  int32 LogClientMoveTrace = 0;
  FAutoConsoleVariableRef CVarLogClientMoveTrace(
    TEXT("gmc.LogClientMoveTrace"),
    LogClientMoveTrace,
    TEXT("Trace the replication of a move from the client to the server and back to the client with all intermediate values. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

  int32 LogNumExecutedRemotePackets = 0;
  FAutoConsoleVariableRef CVarLogNumExecutedRemotePackets(
    TEXT("gmc.LogNumExecutedRemotePackets"),
    LogNumExecutedRemotePackets,
    TEXT("Log how many client move packets are executed on the server each frame. ")
    TEXT("0: Disable, 1: Enable"),
    ECVF_Default
  );

#endif
}

UGenMovementReplicationComponent::UGenMovementReplicationComponent()
{
  SetIsReplicatedByDefault(true);
  LoadNetworkPreset(ENetworkPreset::LAN);
  LoadServerStateReplicationPreset(ROLE_AutonomousProxy);
  LoadServerStateReplicationPreset(ROLE_SimulatedProxy);
}

void UGenMovementReplicationComponent::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime)
{
  SCOPE_CYCLE_COUNTER(STAT_OnWorldTickStart)

  if (!World)
  {
    return;
  }

  if (World != GetWorld())
  {
    // Not our world.
    return;
  }

  checkGMC(!bOnWorldTickStartExecuted)

  // Only continue execution if actors will be ticked this frame.
  const auto NetDriver = World->GetNetDriver();
  bOnWorldTickStartExecuted =
    PawnOwner && PawnOwner->HasActorBegunPlay()
    && TickType != LEVELTICK_TimeOnly
    && !World->IsPaused()
#if UE_VERSION_OLDER_THAN(5, 0, 0)
    && (!NetDriver || !NetDriver->ServerConnection || NetDriver->ServerConnection->State == USOCK_Open);
#else
    && (!NetDriver || !NetDriver->ServerConnection || NetDriver->ServerConnection->GetConnectionState() == USOCK_Open);
#endif
  if (!bOnWorldTickStartExecuted)
  {
    return;
  }

  if (Client_ShouldUseSmoothCorrections() && Client_CorrectionBuffer.HasBegunCorrection())
  {
    Client_HandleSmoothCorrectionOnWorldTickStart();
  }
}

void UGenMovementReplicationComponent::OnWorldTickEnd(UWorld* World, ELevelTick TickType, float DeltaTime)
{
  SCOPE_CYCLE_COUNTER(STAT_OnWorldTickEnd)

  if (!World)
  {
    return;
  }

  if (World != GetWorld())
  {
    // Not our world.
    return;
  }

  // Only continue if @see OnWorldTickStart was also executed fully.
  if (!bOnWorldTickStartExecuted)
  {
    return;
  }
  bOnWorldTickStartExecuted = false;

  if (Client_ShouldUseSmoothCorrections() && Client_CorrectionBuffer.HasData())
  {
    Client_HandleSmoothCorrectionOnWorldTickEnd(DeltaTime, TickType);
  }
}

void UGenMovementReplicationComponent::InitializeComponent()
{
  Super::InitializeComponent();

  if (!HasAnyFlags(NoBindingFlags))
  {
    // Called from here because binding must happen before any (de)serialization of replicated data occurs.
    BindReplicationData();
  }

  // Set the recipient roles of the server states so we can identify them easily outside of the replication component.
  ServerState_AutonomousProxy().RecipientRole = ROLE_AutonomousProxy;
  ServerState_SimulatedProxy().RecipientRole = ROLE_SimulatedProxy;
  // Quantization levels must match between server states.
  ServerState_AutonomousProxy().LocationQuantize = ServerState_SimulatedProxy().LocationQuantize = LocationQuantize;
  ServerState_AutonomousProxy().VelocityQuantize = ServerState_SimulatedProxy().VelocityQuantize = VelocityQuantize;
  ServerState_AutonomousProxy().RotationQuantize = ServerState_SimulatedProxy().RotationQuantize = RotationQuantize;
  ServerState_AutonomousProxy().ControlRotationQuantize = ServerState_SimulatedProxy().ControlRotationQuantize = ControlRotationQuantize;
}

void UGenMovementReplicationComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
  if (!NewUpdatedComponent)
  {
    Super::SetUpdatedComponent(NewUpdatedComponent);
    return;
  }

  const auto NewGenPawnOwner = Cast<AGenPawn>(NewUpdatedComponent->GetOwner());
  if (!NewGenPawnOwner)
  {
    GMC_CLOG(GetWorld() && GetWorld()->IsGameWorld(), Error, TEXT("New updated component must be owned by a pawn of type <AGenPawn>."))
    Super::SetUpdatedComponent(NewUpdatedComponent);
    return;
  }

  Super::SetUpdatedComponent(NewUpdatedComponent);
  checkGMC(IsValid(UpdatedComponent))
  checkGMC(IsValid(UpdatedPrimitive))
  GenPawnOwner = Cast<AGenPawn>(UpdatedComponent->GetOwner());
  checkGMC(GenPawnOwner)
  ServerState_SimulatedProxy().Owner = ServerState_AutonomousProxy().Owner = GenPawnOwner;
}

#if WITH_EDITORONLY_DATA && WITH_EDITOR

void UGenMovementReplicationComponent::HideProperty(UClass* Class, const FName& PropertyName)
{
  if (FProperty* Property = Class->FindPropertyByName(PropertyName))
  {
    const FString& EditCondition = Property->GetMetaData("EditCondition");
    const FString NewEditCondition = EditCondition.IsEmpty() ? TEXT("!bHideProperties") : EditCondition + TEXT(" && !bHideProperties");
    Property->SetMetaData("EditCondition", *NewEditCondition);
    Property->SetMetaData("EditConditionHides", "true");
  }
}

#endif

#if WITH_EDITOR

bool UGenMovementReplicationComponent::CanEditChange(const FProperty* InProperty) const
{
  const bool bCanEditParent = Super::CanEditChange(InProperty);

  const FName PropertyName = InProperty->GetFName();
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, LocationNetTolerance))
  {
    const bool bCanEdit = bUseClientLocation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, RotationNetTolerance))
  {
    const bool bCanEdit = bUseClientRotation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, ControlRotationNetTolerance))
  {
    const bool bCanEdit = bUseClientControlRotation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bUseSmoothCorrections))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, SmoothCorrectionTime))
  {
    const bool bCanEdit = bUseClientPrediction && bUseSmoothCorrections;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bOnlyReplayWhenMoving))
  {
    const bool bCanEdit = bUseClientPrediction && !bAlwaysReplay;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, ReplaySpeedThreshold))
  {
    const bool bCanEdit = bUseClientPrediction && !bAlwaysReplay && bOnlyReplayWhenMoving;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxTimeStepSimulated))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && ExtrapolationMode != EExtrapolationMode::Simple;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxIterationsSimulated))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && ExtrapolationMode != EExtrapolationMode::Simple;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxVelocityError))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxLocationError))
  {
    const bool bCanEdit = bUseClientPrediction && !bUseClientLocation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxRotationError))
  {
    const bool bCanEdit = bUseClientPrediction && !bUseClientRotation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxControlRotationError))
  {
    const bool bCanEdit = bUseClientPrediction && !bUseClientControlRotation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bAssumeClientState))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxClientUpdateWaitTime))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bAlwaysReplay))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bSkipBoneUpdateForRollback))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bRollbackServerPawns))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bRollbackClientPawns))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bRollbackGenericServerActors))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bRollbackGenericClientActors))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, ServerPawnRollbackRadius))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, ClientPawnRollbackRadius))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, GenericServerActorRollbackRadius))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, GenericClientActorRollbackRadius))
  {
    const bool bCanEdit = bUseClientPrediction;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxAllowedTimestampDeviation))
  {
    const bool bCanEdit = bVerifyClientTimestamps;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxStrikeCount))
  {
    const bool bCanEdit = bVerifyClientTimestamps;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, StrikeResetInterval))
  {
    const bool bCanEdit = bVerifyClientTimestamps;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, InterpolationMode))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, SimulationDelay))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && InterpolationMode == EInterpolationMode::FixedDelay;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, AdaptiveDelaySyncInterval))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && InterpolationMode == EInterpolationMode::AdaptiveDelay;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, AdaptiveDelayBufferTime))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && InterpolationMode == EInterpolationMode::AdaptiveDelay;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, AdaptiveDelayTolerance))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && InterpolationMode == EInterpolationMode::AdaptiveDelay;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MatchLatestMaxDeltaTime))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && InterpolationMode == EInterpolationMode::MatchLatest;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MatchLatestMinDeltaTime))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && InterpolationMode == EInterpolationMode::MatchLatest;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bAllowExtrapolation))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, ExtrapolationMode))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None && bAllowExtrapolation;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxExtrapolationRatio))
  {
    const bool bCanEdit =
      InterpolationMethod != EInterpolationMethod::None && bAllowExtrapolation && ExtrapolationMode == EExtrapolationMode::Simple;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, MaxExtrapolationDeltaTime))
  {
    const bool bCanEdit =
      InterpolationMethod != EInterpolationMethod::None && bAllowExtrapolation && ExtrapolationMode != EExtrapolationMode::Simple;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bSmoothCollisionLocation))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None;
    return bCanEditParent && bCanEdit;
  }
  if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, bSmoothCollisionRotation))
  {
    const bool bCanEdit = InterpolationMethod != EInterpolationMethod::None;
    return bCanEditParent && bCanEdit;
  }

  return bCanEditParent;
}

void UGenMovementReplicationComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);

  FProperty* ChangedProperty = PropertyChangedEvent.Property;
  if (ChangedProperty)
  {
    const FName PropertyName = ChangedProperty->GetFName();
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenMovementReplicationComponent, NetworkPreset))
    {
      LoadNetworkPreset(NetworkPreset);
    }
  }
}

#endif

void UGenMovementReplicationComponent::LoadNetworkPreset(ENetworkPreset PresetToLoad)
{
  switch (PresetToLoad)
  {
    case ENetworkPreset::LAN:
    {
      // Networking.
      bUseClientPrediction = true;
      bUseClientLocation = false;
      bUseClientRotation = false;
      bUseClientControlRotation = true;
      LocationNetTolerance = 5.f;
      RotationNetTolerance = 10.f;
      ControlRotationNetTolerance = 10.f;
      MaxServerDeltaTime = 0.05f;
      MaxClientDeltaTime = 0.00834f;
      ClientSendRate = 120;
      ServerMinUpdateRate = 5;
      MoveQueueMaxSize = 64;
      bUseSmoothCorrections = true;
      SmoothCorrectionTime = 0.2f;
      bOnlyReplayWhenMoving = false;
      ReplaySpeedThreshold = 10.f;
      // Verification.
      bVerifyClientTimestamps = false;
      MaxAllowedTimestampDeviation = 0.1f;
      MaxStrikeCount = 2;
      StrikeResetInterval = 3.f;
      // Smoothing.
      InterpolationMethod = EInterpolationMethod::Linear;
      InterpolationMode = EInterpolationMode::FixedDelay;
      SimulationDelay = 0.12f;
      AdaptiveDelaySyncInterval = 2.f;
      AdaptiveDelayBufferTime = 0.1f;
      AdaptiveDelayTolerance = 0.05f;
      MatchLatestMaxDeltaTime = 0.5f;
      MatchLatestMinDeltaTime = 0.1f;
      bAllowExtrapolation = true;
      ExtrapolationMode = EExtrapolationMode::Simple;
      MaxExtrapolationRatio = 3.f;
      MaxExtrapolationDeltaTime = 5.f;
      StateQueueMaxSize = 128;
      // Advanced.
      MaxTimeStep = 0.04f;
      MaxIterations = 10;
      MaxTimeStepSimulated = 1.f;
      MaxIterationsSimulated = 5;
      MaxVelocityError = 2.f;
      MaxLocationError = 1.f;
      MaxRotationError = 5.f;
      MaxControlRotationError = 5.f;
      bAssumeClientState = true;
      RemoteMoveProcessingChunkSize = 2;
      MaxNumBufferedRemoteMovePackets = 5;
      MaxClientUpdateWaitTime = 0.5f;
      MinRepHoldTime = 0.02f;
      FullSerializationInterval = 0.5f;
      bAlwaysReplay = false;
      bSmoothRemoteServerPawn = true;
      bSmoothCollisionLocation = true;
      bSmoothCollisionRotation = true;
      bEnsureValidMoveData = true;
      bOptimizeTraffic = true;
      LocationQuantize = EDecimalQuantization::RoundTwoDecimals;
      VelocityQuantize = EDecimalQuantization::RoundTwoDecimals;
      RotationQuantize = ESizeQuantization::Short;
      ControlRotationQuantize = ESizeQuantization::Short;
      bDetermineSkippedSmoothingStates = false;
      bSkipBoneUpdateForSmoothing = true;
      bSkipBoneUpdateForRollback = true;
      bFillStateQueueForLocalServerPawns = true;
      bFillStateQueueForRemoteServerPawns = true;
      bRollbackServerPawns = true;
      bRollbackClientPawns = true;
      bRollbackGenericServerActors = true;
      bRollbackGenericClientActors = true;
      ServerPawnRollbackRadius = 600.f;
      ClientPawnRollbackRadius = 600.f;
      GenericServerActorRollbackRadius = 600.f;
      GenericClientActorRollbackRadius = 600.f;
      return;
    }
    case ENetworkPreset::Competitive:
    {
      // Networking.
      bUseClientPrediction = true;
      bUseClientLocation = false;
      bUseClientRotation = false;
      bUseClientControlRotation = true;
      LocationNetTolerance = 5.f;
      RotationNetTolerance = 10.f;
      ControlRotationNetTolerance = 10.f;
      MaxServerDeltaTime = 0.05f;
      MaxClientDeltaTime = 0.01667f;
      ClientSendRate = 60;
      ServerMinUpdateRate = 5;
      MoveQueueMaxSize = 128;
      bUseSmoothCorrections = true;
      SmoothCorrectionTime = 0.2f;
      bOnlyReplayWhenMoving = false;
      ReplaySpeedThreshold = 10.f;
      // Verification.
      bVerifyClientTimestamps = false;
      MaxAllowedTimestampDeviation = 0.1f;
      MaxStrikeCount = 2;
      StrikeResetInterval = 3.f;
      // Smoothing.
      InterpolationMethod = EInterpolationMethod::Linear;
      InterpolationMode = EInterpolationMode::FixedDelay;
      SimulationDelay = 0.25f;
      AdaptiveDelaySyncInterval = 2.f;
      AdaptiveDelayBufferTime = 0.1f;
      AdaptiveDelayTolerance = 0.05f;
      MatchLatestMaxDeltaTime = 0.5f;
      MatchLatestMinDeltaTime = 0.1f;
      bAllowExtrapolation = true;
      ExtrapolationMode = EExtrapolationMode::Simple;
      MaxExtrapolationRatio = 3.f;
      MaxExtrapolationDeltaTime = 5.f;
      StateQueueMaxSize = 128;
      // Advanced.
      MaxTimeStep = 0.04f;
      MaxIterations = 10;
      MaxTimeStepSimulated = 1.f;
      MaxIterationsSimulated = 5;
      MaxVelocityError = 2.f;
      MaxLocationError = 1.f;
      MaxRotationError = 5.f;
      MaxControlRotationError = 5.f;
      bAssumeClientState = true;
      RemoteMoveProcessingChunkSize = 2;
      MaxNumBufferedRemoteMovePackets = 5;
      MaxClientUpdateWaitTime = 0.5f;
      MinRepHoldTime = 0.02f;
      FullSerializationInterval = 0.5f;
      bAlwaysReplay = false;
      bSmoothRemoteServerPawn = true;
      bSmoothCollisionLocation = true;
      bSmoothCollisionRotation = true;
      bEnsureValidMoveData = true;
      bOptimizeTraffic = true;
      LocationQuantize = EDecimalQuantization::RoundTwoDecimals;
      VelocityQuantize = EDecimalQuantization::RoundTwoDecimals;
      RotationQuantize = ESizeQuantization::Short;
      ControlRotationQuantize = ESizeQuantization::Short;
      bDetermineSkippedSmoothingStates = false;
      bSkipBoneUpdateForSmoothing = true;
      bSkipBoneUpdateForRollback = true;
      bFillStateQueueForLocalServerPawns = true;
      bFillStateQueueForRemoteServerPawns = true;
      bRollbackServerPawns = true;
      bRollbackClientPawns = true;
      bRollbackGenericServerActors = true;
      bRollbackGenericClientActors = true;
      ServerPawnRollbackRadius = 1200.f;
      ClientPawnRollbackRadius = 1200.f;
      GenericServerActorRollbackRadius = 1200.f;
      GenericClientActorRollbackRadius = 1200.f;
      return;
    }
    case ENetworkPreset::Regular:
    {
      // Networking.
      bUseClientPrediction = true;
      bUseClientLocation = false;
      bUseClientRotation = false;
      bUseClientControlRotation = true;
      LocationNetTolerance = 5.f;
      RotationNetTolerance = 10.f;
      ControlRotationNetTolerance = 20.f;
      MaxServerDeltaTime = 0.05f;
      MaxClientDeltaTime = 0.02223f;
      ClientSendRate = 45;
      ServerMinUpdateRate = 1;
      MoveQueueMaxSize = 128;
      bUseSmoothCorrections = true;
      SmoothCorrectionTime = 0.2f;
      bOnlyReplayWhenMoving = false;
      ReplaySpeedThreshold = 10.f;
      // Verification.
      bVerifyClientTimestamps = false;
      MaxAllowedTimestampDeviation = 0.1f;
      MaxStrikeCount = 2;
      StrikeResetInterval = 3.f;
      // Smoothing.
      InterpolationMethod = EInterpolationMethod::Linear;
      InterpolationMode = EInterpolationMode::AdaptiveDelay;
      SimulationDelay = 0.5f;
      AdaptiveDelaySyncInterval = 2.f;
      AdaptiveDelayBufferTime = 0.3f;
      AdaptiveDelayTolerance = 0.05f;
      MatchLatestMaxDeltaTime = 0.5f;
      MatchLatestMinDeltaTime = 0.1f;
      bAllowExtrapolation = true;
      ExtrapolationMode = EExtrapolationMode::Simple;
      MaxExtrapolationRatio = 3.f;
      MaxExtrapolationDeltaTime = 5.f;
      StateQueueMaxSize = 256;
      // Advanced.
      MaxTimeStep = 0.04f;
      MaxIterations = 10;
      MaxTimeStepSimulated = 1.f;
      MaxIterationsSimulated = 5;
      MaxVelocityError = 2.f;
      MaxLocationError = 1.f;
      MaxRotationError = 5.f;
      MaxControlRotationError = 5.f;
      bAssumeClientState = true;
      RemoteMoveProcessingChunkSize = 2;
      MaxNumBufferedRemoteMovePackets = 5;
      MaxClientUpdateWaitTime = 0.5f;
      MinRepHoldTime = 0.02f;
      FullSerializationInterval = 0.5f;
      bAlwaysReplay = false;
      bSmoothRemoteServerPawn = true;
      bSmoothCollisionLocation = true;
      bSmoothCollisionRotation = true;
      bEnsureValidMoveData = true;
      bOptimizeTraffic = true;
      LocationQuantize = EDecimalQuantization::RoundTwoDecimals;
      VelocityQuantize = EDecimalQuantization::RoundTwoDecimals;
      RotationQuantize = ESizeQuantization::Short;
      ControlRotationQuantize = ESizeQuantization::Short;
      bDetermineSkippedSmoothingStates = false;
      bSkipBoneUpdateForSmoothing = true;
      bSkipBoneUpdateForRollback = true;
      bFillStateQueueForLocalServerPawns = true;
      bFillStateQueueForRemoteServerPawns = true;
      bRollbackServerPawns = true;
      bRollbackClientPawns = true;
      bRollbackGenericServerActors = true;
      bRollbackGenericClientActors = true;
      ServerPawnRollbackRadius = 1800.f;
      ClientPawnRollbackRadius = 1800.f;
      GenericServerActorRollbackRadius = 1800.f;
      GenericClientActorRollbackRadius = 1800.f;
      return;
    }
    case ENetworkPreset::LowEnd:
    {
      // Networking.
      bUseClientPrediction = true;
      bUseClientLocation = false;
      bUseClientRotation = false;
      bUseClientControlRotation = true;
      LocationNetTolerance = 10.f;
      RotationNetTolerance = 20.f;
      ControlRotationNetTolerance = 30.f;
      MaxServerDeltaTime = 0.05f;
      MaxClientDeltaTime = 0.03334f;
      ClientSendRate = 30;
      ServerMinUpdateRate = 0;
      MoveQueueMaxSize = 256;
      bUseSmoothCorrections = true;
      SmoothCorrectionTime = 0.2f;
      bOnlyReplayWhenMoving = false;
      ReplaySpeedThreshold = 10.f;
      // Verification.
      bVerifyClientTimestamps = false;
      MaxAllowedTimestampDeviation = 0.1f;
      MaxStrikeCount = 2;
      StrikeResetInterval = 3.f;
      // Smoothing.
      InterpolationMethod = EInterpolationMethod::Linear;
      InterpolationMode = EInterpolationMode::AdaptiveDelay;
      SimulationDelay = 0.8f;
      AdaptiveDelaySyncInterval = 2.f;
      AdaptiveDelayBufferTime = 0.5f;
      AdaptiveDelayTolerance = 0.05f;
      MatchLatestMaxDeltaTime = 0.5f;
      MatchLatestMinDeltaTime = 0.15f;
      bAllowExtrapolation = true;
      ExtrapolationMode = EExtrapolationMode::Simple;
      MaxExtrapolationRatio = 3.f;
      MaxExtrapolationDeltaTime = 5.f;
      StateQueueMaxSize = 512;
      // Advanced.
      MaxTimeStep = 0.04f;
      MaxIterations = 10;
      MaxTimeStepSimulated = 1.f;
      MaxIterationsSimulated = 5;
      MaxVelocityError = 2.f;
      MaxLocationError = 1.f;
      MaxRotationError = 5.f;
      MaxControlRotationError = 5.f;
      bAssumeClientState = true;
      RemoteMoveProcessingChunkSize = 5;
      MaxNumBufferedRemoteMovePackets = 20;
      MaxClientUpdateWaitTime = 0.5f;
      MinRepHoldTime = 0.02f;
      FullSerializationInterval = 0.5f;
      bAlwaysReplay = false;
      bSmoothRemoteServerPawn = true;
      bSmoothCollisionLocation = true;
      bSmoothCollisionRotation = true;
      bEnsureValidMoveData = true;
      bOptimizeTraffic = true;
      LocationQuantize = EDecimalQuantization::RoundTwoDecimals;
      VelocityQuantize = EDecimalQuantization::RoundTwoDecimals;
      RotationQuantize = ESizeQuantization::Short;
      ControlRotationQuantize = ESizeQuantization::Short;
      bDetermineSkippedSmoothingStates = false;
      bSkipBoneUpdateForSmoothing = true;
      bSkipBoneUpdateForRollback = true;
      bFillStateQueueForLocalServerPawns = true;
      bFillStateQueueForRemoteServerPawns = true;
      bRollbackServerPawns = true;
      bRollbackClientPawns = true;
      bRollbackGenericServerActors = true;
      bRollbackGenericClientActors = true;
      ServerPawnRollbackRadius = 2500.f;
      ClientPawnRollbackRadius = 2500.f;
      GenericServerActorRollbackRadius = 2500.f;
      GenericClientActorRollbackRadius = 2500.f;
      return;
    }
    case ENetworkPreset::Custom:
    {
      // User-defined configuration.
      return;
    }
    default: checkNoEntry();
  }
}

void UGenMovementReplicationComponent::LoadServerStateReplicationPreset(ENetRole RecipientRole)
{
  // Sets the most commonly used serialization options by default. This function should be overridden by subclasses and called in the
  // constructor to set appropriate replication settings for the implemented movement.
  switch (RecipientRole)
  {
    case ROLE_AutonomousProxy:
    {
      ServerState_AutonomousProxy().bSerializeTimestamp = true;
      ServerState_AutonomousProxy().bSerializeMoveValidation = true;
      ServerState_AutonomousProxy().bSerializeLocation = true;
      ServerState_AutonomousProxy().bSerializeVelocity = true;
      ServerState_AutonomousProxy().bSerializeRotationRoll = true;
      ServerState_AutonomousProxy().bSerializeRotationPitch = true;
      ServerState_AutonomousProxy().bSerializeRotationYaw = true;
      ServerState_AutonomousProxy().bSerializeControlRotationRoll = false;
      ServerState_AutonomousProxy().bSerializeControlRotationPitch = false;
      ServerState_AutonomousProxy().bSerializeControlRotationYaw = false;
      ServerState_AutonomousProxy().bSerializeInputMode = true;
      ServerState_AutonomousProxy().bSerializeBoundData = true;
      return;
    }
    case ROLE_SimulatedProxy:
    {
      ServerState_SimulatedProxy().bSerializeTimestamp = true;
      ServerState_SimulatedProxy().bSerializeMoveValidation = false; // Not used by simulated proxies.
      ServerState_SimulatedProxy().bSerializeLocation = true;
      ServerState_SimulatedProxy().bSerializeVelocity = true;
      ServerState_SimulatedProxy().bSerializeRotationRoll = true;
      ServerState_SimulatedProxy().bSerializeRotationPitch = true;
      ServerState_SimulatedProxy().bSerializeRotationYaw = true;
      ServerState_SimulatedProxy().bSerializeControlRotationRoll = false;
      ServerState_SimulatedProxy().bSerializeControlRotationPitch = true;
      ServerState_SimulatedProxy().bSerializeControlRotationYaw = true;
      ServerState_SimulatedProxy().bSerializeInputMode = true;
      ServerState_SimulatedProxy().bSerializeBoundData = true;
      return;
    }
    default: checkNoEntry();
  }
}

void UGenMovementReplicationComponent::BeginPlay()
{
  if (!PawnOwner)
  {
    Super::BeginPlay();
    return;
  }

  if (!PawnOwner->GetRootComponent())
  {
    Super::BeginPlay();
    return;
  }

  // "GenPawnOwner" is initialized in @see SetUpdatedComponent (called from @see UMovementComponent::OnRegister) when the actor is spawned.
  GMC_CLOG(!GenPawnOwner, Error, TEXT("Owning pawn has to be of type <AGenPawn>."))
  checkGMC(GenPawnOwner)

  GMC_CLOG(!UpdatedComponent, Error, TEXT("<UpdatedComponent> is nullptr."))
  checkGMC(UpdatedComponent)

  GMC_CLOG(PawnOwner->bAllowTickBeforeBeginPlay, Warning, TEXT("<bAllowTickBeforeBeginPlay> should be set to false."))
  checkGMC(!PawnOwner->bAllowTickBeforeBeginPlay)

  // Add the simulated root for standalone games also to ensure consistent behaviour across all net modes.
  AddSimulatedRootComponent();
  checkGMC(SimulatedRootComponent)

  if (!IsNetMode(NM_Standalone))
  {
    BeginPlayNetworked();
  }

  // Call the Blueprint BeginPlay event after the replication component has been initialized.
  Super::BeginPlay();
}

void UGenMovementReplicationComponent::BeginPlayNetworked()
{
  checkGMC(!IsNetMode(NM_Standalone))
  checkGMC(PawnOwner)
  checkGMC(PawnOwner->GetRootComponent())

  FWorldDelegates::OnWorldTickStart.AddUObject(this, &UGenMovementReplicationComponent::OnWorldTickStart);
  FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &UGenMovementReplicationComponent::OnWorldTickEnd);

  // Assign the interpolation function.
  SetInterpolationMethod(InterpolationMethod);

  // Save the default settings for toggling server-side physics simulation.
  SavedSettingsServerAuthPhysics.SaveSettings(this);

  // Save the default settings for toggling client-side physics simulation.
  SavedSettingsClientAuthPhysics.SaveSettings(this);

  // Allocate all the memory we need to store states and moves to avoid overhead from resizing when adding or removing elements.
  Client_MoveQueue.Reset(MoveQueueMaxSize);
  StateQueue.Reset(StateQueueMaxSize);

  GMC_CLOG(
    !GetIsReplicated(),
    Warning,
    TEXT("The movement component does not replicate. Set <bReplicates> to true to use GMC networking features.")
  )

  GMC_CLOG(
    !PawnOwner->GetIsReplicated(),
    Warning,
    TEXT("The owning pawn does not replicate. Set <bReplicates> to true on the actor to use GMC networking features.")
  )

  GMC_CLOG(
    PawnOwner->IsReplicatingMovement(),
    Warning,
    TEXT("Built-in actor movement replication is being used. Set <bReplicateMovement> to false to use GMC networking features.")
  )

  GMC_CLOG(
    PawnOwner->GetRootComponent()->GetIsReplicated(),
    Warning,
    TEXT("The pawn's root component must not be set to replicate.")
  )

  GMC_CLOG(
    bUseClientLocation && !LocalMove().bSerializeOutLocation,
    Warning,
    TEXT("<bUseClientLocation> is true but the autonomous proxy is not sending its location to the server.")
  )

  GMC_CLOG(
    bUseClientLocation && ServerState_AutonomousProxy().bSerializeLocation,
    Warning,
    TEXT("<bUseClientLocation> is true but the actor location is still being replicated back to the autonomous proxy.")
  )

  GMC_CLOG(
    bUseClientRotation && (
      !LocalMove().bSerializeOutRotationRoll &&
      !LocalMove().bSerializeOutRotationPitch &&
      !LocalMove().bSerializeOutRotationYaw
    ),
    Warning,
    TEXT("<bUseClientRotation> is true but the autonomous proxy is not sending any actor rotation components to the server.")
  )

  GMC_CLOG(
    bUseClientRotation && (
      ServerState_AutonomousProxy().bSerializeRotationRoll ||
      ServerState_AutonomousProxy().bSerializeRotationPitch ||
      ServerState_AutonomousProxy().bSerializeRotationYaw
    ),
    Warning,
    TEXT("<bUseClientRotation> is true but actor rotation components are still being replicated back to the autonomous proxy.")
  )

  GMC_CLOG(
    bUseClientControlRotation && (
      !LocalMove().bSerializeOutControlRotationRoll &&
      !LocalMove().bSerializeOutControlRotationPitch &&
      !LocalMove().bSerializeOutControlRotationYaw
    ),
    Warning,
    TEXT("<bUseClientControlRotation> is true but the autonomous proxy is not sending any control rotation components to the server.")
  )

  GMC_CLOG(
    bUseClientControlRotation && (
      ServerState_AutonomousProxy().bSerializeControlRotationRoll ||
      ServerState_AutonomousProxy().bSerializeControlRotationPitch ||
      ServerState_AutonomousProxy().bSerializeControlRotationYaw
    ),
    Warning,
    TEXT("<bUseClientControlRotation> is true but control rotation components are still being replicated back to the autonomous proxy.")
  )

  GMC_CLOG(
    1.f / ClientSendRate > MaxServerDeltaTime,
    Warning,
    TEXT("Client send interval > max server delta time, increasing max server delta time to 1 / client send rate seconds.")
  )
  MaxServerDeltaTime = FMath::Max(MaxServerDeltaTime, 1.f / ClientSendRate);

  GMC_CLOG(
    MaxClientDeltaTime > MaxServerDeltaTime,
    Warning,
    TEXT("Max client delta time > max server delta time, reducing the max client delta time to match the max server delta time.")
  )
  MaxClientDeltaTime = FMath::Min(MaxClientDeltaTime, MaxServerDeltaTime);

  if (ServerMinUpdateRate > 0)
  {
    GMC_CLOG(
      ServerMinUpdateRate > ClientSendRate,
      Warning,
      TEXT("ServerMinUpdateRate > ClientSendRate, lowering the server min update rate to match the client send rate.")
    )
    ServerMinUpdateRate = FMath::Min(ServerMinUpdateRate, ClientSendRate);
  }

  if (IsNetMode(NM_DedicatedServer) || IsNetMode(NM_ListenServer))
  {
    if (const auto World = GetWorld())
    {
      // We set the timers for all server pawns because 1) any pawn may be possessed by a client at some point 2) the pawn may not have a
      // controller assigned yet when BeginPlay runs (so we can't check the net role at this point).
      checkGMC(IsServerPawn())

      if (bVerifyClientTimestamps)
      {
        World->GetTimerManager().SetTimer(
          Server_ResetClientStrikesHandle,
          this,
          &UGenMovementReplicationComponent::Server_ResetClientStrikes,
          StrikeResetInterval,
          true,
          0.f
        );
      }

      if (FullSerializationInterval > 0.f)
      {
        World->GetTimerManager().SetTimer(
          Server_SetForceFullSerializationFlagPeriodicHandle,
          this,
          &UGenMovementReplicationComponent::Server_SetForceFullSerializationFlagPeriodic,
          FullSerializationInterval,
          true,
          -1.f
        );
      }
    }
    else
    {
      checkNoEntryGMC()
    }
  }
}

void UGenMovementReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  SCOPE_CYCLE_COUNTER(STAT_TickComponent)

  IncrementTimers(DeltaTime);

  ConsumeInputVector();

  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!ShouldRunGMCUpdates())
  {
    ClearTransientData();
    return;
  }

  ManagePrerequisiteTicks();

  ManageAdaptiveSimulationDelay();

  if (PawnOwner->IsLocallyControlled())
  {
    RefreshLocalMove();

    if (IsServerPawn())
    {
      UpdateLocallyControlledServerPawn();
    }
    else if (IsAutonomousProxy())
    {
      UpdateAutonomousProxyPawn();
    }
  }
  else if (IsRemotelyControlledServerPawn())
  {
    UpdateRemotelyControlledServerPawn();
  }

  if (IsSimulatedProxy() || IsSmoothedListenServerPawn() || IsAutonomousProxy() && !bUseClientPrediction)
  {
    SimulatePawn();
  }

  DEBUG_SHOW_PAWN_NET_ROLE
  DEBUG_SHOW_PAWN_SIMULATION_DELAY
  DEBUG_DISPLAY_LOCAL_STATS
  DEBUG_DISPLAY_NET_STATS
}

void UGenMovementReplicationComponent::IncrementTimers(float DeltaTime)
{
  if (IsNetMode(NM_Standalone))
  {
    return;
  }

  if (IsAutonomousProxy())
  {
    Client_TimeSinceLastMoveWasSent += DeltaTime;
    return;
  }

  if (IsServerPawn())
  {
    if (InterpolationMode == EInterpolationMode::AdaptiveDelay && InterpolationMethod != EInterpolationMethod::None)
    {
      if (IsSmoothedListenServerPawn())
      {
        Server_SADTimeSinceLastLocalDelayUpdate += DeltaTime;
      }
      // @attention The client parameter map is not a uproperty so there's a chance it might contain dangling references to destroyed client
      // controllers (meaning you should not try to access the controller key here).
      for (auto& Entry : Server_SADClientParameters)
      {
        Entry.Value.UpdateTimer += DeltaTime;
      }
    }
  }
}

bool UGenMovementReplicationComponent::ShouldRunGMCUpdates() const
{
  if (!IsGMCEnabled())
  {
    return false;
  }

  if (!IsValid(PawnOwner) || !GenPawnOwner)
  {
    return false;
  }

  if (!UpdatedComponent || !PawnOwner->GetRootComponent())
  {
    return false;
  }

  if (IsServerPawn() && !PawnOwner->CheckStillInWorld())
  {
    return false;
  }

  if (!IsNetMode(NM_Client))
  {
    return true;
  }

  // @attention The following checks are only relevant for clients.

  if (!GetIsReplicated() || !PawnOwner->GetIsReplicated())
  {
    // The movement component and its owner must be set to replicate.
    return false;
  }

  if (PawnOwner->IsReplicatingMovement())
  {
    // No GMC updates when using built-in movement replication.
    return false;
  }

  return true;
}

void UGenMovementReplicationComponent::UpdateLocallyControlledServerPawn()
{
  SCOPE_CYCLE_COUNTER(STAT_UpdateLocallyControlledServerPawn)

  checkGMC(IsLocallyControlledServerPawn())

  // Update the states of all rollback actors on the server before executing the current move.
  TickGenericRollbackActors(
    LocalMove().Timestamp,
    LocalMove().DeltaTime,
    LocalMove(),
    EActorRollbackContext::LocalServerPawnExecutingMove
  );

  // We are either the listen server in a network, a bot with authority or running a standalone game. Perform the movement directly.
  ExecuteMove(LocalMove(), EImmediateContext::LocalServerPawnExecutingMove);
  FillMoveWithData(LocalMove(), FMove::EStateVars::Output);

  if (IsNetworkedServer())
  {
    // Reset locks that have expired.
    Server_ResetLockedBoundInputFlags();
    Server_ResetLockedBoundData();

    // Locally controlled server pawns (listen server) and AI controlled server bots save their state so clients can simulate it.
    Server_SaveServerState(ROLE_SimulatedProxy, LocalMove());

    // Quantize the local pawn state to account for replication compression, so we remain accurately synchronised on the client.
    Server_QuantizePawnStateFrom(ServerState_SimulatedProxy());

    // Replication of the server state happens in intervals so the client usually doesn't know about all intermediate values of a
    // variable. If this is required for a certain property however we can force a net update to ensure the client is notified about
    // every change.
    if (Server_ShouldForceNetUpdate(ROLE_SimulatedProxy))
    {
      PawnOwner->ForceNetUpdate();
    }

    if (bFillStateQueueForLocalServerPawns)
    {
      AddToStateQueue(ServerState_SimulatedProxy());
    }
  }
}

void UGenMovementReplicationComponent::UpdateRemotelyControlledServerPawn()
{
  SCOPE_CYCLE_COUNTER(STAT_UpdateRemotelyControlledServerPawn)

  checkGMC(IsRemotelyControlledServerPawn())
  checkGMC(!Server_bIsExecutingRemoteMoves)

  const bool bUsesMinUpdateRate = ServerMinUpdateRate > 0;
  const int32 NumPendingMovePackets = Server_PendingMovePackets.Num();
  const bool bHasPendingMoves = NumPendingMovePackets > 0;
  if (bUsesMinUpdateRate && !bHasPendingMoves)
  {
    if (Server_ProcessProxyMove())
    {
      DEBUG_LOG_NUM_EXECUTED_REMOTE_PACKETS_PROXY_MOVE
      return;
    }
  }

  checkGMC(!Server_bIsExecutingRemoteMoves)

  if (!bHasPendingMoves)
  {
    return;
  }

  // Calculate the number of move packets to process.
  checkGMC(NumPendingMovePackets > 0)
  checkGMC(RemoteMoveProcessingChunkSize > 0)
  checkGMC(MaxNumBufferedRemoteMovePackets >= 0)
  const int32 NumExceedingPackets = FMath::Clamp(NumPendingMovePackets - MaxNumBufferedRemoteMovePackets, 0, MAX_int32);
  const int32 NumPacketsToExecute = FMath::Clamp(
    NumExceedingPackets + FMath::Clamp((NumPendingMovePackets - NumExceedingPackets) / RemoteMoveProcessingChunkSize, 1, MAX_int32),
    1, NumPendingMovePackets
  );

  // Execute the calculated number of move packets.
  checkGMC(NumPacketsToExecute > 0)
  checkGMC(NumPacketsToExecute <= NumPendingMovePackets)
  int32 NumPacketsExecuted = 0;
  do
  {
    Server_ExecuteClientMoves(Server_PendingMovePackets[0]);
    Server_PendingMovePackets.RemoveAt(0, 1, false);
    ++NumPacketsExecuted;
  }
  while (NumPacketsExecuted < NumPacketsToExecute);
  checkGMC(Server_PendingMovePackets.Num() <= MaxNumBufferedRemoteMovePackets)
  DEBUG_LOG_NUM_EXECUTED_REMOTE_PACKETS
}

void UGenMovementReplicationComponent::UpdateAutonomousProxyPawn()
{
  SCOPE_CYCLE_COUNTER(STAT_UpdateAutonomousProxyPawn)

  checkGMC(IsAutonomousProxy())

  // As a client we need to maintain a move queue for replication and replay.
  bool bStartedNewMove{false};
  bool bMoveQueueFull{false};
  const bool bMoveDiscarded = !Client_MaintainMoveQueue(LocalMove(), bStartedNewMove, bMoveQueueFull);
  checkGMC(Client_MoveQueue.Num() > 0 || bMoveDiscarded)

  // Perform client movement.
  if (Client_MoveQueue.Num() > 1 && !bMoveDiscarded)
  {
    // When this branch is entered everything is working as expected.
    auto& CurrentMove = Client_MoveQueue.Last();
    const auto& PreviousMove = Client_MoveQueue.Last(1);

    // Use a delta time value that is calculated in the same way as the server does it so we get the same integration results on the client
    // and the server. The lower bound here is the max server delta time, NOT the max client delta time.
    CurrentMove.DeltaTime = FMath::Min(CurrentMove.Timestamp - PreviousMove.Timestamp, MaxServerDeltaTime);

    // Update the states of all rollback actors on the client before executing the current move.
    TickGenericRollbackActors(
      CurrentMove.Timestamp,
      CurrentMove.DeltaTime,
      CurrentMove,
      EActorRollbackContext::LocalClientPawnExecutingReplicatedMove
    );

    // Perform the movement locally with the server delta time.
    Client_ExecuteMove(CurrentMove, EImmediateContext::LocalClientPawnExecutingReplicatedMove, bUseClientPrediction);

    // Add to pending moves.
    if (bStartedNewMove)
    {
      // If we added a new move to the queue this tick we can send the previous one (which is now complete). This is the only chance a move
      // gets to be send to the server, afterwards it will/should soon be cleared from the queue with a server update.
      Client_PendingMoves.Emplace(PreviousMove);
    }
  }
  else if (Client_MoveQueue.Num() == 1 && !bMoveDiscarded)
  {
    // Execute the movement with the local delta time because there is no previous move available. This always happens once when just
    // spawned, but if it happens frequently there is most likely a problem with the time synchronisation process.
    auto& CurrentMove = Client_MoveQueue[0];

    // Update the states of all rollback actors on the client before executing the current move.
    TickGenericRollbackActors(
      CurrentMove.Timestamp,
      CurrentMove.DeltaTime,
      CurrentMove,
      EActorRollbackContext::LocalClientPawnExecutingReplicatedMove
    );

    Client_ExecuteMove(CurrentMove, EImmediateContext::LocalClientPawnExecutingReplicatedMove, bUseClientPrediction);

    GMC_LOG(
      Verbose,
      TEXT("A client move was executed with the local delta time because there is only one move in the queue ")
      TEXT("(the client world time may be out of sync with the server world time).")
    )
  }
  else if (bMoveDiscarded)
  {
    // Update the states of all rollback actors on the client before executing the local move.
    TickGenericRollbackActors(
      LocalMove().Timestamp,
      LocalMove().DeltaTime,
      LocalMove(),
      EActorRollbackContext::LocalClientPawnExecutingDiscardedMove
    );

    // The current move was discarded which can happen occasionally after the client time was synchronised, because the move queue is full
    // or due to a major lag spike that caused the reliable buffer to fill up too much. We execute the move completely locally in this case
    // meaning it will not be replicated to the server. This may lead to a teleport for the client if a replay is triggered.
    Client_ExecuteMove(LocalMove(), EImmediateContext::LocalClientPawnExecutingDiscardedMove, bUseClientPrediction, false);

    GMC_LOG(Verbose, TEXT("A client move was discarded from the move queue and will not be replicated to the server."))
  }

  // Communicate with the server.
  if (Client_ShouldSendMoves())
  {
    // If a value in the move is different from the last one that was sent, it needs to be serialized again. Otherwise only 1 bit will be
    // sent to indicate to the server that the previously received value can be used again because it hasn't changed.
    Client_DetermineValuesToSend();
    // Clients send their moves to the server where the moves get simulated locally, the resulting state is saved, and then gets replicated
    // back to the client for verification and potentially corrections.
    Client_SendMovesToServer();
    Client_TimeSinceLastMoveWasSent = 0.f;
    DEBUG_LOG_CLIENT_SENT_MOVES
    // After sending we need to clear the array, we don't want to send the same move twice.
    Client_PendingMoves.Reset();
  }
}

void UGenMovementReplicationComponent::SimulatePawn()
{
  SCOPE_CYCLE_COUNTER(STAT_SimulatePawn)

  checkGMC(IsSimulatedProxy() || IsSmoothedListenServerPawn() || IsAutonomousProxy() && !bUseClientPrediction)
  checkGMC(!IsNetMode(NM_DedicatedServer))

  const auto World = GetWorld();
  if (!World) return;

  const float DeltaTime = World->GetDeltaSeconds();
  const bool bIsSimulatedProxy = IsSimulatedProxy();
  const bool bIsNonPredictedClient = IsAutonomousProxy() && !bUseClientPrediction;

  ESimulatedContext Context =
    bIsSimulatedProxy ? ESimulatedContext::SmoothingSimulatedProxy :
    bIsNonPredictedClient ? ESimulatedContext::SmoothingAutonomousProxy :
    ESimulatedContext::SmoothingRemoteListenServerPawn;

  // This will contain the indices of states that have been skipped by the smoothing algorithm during this iteration.
  TArray<int32> SkippedStateIndices;

  if (InterpolationFunction)
  {
    // We receive remote data only in discrete intervals and non-uniformly. To be able to display smooth movement visuals we need to "fill
    // in the gaps" i.e. approximate what happened between two received packets by using interpolation.
    SmoothMovement(Context, SkippedStateIndices);
  }
  else if (StateQueue.Num() > 0)
  {
    // No interpolation function has been configured, set the pawn directly to the most recent state that was received. While this ensures
    // the lowest latency possible, it will usually also produce undesirable choppy visuals.
    SetPawnStateNoInterpolation(Context, SkippedStateIndices);
  }

  if (StateQueue.Num() == 0)
  {
    // The state queue is empty and no valid start and target states are available.
    // @attention We do not want to overwrite the interpolation states in this case, the start and target state indices can be checked to
    // determine whether the interpolation data is current (@see IsInterpolationDataCurrent).
    CurrentStartStateIndex = CurrentTargetStateIndex = -1;
  }

  // Call the tick function for simulated proxies on a client and for smoothed pawns on a listen server.
  DEBUG_LOG_SIMULATED_TICK_ARGUMENTS
  const FState& SmoothState = bUsingExtrapolatedData ? ExtrapolatedState : InterpolatedState;
  SimulatedTick(DeltaTime, SmoothState, CurrentStartStateIndex, CurrentTargetStateIndex, SkippedStateIndices);

  // Save the timestamp of the current target state as indices might change before the next tick is executed.
  PreviousInterpolationTargetStateTimestamp =
    IsValidStateQueueIndex(CurrentTargetStateIndex) ? StateQueue[CurrentTargetStateIndex].Timestamp : -1.f;

  // Save the value of "bUsingExtrapolatedData" for the next frame.
  bWasUsingExtrapolatedDataLastFrame = bUsingExtrapolatedData;
}

void UGenMovementReplicationComponent::Client_OnRepServerState_AutonomousProxy()
{
  SCOPE_CYCLE_COUNTER(STAT_Client_OnRepServerState_AutonomousProxy)

  Client_UnpackReplicationUpdate(ServerState_AutonomousProxy());
  DEBUG_LOG_CLIENT_RECEIVED_STATE

  if (!IsAutonomousProxy())
  {
    // Can happen during possession change.
    return;
  }

  if (!ServerState_AutonomousProxy().IsValid())
  {
    // Should not happen.
    checkNoEntryGMC()
    return;
  }

  if (!IsGMCEnabled() || !PawnOwner->HasActorBegunPlay())
  {
    return;
  }

  DEBUG_LOG_MOVE_QUEUE_SIZE_BEFORE_CLEARING
  const float AckTimestamp = ServerState_AutonomousProxy().bIsUsingServerAuthPhysicsReplication ?
    ServerState_AutonomousProxy().ServerAuthPhysicsAckTimestamp :
    ServerState_AutonomousProxy().Timestamp;
  FMove SourceMove = Client_ClearAcknowledgedMoves(AckTimestamp);
  DEBUG_LOG_MOVE_QUEUE_SIZE_AFTER_CLEARING

  if (!bUseClientPrediction)
  {
    if (InterpolationMode == EInterpolationMode::MatchLatest)
    {
      SMLSaveTimestamps(ServerState_AutonomousProxy());
    }
    AddToStateQueue(ServerState_AutonomousProxy());
    return;
  }

  // Never use client prediction with server-authoritative physics replication.
  checkGMC(!IsUsingServerAuthPhysicsReplication())

  const float CurrentTime = GetTime();
  if (!SourceMove.IsValid())
  {
    // Only reset the client if we are exceeding the max wait time.
    const float ElapsedTimeSinceLastValidUpdate = CurrentTime - Client_LastValidUpdateTime;
    GMC_LOG(
      Verbose,
      TEXT("Elapsed time since last valid server state update is %f s."),
      ElapsedTimeSinceLastValidUpdate
    )
    if (MaxClientUpdateWaitTime <= 0.f || ElapsedTimeSinceLastValidUpdate > MaxClientUpdateWaitTime)
    {
      // Without a valid source move we cannot check the client state or execute a replay. We have no choice but to set the pawn state
      // directly to the received server state (which will almost certainly cause a teleport for the client). This should only occur very
      // rarely e.g. when the client wasn't able to meet the server's min required update rate (@see ServerMinUpdateRate).
      Client_AdoptServerStateDirectly();
      GMC_LOG(
        Verbose,
        TEXT("Client adopted server state directly (no valid source move for %f s > MaxClientUpdateWaitTime = %f s)."),
        ElapsedTimeSinceLastValidUpdate, MaxClientUpdateWaitTime
      )

      // @attention Do not set @see Client_LastValidUpdateTime here. It makes sense to keep updating the pawn directly after the wait time
      // was exceeded until a valid source move is available again.
    }
    return;
  }

  // We have a valid source move, set the timestamp for the last valid update.
  Client_LastValidUpdateTime = CurrentTime;

  if (Client_ShouldReplay(SourceMove))
  {
    if (Client_ShouldUseSmoothCorrections() && !Client_CorrectionBuffer.HasData())
    {
      // Save the current deviating transform of the pawn as the starting point for interpolating towards the corrected state. We don't
      // reset the start transform until the correction has been completed (i.e. an interpolation ration of 1 has been reached) even if we
      // receive more server corrections while still interpolating (since the target transform will be updated continuously with the final
      // corrected client transform).
      Client_SaveCorrectionTransform(Client_StartCorrectionTransform);
      // Set the accumulated time to 0 which indicates that a smoothed correction has started.
      checkGMC(Client_CorrectionBuffer.AccumulatedTime == -1.f)
      Client_CorrectionBuffer.AccumulatedTime = 0.f;
    }
    GMC_CLOG(
      !bAlwaysReplay,
      VeryVerbose,
      TEXT("A replay was triggered - current client state   : Velocity = %15.6f, %15.6f, %15.6f | Location = %15.6f, %15.6f, %15.6f | Rotation = %15.6f, %15.6f, %15.6f | ControlRotation = %15.6f, %15.6f, %15.6f"),
      Velocity.X,
      Velocity.Y,
      Velocity.Z,
      PawnOwner->GetActorLocation().X,
      PawnOwner->GetActorLocation().Y,
      PawnOwner->GetActorLocation().Z,
      PawnOwner->GetActorRotation().Roll,
      PawnOwner->GetActorRotation().Pitch,
      PawnOwner->GetActorRotation().Yaw,
      PawnOwner->GetControlRotation().Roll,
      PawnOwner->GetControlRotation().Pitch,
      PawnOwner->GetControlRotation().Yaw
    )
    DEBUG_NET_CORRECTION_ORIGINAL_CLIENT_LOCATION
    DEBUG_NET_CORRECTION_ORIGINAL_CLIENT_ROTATION
    DEBUG_LOG_REPLAY_CLIENT_STATE_INITIAL
    DEBUG_LOG_REPLAY_SERVER_STATE
    Client_AdoptServerStateForReplay(ServerState_AutonomousProxy().bContainsFullRepBatch, SourceMove);
    DEBUG_NET_CORRECTION_UPDATED_CLIENT_LOCATION
    DEBUG_NET_CORRECTION_UPDATED_CLIENT_ROTATION
    Client_ReplayMoves();
    DEBUG_NET_CORRECTION_REPLAYED_CLIENT_LOCATION
    DEBUG_NET_CORRECTION_REPLAYED_CLIENT_ROTATION
    DEBUG_NET_CORRECTION_DRAW_CLIENT_SHAPES
    GMC_CLOG(
      !bAlwaysReplay,
      VeryVerbose,
      TEXT("Replay finished        - corrected client state : Velocity = %15.6f, %15.6f, %15.6f | Location = %15.6f, %15.6f, %15.6f | Rotation = %15.6f, %15.6f, %15.6f | ControlRotation = %15.6f, %15.6f, %15.6f"),
      Velocity.X,
      Velocity.Y,
      Velocity.Z,
      PawnOwner->GetActorLocation().X,
      PawnOwner->GetActorLocation().Y,
      PawnOwner->GetActorLocation().Z,
      PawnOwner->GetActorRotation().Roll,
      PawnOwner->GetActorRotation().Pitch,
      PawnOwner->GetActorRotation().Yaw,
      PawnOwner->GetControlRotation().Roll,
      PawnOwner->GetControlRotation().Pitch,
      PawnOwner->GetControlRotation().Yaw
    )
    GMC_CLOG(
      !bAlwaysReplay,
      Verbose,
      TEXT("Replayed %d moves."),
      Client_MoveQueue.Num()
    )
    GMC_CLOG(
      !bUseClientLocation && (bAlwaysReplay ? !Debug_OriginalPawnLocation.Equals(Debug_ReplayedPawnLocation, 0.1f) : true),
      Verbose,
      TEXT("Replayed client location differs by %.2f cm from the original location."),
      (Debug_ReplayedPawnLocation - Debug_OriginalPawnLocation).Size()
    )
    GMC_CLOG(
      !bUseClientRotation && (bAlwaysReplay ? !Debug_OriginalPawnRotation.Equals(Debug_ReplayedPawnRotation, 0.1f) : true),
      Verbose,
      TEXT("Replayed client rotation differs by %.2f deg from the original rotation."),
      FMath::RadiansToDegrees(Debug_ReplayedPawnRotation.AngularDistance(Debug_OriginalPawnRotation))
    )
  }
}

void UGenMovementReplicationComponent::Client_OnRepServerState_SimulatedProxy()
{
  SCOPE_CYCLE_COUNTER(STAT_Client_OnRepServerState_SimulatedProxy)

  checkGMC(ServerState_SimulatedProxy().bContainsFullRepBatch)

  Client_UnpackReplicationUpdate(ServerState_SimulatedProxy());

  if (!IsSimulatedProxy())
  {
    // Can happen during possession change.
    return;
  }

  if (!ServerState_SimulatedProxy().IsValid())
  {
    // Should not happen.
    checkNoEntryGMC()
    return;
  }

  if (!IsGMCEnabled() || !PawnOwner->HasActorBegunPlay())
  {
    return;
  }

  if (InterpolationMode == EInterpolationMode::MatchLatest)
  {
    SMLSaveTimestamps(ServerState_SimulatedProxy());
  }
  AddToStateQueue(ServerState_SimulatedProxy());
}

void UGenMovementReplicationComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
  // Clear timers when this object gets destroyed.
  if (const auto World = GetWorld())
  {
    World->GetTimerManager().ClearTimer(Server_ResetClientStrikesHandle);
    World->GetTimerManager().ClearTimer(Server_SetForceFullSerializationFlagPeriodicHandle);
  }

  // Call the Blueprint EndPlay event after the replication component has been deinitialized.
  Super::EndPlay(EndPlayReason);
}

void UGenMovementReplicationComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
  Super::PreReplication(ChangedPropertyTracker);

  Server_MaintainSerializationMap();
}

void UGenMovementReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME_CONDITION(UGenMovementReplicationComponent, ServerState_AutonomousProxy_Default, COND_AutonomousOnly);
  DOREPLIFETIME_CONDITION(UGenMovementReplicationComponent, ServerState_SimulatedProxy_Default, COND_SimulatedOnly);
}

void UGenMovementReplicationComponent::EnableGMC(bool bEnable)
{
  if (bGMCEnabled == bEnable) return;

  if (bEnable)
  {
    bGMCEnabled = true;
    OnGMCEnabled();
  }
  else
  {
    bGMCEnabled = false;
    OnGMCDisabled();
  }
}

void UGenMovementReplicationComponent::OnServerAuthPhysicsSimulationToggled_Implementation(
  bool bEnabled,
  FServerAuthPhysicsSettings Settings
)
{
  checkGMC(!(bIsUsingServerAuthPhysicsReplication && bIsUsingClientAuthPhysicsReplication))

  if (IsNetMode(NM_Standalone) || IsSimulatedProxy())
  {
    return;
  }

  ClearTransientData();

  if (bEnabled)
  {
    SavedSettingsServerAuthPhysics.SaveSettings(this);

    // The server may use different interpolation settings to display the pawn locally so only apply the settings on the client.
    // @attention Make sure the parameters for the interpolation are set appropriately at this point.
    if (IsAutonomousProxy())
    {
      // @attention The values of @see bUseClientPrediction and @see bSmoothRemoteServerPawn must always be false during physics simulation
      // so don't forget to overwrite them afterwards.
      Settings.ApplySettings(this);
    }

    // These must be false regardless of the values in "Settings".
    // @attention On the server the remote role may not be assigned yet (e.g. when called from on BeginPlay) so don't do any checks here
    // that would require it (e.g. @see IsRemotelyControlledServerPawn).
    bUseClientPrediction = false;
    bSmoothRemoteServerPawn = false;

    // Reset the state buffer.
    Server_bStateBufferInitialized = false;
  }
  else
  {
    // Restore the original settings when physics simulation is toggled off.
    SavedSettingsServerAuthPhysics.ApplySettings(this);
  }
}

void UGenMovementReplicationComponent::EnableServerAuthPhysicsSimulationLocal(bool bEnable, FServerAuthPhysicsSettings Settings)
{
  if (!PawnOwner)
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  if (bIsUsingClientAuthPhysicsReplication)
  {
    GMC_LOG(
      Warning,
      TEXT("Client-authoritative physics replication is already active, cannot enable server-authoritative physics replication.")
    )
    checkGMC(false)
    return;
  }

  if (bIsUsingServerAuthPhysicsReplication == bEnable)
  {
    return;
  }

  bIsUsingServerAuthPhysicsReplication = bEnable;

  // Physics are only simulated on the server, clients will just display the replicated transform.
  RootComponent->SetSimulatePhysics(bEnable && IsServerPawn());

  OnServerAuthPhysicsSimulationToggled(bEnable, Settings);
}

bool UGenMovementReplicationComponent::IsUsingServerAuthPhysicsReplication() const
{
  checkGMC(!(bIsUsingServerAuthPhysicsReplication && bIsUsingClientAuthPhysicsReplication))
  return bIsUsingServerAuthPhysicsReplication;
}

void UGenMovementReplicationComponent::OnClientAuthPhysicsSimulationToggled_Implementation(
  bool bEnabled,
  FClientAuthPhysicsSettings Settings
)
{
  checkGMC(!(bIsUsingServerAuthPhysicsReplication && bIsUsingClientAuthPhysicsReplication))

  if (IsNetMode(NM_Standalone) || IsSimulatedProxy())
  {
    return;
  }

  ClearTransientData();

  if (bEnabled)
  {
    SavedSettingsClientAuthPhysics.SaveSettings(this);

    // @attention The value of @see bAssumeClientState must always be true during physics simulation so don't forget to overwrite it
    // afterwards.
    Settings.ApplySettings(this);

    // These must be true regardless of the value in "Settings".
    bAssumeClientState = true;
    bUseClientLocation = true;
    bUseClientRotation = true;
    bUseClientControlRotation = true;

    // Reset the state buffer.
    Server_bStateBufferInitialized = false;
  }
  else
  {
    // Restore the original settings when physics simulation is toggled off.
    SavedSettingsClientAuthPhysics.ApplySettings(this);
  }
}

void UGenMovementReplicationComponent::EnableClientAuthPhysicsSimulationLocal(bool bEnable, FClientAuthPhysicsSettings Settings)
{
  if (!PawnOwner)
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  if (bIsUsingServerAuthPhysicsReplication)
  {
    GMC_LOG(
      Warning,
      TEXT("Server-authoritative physics replication is already active, cannot enable client-authoritative physics replication.")
    )
    checkGMC(false)
    return;
  }

  if (bIsUsingClientAuthPhysicsReplication == bEnable)
  {
    return;
  }

  bIsUsingClientAuthPhysicsReplication = bEnable;

  // Each pawn simulates their own physics. For clients this means that the server will just assume the received transform.
  RootComponent->SetSimulatePhysics(bEnable && PawnOwner->IsLocallyControlled());

  OnClientAuthPhysicsSimulationToggled(bEnable, Settings);
}

bool UGenMovementReplicationComponent::IsUsingClientAuthPhysicsReplication() const
{
  checkGMC(!(bIsUsingServerAuthPhysicsReplication && bIsUsingClientAuthPhysicsReplication))
  return bIsUsingClientAuthPhysicsReplication;
}

void UGenMovementReplicationComponent::ClearTransientData()
{
  // @attention Do not clear the local move or the server states.
  // @attention Keep clearing every tick because data can still arrive over the network after disabling the system locally.

  Client_MoveQueue.Reset();
  Client_PendingMoves.Reset();
  Client_LastSignificantMove = FMove{};

  StateBuffer() = FState{};
  Server_bStateBufferInitialized = false;

  RollbackRestoreState = FState{};
  RollbackRestoreTransform_SimulatedRoot = FTransform{};

  StateQueue.Reset();
  InterpolatedState = FState{};
  ExtrapolatedState = FState{};
  bUsingExtrapolatedData = false;
  CurrentStartStateIndex = -1;
  CurrentTargetStateIndex = -1;
  PreviousInterpolationTargetStateTimestamp = 0.f;
  AccumulatedFullSimulatedExtrapolationTime = 0.f;

  SMLUpdateTime = 0.f;
  SMLSimulationTime = 0.f;
  SMLTargetStateTimestamp = 0.f;
  SMLTargetDelta = 0.f;
  SMLStartState = FState{};

  Server_SADClientParameters.Reset();
  Client_SADLocalParameters = TStaticArray<FSADParameters, SAD_NUM_SAVED_PARAMS>();

  // Make sure any offset of the simulated root from smoothing is reset.
  if (SimulatedRootComponent)
  {
    SimulatedRootComponent->SetRelativeLocationAndRotation(
      FVector::ZeroVector,
      FRotator::ZeroRotator,
      false,
      nullptr,
      ETeleportType::TeleportPhysics
    );
  }
}

void UGenMovementReplicationComponent::ManagePrerequisiteTicks()
{
  SCOPE_CYCLE_COUNTER(STAT_ManagePrerequisiteTicks)

  // If this pawn has a local player controller it must tick before the replication component to update the saved rotation input
  // (@see AGenPlayerController::SavedRotationInput).
  if (const auto GenPC = Cast<AGenPlayerController>(PawnOwner->GetController()))
  {
    if (GenPC->IsLocalController()) AddTickPrerequisiteActor(GenPC);
  }

  if (!IsClientPawn())
  {
    return;
  }

  // On the client the local player controller must tick before the replication components of all pawns to update the world time.
  if (const auto LocalGenPC = GEngine ? Cast<AGenPlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld())) : nullptr)
  {
    AddTickPrerequisiteActor(LocalGenPC);
  }

  // Gather all actors of type AGenPawn.
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenPawn::StaticClass(), Actors);

  if (IsSimulatedProxy())
  {
    // A simulated proxy could have been previously possessed by the client in which case we need to remove the prerequisite ticks from the
    // unpossessed pawn as they would otherwise form a cycle.
    // @note AController::OnUnPossess is not called on clients so we need to do it like this.
    for (const auto Actor : Actors)
    {
      const auto GenPawn = Cast<AGenPawn>(Actor);
      check(GenPawn)
      if (const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent()))
      {
        RemoveTickPrerequisiteComponent(ReplicationComponent);
      }
    }
  }
  else if (IsAutonomousProxy())
  {
    // All simulated proxy replication components must tick before the replication component of the autonomous proxy. This guarantees that
    // the interpolation data of smoothed pawns is up-to-date when accessed from the locally controlled pawn, which is particularly
    // important for pawn rollback.
    for (const auto Actor : Actors)
    {
      const auto GenPawn = Cast<AGenPawn>(Actor);
      check(GenPawn)
      if (GenPawn->GetLocalRole() == ROLE_SimulatedProxy)
      {
        if (const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent()))
        {
          AddTickPrerequisiteComponent(ReplicationComponent);
        }
      }
    }
  }
}

void UGenMovementReplicationComponent::ManageAdaptiveSimulationDelay()
{
  SCOPE_CYCLE_COUNTER(STAT_ManageAdaptiveSimulationDelay)

  if (
    IsNetMode(NM_Standalone)
    || InterpolationMode != EInterpolationMode::AdaptiveDelay
    || InterpolationMethod == EInterpolationMethod::None
  )
  {
    return;
  }

  if (IsClientPawn())
  {
    Client_SADUpdateLocalDelay();
    return;
  }

  if (IsServerPawn())
  {
    if (IsSmoothedListenServerPawn())
    {
      Server_SADUpdateLocalDelay();
    }
    Server_SADUpdateClientDelays();
    return;
  }
}

void UGenMovementReplicationComponent::RefreshLocalMove()
{
  SCOPE_CYCLE_COUNTER(STAT_RefreshLocalMove)

  if (const auto World = GetWorld())
  {
    // Clamp input vector to max value.
    FVector InputVector = GetLastInputVector();
    InputVector = FVector(
      FMath::Clamp(InputVector.X, (float)-MAX_DIRECTION_INPUT, (float)MAX_DIRECTION_INPUT),
      FMath::Clamp(InputVector.Y, (float)-MAX_DIRECTION_INPUT, (float)MAX_DIRECTION_INPUT),
      FMath::Clamp(InputVector.Z, (float)-MAX_DIRECTION_INPUT, (float)MAX_DIRECTION_INPUT)
    );

    // Get the view rotation input from the player controller.
    // @attention For pawn's without a controller of type <AGenPlayerController> (e.g. bots) the view rotation input is not available (but
    // they also don't receive any with Unreal's default AI implementations).
    FRotator RotationInput{0};
    if (const auto GenPlayerController = Cast<AGenPlayerController>(PawnOwner->GetController()))
    {
      RotationInput = GenPlayerController->GetRotationInput();
      checkCodeGMC(
        // The rotation input should have been clamped only if it is going to be compressed.
        if (LocalMove().RotationInputQuantize < ESizeQuantization::None)
        {
          checkGMC(RotationInput.Roll  <= MAX_ROTATION_INPUT && RotationInput.Roll  >= -MAX_ROTATION_INPUT)
          checkGMC(RotationInput.Pitch <= MAX_ROTATION_INPUT && RotationInput.Pitch >= -MAX_ROTATION_INPUT)
          checkGMC(RotationInput.Yaw   <= MAX_ROTATION_INPUT && RotationInput.Yaw   >= -MAX_ROTATION_INPUT)
        }
      )
    }

    // Update the local move.
    LocalMove().Timestamp = GetTime();
    LocalMove().DeltaTime = World->GetDeltaSeconds();
    LocalMove().InputVector = InputVector;
    LocalMove().RotationInput = RotationInput;
    LocalMove().bInputFlag1 = GenPawnOwner->InputFlag1();
    LocalMove().bInputFlag2 = GenPawnOwner->InputFlag2();
    LocalMove().bInputFlag3 = GenPawnOwner->InputFlag3();
    LocalMove().bInputFlag4 = GenPawnOwner->InputFlag4();
    LocalMove().bInputFlag5 = GenPawnOwner->InputFlag5();
    LocalMove().bInputFlag6 = GenPawnOwner->InputFlag6();
    LocalMove().bInputFlag7 = GenPawnOwner->InputFlag7();
    LocalMove().bInputFlag8 = GenPawnOwner->InputFlag8();
    LocalMove().bInputFlag9 = GenPawnOwner->InputFlag9();
    LocalMove().bInputFlag10 = GenPawnOwner->InputFlag10();
    LocalMove().bInputFlag11 = GenPawnOwner->InputFlag11();
    LocalMove().bInputFlag12 = GenPawnOwner->InputFlag12();
    LocalMove().bInputFlag13 = GenPawnOwner->InputFlag13();
    LocalMove().bInputFlag14 = GenPawnOwner->InputFlag14();
    LocalMove().bInputFlag15 = GenPawnOwner->InputFlag15();
    LocalMove().bInputFlag16 = GenPawnOwner->InputFlag16();
    LocalMove().InVelocity = GetVelocity();
    LocalMove().InLocation = PawnOwner->GetActorLocation();
    LocalMove().InRotation = PawnOwner->GetActorRotation();
    LocalMove().InControlRotation = PawnOwner->GetControlRotation();
    LocalMove().InInputMode = GenPawnOwner->GetInputMode();
    // Save the values of data bound to the pre-replicated variables of the move. Mainly used for move combining and replay on the client.
    SaveBoundDataToMove(LocalMove(), FMove::EStateVars::Input);
  }
}

void UGenMovementReplicationComponent::LoadMove(const FMove& MoveToLoad, EImmediateContext Context)
{
  SCOPE_CYCLE_COUNTER(STAT_LoadMove)

  // Set the pawn state.
  SetVelocity(MoveToLoad.InVelocity);
  // If the client-authoritative values of the location, rotation and/or control rotation are used they may contain NANs.
  PawnOwner->SetActorLocation(GetValidActorLocation(MoveToLoad.InLocation), false, nullptr, ETeleportType::TeleportPhysics);
  PawnOwner->SetActorRotation(GetValidActorRotation(MoveToLoad.InRotation), ETeleportType::TeleportPhysics);
  if (const auto Controller = PawnOwner->GetController())
  {
    Controller->SetControlRotation(GetValidControlRotation(MoveToLoad.InControlRotation));
  }
  LoadInInputModeFromMove(MoveToLoad);

  // Update bound member variables.
  LoadBoundInputFlagsFromMove(MoveToLoad);
  LoadInBoundDataFromMove(MoveToLoad);

  OnImmediateStateLoaded(Context);
}

FVector UGenMovementReplicationComponent::ExecuteMove(const FMove& Move, EImmediateContext Context)
{
  SCOPE_CYCLE_COUNTER(STAT_ExecuteMove)

  checkGMC(!IsSimulatedProxy())
  if (Move.DeltaTime < MIN_DELTA_TIME) return FVector{0};

  // Setup the correct start state for executing the move.
  LoadMove(Move, Context);

  // Create a mutable copy of the move so we can update the delta time and velocity per iteration, and possibly overwrite NANs for move
  // execution.
  FMove IterationMove = Move;
  if (IsRemotelyControlledServerPawn())
  {
    if (bEnsureValidMoveData)
    {
      // On the server, move properties that have serialization disabled and are not sent by the client are marked with NAN during unpacking.
      // However, the information whether a variable was actually replicated by the client is rarely used during move execution and the
      // expectation is usually that non-replicated values are just 0. This behaviour can be enforced by setting "bEnsureValidMoveData" to
      // true.
      // @attention This is intentionally done just for the temporary iteration move because the server still requires non-serialized values
      // to be marked with NAN after the move was executed.
      Server_EnsureValidMoveData(IterationMove);
    }

    Server_PreRemoteMoveExecution(IterationMove);
  }
  else if (IsReplaying())
  {
    checkGMC(IsAutonomousProxy())
    Client_PreReplayMoveExecution(IterationMove);
  }

  int32 Iterations{0};
  float RemainingTime{Move.DeltaTime};
  float& SubDeltaTime{IterationMove.DeltaTime};
  FVector& IterationVelocity{IterationMove.InVelocity};
  FVector& IterationLocation{IterationMove.InLocation};
  FRotator& IterationRotation{IterationMove.InRotation};
  FRotator& IterationControlRotation{IterationMove.InControlRotation};
  EInputMode& IterationInputMode{IterationMove.InInputMode};
  FVector LocationDelta{0};
  while (RemainingTime >= MIN_DELTA_TIME)
  {
    ++Iterations;
    SubDeltaTime = CalculateSubDeltaTime(Iterations, RemainingTime, MaxTimeStep, MaxIterations);
    // The remaining time will eventually be zero and break the loop, at the latest when we have reached the maximum number of iterations.
    RemainingTime -= SubDeltaTime;

    DEBUG_LOG_MOVE_BEFORE_ITERATION
    UpdatePhysicsVelocity();
    ReplicatedTick(IterationMove, Iterations, RemainingTime >= MIN_DELTA_TIME);
    UpdateComponentVelocity();
    IterationVelocity = Velocity;
    IterationLocation = PawnOwner->GetActorLocation();
    IterationRotation = PawnOwner->GetActorRotation();
    IterationControlRotation = PawnOwner->GetControlRotation();
    IterationInputMode = GenPawnOwner->GetInputMode();
    LocationDelta = IterationLocation - Move.InLocation;
    DEBUG_LOG_MOVE_AFTER_ITERATION
  }
  return LocationDelta;
}

float UGenMovementReplicationComponent::CalculateSubDeltaTime(
  int32 Iterations,
  float RemainingTime,
  float InMaxTimeStep,
  int32 InMaxIterations
) const
{
  checkGMC(Iterations > 0)
  float SubDeltaTime = 0.f;
  if (RemainingTime > InMaxTimeStep)
  {
    if (Iterations < InMaxIterations)
    {
      // The remaining time is larger than the max allowed time step and we have iterations left. Dividing the remaining time by 2 gives us
      // equal time steps for the last two iterations.
      SubDeltaTime = FMath::Min(InMaxTimeStep, RemainingTime / 2.f);
    }
    else
    {
      // This is the last iteration, use all the remaining time. If the max iteration count is 1 this will be the same as the original delta
      // time.
      SubDeltaTime = RemainingTime;
      GMC_CLOG(
        InMaxIterations > 1,
        Verbose,
        TEXT("Reached maximum amount of %d iterations for move execution with %f s > max time step (%f s) remaining."),
        InMaxIterations,
        RemainingTime,
        InMaxTimeStep
      );
    }
  }
  else
  {
    // Remaining time is already lower than the max allowed time step.
    SubDeltaTime = RemainingTime;
  }
  return SubDeltaTime;
}

void UGenMovementReplicationComponent::UpdatePhysicsVelocity()
{
  if (!PawnOwner)
  {
    return;
  }

  const auto RootComponent = PawnOwner->GetRootComponent();
  if (!RootComponent)
  {
    return;
  }

  if (RootComponent->IsSimulatingPhysics())
  {
    Velocity = RootComponent->GetComponentVelocity();
  }
}

void UGenMovementReplicationComponent::FillMoveWithData(FMove& Move, FMove::EStateVars VarsToFill) const
{
  SCOPE_CYCLE_COUNTER(STAT_FillMoveWithData)

  if (VarsToFill == FMove::EStateVars::Input)
  {
    Move.InVelocity = GetVelocity();
    // When using the client-authoritative state values we want to keep the in-members of a move during a replay even when they are not
    // being replicated. By definition these cannot be "wrong" and they may be read by the movement logic.
    if (!IsReplaying() || !bUseClientLocation) Move.InLocation = PawnOwner->GetActorLocation();
    if (!IsReplaying() || !bUseClientRotation) Move.InRotation = PawnOwner->GetActorRotation();
    if (!IsReplaying() || !bUseClientControlRotation) Move.InControlRotation = PawnOwner->GetControlRotation();
    Move.InInputMode = GenPawnOwner->GetInputMode();
    SaveBoundDataToMove(Move, FMove::EStateVars::Input);
    return;
  }
  if (VarsToFill == FMove::EStateVars::Output)
  {
    Move.OutVelocity = GetVelocity();
    Move.OutLocation = PawnOwner->GetActorLocation();
    Move.OutRotation = PawnOwner->GetActorRotation();
    Move.OutControlRotation = PawnOwner->GetControlRotation();
    Move.OutInputMode = GenPawnOwner->GetInputMode();
    SaveBoundDataToMove(Move, FMove::EStateVars::Output);
    return;
  }
  checkNoEntryGMC()
}

FVector UGenMovementReplicationComponent::GetValidVector(const FVector& UnpackedVector, const FVector& CompletiveVector)
{
  checkGMC(!CompletiveVector.ContainsNaN())
  FVector NewVector = CompletiveVector;
  if (Rep_IsValid(UnpackedVector.X)) NewVector.X = UnpackedVector.X;
  if (Rep_IsValid(UnpackedVector.Y)) NewVector.Y = UnpackedVector.Y;
  if (Rep_IsValid(UnpackedVector.Z)) NewVector.Z = UnpackedVector.Z;
  return NewVector;
}

FRotator UGenMovementReplicationComponent::GetValidRotator(const FRotator& UnpackedRotator, const FRotator& CompletiveRotator)
{
  checkGMC(!CompletiveRotator.ContainsNaN())
  FRotator NewRotator = CompletiveRotator;
  if (Rep_IsValid(UnpackedRotator.Roll)) NewRotator.Roll = UnpackedRotator.Roll;
  if (Rep_IsValid(UnpackedRotator.Pitch)) NewRotator.Pitch = UnpackedRotator.Pitch;
  if (Rep_IsValid(UnpackedRotator.Yaw)) NewRotator.Yaw = UnpackedRotator.Yaw;
  return NewRotator;
}

FVector UGenMovementReplicationComponent::GetValidInputVector(const FVector& UnpackedInputVector, const FVector& CompletiveInputVector)
{
  checkGMC(!CompletiveInputVector.ContainsNaN())
  FVector NewInputVector = CompletiveInputVector;
  if (Rep_IsValid(UnpackedInputVector.X)) NewInputVector.X = UnpackedInputVector.X;
  if (Rep_IsValid(UnpackedInputVector.Y)) NewInputVector.Y = UnpackedInputVector.Y;
  if (Rep_IsValid(UnpackedInputVector.Z)) NewInputVector.Z = UnpackedInputVector.Z;
  return NewInputVector;
}

FRotator UGenMovementReplicationComponent::GetValidRotationInput(
  const FRotator& UnpackedRotationInput,
  const FRotator& CompletiveRotationInput
)
{
  checkGMC(!CompletiveRotationInput.ContainsNaN())
  FRotator NewRotationInput = CompletiveRotationInput;
  if (Rep_IsValid(UnpackedRotationInput.Roll)) NewRotationInput.Roll = UnpackedRotationInput.Roll;
  if (Rep_IsValid(UnpackedRotationInput.Pitch)) NewRotationInput.Pitch = UnpackedRotationInput.Pitch;
  if (Rep_IsValid(UnpackedRotationInput.Yaw)) NewRotationInput.Yaw = UnpackedRotationInput.Yaw;
  return NewRotationInput;
}

FVector UGenMovementReplicationComponent::GetValidVelocity(const FVector& UnpackedVelocity) const
{
  FVector NewVelocity = GetVelocity();
  if (Rep_IsValid(UnpackedVelocity.X)) NewVelocity.X = UnpackedVelocity.X;
  if (Rep_IsValid(UnpackedVelocity.Y)) NewVelocity.Y = UnpackedVelocity.Y;
  if (Rep_IsValid(UnpackedVelocity.Z)) NewVelocity.Z = UnpackedVelocity.Z;
  return NewVelocity;
}

FVector UGenMovementReplicationComponent::GetValidActorLocation(const FVector& UnpackedLocation) const
{
  if (!PawnOwner) return FVector{0};
  FVector NewLocation = PawnOwner->GetActorLocation();
  if (Rep_IsValid(UnpackedLocation.X)) NewLocation.X = UnpackedLocation.X;
  if (Rep_IsValid(UnpackedLocation.Y)) NewLocation.Y = UnpackedLocation.Y;
  if (Rep_IsValid(UnpackedLocation.Z)) NewLocation.Z = UnpackedLocation.Z;
  return NewLocation;
}

FRotator UGenMovementReplicationComponent::GetValidActorRotation(const FRotator& UnpackedRotation) const
{
  if (!PawnOwner) return FRotator{0};
  FRotator NewRotation = PawnOwner->GetActorRotation();
  if (Rep_IsValid(UnpackedRotation.Roll)) NewRotation.Roll = UnpackedRotation.Roll;
  if (Rep_IsValid(UnpackedRotation.Pitch)) NewRotation.Pitch = UnpackedRotation.Pitch;
  if (Rep_IsValid(UnpackedRotation.Yaw)) NewRotation.Yaw = UnpackedRotation.Yaw;
  return NewRotation;
}

FRotator UGenMovementReplicationComponent::GetValidControlRotation(const FRotator& UnpackedControlRotation) const
{
  if (!PawnOwner) return FRotator{0};
  FRotator NewControlRotation = PawnOwner->GetControlRotation();
  if (Rep_IsValid(UnpackedControlRotation.Roll)) NewControlRotation.Roll = UnpackedControlRotation.Roll;
  if (Rep_IsValid(UnpackedControlRotation.Pitch)) NewControlRotation.Pitch = UnpackedControlRotation.Pitch;
  if (Rep_IsValid(UnpackedControlRotation.Yaw)) NewControlRotation.Yaw = UnpackedControlRotation.Yaw;
  return NewControlRotation;
}

void UGenMovementReplicationComponent::PackBoolsToByte(
  uint8& OutByte,
  bool InBit1,
  bool InBit2,
  bool InBit3,
  bool InBit4,
  bool InBit5,
  bool InBit6,
  bool InBit7,
  bool InBit8
)
{
  OutByte = 0;
  OutByte |= static_cast<uint8>(InBit1);
  OutByte |= static_cast<uint8>(InBit2) << 1;
  OutByte |= static_cast<uint8>(InBit3) << 2;
  OutByte |= static_cast<uint8>(InBit4) << 3;
  OutByte |= static_cast<uint8>(InBit5) << 4;
  OutByte |= static_cast<uint8>(InBit6) << 5;
  OutByte |= static_cast<uint8>(InBit7) << 6;
  OutByte |= static_cast<uint8>(InBit8) << 7;
}

void UGenMovementReplicationComponent::UnpackBoolsFromByte(
  const uint8& InByte,
  bool& OutBit1,
  bool& OutBit2,
  bool& OutBit3,
  bool& OutBit4,
  bool& OutBit5,
  bool& OutBit6,
  bool& OutBit7,
  bool& OutBit8
)
{
  OutBit1 = InByte & 0x1;
  OutBit2 = InByte & 0x2;
  OutBit3 = InByte & 0x4;
  OutBit4 = InByte & 0x8;
  OutBit5 = InByte & 0x10;
  OutBit6 = InByte & 0x20;
  OutBit7 = InByte & 0x40;
  OutBit8 = InByte & 0x80;
}

void UGenMovementReplicationComponent::SaveLocalPawnState(FState& OutState) const
{
  SCOPE_CYCLE_COUNTER(STAT_SaveLocalPawnState)

  OutState = FState();
  const auto GenPawn = Cast<AGenPawn>(PawnOwner);
  if (!GenPawn) return;

  OutState.Timestamp = GetTime();
  OutState.Velocity = GetVelocity();
  OutState.Location = GenPawn->GetActorLocation();
  OutState.Rotation = GenPawn->GetActorRotation();
  OutState.ControlRotation = GenPawn->GetControlRotation();
  OutState.InputMode = GenPawn->GetInputMode();
  SaveBoundInputFlagsToState(OutState);
  SaveBoundDataToState(OutState);
}

void UGenMovementReplicationComponent::SetLocalPawnState(const FState& State, bool bSkipBoneUpdate, EImmediateContext Context)
{
  SCOPE_CYCLE_COUNTER(STAT_SetLocalPawnState)

  const auto GenPawn = Cast<AGenPawn>(PawnOwner);
  if (!GenPawn) return;

  SetPawnState(State, UpdateAll, bSkipBoneUpdate);
  LoadInputModeFromState(State);
  LoadBoundInputFlagsFromState(State);
  LoadBoundDataFromState(State);
  OnImmediateStateLoaded(Context);
}

void UGenMovementReplicationComponent::SetReplicatedPawnState(
  const FState& SmoothState,
  const FState& StartState,
  const FState& TargetState,
  bool bSkipBoneUpdate,
  ESimulatedContext Context
)
{
  SCOPE_CYCLE_COUNTER(STAT_SetReplicatedPawnState)

  if (!bSmoothCollisionLocation && !bSmoothCollisionRotation)
  {
    SetPawnState(TargetState, UpdateLocationRoot | UpdateRotationRoot, bSkipBoneUpdate);
    SetPawnState(SmoothState, UpdateLocationSimulatedRoot | UpdateRotationSimulatedRoot, bSkipBoneUpdate);
  }
  else if (!bSmoothCollisionLocation && bSmoothCollisionRotation)
  {
    SetPawnState(TargetState, UpdateLocationRoot, bSkipBoneUpdate);
    SetPawnState(SmoothState, UpdateLocationSimulatedRoot | UpdateRotationRoot, bSkipBoneUpdate);
  }
  else if (bSmoothCollisionLocation && !bSmoothCollisionRotation)
  {
    SetPawnState(TargetState, UpdateRotationRoot, bSkipBoneUpdate);
    SetPawnState(SmoothState, UpdateLocationRoot | UpdateRotationSimulatedRoot, bSkipBoneUpdate);
  }
  else if (bSmoothCollisionLocation && bSmoothCollisionRotation)
  {
    SetPawnState(
      SmoothState,
      UpdateLocationRoot | UpdateLocationSimulatedRoot | UpdateRotationRoot | UpdateRotationSimulatedRoot,
      bSkipBoneUpdate
    );
  }

  SetPawnState(SmoothState, UpdateVelocity, bSkipBoneUpdate);

  SetPawnState(SmoothState, UpdateControlRotation, bSkipBoneUpdate);
  if (IsSimulatedProxy()) SmoothedControlRotation = ControlRotationToLocal(GetValidControlRotation(SmoothState.ControlRotation));

  // Update bound values that are set to be serialized.
  LoadReplicatedInputModeFromState(SmoothState);
  LoadReplicatedBoundInputFlagsFromState(SmoothState);
  LoadReplicatedBoundDataFromState(SmoothState);
  OnSimulatedStateLoaded(SmoothState, StartState, TargetState, Context);
}

FState& UGenMovementReplicationComponent::GetServerStateFromRole(ENetRole RecipientRole)
{
  checkfGMC(
    RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy,
    TEXT("The recipient must be an autonomous or simulated proxy.")
  )
  return RecipientRole == ROLE_AutonomousProxy ? ServerState_AutonomousProxy() : ServerState_SimulatedProxy();
}

const FState& UGenMovementReplicationComponent::GetServerStateFromRole(ENetRole RecipientRole) const
{
  checkfGMC(
    RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy,
    TEXT("The recipient must be an autonomous or simulated proxy.")
  )
  return RecipientRole == ROLE_AutonomousProxy ? ServerState_AutonomousProxy() : ServerState_SimulatedProxy();
}

FState& UGenMovementReplicationComponent::GetLastServerStateFromRole(ENetRole RecipientRole)
{
  checkfGMC(
    RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy,
    TEXT("The recipient must be an autonomous or simulated proxy.")
  )
  return RecipientRole == ROLE_AutonomousProxy ? ServerState_AutonomousProxy_Buffered() : ServerState_SimulatedProxy_Buffered();
}

const FState& UGenMovementReplicationComponent::GetLastServerStateFromRole(ENetRole RecipientRole) const
{
  checkfGMC(
    RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy,
    TEXT("The recipient must be an autonomous or simulated proxy.")
  )
  return RecipientRole == ROLE_AutonomousProxy ? ServerState_AutonomousProxy_Buffered() : ServerState_SimulatedProxy_Buffered();
}

void UGenMovementReplicationComponent::SaveInputModeToMove(FMove& Move, FMove::EStateVars VarsToSave) const
{
  check(GenPawnOwner)
  if (VarsToSave == FMove::EStateVars::Input)
  {
    Move.InInputMode = GenPawnOwner->GetInputMode();
    return;
  }
  if (VarsToSave == FMove::EStateVars::Output)
  {
    Move.OutInputMode = GenPawnOwner->GetInputMode();
    return;
  }
  checkNoEntryGMC()
}

void UGenMovementReplicationComponent::SaveInputModeToState(FState& State) const
{
  check(GenPawnOwner)
  State.InputMode = GenPawnOwner->GetInputMode();
}

void UGenMovementReplicationComponent::LoadInInputModeFromMove(const FMove& Move)
{
  check(GenPawnOwner)
  GenPawnOwner->SetInputMode(Move.InInputMode);
}

void UGenMovementReplicationComponent::LoadInputModeFromState(const FState& State)
{
  check(GenPawnOwner)
  GenPawnOwner->SetInputMode(State.InputMode);
}

void UGenMovementReplicationComponent::LoadReplicatedInputModeFromState(const FState& State)
{
  checkGMC(State.RecipientRole == ROLE_SimulatedProxy ?
    ServerState_SimulatedProxy().bSerializeInputMode == State.bSerializeInputMode : true)
  checkGMC(State.RecipientRole == ROLE_AutonomousProxy ?
    ServerState_AutonomousProxy().bSerializeInputMode == State.bSerializeInputMode : true)
  if (State.bSerializeInputMode)
  {
    GenPawnOwner->SetInputMode(State.InputMode);
  }
}

FString UGenMovementReplicationComponent::GetInputModeAsString(EInputMode InputMode) const
{
  switch (InputMode)
  {
    case EInputMode::None: return TEXT("None");
    case EInputMode::AllRelative: return TEXT("AllRelative");
    case EInputMode::AbsoluteZ: return TEXT("AbsoluteZ");
    case EInputMode::AllAbsolute: return TEXT("AllAbsolute");
    case EInputMode::MAX: return TEXT("MAX");
    default: checkNoEntryGMC();
  }
  checkNoEntryGMC()
  return "";
}

void UGenMovementReplicationComponent::Server_SendMoves_Implementation(const TArray<FMove>& RemoteMoves)
{
  // @attention Do not add anything else to this function (potential custom serialization override).
  Server_ProcessClientMoves(RemoteMoves);
}

bool UGenMovementReplicationComponent::Server_SendMoves_Validate(const TArray<FMove>& RemoteMoves)
{
  // @attention Do not add anything else to this function (potential custom serialization override).
  return Server_ValidateRemoteMoves(RemoteMoves);
}

void UGenMovementReplicationComponent::Server_ProcessClientMoves(const TArray<FMove>& RemoteMoves)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_ProcessClientMoves)

  checkGMC(RemoteMoves.Num() > 0)

  const int32 NumRemoteMoves = RemoteMoves.Num();
  TArray<FMove> ClientMoves;
  ClientMoves.Reset(NumRemoteMoves);
  for (int32 Index = 0; Index < NumRemoteMoves; ++Index)
  {
    // Unpacking should happen regardless of whether the client move will actually be processed or not. This ensures that the client and the
    // server are always on the same page about what data was sent and received.
    const FMove UnpackedMove = Server_UnpackClientMove(RemoteMoves[Index]);

    // Discard moves that are older than the latest timestamp in the state queue. This can happen if proxy moves with a newer timestamp were
    // created by the server because the client wasn't able to reach the min required update rate (@see ServerMinUpdateRate).
    if (StateQueue.Num() == 0 || UnpackedMove.Timestamp > StateQueue.Last().Timestamp)
    {
      ClientMoves.Emplace(UnpackedMove);
    }
  }

  // Verify the timestamps of the moves that the client sent. If they are determined to be not valid and the client received more strikes
  // than allowed, the moves won't be executed. The strikes get reset periodically with a timer function (@see Server_ResetClientStrikes).
  bool bClientCredible = true;
  if (bVerifyClientTimestamps)
  {
    // The unpacked client move array could potentially be empty so pass the raw move array here (which will never be empty).
    if (!Server_VerifyTimestamps(RemoteMoves))
    {
      ++Server_ClientStrikeCount;
      if (Server_ClientStrikeCount > MaxStrikeCount)
      {
        // Client exceeds the max strike count and the move timestamps were determined to be not valid, don't execute this batch.
        bClientCredible = false;
        Server_bLastClientMoveWasValid = false;
        Server_HandleConspicuousClient();
        GMC_LOG(
          Warning,
          TEXT("Remote moves were not valid and client is exceeding the max strike count within the current verification interval ")
          TEXT("(current strike count = %d)."),
          Server_ClientStrikeCount
        )
      }
    }
  }

  // Remote moves will only be enqueued for execution if the client is still in good standing.
  if (bClientCredible && ClientMoves.Num() > 0)
  {
    Server_PendingMovePackets.Emplace(MoveTemp(ClientMoves));
  }
}

void UGenMovementReplicationComponent::Server_ExecuteClientMoves(TArray<FMove>& ClientMoves, bool bIsProxyMove)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_ExecuteClientMoves)

  checkGMC(!Server_bIsExecutingRemoteMoves)
  checkGMC(ClientMoves.Num() > 0)

  // Set the timestamp to enforce @see ServerMinUpdateRate.
  Server_LastRemotePawnUpdateTime = GetTime();

  if (!IsGMCEnabled())
  {
    // Moves can still arrive after the GMC was disabled locally.
    return;
  }

  if (!Cast<APlayerController>(PawnOwner->GetController()))
  {
    // Do not execute moves on pawns that do not have a player controller (i.e. owning connection) anymore. This can happen when switching
    // pawns as the remote moves can arrive after the pawn was already unpossessed on the server.
    return;
  }

  // @attention No more early returns after this point.

  Server_bIsExecutingRemoteMoves = true;

  // Reset locks that have expired prior to entering the move execution loop.
  Server_ResetLockedBoundInputFlags();
  Server_ResetLockedBoundData();

  if (IsSmoothedListenServerPawn())
  {
    // Initialize the state buffer the first time we swap values so we don't override the pawn state with default constructor values.
    if (!Server_bStateBufferInitialized) Server_InitializeStateBuffer();
    // Set the pawn to its un-smoothed state for client movement simulation.
    Server_SwapStateBuffer(EImmediateContext::UnSmoothingRemoteListenServerPawn);
  }

  // Rollback only makes sense for predicted clients and actual client moves (not proxy moves).
  const bool bShouldRollBackServerPawns = bRollbackServerPawns && bUseClientPrediction && !bIsProxyMove;
  const bool bShouldRollBackGenericServerActors = bRollbackGenericServerActors && bUseClientPrediction && !bIsProxyMove;

  TArray<AGenPawn*> RollbackPawnList;
  if (bShouldRollBackServerPawns)
  {
    RollbackPawnList = GatherRollbackPawns();
  }

  TArray<AGenRollbackActor*> GenericRollbackActorList;
  if (bShouldRollBackGenericServerActors)
  {
    GenericRollbackActorList = GatherGenericRollbackActors();
    BufferGenericRollbackActorStates(GenericRollbackActorList, EActorRollbackContext::RemoteServerPawnExecutingMove);
  }

  Server_PreRemoteMovesProcessing();

  for (int32 Index = 0; Index < ClientMoves.Num(); ++Index)
  {
    // Fill the move input with the current local state. This must happen per iteration because the output of the current move will be the
    // input for the next move.
    Server_SetClientMoveInput(ClientMoves[Index]);

    // The client move to execute.
    const auto& ClientMove = ClientMoves[Index];
    DEBUG_LOG_SERVER_RECEIVED_MOVES

    if (bShouldRollBackServerPawns)
    {
      // Roll back all other pawns for move execution.
      RollbackPawns(ClientMove.Timestamp, RollbackPawnList, ESimulatedContext::RollingBackServerPawn);
    }

    if (bShouldRollBackGenericServerActors)
    {
      // Roll back all generic actors for move execution.
      RollbackGenericActors(
        ClientMove.Timestamp,
        ClientMove.DeltaTime,
        ClientMove,
        GenericRollbackActorList,
        EActorRollbackContext::RemoteServerPawnExecutingMove
      );
    }

    // Move the client's pawn on the server.
    ExecuteMove(ClientMove, EImmediateContext::RemoteServerPawnExecutingMove);
    DEBUG_LOG_SERVER_EXECUTED_MOVE_RAW

    GMC_CLOG(
      bUseClientLocation &&
      !GetValidActorLocation(ClientMove.InLocation).Equals(PawnOwner->GetActorLocation(), 0.01f),
      Warning,
      TEXT("<bUseClientLocation> is true but the pawn's location changed during move execution on the server: ")
      TEXT("PreMoveExecutionLocation = %s | PostMoveExecutionLocation = %s"),
      *GetValidActorLocation(ClientMove.InLocation).ToString(),
      *PawnOwner->GetActorLocation().ToString()
    )
    checkGMC(bUseClientLocation ?
      GetValidActorLocation(ClientMove.InLocation).Equals(PawnOwner->GetActorLocation(), 0.01f) : true)
    GMC_CLOG(
      bUseClientRotation &&
      !GetValidActorRotation(ClientMove.InRotation).Equals(PawnOwner->GetActorRotation(), 0.1f),
      Warning,
      TEXT("<bUseClientRotation> is true but the pawn's rotation changed during move execution on the server: ")
      TEXT("PreMoveExecutionRotation = %s | PostMoveExecutionRotation = %s"),
      *GetValidActorRotation(ClientMove.InRotation).ToString(),
      *PawnOwner->GetActorRotation().ToString()
    )
    checkGMC(bUseClientRotation ?
      GetValidActorRotation(ClientMove.InRotation).Equals(PawnOwner->GetActorRotation(), 0.1f) : true)
    check(PawnOwner->GetController())
    GMC_CLOG(
      bUseClientControlRotation &&
      !GetValidControlRotation(ClientMove.InControlRotation).Equals(PawnOwner->GetController()->GetControlRotation(), 0.1f),
      Warning,
      TEXT("<bUseClientControlRotation> is true but the pawn's control rotation changed during move execution on the server: ")
      TEXT("PreMoveExecutionControlRotation = %s | PostMoveExecutionControlRotation = %s"),
      *GetValidControlRotation(ClientMove.InControlRotation).ToString(),
      *PawnOwner->GetController()->GetControlRotation().ToString()
    )
    checkGMC(bUseClientControlRotation ?
      GetValidControlRotation(ClientMove.InControlRotation).Equals(PawnOwner->GetController()->GetControlRotation(), 0.1f) : true)

    if (bUseClientPrediction && !bIsProxyMove)
    {
      Server_ResolveClientDiscrepancy(
        PawnOwner->GetActorLocation(),
        PawnOwner->GetActorRotation(),
        PawnOwner->GetControlRotation(),
        GetValidActorLocation(ClientMove.OutLocation),
        GetValidActorRotation(ClientMove.OutRotation),
        GetValidControlRotation(ClientMove.OutControlRotation),
        ClientMove.Timestamp
      );
      DEBUG_SHOW_CLIENT_LOCATION_ERRORS_ON_SERVER
    }
    DEBUG_LOG_SERVER_EXECUTED_MOVE_RESOLVED

    if (!bUseClientPrediction || bAlwaysReplay || bIsProxyMove)
    {
      // When not using client prediction there's not really any question of the client move being valid or not but we always set the flag
      // to false in this case to force a complete serialization of the server state. The same applies when "bAlwaysReplay" is true (which
      // is irrelevant when not using client prediction) and when executing a proxy move.
      Server_bLastClientMoveWasValid = false;
    }

    // Save the server state for replication to the autonomous proxy.
    Server_SaveServerState(ROLE_AutonomousProxy, ClientMove);

    checkCodeGMC(
      const auto& CheckState = ServerState_AutonomousProxy();
      if (!Server_bLastClientMoveWasValid)
      {
        // The autonomous proxy should receive all the state information.
        check(CheckState.bContainsFullRepBatch)
      }
      else
      {
        // The autonomous proxy does not require all the state information because its move was valid.
        check(!CheckState.bContainsFullRepBatch)
        // Client out location/rotation/control rotation should be equal to the server state at this point (rotations are less accurate).
        check(CheckState.Location.Equals(
          GetValidActorLocation(ClientMove.OutLocation), bAssumeClientState ? 0.01f : MaxLocationError))
        check(CheckState.Rotation.Equals(
          GetValidActorRotation(ClientMove.OutRotation), bAssumeClientState ? 0.1f : MaxRotationError))
        check(CheckState.ControlRotation.Equals(
          GetValidControlRotation(ClientMove.OutControlRotation), bAssumeClientState ? 0.1f : MaxControlRotationError))
      }
    )

    // Quantize the pawn state for accurate synchronisation between client and server.
    Server_QuantizePawnStateFrom(ServerState_AutonomousProxy());

    if (Server_ShouldForceNetUpdate(ROLE_AutonomousProxy))
    {
      PawnOwner->ForceNetUpdate();
    }

    DEBUG_LOG_SERVER_SAVED_AUTONOMOUS_PROXY_STATE

    // Save the final state for the simulated proxy. Although both states should contain the same pawn data afterwards they probably have
    // different replication settings (so don't do something like ServerState_SimulatedProxy() = ServerState_AutonomousProxy()). Since the
    // simulated proxy server state is filled with the pawn state that was previously quantized from the autonomous proxy server state, it
    // does not need to be quantized again.
    Server_SaveServerState(ROLE_SimulatedProxy, ClientMove);

    if (Server_ShouldForceNetUpdate(ROLE_SimulatedProxy))
    {
      PawnOwner->ForceNetUpdate();
    }

    // Smoothing and rolling back pawns requires the state queue to be filled. Keep in mind that pawns are also considered smoothed when
    // the interpolation method is "None" (which therefore also requires the state queue to be filled).
    if (bFillStateQueueForRemoteServerPawns)
    {
      const FState FilteredServerState = Server_GetFilteredServerState();
      if (InterpolationMode == EInterpolationMode::MatchLatest)
      {
        SMLSaveTimestamps(FilteredServerState);
      }
      AddToStateQueue(FilteredServerState);
    }

    Server_PostRemoteMoveExecution(ClientMove);
  }

  Server_OnRemoteMovesProcessed();

  if (bShouldRollBackServerPawns)
  {
    // Restore the states of all pawns that were rolled back for move execution.
    RestoreRolledBackPawns(RollbackPawnList);
  }

  if (bShouldRollBackGenericServerActors)
  {
    // Restore the states of all generic actors that were rolled back for move execution.
    RestoreRolledBackGenericActors(GenericRollbackActorList, EActorRollbackContext::RestoringRolledBackServerActors);
  }

  if (IsSmoothedListenServerPawn())
  {
    // The client moves have been processed, saved for replication and added to the state queue. We can set the pawn back to its smoothed
    // state now.
    Server_SwapStateBuffer(EImmediateContext::ReSmoothingRemoteListenServerPawn);
  }

  Server_bIsExecutingRemoteMoves = false;
}

FMove UGenMovementReplicationComponent::Server_UnpackClientMove(const FMove& RemoteMove)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_UnpackClientMove)

  FMove UnpackedMove = RemoteMove;
  // We calculate the delta time of client moves from the timestamps that we have received.
  const float CalculatedDeltaTime = RemoteMove.Timestamp - Server_LastUnpackedClientMove.Timestamp;
  const float ClampedDeltaTime = FMath::Clamp(CalculatedDeltaTime, MIN_DELTA_TIME, MaxServerDeltaTime);
  GMC_CLOG(
    !FMath::IsNearlyEqual(ClampedDeltaTime, CalculatedDeltaTime, KINDA_SMALL_NUMBER),
    Verbose,
    TEXT("Calculated server delta time of %f s was clamped to %f s."),
    CalculatedDeltaTime,
    ClampedDeltaTime
  )
  UnpackedMove.DeltaTime = ClampedDeltaTime;
  // Shorter names for readability.
  const auto& CM = RemoteMove;
  const auto& LUCM = Server_LastUnpackedClientMove;
  auto& UM = UnpackedMove;
  // Copy values from previous move if necessary. The input flags are always deserialized directly (as they contain just 1 bit of
  // information anyway).
  // @attention The serialization options must come from the original argument of the server RPC because the settings may be overridden by a
  // subclass (meaning the underlying type may be a subclass of FMove).
  if (CM.bSerializeInputVectorX            && !CM.bHasNewInputVectorX)            UM.InputVector.X            = LUCM.InputVector.X;
  if (CM.bSerializeInputVectorY            && !CM.bHasNewInputVectorY)            UM.InputVector.Y            = LUCM.InputVector.Y;
  if (CM.bSerializeInputVectorZ            && !CM.bHasNewInputVectorZ)            UM.InputVector.Z            = LUCM.InputVector.Z;
  if (CM.bSerializeRotationInputRoll       && !CM.bHasNewRotationInputRoll)       UM.RotationInput.Roll       = LUCM.RotationInput.Roll;
  if (CM.bSerializeRotationInputPitch      && !CM.bHasNewRotationInputPitch)      UM.RotationInput.Pitch      = LUCM.RotationInput.Pitch;
  if (CM.bSerializeRotationInputYaw        && !CM.bHasNewRotationInputYaw)        UM.RotationInput.Yaw        = LUCM.RotationInput.Yaw;
  if (CM.bSerializeOutVelocity             && !CM.bHasNewOutVelocity)             UM.OutVelocity              = LUCM.OutVelocity;
  if (CM.bSerializeOutLocation             && !CM.bHasNewOutLocation)             UM.OutLocation              = LUCM.OutLocation;
  if (CM.bSerializeOutRotationRoll         && !CM.bHasNewOutRotationRoll)         UM.OutRotation.Roll         = LUCM.OutRotation.Roll;
  if (CM.bSerializeOutRotationPitch        && !CM.bHasNewOutRotationPitch)        UM.OutRotation.Pitch        = LUCM.OutRotation.Pitch;
  if (CM.bSerializeOutRotationYaw          && !CM.bHasNewOutRotationYaw)          UM.OutRotation.Yaw          = LUCM.OutRotation.Yaw;
  if (CM.bSerializeOutControlRotationRoll  && !CM.bHasNewOutControlRotationRoll)  UM.OutControlRotation.Roll  = LUCM.OutControlRotation.Roll;
  if (CM.bSerializeOutControlRotationPitch && !CM.bHasNewOutControlRotationPitch) UM.OutControlRotation.Pitch = LUCM.OutControlRotation.Pitch;
  if (CM.bSerializeOutControlRotationYaw   && !CM.bHasNewOutControlRotationYaw)   UM.OutControlRotation.Yaw   = LUCM.OutControlRotation.Yaw;
  // Clamp the input vector for safety.
  GMC_CLOG(
    UnpackedMove.InputVector.X > MAX_DIRECTION_INPUT || UnpackedMove.InputVector.X < -MAX_DIRECTION_INPUT,
    Verbose,
    TEXT("Received input vector X-component was clamped from %f to %d."),
    UnpackedMove.InputVector.X,
    UnpackedMove.InputVector.X > MAX_DIRECTION_INPUT ? MAX_DIRECTION_INPUT : -MAX_DIRECTION_INPUT
  )
  GMC_CLOG(
    UnpackedMove.InputVector.Y > MAX_DIRECTION_INPUT || UnpackedMove.InputVector.Y < -MAX_DIRECTION_INPUT,
    Verbose,
    TEXT("Received input vector Y-component was clamped from %f to %d."),
    UnpackedMove.InputVector.Y,
    UnpackedMove.InputVector.Y > MAX_DIRECTION_INPUT ? MAX_DIRECTION_INPUT : -MAX_DIRECTION_INPUT
  )
  GMC_CLOG(
    UnpackedMove.InputVector.Z > MAX_DIRECTION_INPUT || UnpackedMove.InputVector.Z < -MAX_DIRECTION_INPUT,
    Verbose,
    TEXT("Received input vector Z-component was clamped from %f to %d."),
    UnpackedMove.InputVector.Z,
    UnpackedMove.InputVector.Z > MAX_DIRECTION_INPUT ? MAX_DIRECTION_INPUT : -MAX_DIRECTION_INPUT
  )
  UnpackedMove.InputVector = FVector(
    FMath::Clamp(UnpackedMove.InputVector.X, (float)-MAX_DIRECTION_INPUT, (float)MAX_DIRECTION_INPUT),
    FMath::Clamp(UnpackedMove.InputVector.Y, (float)-MAX_DIRECTION_INPUT, (float)MAX_DIRECTION_INPUT),
    FMath::Clamp(UnpackedMove.InputVector.Z, (float)-MAX_DIRECTION_INPUT, (float)MAX_DIRECTION_INPUT)
  );
  // Clamp the rotation input as well but only if it was quantized.
  if (UnpackedMove.RotationInputQuantize < ESizeQuantization::None)
  {
    GMC_CLOG(
      UnpackedMove.RotationInput.Roll > MAX_ROTATION_INPUT || UnpackedMove.RotationInput.Roll < -MAX_ROTATION_INPUT,
      Verbose,
      TEXT("Received rotation input roll-component was clamped from %f to %d."),
      UnpackedMove.RotationInput.Roll,
      UnpackedMove.RotationInput.Roll > MAX_ROTATION_INPUT ? MAX_ROTATION_INPUT : -MAX_ROTATION_INPUT
    )
    GMC_CLOG(
      UnpackedMove.RotationInput.Pitch > MAX_ROTATION_INPUT || UnpackedMove.RotationInput.Pitch < -MAX_ROTATION_INPUT,
      Verbose,
      TEXT("Received rotation input pitch-component was clamped from %f to %d."),
      UnpackedMove.RotationInput.Pitch,
      UnpackedMove.RotationInput.Pitch > MAX_ROTATION_INPUT ? MAX_ROTATION_INPUT : -MAX_ROTATION_INPUT
    )
    GMC_CLOG(
      UnpackedMove.RotationInput.Yaw > MAX_ROTATION_INPUT || UnpackedMove.RotationInput.Yaw < -MAX_ROTATION_INPUT,
      Verbose,
      TEXT("Received rotation input yaw-component was clamped from %f to %d."),
      UnpackedMove.RotationInput.Yaw,
      UnpackedMove.RotationInput.Yaw > MAX_ROTATION_INPUT ? MAX_ROTATION_INPUT : -MAX_ROTATION_INPUT
    )
    UnpackedMove.RotationInput = FRotator(
      FMath::Clamp(UnpackedMove.RotationInput.Pitch, (float)-MAX_ROTATION_INPUT, (float)MAX_ROTATION_INPUT),
      FMath::Clamp(UnpackedMove.RotationInput.Yaw,   (float)-MAX_ROTATION_INPUT, (float)MAX_ROTATION_INPUT),
      FMath::Clamp(UnpackedMove.RotationInput.Roll,  (float)-MAX_ROTATION_INPUT, (float)MAX_ROTATION_INPUT)
    );
  }
  // Normalize rotations into [-180,+180] range.
  // @attention Never normalize the rotation input.
  UnpackedMove.OutRotation.Normalize();
  UnpackedMove.OutControlRotation.Normalize();
  // Quantize so we use the same values as the client.
  UnpackedMove.QuantizeInputVector();
  UnpackedMove.QuantizeRotationInput();
  UnpackedMove.QuantizeOutVelocity();
  UnpackedMove.QuantizeOutLocation();
  UnpackedMove.QuantizeOutRotation();
  UnpackedMove.QuantizeOutControlRotation();
  // Values that are not available (were not deserialized) are invalid. We do this as the last thing now so we didn't have to check for NANs
  // everywhere before.
  // @attention Do not use the out values for bound data or the input mode of the unpacked move as they are considered invalid as well (i.e.
  // not sent by the client). Since these encompass more than just numeric types they cannot be marked with NAN, and keep their default
  // constructor values. Non-replicated input flags also just remain "false".
  if (!CM.bSerializeInputVectorX)            Rep_SetInvalid(UM.InputVector.X);
  if (!CM.bSerializeInputVectorY)            Rep_SetInvalid(UM.InputVector.Y);
  if (!CM.bSerializeInputVectorZ)            Rep_SetInvalid(UM.InputVector.Z);
  if (!CM.bSerializeRotationInputRoll)       Rep_SetInvalid(UM.RotationInput.Roll);
  if (!CM.bSerializeRotationInputPitch)      Rep_SetInvalid(UM.RotationInput.Pitch);
  if (!CM.bSerializeRotationInputYaw)        Rep_SetInvalid(UM.RotationInput.Yaw);
  if (!CM.bSerializeOutVelocity)             Rep_SetInvalid(UM.OutVelocity);
  if (!CM.bSerializeOutLocation)             Rep_SetInvalid(UM.OutLocation);
  if (!CM.bSerializeOutRotationRoll)         Rep_SetInvalid(UM.OutRotation.Roll);
  if (!CM.bSerializeOutRotationPitch)        Rep_SetInvalid(UM.OutRotation.Pitch);
  if (!CM.bSerializeOutRotationYaw)          Rep_SetInvalid(UM.OutRotation.Yaw);
  if (!CM.bSerializeOutControlRotationRoll)  Rep_SetInvalid(UM.OutControlRotation.Roll);
  if (!CM.bSerializeOutControlRotationPitch) Rep_SetInvalid(UM.OutControlRotation.Pitch);
  if (!CM.bSerializeOutControlRotationYaw)   Rep_SetInvalid(UM.OutControlRotation.Yaw);
  // Save this unpacked move for future reference.
  Server_LastUnpackedClientMove = UnpackedMove;
  return UnpackedMove;
}

FMove UGenMovementReplicationComponent::Server_CreateProxyMove(float Timestamp, float DeltaTime) const
{
  FMove ProxyMove;
  ProxyMove.Timestamp = Timestamp;
  ProxyMove.DeltaTime = DeltaTime;
  checkGMC(ProxyMove.InputVector == FVector::ZeroVector)
  checkGMC(ProxyMove.RotationInput == FRotator::ZeroRotator)
  ProxyMove.OutVelocity = GetVelocity();
  ProxyMove.OutLocation = PawnOwner->GetActorLocation();
  ProxyMove.OutRotation = PawnOwner->GetActorRotation().GetNormalized();
  ProxyMove.OutControlRotation = PawnOwner->GetControlRotation().GetNormalized();
  // Get the serialization options from the local move because the settings may be overridden by a subclass.
  if (!LocalMove().bSerializeInputVectorX)            Rep_SetInvalid(ProxyMove.InputVector.X);
  if (!LocalMove().bSerializeInputVectorY)            Rep_SetInvalid(ProxyMove.InputVector.Y);
  if (!LocalMove().bSerializeInputVectorZ)            Rep_SetInvalid(ProxyMove.InputVector.Z);
  if (!LocalMove().bSerializeRotationInputRoll)       Rep_SetInvalid(ProxyMove.RotationInput.Roll);
  if (!LocalMove().bSerializeRotationInputPitch)      Rep_SetInvalid(ProxyMove.RotationInput.Pitch);
  if (!LocalMove().bSerializeRotationInputYaw)        Rep_SetInvalid(ProxyMove.RotationInput.Yaw);
  if (!LocalMove().bSerializeOutVelocity)             Rep_SetInvalid(ProxyMove.OutVelocity);
  if (!LocalMove().bSerializeOutLocation)             Rep_SetInvalid(ProxyMove.OutLocation);
  if (!LocalMove().bSerializeOutRotationRoll)         Rep_SetInvalid(ProxyMove.OutRotation.Roll);
  if (!LocalMove().bSerializeOutRotationPitch)        Rep_SetInvalid(ProxyMove.OutRotation.Pitch);
  if (!LocalMove().bSerializeOutRotationYaw)          Rep_SetInvalid(ProxyMove.OutRotation.Yaw);
  if (!LocalMove().bSerializeOutControlRotationRoll)  Rep_SetInvalid(ProxyMove.OutControlRotation.Roll);
  if (!LocalMove().bSerializeOutControlRotationPitch) Rep_SetInvalid(ProxyMove.OutControlRotation.Pitch);
  if (!LocalMove().bSerializeOutControlRotationYaw)   Rep_SetInvalid(ProxyMove.OutControlRotation.Yaw);
  return ProxyMove;
}

void UGenMovementReplicationComponent::Server_SetClientMoveInput(FMove& ClientMove) const
{
  SCOPE_CYCLE_COUNTER(STAT_Server_SetClientMoveInput)

  // Fill in local data that the client does not send. Client-authoritative values must not be changed by the movement logic meaning the
  // in-values are the same as the out-values.
  ClientMove.InVelocity = GetVelocity();
  ClientMove.InLocation = bUseClientLocation ? ClientMove.OutLocation : PawnOwner->GetActorLocation();
  ClientMove.InRotation = bUseClientRotation ? ClientMove.OutRotation : PawnOwner->GetActorRotation();
  ClientMove.InControlRotation = bUseClientControlRotation ? ClientMove.OutControlRotation : PawnOwner->GetControlRotation();
  ClientMove.InInputMode = GenPawnOwner->GetInputMode();
  SaveBoundDataToMove(ClientMove, FMove::EStateVars::Input);
}

void UGenMovementReplicationComponent::Server_EnsureValidMoveData(FMove& Move)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_EnsureValidMoveData)

  checkGMC(IsRemotelyControlledServerPawn())
  checkGMC(bEnsureValidMoveData)
  Move.InputVector = GetValidInputVector(Move.InputVector);
  Move.RotationInput = GetValidRotationInput(Move.RotationInput);
  Move.InLocation = GetValidVector(Move.InLocation, FVector{0});
  Move.InRotation = GetValidRotator(Move.InRotation, FRotator{0});
  Move.InControlRotation = GetValidRotator(Move.InControlRotation, FRotator{0});
  Move.OutVelocity = GetValidVector(Move.OutVelocity, FVector{0});
  Move.OutLocation = GetValidVector(Move.OutLocation, FVector{0});
  Move.OutRotation = GetValidRotator(Move.OutRotation, FRotator{0});
  Move.OutControlRotation = GetValidRotator(Move.OutControlRotation, FRotator{0});
}

void UGenMovementReplicationComponent::Server_SaveServerState(ENetRole RecipientRole, const FMove& SourceMove)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_SaveServerState)

  checkfGMC(
    RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy,
    TEXT("The recipient must be an autonomous or simulated proxy.")
  )

  auto& OutState = GetServerStateFromRole(RecipientRole);
  if (RecipientRole == ROLE_AutonomousProxy)
  {
    Server_BufferServerState_AutonomousProxy();
  }
  else
  {
    checkGMC(RecipientRole == ROLE_SimulatedProxy)
    Server_BufferServerState_SimulatedProxy();
  }

  OutState.bIsUsingServerAuthPhysicsReplication = IsUsingServerAuthPhysicsReplication();
  if (!OutState.bIsUsingServerAuthPhysicsReplication)
  {
    // Timestamps have a different purpose depending on the role of the pawn that receives the data. For autonomous proxies the timestamp is
    // used to determine what the last move was that the server has acknowledged. This move will be used to ascertain if the client state is
    // still valid or if a replay needs to be executed, and older moves will be deleted from the client's move queue. Therefore it is
    // essential that the timestamp in the server state is the exact same one as the one that was received with the source move, and that it
    // does not get compressed during replication. For simulated proxies the timestamp is used to determine which states should be used for
    // interpolation at any point in time based on the set simulation delay. We still need to use the original move timestamp for this since
    // we can't mimic the time between the original client moves on the server.
    OutState.Timestamp = SourceMove.Timestamp;
  }
  else
  {
    checkGMC(!IsUsingClientAuthPhysicsReplication())
    // Since the physics are calculated on the server in this case we need to use the server time for accurate simulation, but we still need
    // the original client timestamp for identifying acknowledged moves on the client.
    OutState.Timestamp = GetTime();
    OutState.ServerAuthPhysicsAckTimestamp = SourceMove.Timestamp;
  }

  OutState.bOptimizeTraffic = bOptimizeTraffic;
  Server_FillServerStateWithData(OutState, RecipientRole, bUseClientPrediction);
  if (RecipientRole == ROLE_AutonomousProxy)
  {
    // Autonomous proxies do not need to have their input flags replicated.
    if (!Server_bLastClientMoveWasValid)
    {
      // The client needs to replay, so we must serialize all of the required replication data.
      OutState.bContainsFullRepBatch = true;
    }
    else
    {
      // There is no reason to send the full batch of replay data.
      OutState.bContainsFullRepBatch = false;
    }
  }
  else
  {
    checkGMC(RecipientRole == ROLE_SimulatedProxy)
    // Simulated proxies may need to know about the input flags.
    Server_SaveBoundInputFlagsToServerState(OutState, SourceMove);
    // For the simulated proxy server state we always want to serialize all of the replication data, so the property should never be changed
    // from its constructor value "true" for those pawns.
    checkGMC(OutState.bContainsFullRepBatch);
  }
}

void UGenMovementReplicationComponent::Server_FillServerStateWithData(
  FState& State,
  ENetRole RecipientRole,
  bool bUsingClientPrediction
) const
{
  State.Velocity = GetVelocity();
  State.Location = PawnOwner->GetActorLocation();
  State.Rotation = PawnOwner->GetActorRotation();
  State.ControlRotation = PawnOwner->GetControlRotation();
  State.InputMode = GenPawnOwner->GetInputMode();
  Server_SaveBoundDataToServerState(State, RecipientRole, bUsingClientPrediction);
}

bool UGenMovementReplicationComponent::Server_ShouldForceNetUpdate(ENetRole RecipientRole)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_ShouldForceNetUpdate)

  checkfGMC(
    RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy,
    TEXT("The recipient must be an autonomous or simulated proxy.")
  )

  if (RecipientRole == ROLE_AutonomousProxy)
  {
    if (ServerState_AutonomousProxy().bContainsFullRepBatch && bUseClientPrediction && !bAlwaysReplay)
    {
      checkGMC(!Server_bLastClientMoveWasValid)
      // If this server state contains the full replication batch it implies that the last move was not valid. We force a replication update
      // so the client can correct its state as soon as possible. While it is common to force net updates to the simulated proxy for
      // animation purposes, this is the only occasion where we do it for the (predicted) autonomous proxy.
      // @attention If "bAlwaysReplay" is true or "bUseClientPrediction" is false we don't want to do this because we would essentially send
      // every saved server state to the autonomous proxy. In the case of not using client prediction that would be unnecessary and a waste
      // of bandwidth. In the case of always replaying we primarily want to limit the performance hit on the client because replays are
      // already pretty demanding.
      // @attention "bAlwaysReplay" is irrelevant when not using client prediction.
      return true;
    }

    if (!bUseClientPrediction)
    {
      checkGMC(ServerState_AutonomousProxy().bContainsFullRepBatch)
      checkGMC(!Server_bLastClientMoveWasValid)
      // Pre-replicated data is only checked if the bind has the "bForceNetUpdate" flag enabled.
      if (Server_ForceNetUpdateCheckBoundData(ServerState_AutonomousProxy(), ROLE_AutonomousProxy))
      {
        return true;
      }
    }
  }

  // Check whether a net update should be forced to simulated proxies.
  if (RecipientRole == ROLE_SimulatedProxy)
  {
    // A change in any of the input flags forces a net update by default.
    if (Server_ForceNetUpdateCheckBoundInputFlags())
    {
      return true;
    }
    // Pre-replicated data is only checked if the bind has the "bForceNetUpdate" flag enabled.
    if (Server_ForceNetUpdateCheckBoundData(ServerState_SimulatedProxy(), ROLE_SimulatedProxy))
    {
      return true;
    }
    // @note A change in the input mode currently doesn't force a net update.
  }

  // Custom checks from child classes.
  if (Server_ShouldForceNetUpdate_Custom(RecipientRole))
  {
    return true;
  }

  // No important values changed, we don't need to force a net update.
  return false;
}

bool UGenMovementReplicationComponent::Server_VerifyTimestamps(const TArray<FMove>& RemoteMoves) const
{
  SCOPE_CYCLE_COUNTER(STAT_Server_VerifyTimestamps)

  checkGMC(RemoteMoves.Num() > 0)

  // We estimate what the timestamp of the latest client move should be based on the current server world time and the client's ping. Keep
  // in mind that this is a rough estimation and that the actual difference depends on a lot of variable factors.
  if (const auto Controller = PawnOwner->GetController())
  {
    if (const auto NetConnection = Controller->GetNetConnection())
    {
      const float ClientPing = NetConnection->AvgLag / 2.f;
      const float EstimatedTimestampLatestMove = GetTime() - ClientPing;
      const float CalculatedTimestampDiff = FMath::Abs(RemoteMoves.Last().Timestamp - EstimatedTimestampLatestMove);
      if (CalculatedTimestampDiff > MaxAllowedTimestampDeviation)
      {
        GMC_LOG(
          VeryVerbose,
          TEXT("Verification failed, the newest move timestamp (%f s) differed by more than the max allowed deviation (%f s)."),
          CalculatedTimestampDiff,
          MaxAllowedTimestampDeviation
        )
        return false;
      }
    }
  }

  // The difference between the timestamps of the newest and oldest received move should not be greater than the max server delta time (the
  // max server delta time will never be lower than the client send interval, we check this when beginning play).
  if (RemoteMoves[0].Timestamp < RemoteMoves.Last().Timestamp - MaxServerDeltaTime)
  {
    GMC_LOG(
      VeryVerbose,
      TEXT("Verification failed, the oldest move timestamp (%f s) was lower than the min expected time (%f s)."),
      RemoteMoves[0].Timestamp,
      RemoteMoves.Last().Timestamp - MaxServerDeltaTime
    )
    return false;
  }

  // The accumulated delta time of all client moves should not be greater than the max server delta time.
  float TimestampPreviousMove{0.f};
  float CalculatedDeltaTime{0.f};
  float AccumulatedClientDeltaTime{0.f};
  for (int32 Index = 0; Index < RemoteMoves.Num(); ++Index)
  {
    if (Index == 0)
    {
      // We can't really rely on a timestamp from the previous batch for verification so we skip the delta time of the first move. This will
      // make the accumulated delta time a little less than it should be but it's better than not checking at all.
      TimestampPreviousMove = RemoteMoves[Index].Timestamp;
      continue;
    }
    if (RemoteMoves[Index].Timestamp <= TimestampPreviousMove)
    {
      GMC_LOG(VeryVerbose, TEXT("Verification failed, received batch of client moves had inconsistent timestamps."))
      return false;
    }
    CalculatedDeltaTime = RemoteMoves[Index].Timestamp - TimestampPreviousMove;
    TimestampPreviousMove = RemoteMoves[Index].Timestamp;
    AccumulatedClientDeltaTime += CalculatedDeltaTime;
  }
  if (AccumulatedClientDeltaTime > MaxServerDeltaTime)
  {
    GMC_LOG(
      VeryVerbose,
      TEXT("Verification failed, the received batch of client moves had an accumulated delta time (%f s) ")
      TEXT("greater than the max server delta time (%f s)."),
      AccumulatedClientDeltaTime,
      MaxServerDeltaTime
    )
    return false;
  }

  // The client data passed all verification checks.
  return true;
}

void UGenMovementReplicationComponent::Server_ResetClientStrikes()
{
  Server_ClientStrikeCount = 0;
}

bool UGenMovementReplicationComponent::Server_ResolveClientDiscrepancy(
  const FVector& ServerLocation,
  const FRotator& ServerRotation,
  const FRotator& ServerControlRotation,
  const FVector& ClientLocation,
  const FRotator& ClientRotation,
  const FRotator& ClientControlRotation,
  float MoveTimestamp
)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_ResolveClientDiscrepancy)

  checkGMC(IsServerPawn())
  checkGMC(!PawnOwner->IsLocallyControlled())
  checkGMC(bUseClientPrediction)

  Server_OnResolveClientDiscrepancy(ClientLocation, ClientRotation, ClientControlRotation);

  // For validation, a theoretical min tolerance would have to fit the quantization level with which the original value was compressed on
  // the client (e.g. 0.01 for "RoundTwoDecimals"). In reality however, inaccuracies sneak in from all sides and implementing everything
  // completely deterministically is nearly impossible and also not worth the trouble. It is much more practical to just choose a tolerance
  // that is "good enough" for the given application. However, if the client moves are getting rejected with a higher error tolerance as
  // well there is usually something wrong with the movement implementation.
  const bool bClientLocationValid = bUseClientLocation ? true : ServerLocation.Equals(ClientLocation, MaxLocationError);
  const bool bClientRotationValid = bUseClientRotation ? true : ServerRotation.Equals(ClientRotation, MaxRotationError);
  const bool bClientControlRotationValid = bUseClientControlRotation ?
                                             true :
                                             ServerControlRotation.Equals(ClientControlRotation, MaxControlRotationError);

  // Either accept or reject the client move.
  if (bClientLocationValid && bClientRotationValid && bClientControlRotationValid)
  {
    if (bAssumeClientState)
    {
      PawnOwner->SetActorLocation(ClientLocation, false, nullptr, ETeleportType::TeleportPhysics);
      PawnOwner->SetActorRotation(ClientRotation, ETeleportType::TeleportPhysics);
      if (const auto Controller = PawnOwner->GetController()) Controller->SetControlRotation(ClientControlRotation);
    }
    Server_bLastClientMoveWasValid = true;
  }
  else
  {
    Server_bLastClientMoveWasValid = false;
    // If the client state is determined to be not valid it is important to note that the client will only be off until he receives the
    // update from the server that makes him aware of that fact (so for a duration equal to his ping). At that point in time a replay will
    // be triggered on the client and consecutive moves in the client's move queue will have a corrected state result which most likely will
    // coincide with the current server state. However, some of these now corrected moves may already be underway to the server with the old
    // (faulty) values which also won't be accepted by the server even though these values don't actually represent the client's current
    // state anymore. This means that not every time the server reports an invalid state the client is actually off. The server may be
    // handling older move values that have already been corrected on the client (but the server has no way of knowing this). The client
    // side log is more accurate in this regard.
  }

  GMC_CLOG(
    !bClientLocationValid,
    VeryVerbose,
    TEXT("Client location was not valid, server-client difference is %f > %f (tolerance). ")
    TEXT("Move timestamp = %f | Server location = %f, %f, %f | Client location = %f, %f, %f"),
    (ServerLocation - ClientLocation).Size(),
    MaxLocationError,
    MoveTimestamp,
    ServerLocation.X,
    ServerLocation.Y,
    ServerLocation.Z,
    ClientLocation.X,
    ClientLocation.Y,
    ClientLocation.Z
  )
  GMC_CLOG(
    bClientLocationValid && !bUseClientLocation && !bAssumeClientState,
    VeryVerbose,
    TEXT("Current location server-client difference is %f (configured tolerance is %f). ")
    TEXT("Move timestamp = %f | Server location = %f, %f, %f | Client location = %f, %f, %f"),
    (ServerLocation - ClientLocation).Size(),
    MaxLocationError,
    MoveTimestamp,
    ServerLocation.X,
    ServerLocation.Y,
    ServerLocation.Z,
    ClientLocation.X,
    ClientLocation.Y,
    ClientLocation.Z
  )
  GMC_CLOG(
    !bClientRotationValid,
    VeryVerbose,
    TEXT("Client rotation was not valid, server-client difference is %f > %f (tolerance). ")
    TEXT("Move timestamp = %f | Server rotation = %f, %f, %f | Client rotation = %f, %f, %f"),
    (ServerRotation.GetDenormalized() - ClientRotation.GetDenormalized()).Euler().Size(),
    MaxRotationError,
    MoveTimestamp,
    ServerRotation.Roll,
    ServerRotation.Pitch,
    ServerRotation.Yaw,
    ClientRotation.Roll,
    ClientRotation.Pitch,
    ClientRotation.Yaw
  )
  GMC_CLOG(
    bClientRotationValid && !bUseClientRotation && !bAssumeClientState,
    VeryVerbose,
    TEXT("Current rotation server-client difference is %f (configured tolerance is %f). ")
    TEXT("Move timestamp = %f | Server rotation = %f, %f, %f | Client rotation = %f, %f, %f"),
    (ServerRotation.GetDenormalized() - ClientRotation.GetDenormalized()).Euler().Size(),
    MaxRotationError,
    MoveTimestamp,
    ServerRotation.Roll,
    ServerRotation.Pitch,
    ServerRotation.Yaw,
    ClientRotation.Roll,
    ClientRotation.Pitch,
    ClientRotation.Yaw
  )
  GMC_CLOG(
    !bClientControlRotationValid,
    VeryVerbose,
    TEXT("Client control rotation was not valid, server-client difference is %f > %f (tolerance). ")
    TEXT("Move timestamp = %f | Server control rotation = %f, %f, %f | Client control rotation = %f, %f, %f"),
    (ServerControlRotation.GetDenormalized() - ClientControlRotation.GetDenormalized()).Euler().Size(),
    MaxControlRotationError,
    MoveTimestamp,
    ServerControlRotation.Roll,
    ServerControlRotation.Pitch,
    ServerControlRotation.Yaw,
    ClientControlRotation.Roll,
    ClientControlRotation.Pitch,
    ClientControlRotation.Yaw
  )
  GMC_CLOG(
    bClientControlRotationValid && !bUseClientControlRotation && !bAssumeClientState,
    VeryVerbose,
    TEXT("Curret control rotation server-client difference is %f (configured tolerance is %f). ")
    TEXT("Move timestamp = %f | Server control rotation = %f, %f, %f | Client control rotation = %f, %f, %f"),
    (ServerControlRotation.GetDenormalized() - ClientControlRotation.GetDenormalized()).Euler().Size(),
    MaxControlRotationError,
    MoveTimestamp,
    ServerControlRotation.Roll,
    ServerControlRotation.Pitch,
    ServerControlRotation.Yaw,
    ClientControlRotation.Roll,
    ClientControlRotation.Pitch,
    ClientControlRotation.Yaw
  )

  GMC_CLOG(
    !bClientLocationValid,
    Verbose,
    TEXT("Client move with timestamp %f rejected by server (location result deviates)."),
    MoveTimestamp
  )
  GMC_CLOG(
    !bClientRotationValid,
    Verbose,
    TEXT("Client move with timestamp %f rejected by server (rotation result deviates)."),
    MoveTimestamp
  )
  GMC_CLOG(
    !bClientControlRotationValid,
    Verbose,
    TEXT("Client move with timestamp %f rejected by server (control rotation result deviates)."),
    MoveTimestamp
  )

  return Server_bLastClientMoveWasValid;
}

void UGenMovementReplicationComponent::Server_QuantizePawnStateFrom(FState& ServerState)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_QuantizePawnStateFrom)

  ServerState.QuantizeVelocity();
  SetVelocity(ServerState.Velocity);
  if (!bUseClientLocation)
  {
    ServerState.QuantizeLocation();
    PawnOwner->SetActorLocation(ServerState.Location, false, nullptr, ETeleportType::TeleportPhysics);
  }
  ServerState.Rotation.Normalize();
  if (!bUseClientRotation)
  {
    ServerState.QuantizeRotation();
    PawnOwner->SetActorRotation(ServerState.Rotation, ETeleportType::TeleportPhysics);
  }
  ServerState.ControlRotation.Normalize();
  if (!bUseClientControlRotation)
  {
    ServerState.QuantizeControlRotation();
    if (const auto Controller = PawnOwner->GetController()) Controller->SetControlRotation(ServerState.ControlRotation);
  }
}

void UGenMovementReplicationComponent::Server_InitializeStateBuffer()
{
  StateBuffer().Velocity = GetVelocity();
  StateBuffer().Location = PawnOwner->GetActorLocation();
  StateBuffer().Rotation = PawnOwner->GetActorRotation();
  StateBuffer().SimulatedRootRelativeLocation = SimulatedRootComponent->GetRelativeLocation();
  StateBuffer().SimulatedRootRelativeRotation = SimulatedRootComponent->GetRelativeRotation();
  StateBuffer().ControlRotation = PawnOwner->GetControlRotation();
  StateBuffer().InputMode = GenPawnOwner->GetInputMode();
  SaveBoundInputFlagsToState(StateBuffer());
  SaveBoundDataToState(StateBuffer());
  Server_OnInitializeStateBuffer(StateBuffer());
  Server_bStateBufferInitialized = true;
}

void UGenMovementReplicationComponent::Server_SwapStateBuffer(EImmediateContext Context)
{
  SCOPE_CYCLE_COUNTER(STAT_Server_SwapStateBuffer)

  const FVector SwapVelocity = GetVelocity();
  const FVector SwapActorLocation = PawnOwner->GetActorLocation();
  const FRotator SwapActorRotation = PawnOwner->GetActorRotation();
  const FVector SwapSimulatedRootRelativeLocation = SimulatedRootComponent->GetRelativeLocation();
  const FRotator SwapSimulatedRootRelativeRotation = SimulatedRootComponent->GetRelativeRotation();
  const FRotator SwapControlRotation = PawnOwner->GetControlRotation();
  const EInputMode SwapInputMode = GenPawnOwner->GetInputMode();
  SetVelocity(StateBuffer().Velocity);
  PawnOwner->SetActorLocation(StateBuffer().Location, false, nullptr, ETeleportType::TeleportPhysics);
  PawnOwner->SetActorRotation(StateBuffer().Rotation, ETeleportType::TeleportPhysics);
  SimulatedRootComponent->SetRelativeLocation(StateBuffer().SimulatedRootRelativeLocation, false, nullptr, ETeleportType::TeleportPhysics);
  SimulatedRootComponent->SetRelativeRotation(StateBuffer().SimulatedRootRelativeRotation, false, nullptr, ETeleportType::TeleportPhysics);
  if (const auto Controller = PawnOwner->GetController()) Controller->SetControlRotation(StateBuffer().ControlRotation);
  GenPawnOwner->SetInputMode(StateBuffer().InputMode);
  StateBuffer().Velocity = SwapVelocity;
  StateBuffer().Location = SwapActorLocation;
  StateBuffer().Rotation = SwapActorRotation;
  StateBuffer().SimulatedRootRelativeLocation = SwapSimulatedRootRelativeLocation;
  StateBuffer().SimulatedRootRelativeRotation = SwapSimulatedRootRelativeRotation;
  StateBuffer().ControlRotation = SwapControlRotation;
  StateBuffer().InputMode = SwapInputMode;
  Server_SwapStateBufferBoundInputFlags();
  Server_SwapStateBufferBoundData();
  Server_OnSwapStateBuffer(StateBuffer(), Context);
  // Although the state will get loaded again soon for move execution, we want to trigger the event here because there are user-overridable
  // functions in between which might depend on a correct pawn state.
  OnImmediateStateLoaded(Context);
}

void UGenMovementReplicationComponent::Server_SwapServerState()
{
  if (IsSmoothedListenServerPawn() && Server_bStateBufferInitialized)
  {
    Server_SwapStateBuffer(EImmediateContext::NoContextInformation);
  }
}

FState UGenMovementReplicationComponent::Server_GetFilteredServerState() const
{
  // @attention The filtered state must be based on the simulated proxy server state, not the autonomous proxy server state, because the
  // simulated proxy state contains the values that will be replicated to other clients (autonomous proxy states are not relevant for
  // smoothing or rollback).
  FState FilteredState = ServerState_SimulatedProxy();
  // Preserve NANs for non-replicated values.
  if (!ServerState_SimulatedProxy().bSerializeVelocity)             Rep_SetInvalid(FilteredState.Velocity);
  if (!ServerState_SimulatedProxy().bSerializeLocation)             Rep_SetInvalid(FilteredState.Location);
  if (!ServerState_SimulatedProxy().bSerializeRotationRoll)         Rep_SetInvalid(FilteredState.Rotation.Roll);
  if (!ServerState_SimulatedProxy().bSerializeRotationPitch)        Rep_SetInvalid(FilteredState.Rotation.Pitch);
  if (!ServerState_SimulatedProxy().bSerializeRotationYaw)          Rep_SetInvalid(FilteredState.Rotation.Yaw);
  if (!ServerState_SimulatedProxy().bSerializeControlRotationRoll)  Rep_SetInvalid(FilteredState.ControlRotation.Roll);
  if (!ServerState_SimulatedProxy().bSerializeControlRotationPitch) Rep_SetInvalid(FilteredState.ControlRotation.Pitch);
  if (!ServerState_SimulatedProxy().bSerializeControlRotationYaw)   Rep_SetInvalid(FilteredState.ControlRotation.Yaw);
  return FilteredState;
}

void UGenMovementReplicationComponent::Server_MaintainSerializationMap()
{
  SCOPE_CYCLE_COUNTER(STAT_Server_MaintainSerializationMap)

  if (!PawnOwner) return;

  checkGMC(IsServerPawn())

  const auto World = GetWorld();
  if (!World) return;

  // Check for new player controllers (clients) that may have joined the game and add them to the simulated proxy map.
  TArray<APlayerController*> PlayerControllerList;
  for (auto Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
  {
    checkGMC(Iterator->IsValid())
    const auto Controller = Iterator->Get();
    checkGMC(IsValid(Controller))
    if (Controller == PawnOwner->GetController() || Controller->IsLocalPlayerController())
    {
      // This is the owning client controller or a local server controller.
      continue;
    }
    if (!ServerState_SimulatedProxy().LastSerialized.Contains(Controller))
    {
      ServerState_SimulatedProxy().LastSerialized.Emplace(Controller);
    }
    PlayerControllerList.Emplace(Controller);
  }

  // If this pawn has a client owner add its player controller to the map in the autonomous proxy server state. If it doesn't the map will
  // remain empty (e.g. if this is a locally or AI controlled server pawn).
  const auto NetOwner = Cast<APlayerController>(PawnOwner->GetController());
  if (NetOwner && !NetOwner->IsLocalPlayerController())
  {
    if (!ServerState_AutonomousProxy().LastSerialized.Contains(NetOwner))
    {
      // Autonomous proxy server pawns have only one connection to replicate to.
      ServerState_AutonomousProxy().LastSerialized.Reset();
      ServerState_AutonomousProxy().LastSerialized.Emplace(NetOwner);
    }
  }

  // There should never be more than one client connection (if any) in the autonomous proxy server state.
  checkGMC(ServerState_AutonomousProxy().LastSerialized.Num() <= 1)

  // Check if a client has disconnected and if so, remove its entry from the simulated proxy map.
  TArray<APlayerController*> ControllersToRemove;
  for (auto& Entry : ServerState_SimulatedProxy().LastSerialized)
  {
    auto Controller = Entry.Key;
    if (!IsValid(Controller) || !PlayerControllerList.Contains(Controller))
    {
      // Do not remove entries from the map while iterating over it.
      ControllersToRemove.Emplace(Controller);
      continue;
    }

    // Check if the connection has just become net relevant for this pawn.
    // @attention There is no need to check this for the autonomous proxy server state since the autonomous proxy pawn will always be
    // relevant to itself on the server.
    Server_CheckNetRelevancy(Controller, Entry.Value);
  }
  for (const auto Controller : ControllersToRemove)
  {
    ServerState_SimulatedProxy().LastSerialized.Remove(Controller);
  }

  // Also check if the owning pawn has the flag to fully serialize set.
  Server_CheckPawnFullSerializationFlag();
}

void UGenMovementReplicationComponent::Server_SADUpdateLocalDelay()
{
  checkGMC(IsRemotelyControlledListenServerPawn())
  checkGMC(InterpolationMode == EInterpolationMode::AdaptiveDelay)

  const auto ClientController = Cast<AGenPlayerController>(PawnOwner->GetController());
  if (!ClientController)
  {
    GMC_LOG(Error, TEXT("All player controllers must be of type <AGenPlayerController>."))
    checkGMC(false)
    return;
  }

  // For the local simulation delay on a listen server we only need to consider the latency of the controlling client.
  const float PlayerPing = ClientController->GetPingInMilliseconds() / 1000.f;
  const float EstimatedPlayerLatency = PlayerPing / 2.f;
  const float ProposedDelay = EstimatedPlayerLatency + AdaptiveDelayBufferTime;

  // We could just set the simulation delay directly every frame, but to prevent the value from changing too frequently and to get
  // consistent behaviour for all roles we'll just use the same throttling mechanism that we use for the client pawns here.
  if (SimulationDelay > 0.f && Server_SADTimeSinceLastLocalDelayUpdate < AdaptiveDelaySyncInterval)
  {
    if (FMath::IsNearlyEqual(ProposedDelay, SimulationDelay, AdaptiveDelayTolerance))
    {
      return;
    }
  }

  SimulationDelay = ProposedDelay;
  Server_SADTimeSinceLastLocalDelayUpdate = 0.f;
}

void UGenMovementReplicationComponent::Server_SADUpdateClientDelays()
{
  checkGMC(IsServerPawn())
  checkGMC(InterpolationMode == EInterpolationMode::AdaptiveDelay)

  const auto World = GetWorld();
  if (!World) return;

  const auto PawnController = PawnOwner->GetController();
  if (!PawnController)
  {
    // Only pawns with a controller can be synchronised over the network.
    return;
  }

  for (auto PCIterator = World->GetPlayerControllerIterator(); PCIterator; ++PCIterator)
  {
    checkGMC(PCIterator->IsValid())
    static_assert(SAD_NUM_SAVED_PARAMS > 1, "SAD_NUM_SAVED_PARAMS must be greater than 1.");

    const auto ClientController = Cast<AGenPlayerController>(*PCIterator);
    if (!IsValid(ClientController) || ClientController->IsLocalController())
    {
      // We are only interested in the client controllers.
      continue;
    }

    const bool bIsClientControllerPawn = ClientController == PawnController;
    if (bIsClientControllerPawn && bUseClientPrediction)
    {
      // Predicted clients don't use a simulation delay.
      continue;
    }

    // @attention We just leave controllers that are not valid anymore in the map. Deleting them shouldn't be necessary as the client pawn
    // is usually destroyed with the client controller. Nevertheless, the map should only be accessed into through known valid controllers
    // (do not iterate over the controllers in the map).
    auto& ClientData = Server_SADClientParameters.FindOrAdd(ClientController);
    auto& ClientParameters = ClientData.Parameters;
    auto& NewestClientParameters = ClientParameters[SAD_NUM_SAVED_PARAMS - 1];

    const float CurrentTime = GetTime();
    if (CurrentTime < NewestClientParameters.SyncTime)
    {
      // We don't bother calculating a new simulation delay when the current one hasn't even been committed yet.
      continue;
    }

    const float PlayerPing = ClientController->GetPingInMilliseconds() / 1000.f;
    const float EstimatedPlayerLatency = PlayerPing / 2.f;

    // For locally controlled server pawns we only need to consider the time a packet takes to get from the server to the client. This may
    // be a listen server player or an AI-controlled pawn.
    float ProposedDelay = EstimatedPlayerLatency;
    if (bIsClientControllerPawn)
    {
      // If this is the pawn of the current client controller we also need to consider the way to the server from the client.
      ProposedDelay += EstimatedPlayerLatency;
    }
    else if (IsRemotelyControlledServerPawn())
    {
      // If this is another client pawn we need to consider the latency of the other client.
      if (const auto PawnGenController = Cast<AGenPlayerController>(PawnController))
      {
        const float OtherPlayerPing = PawnGenController->GetPingInMilliseconds() / 1000.f;
        ProposedDelay += OtherPlayerPing / 2.f;
      }
      else
      {
        GMC_LOG(Error, TEXT("All player controllers must be of type <AGenPlayerController>."))
        checkGMC(false)
      }
    }
    else
    {
      checkGMC(PawnController->IsLocalController())
    }

    // Add the buffer time to the proposed delay.
    ProposedDelay += AdaptiveDelayBufferTime;

    // Never skip on the first run, then always resend when a full update interval has passed (with some variance).
    if (
      NewestClientParameters.DelayValue > 0.f
      && ClientData.UpdateTimer < AdaptiveDelaySyncInterval + FMath::FRandRange(0.f, 1.f)
    )
    {
      if (FMath::IsNearlyEqual(ProposedDelay, NewestClientParameters.DelayValue, AdaptiveDelayTolerance))
      {
        // The proposed delay is still within the configured tolerance.
        continue;
      }
    }

    // Shift the whole array to the left to enqueue the new values. This will discard the oldest parameters.
    FMemory::Memmove(&ClientParameters[0], &ClientParameters[1], sizeof(FSADParameters) * (SAD_NUM_SAVED_PARAMS - 1));

    // The new delay is different enough from the current one and will be used next.
    NewestClientParameters.DelayValue = ProposedDelay;

    // We can't immediately set the new delay because then the values would deviate while the packet is in transit to the client. Instead we
    // choose a time in the future at which the client has almost certainly received the packet to set the new delay.
    constexpr float Tolerance = 0.5f;
    NewestClientParameters.SyncTime = CurrentTime + PlayerPing / 2.f + Tolerance;

    if (Client_CheckReliableBuffer(ClientController))
    {
      // Send the new values to the client. We must route this call through the client controller because the server can't call RPCs on pawns
      // that are not owned by the client (e.g. simulated proxies).
      ClientController->Client_SendParametersForAdaptiveSimulationDelay(
        this, NewestClientParameters.DelayValue, NewestClientParameters.SyncTime
      );
    }

    ClientData.UpdateTimer = 0.f;
  }
}

float UGenMovementReplicationComponent::Server_SADGetSimulationDelayForRollback(float Time, APlayerController* ClientController)
{
  if (!IsValid(ClientController)) return 0.f;

  const auto& ClientParameters =  ClientController->GetLocalRole() == ROLE_Authority ?
    Server_SADClientParameters.FindOrAdd(ClientController).Parameters : Client_SADLocalParameters;

  for (int32 Index = SAD_NUM_SAVED_PARAMS - 1; Index >= 0; --Index)
  {
    if (Time >= ClientParameters[Index].SyncTime)
    {
      return ClientParameters[Index].DelayValue;
    }
  }
  GMC_LOG(Verbose, TEXT("No simulation delay value found for rollback (possibly too few saved entries)."))
  return 0.f;
}

bool UGenMovementReplicationComponent::Server_ProcessProxyMove()
{
  SCOPE_CYCLE_COUNTER(STAT_Server_ProcessProxyMove)

  // Remotely controlled server pawns should maintain a certain min update rate so they don't completely "freeze" in case of a temporary
  // connection issue.
  checkGMC(ServerMinUpdateRate > 0)
  const float CurrentTime = GetTime();
  const float ElapsedTime = CurrentTime - Server_LastRemotePawnUpdateTime;
  const float MinUpdateInterval = 1.f / ServerMinUpdateRate;
  if (ElapsedTime >= MinUpdateInterval)
  {
    // The proxy move is a "fake" client move that has no input (i.e. it assumes the client hasn't pressed any buttons). We will execute
    // this move instead of a real one because we haven't received any move data from the client for 1 / "ServerMinUpdateRate" seconds. This
    // will ensure that if one client experiences a lag spike the game can continue relatively smoothly for the other connected players. It
    // is also possible to produce such lag spikes artificially (a form of cheating called "lag switching") so this functionality also
    // increases security.
    TArray<FMove> ProxyMove;

    // The timestamp and delta time of the proxy move is based off the elapsed time since the last update (which is the best approximation
    // we can make).
    // @attention Do not clamp the proxy move delta time to @see MaxServerDeltaTime here.
    float ProxyMoveTimestamp = StateQueue.Num() > 0 ? StateQueue.Last().Timestamp + ElapsedTime : CurrentTime;
    float ProxyMoveDeltaTime = FMath::Clamp(ElapsedTime, MIN_DELTA_TIME, BIG_NUMBER);
    ProxyMove.Emplace(Server_CreateProxyMove(ProxyMoveTimestamp, ProxyMoveDeltaTime));
    Server_ExecuteClientMoves(ProxyMove, true);
    GMC_LOG(
      Verbose,
      TEXT("The server executed a proxy move because the client did not meet the configured min update rate: ")
      TEXT("ServerMinUpdateRate = %d (interval: %f s), time since last client move execution = %f s"),
      ServerMinUpdateRate, MinUpdateInterval, ElapsedTime
    )
    return true;
  }
  return false;
}

void UGenMovementReplicationComponent::Server_CheckNetRelevancy(APlayerController* Connection, FStateReduced& State)
{
  checkGMC(IsServerPawn())
  checkGMC(IsValid(Connection))

  // @attention "bForceFullSerializationOnNextUpdate" is reset directly at the end of @see FState::NetSerialize for the appropriate target
  // connection after the data has been fully serialized once.
  const auto& Pawn = Connection->GetPawn();
  FRotator ViewRotation{0};
  FVector ViewLocation{0};
  Connection->GetPlayerViewPoint(ViewLocation, ViewRotation);
  const bool bIsNetRelevant = PawnOwner->IsNetRelevantFor(Connection, Pawn, ViewLocation);
  if (!State.bWasNetRelevantLastUpdate && bIsNetRelevant)
  {
    State.bForceFullSerializationOnNextUpdate = true;
  }
  State.bWasNetRelevantLastUpdate = bIsNetRelevant;
}

void UGenMovementReplicationComponent::Server_CheckPawnFullSerializationFlag()
{
  checkGMC(IsServerPawn())

  if (GenPawnOwner->Server_bNeedsFullNetSerializationOnNextUpdate)
  {
    // @attention "bForceFullSerializationOnNextUpdate" is reset directly at the end of @see FState::NetSerialize for the appropriate
    // target connection after the data has been fully serialized once.
    for (auto& Entry : ServerState_SimulatedProxy().LastSerialized)
    {
      // The calling function (@see Server_MaintainSerializationMap) should have ensured that all simulated proxy connections are valid.
      checkGMC(IsValid(Entry.Key))
      auto& State = Entry.Value;
      State.bForceFullSerializationOnNextUpdate = true;
    }
    for (auto& Entry : ServerState_AutonomousProxy().LastSerialized)
    {
      // The calling function (@see Server_MaintainSerializationMap) should have ensured that the autonomous proxy connection is valid.
      checkGMC(IsValid(Entry.Key))
      auto& State = Entry.Value;
      State.bForceFullSerializationOnNextUpdate = true;
    }

    // Clear the flag on the owner after it has been processed.
    GenPawnOwner->Server_bNeedsFullNetSerializationOnNextUpdate = false;
  }
}

void UGenMovementReplicationComponent::Server_SetForceFullSerializationFlagPeriodic()
{
  checkGMC(IsServerPawn())
  if (FullSerializationInterval <= 0.f)
  {
    checkGMC(false)
    return;
  }

  // @attention "bForceFullSerializationOnNextUpdate" is reset directly at the end of @see FState::NetSerialize for the appropriate target
  // connection after the data has been fully serialized once.
  for (auto& Entry : ServerState_SimulatedProxy().LastSerialized)
  {
    if (!IsValid(Entry.Key))
    {
      // Connections that don't exist anymore are handled by @see Server_MaintainSerializationMap.
      continue;
    }
    auto& State = Entry.Value;
    State.bForceFullSerializationOnNextUpdate = true;
  }
  for (auto& Entry : ServerState_AutonomousProxy().LastSerialized)
  {
    if (!IsValid(Entry.Key))
    {
      // Connections that don't exist anymore are handled by @see Server_MaintainSerializationMap.
      continue;
    }
    auto& State = Entry.Value;
    State.bForceFullSerializationOnNextUpdate = true;
  }
}

void UGenMovementReplicationComponent::Server_SetReplicationFlag(APlayerController* TargetConnection)
{
  if (StateQueue.Num() > 0 && StateQueue.Last().LastSerialized.Contains(TargetConnection))
  {
    checkGMC(StateQueue.Last().Timestamp == ServerState_SimulatedProxy().Timestamp)
    // @attention It is possible that this is already set to true (in case of a replication retry).
    StateQueue.Last().LastSerialized[TargetConnection].bReplicatedToSimulatedProxy = true;
  }
}

void UGenMovementReplicationComponent::Server_EnableServerAuthPhysicsSimulation(bool bEnable, FServerAuthPhysicsSettings Settings)
{
  if (!IsServerPawn())
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  if (bIsUsingClientAuthPhysicsReplication)
  {
    GMC_LOG(
      Warning,
      TEXT("Client-authoritative physics replication is already active, cannot enable server-authoritative physics replication.")
    )
    checkGMC(false)
    return;
  }

  if (bIsUsingServerAuthPhysicsReplication == bEnable)
  {
    return;
  }

  bIsUsingServerAuthPhysicsReplication = bEnable;

  RootComponent->SetSimulatePhysics(bEnable);

  if (IsRemotelyControlledServerPawn())
  {
    Client_OnServerAuthPhysicsSimulationToggled(bEnable, Settings);
  }

  OnServerAuthPhysicsSimulationToggled(bEnable, Settings);
}

void UGenMovementReplicationComponent::Server_EnableClientAuthPhysicsSimulation(bool bEnable, FClientAuthPhysicsSettings Settings)
{
  if (!IsServerPawn())
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  if (bIsUsingServerAuthPhysicsReplication)
  {
    GMC_LOG(
      Warning,
      TEXT("Server-authoritative physics replication is already active, cannot enable client-authoritative physics replication.")
    )
    checkGMC(false)
    return;
  }

  if (bIsUsingClientAuthPhysicsReplication == bEnable)
  {
    return;
  }

  bIsUsingClientAuthPhysicsReplication = bEnable;

  // Server pawns do not simulate the physics, they just assume the received client transform.
  RootComponent->SetSimulatePhysics(false);

  if (IsRemotelyControlledServerPawn())
  {
    Client_OnClientAuthPhysicsSimulationToggled(bEnable, Settings);
  }

  OnClientAuthPhysicsSimulationToggled(bEnable, Settings);
}

void UGenMovementReplicationComponent::Client_OnServerAuthPhysicsSimulationToggled_Implementation(
  bool bEnabled,
  FServerAuthPhysicsSettings Settings
)
{
  checkGMC(IsAutonomousProxy())

  if (!PawnOwner)
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  // Client pawns do not simulate the physics, they just display the replicated transform.
  RootComponent->SetSimulatePhysics(false);

  OnServerAuthPhysicsSimulationToggled(bEnabled, Settings);
}

void UGenMovementReplicationComponent::Client_OnClientAuthPhysicsSimulationToggled_Implementation(
  bool bEnabled,
  FClientAuthPhysicsSettings Settings
)
{
  checkGMC(IsAutonomousProxy())

  if (!PawnOwner)
  {
    return;
  }

  const auto RootComponent = Cast<UPrimitiveComponent>(PawnOwner->GetRootComponent());
  if (!RootComponent)
  {
    return;
  }

  RootComponent->SetSimulatePhysics(bEnabled);

  OnClientAuthPhysicsSimulationToggled(bEnabled, Settings);
}

void UGenMovementReplicationComponent::Client_ExecuteMove(FMove& Move, EImmediateContext Context, bool bPredicted, bool bReplicated)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_ExecuteMove)

  checkGMC(IsAutonomousProxy())
  if (bReplicated) Client_QuantizeInputValues(Move);
  // Buffer non-replicated state values before the movement. These values will not be affected by the move execution logic.
  Client_BufferLocalState();
  if (bPredicted) ExecuteMove(Move, Context);
  FillMoveWithData(Move, FMove::EStateVars::Output);
  DEBUG_LOG_CLIENT_EXECUTED_MOVE
  if (bReplicated) Client_QuantizePawnStateFrom(Move);
  // Apply the buffered non-replicated values back to the pawn after movement. Any changes made to these values will be undone with this. If
  // they need to be changed, it must be done someplace else.
  Client_ApplyBufferedLocalState();
}

void UGenMovementReplicationComponent::Client_UnpackReplicationUpdate(FState& ServerState)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_UnpackReplicationUpdate)

  // Shorter names for readability.
  auto& SS = ServerState;
  const auto& LRSS = Client_LastReceivedServerState;
  // Fill the updated server state with the correct values.
  if (SS.bSerializeVelocity             && !SS.bReadNewVelocity)             SS.Velocity = LRSS.Velocity;
  if (SS.bSerializeLocation             && !SS.bReadNewLocation)             SS.Location = LRSS.Location;
  if (SS.bSerializeRotationRoll         && !SS.bReadNewRotationRoll)         SS.Rotation.Roll = LRSS.Rotation.Roll;
  if (SS.bSerializeRotationPitch        && !SS.bReadNewRotationPitch)        SS.Rotation.Pitch = LRSS.Rotation.Pitch;
  if (SS.bSerializeRotationYaw          && !SS.bReadNewRotationYaw)          SS.Rotation.Yaw = LRSS.Rotation.Yaw;
  if (SS.bSerializeControlRotationRoll  && !SS.bReadNewControlRotationRoll)  SS.ControlRotation.Roll = LRSS.ControlRotation.Roll;
  if (SS.bSerializeControlRotationPitch && !SS.bReadNewControlRotationPitch) SS.ControlRotation.Pitch = LRSS.ControlRotation.Pitch;
  if (SS.bSerializeControlRotationYaw   && !SS.bReadNewControlRotationYaw)   SS.ControlRotation.Yaw = LRSS.ControlRotation.Yaw;
  if (SS.bSerializeInputMode            && !SS.bReadNewInputMode)            SS.InputMode = LRSS.InputMode;
  // Normalize rotations into [-180,+180] range.
  ServerState.Rotation.Normalize();
  ServerState.ControlRotation.Normalize();
  // Quantize so we have the same values as the server.
  ServerState.QuantizeVelocity();
  ServerState.QuantizeLocation();
  ServerState.QuantizeRotation();
  ServerState.QuantizeControlRotation();
  // Values that are not available (were not deserialized) are invalid.
  if (!SS.bSerializeVelocity)             Rep_SetInvalid(SS.Velocity);
  if (!SS.bSerializeLocation)             Rep_SetInvalid(SS.Location);
  if (!SS.bSerializeRotationRoll)         Rep_SetInvalid(SS.Rotation.Roll);
  if (!SS.bSerializeRotationPitch)        Rep_SetInvalid(SS.Rotation.Pitch);
  if (!SS.bSerializeRotationYaw)          Rep_SetInvalid(SS.Rotation.Yaw);
  if (!SS.bSerializeControlRotationRoll)  Rep_SetInvalid(SS.ControlRotation.Roll);
  if (!SS.bSerializeControlRotationPitch) Rep_SetInvalid(SS.ControlRotation.Pitch);
  if (!SS.bSerializeControlRotationYaw)   Rep_SetInvalid(SS.ControlRotation.Yaw);
  // Unpack the received bound data.
  Client_UnpackBoundData(ServerState);
  // Save the received state for future reference.
  Client_LastReceivedServerState = ServerState;
}

bool UGenMovementReplicationComponent::Client_CheckReliableBuffer(AActor* Owner) const
{
  if (!Owner)
  {
    return false;
  }

  const auto Connection = Owner->GetNetConnection();
  if (!Connection)
  {
    return false;
  }

  const auto ActorChannel = Connection->FindActorChannelRef(Owner);
  if (!ActorChannel)
  {
    return false;
  }

  // Check the number of out packets waiting to be acked. The margin should not be too small so that other reliable client RPCs that may
  // exist outside of the replication component won't cause a disconnect.
  // @attention The protection margin should at least be 1 since we may still have pending moves waiting to be sent.
  static_assert(RELIABLE_BUFFER_OVERFLOW_PROTECTION_MARGIN >= 1, "The protection margin must not be less than 1.");
  if (ActorChannel->NumOutRec < (RELIABLE_BUFFER - 1) - RELIABLE_BUFFER_OVERFLOW_PROTECTION_MARGIN)
  {
    return true;
  }

  GMC_LOG(
    Verbose,
    TEXT("Reliable RPC buffer is about to overflow (num unacknowledged packets = %d, tolerance = %d)."),
    ActorChannel->NumOutRec,
    RELIABLE_BUFFER_OVERFLOW_PROTECTION_MARGIN
  )
  return false;
}

bool UGenMovementReplicationComponent::Client_MaintainMoveQueue(const FMove& NewMove, bool& bOutStartedNewMove, bool& bOutMoveQueueFull)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_MaintainMoveQueue)

  // A full move queue doesn't necessarily mean that the new move will be discarded, as it can still be combined with the last move.
  bOutMoveQueueFull = Client_MoveQueue.Num() == MoveQueueMaxSize;

  bool bCombineMove{false};
  if ((bOutStartedNewMove = Client_ShouldEnqueueMove(NewMove, bCombineMove)) == true)
  {
    checkGMC(bCombineMove == false)
    // Something important changed with this move so we enqueue the move. This finalizes the last move.
    return Client_AddToMoveQueue(NewMove, bOutMoveQueueFull);
  }

  if (bCombineMove)
  {
    checkGMC(Client_MoveQueue.Num() > 0)
    checkGMC(NewMove.Timestamp > Client_MoveQueue.Last().Timestamp)
    auto& LatestMoveInQueue = Client_MoveQueue.Last();
    // The new move can be combined with the previous one.
    LatestMoveInQueue.Timestamp = NewMove.Timestamp;
    LatestMoveInQueue.DeltaTime += NewMove.DeltaTime;
    LatestMoveInQueue.RotationInput += NewMove.RotationInput;
    // The client-authoritative in-values of the last move must be overwritten, otherwise changes smaller than the configured net tolerance
    // of the respective property would just be ignored.
    if (bUseClientLocation) LatestMoveInQueue.InLocation = NewMove.InLocation;
    if (bUseClientRotation) LatestMoveInQueue.InRotation = NewMove.InRotation;
    if (bUseClientControlRotation) LatestMoveInQueue.InControlRotation = NewMove.InControlRotation;
    return true;
  }

  // This move has an inconsistent timestamp or could cause a reliable buffer overflow, it will not be added to the move queue.
  return false;
}

bool UGenMovementReplicationComponent::Client_ShouldEnqueueMove(const FMove& CurrentMove, bool& bOutCombineMove)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_ShouldEnqueueMove)

  bOutCombineMove = false;

  if (!Client_CheckReliableBuffer(PawnOwner))
  {
    // The reliable buffer is about to overflow, do not accept any more moves until the buffer has more capacity again.
    GMC_LOG(Verbose, TEXT("A client move was discarded from the move queue to prevent a reliable buffer overflow."))
    checkGMC(bOutCombineMove == false)
    return false;
  }
  const int32 ClientMoveQueueNum = Client_MoveQueue.Num();
  if (ClientMoveQueueNum == 0)
  {
    // The move queue should never be empty except when just spawned, but it can still happen sometimes if the timestamps were inconsistent
    // after a client world time update.
    Client_LastSignificantMove = CurrentMove;
    return true;
  }
  const auto& ClientMoveQueueLast = Client_MoveQueue.Last();
  if (CurrentMove.Timestamp <= ClientMoveQueueLast.Timestamp)
  {
    // If the move has an inconsistent timestamp we don't enqueue because the delta time of a move is calculated from the timestamp
    // difference with the previous move (which would end up being either 0 or negative in this case).
    GMC_LOG(
      VeryVerbose,
      TEXT("Current move has an inconsistent timestamp: timestamp current move = %f | timestamp newest move in the move queue = %f"),
      CurrentMove.Timestamp,
      ClientMoveQueueLast.Timestamp
    )
    GMC_LOG(Verbose, TEXT("A client move was discarded from the move queue due to an inconsistent timestamp."))
    checkGMC(bOutCombineMove == false)
    return false;
  }
  if (ClientMoveQueueNum == 1)
  {
    // We should have at least 2 entries in the move queue for move execution. In very low latency situations it is possible that we
    // frequently only have one move in the queue after moves were cleared, which is completely fine as we are going to add another one now.
    Client_LastSignificantMove = CurrentMove;
    return true;
  }
  if (ClientMoveQueueLast.DeltaTime + CurrentMove.DeltaTime > MaxClientDeltaTime)
  {
    // Enqueue if we exceed the max client delta time, which can be used to ensure that we send updates to the server frequently enough,
    // even if no significant input changes (important for the interpolation of our pawn on other clients or a listen server).
    Client_LastSignificantMove = CurrentMove;
    return true;
  }

  // Implementation specific checks. We continue with the default checks if this returns 0.
  const int32 ResultCustom = Client_ShouldEnqueueMove_Custom(CurrentMove, Client_LastSignificantMove);
  if (ResultCustom > 0)
  {
    // Custom checking forced this move to be enqueued.
    Client_LastSignificantMove = CurrentMove;
    return true;
  }
  if (ResultCustom < 0)
  {
    // Custom checking forced this move to be combined.
    bOutCombineMove = true;
    return false;
  }

  // Check the input vector for any significant change.
  if (
    !CurrentMove.InputVector.Equals(Client_LastSignificantMove.InputVector, GetCompareToleranceVectorMax1(CurrentMove.InputVectorQuantize))
  )
  {
    Client_LastSignificantMove = CurrentMove;
    return true;
  }

  // When we use the client values directly they should not be changed within the replicated tick and could be altered without the move
  // input changing. By choosing a net tolerance for those values we can ensure that they never differ by more than the set amount between
  // client and server.
  if (bUseClientLocation && LocalMove().bSerializeOutLocation)
  {
    if (!CurrentMove.InLocation.Equals(Client_LastSignificantMove.InLocation, LocationNetTolerance))
    {
      Client_LastSignificantMove = CurrentMove;
      return true;
    }
  }
  if (
    bUseClientRotation
    && (
      LocalMove().bSerializeOutRotationRoll
      || LocalMove().bSerializeOutRotationPitch
      || LocalMove().bSerializeOutRotationYaw
    )
  )
  {
    if (!CurrentMove.InRotation.Equals(Client_LastSignificantMove.InRotation, RotationNetTolerance))
    {
      Client_LastSignificantMove = CurrentMove;
      return true;
    }
  }
  if (
    bUseClientControlRotation
    && (
      LocalMove().bSerializeOutControlRotationRoll
      || LocalMove().bSerializeOutControlRotationPitch
      || LocalMove().bSerializeOutControlRotationYaw
    )
  )
  {
    if (!CurrentMove.InControlRotation.Equals(Client_LastSignificantMove.InControlRotation, ControlRotationNetTolerance))
    {
      Client_LastSignificantMove = CurrentMove;
      return true;
    }
  }

  // Check the currently bound input flags.
  if (Client_EnqueueMoveCheckBoundInputFlags(CurrentMove))
  {
    Client_LastSignificantMove = CurrentMove;
    return true;
  }

  // This move can be combined with the previous one.
  bOutCombineMove = true;
  return false;
}

bool UGenMovementReplicationComponent::Client_ShouldSendMoves() const
{
  checkGMC(ClientSendRate > 0)
  if (Client_PendingMoves.Num() > 0 && Client_TimeSinceLastMoveWasSent > 1.f / FMath::Max(ClientSendRate, 1))
  {
    return true;
  }
  return false;
}

void UGenMovementReplicationComponent::Client_DetermineValuesToSend()
{
  // Since the values are compressed, we should not serialize any values that differ by less than the accuracy of the set quantization level
  // from the previous value. This would be a waste of bandwidth since the server would not see any significant change in the value.
  const float InputVectorTolerance = GetCompareToleranceVectorMax1(LocalMove().InputVectorQuantize);
  const float RotationInputTolerance = GetCompareToleranceRotatorMax200(LocalMove().RotationInputQuantize);
  const float OutVelocityTolerance = GetCompareTolerance(LocalMove().OutVelocityQuantize);
  const float OutLocationTolerance = GetCompareTolerance(LocalMove().OutLocationQuantize);
  const float OutRotationTolerance = GetCompareToleranceRotator(LocalMove().OutRotationQuantize);
  const float OutControlRotationTolerance = GetCompareToleranceRotator(LocalMove().OutControlRotationQuantize);
  for (auto& Move : Client_PendingMoves) {
    // Get the serialization options from the local move because the settings may be overridden by a subclass.
    if (LocalMove().bSerializeInputVectorX)            Move.bHasNewInputVectorX            = HasValueChanged(Move.InputVector.X,            Client_LastSentInputVectorX,         InputVectorTolerance);
    if (LocalMove().bSerializeInputVectorY)            Move.bHasNewInputVectorY            = HasValueChanged(Move.InputVector.Y,            Client_LastSentInputVectorY,         InputVectorTolerance);
    if (LocalMove().bSerializeInputVectorZ)            Move.bHasNewInputVectorZ            = HasValueChanged(Move.InputVector.Z,            Client_LastSentInputVectorZ,         InputVectorTolerance);
    if (LocalMove().bSerializeRotationInputRoll)       Move.bHasNewRotationInputRoll       = HasValueChanged(Move.RotationInput.Roll,       Client_LastSentRotationInputRoll,    RotationInputTolerance);
    if (LocalMove().bSerializeRotationInputPitch)      Move.bHasNewRotationInputPitch      = HasValueChanged(Move.RotationInput.Pitch,      Client_LastSentRotationInputPitch,   RotationInputTolerance);
    if (LocalMove().bSerializeRotationInputYaw)        Move.bHasNewRotationInputYaw        = HasValueChanged(Move.RotationInput.Yaw,        Client_LastSentRotationInputYaw,     RotationInputTolerance);
    if (LocalMove().bSerializeOutLocation)             Move.bHasNewOutLocation             = HasValueChanged(Move.OutLocation,              Client_LastSentOutLocation,          OutLocationTolerance);
    if (LocalMove().bSerializeOutVelocity)             Move.bHasNewOutVelocity             = HasValueChanged(Move.OutVelocity,              Client_LastSentOutVelocity,          OutVelocityTolerance);
    if (LocalMove().bSerializeOutRotationRoll)         Move.bHasNewOutRotationRoll         = HasValueChanged(Move.OutRotation.Roll,         Client_LastSentOutRotationRoll,      OutRotationTolerance);
    if (LocalMove().bSerializeOutRotationPitch)        Move.bHasNewOutRotationPitch        = HasValueChanged(Move.OutRotation.Pitch,        Client_LastSentOutRotationPitch,     OutRotationTolerance);
    if (LocalMove().bSerializeOutRotationYaw)          Move.bHasNewOutRotationYaw          = HasValueChanged(Move.OutRotation.Yaw,          Client_LastSentOutRotationYaw,       OutRotationTolerance);
    if (LocalMove().bSerializeOutControlRotationRoll)  Move.bHasNewOutControlRotationRoll  = HasValueChanged(Move.OutControlRotation.Roll,  Client_LastSentControlRotationRoll,  OutControlRotationTolerance);
    if (LocalMove().bSerializeOutControlRotationPitch) Move.bHasNewOutControlRotationPitch = HasValueChanged(Move.OutControlRotation.Pitch, Client_LastSentControlRotationPitch, OutControlRotationTolerance);
    if (LocalMove().bSerializeOutControlRotationYaw)   Move.bHasNewOutControlRotationYaw   = HasValueChanged(Move.OutControlRotation.Yaw,   Client_LastSentControlRotationYaw,   OutControlRotationTolerance);
  }
}

void UGenMovementReplicationComponent::Client_BufferLocalState()
{
  SCOPE_CYCLE_COUNTER(STAT_Client_BufferLocalState)

  if (!ServerState_AutonomousProxy().bSerializeVelocity)
  {
    StateBuffer().Velocity = GetVelocity();
  }
  if (!ServerState_AutonomousProxy().bSerializeLocation)
  {
    StateBuffer().Location = PawnOwner->GetActorLocation();
  }
  {
    const FRotator CurrentRotation = PawnOwner->GetActorRotation();
    if (!ServerState_AutonomousProxy().bSerializeRotationRoll)
    {
      StateBuffer().Rotation.Roll = CurrentRotation.Roll;
    }
    if (!ServerState_AutonomousProxy().bSerializeRotationPitch)
    {
      StateBuffer().Rotation.Pitch = CurrentRotation.Pitch;
    }
    if (!ServerState_AutonomousProxy().bSerializeRotationYaw)
    {
      StateBuffer().Rotation.Yaw = CurrentRotation.Yaw;
    }
  }
  {
    const FRotator CurrentControlRotation = PawnOwner->GetControlRotation();
    if (!ServerState_AutonomousProxy().bSerializeControlRotationRoll)
    {
      StateBuffer().ControlRotation.Roll = CurrentControlRotation.Roll;
    }
    if (!ServerState_AutonomousProxy().bSerializeControlRotationPitch)
    {
      StateBuffer().ControlRotation.Pitch = CurrentControlRotation.Pitch;
    }
    if (!ServerState_AutonomousProxy().bSerializeControlRotationYaw)
    {
      StateBuffer().ControlRotation.Yaw = CurrentControlRotation.Yaw;
    }
  }
  if (!ServerState_AutonomousProxy().bSerializeInputMode)
  {
    StateBuffer().InputMode = GenPawnOwner->GetInputMode();
  }
}

void UGenMovementReplicationComponent::Client_ApplyBufferedLocalState()
{
  SCOPE_CYCLE_COUNTER(STAT_Client_ApplyBufferedLocalState)

  if (!ServerState_AutonomousProxy().bSerializeVelocity)
  {
    SetVelocity(StateBuffer().Velocity);
  }
  if (!ServerState_AutonomousProxy().bSerializeLocation)
  {
    PawnOwner->SetActorLocation(StateBuffer().Location, false, nullptr, ETeleportType::TeleportPhysics);
  }
  {
    FRotator NewRotation = PawnOwner->GetActorRotation();
    if (!ServerState_AutonomousProxy().bSerializeRotationRoll)
    {
      NewRotation.Roll = StateBuffer().Rotation.Roll;
    }
    if (!ServerState_AutonomousProxy().bSerializeRotationPitch)
    {
      NewRotation.Pitch = StateBuffer().Rotation.Pitch;
    }
    if (!ServerState_AutonomousProxy().bSerializeRotationYaw)
    {
      NewRotation.Yaw = StateBuffer().Rotation.Yaw;
    }
    PawnOwner->SetActorRotation(NewRotation, ETeleportType::TeleportPhysics);
  }
  if (const auto Controller = PawnOwner->GetController())
  {
    FRotator NewControlRotation = PawnOwner->GetControlRotation();
    if (!ServerState_AutonomousProxy().bSerializeControlRotationRoll)
    {
      NewControlRotation.Roll = StateBuffer().ControlRotation.Roll;
    }
    if (!ServerState_AutonomousProxy().bSerializeControlRotationPitch)
    {
      NewControlRotation.Pitch = StateBuffer().ControlRotation.Pitch;
    }
    if (!ServerState_AutonomousProxy().bSerializeControlRotationYaw)
    {
      NewControlRotation.Yaw = StateBuffer().ControlRotation.Yaw;
    }
    Controller->SetControlRotation(NewControlRotation);
  }
  if (!ServerState_AutonomousProxy().bSerializeInputMode)
  {
    GenPawnOwner->SetInputMode(StateBuffer().InputMode);
  }
}

void UGenMovementReplicationComponent::Client_SADSendParameters(float NextDelayValue, float NextSyncTime)
{
  static_assert(SAD_NUM_SAVED_PARAMS > 1, "SAD_NUM_SAVED_PARAMS must be greater than 1.");

  // Shift the whole array to the left to enqueue the new values. This will discard the oldest parameters.
  FMemory::Memmove(&Client_SADLocalParameters[0], &Client_SADLocalParameters[1], sizeof(FSADParameters) * (SAD_NUM_SAVED_PARAMS - 1));

  // Save the new values to the highest index.
  auto& NewestParameters = Client_SADLocalParameters[SAD_NUM_SAVED_PARAMS - 1];
  NewestParameters.DelayValue = NextDelayValue;
  NewestParameters.SyncTime = NextSyncTime;
}

void UGenMovementReplicationComponent::Client_SADUpdateLocalDelay()
{
  checkGMC(IsClientPawn())
  checkGMC(InterpolationMode == EInterpolationMode::AdaptiveDelay)
  static_assert(SAD_NUM_SAVED_PARAMS > 1, "SAD_NUM_SAVED_PARAMS must be greater than 1.");

  const auto& NewestParameters = Client_SADLocalParameters[SAD_NUM_SAVED_PARAMS - 1];
  if (GetTime() >= NewestParameters.SyncTime)
  {
    SimulationDelay = NewestParameters.DelayValue;
  }
}

bool UGenMovementReplicationComponent::Client_IsVelocityValid(const FMove& SourceMove) const
{
  return GetValidVector(ServerState_AutonomousProxy().Velocity, FVector{0}).Equals(
    GetValidVector(SourceMove.OutVelocity, FVector{0}),
    MaxVelocityError
  );
}

bool UGenMovementReplicationComponent::Client_IsLocationValid(const FMove& SourceMove) const
{
  return GetValidVector(ServerState_AutonomousProxy().Location, FVector{0}).Equals(
    GetValidVector(SourceMove.OutLocation, FVector{0}),
    MaxLocationError
  );
}

bool UGenMovementReplicationComponent::Client_IsRotationValid(const FMove& SourceMove) const
{
  return GetValidRotator(ServerState_AutonomousProxy().Rotation, FRotator{0}).Equals(
    GetValidRotator(SourceMove.OutRotation, FRotator{0}),
    MaxRotationError
  );
}

bool UGenMovementReplicationComponent::Client_IsControlRotationValid(const FMove& SourceMove) const
{
  return GetValidRotator(ServerState_AutonomousProxy().ControlRotation, FRotator{0}).Equals(
    GetValidRotator(SourceMove.OutControlRotation, FRotator{0}),
    MaxControlRotationError
  );
}

bool UGenMovementReplicationComponent::Client_IsInputModeValid(const FMove& SourceMove) const
{
  if (ServerState_AutonomousProxy().bSerializeInputMode)
  {
    return ServerState_AutonomousProxy().InputMode == SourceMove.OutInputMode;
  }
  return true;
}

void UGenMovementReplicationComponent::Client_LoadInputModeForReplay(const FMove& SourceMove)
{
  if (ServerState_AutonomousProxy().bSerializeInputMode)
  {
    GenPawnOwner->SetInputMode(ServerState_AutonomousProxy().InputMode);
  }
  else
  {
    GenPawnOwner->SetInputMode(SourceMove.OutInputMode);
  }
}

void UGenMovementReplicationComponent::Client_ReplayMoves()
{
  SCOPE_CYCLE_COUNTER(STAT_Client_ReplayMoves)

  checkGMC(IsAutonomousProxy())
  checkGMC(bUseClientPrediction)

  DEBUG_LOG_REPLAY_CLIENT_STATE_BEFORE_REPLAY
  if (Client_MoveQueue.Num() > 0)
  {
    Client_BufferLocalState();
    checkGMC(!Client_bIsReplaying)
    Client_bIsReplaying = true;

    TArray<AGenPawn*> RollbackPawnList;
    if (bRollbackClientPawns)
    {
      RollbackPawnList = GatherRollbackPawns();
    }

    TArray<AGenRollbackActor*> GenericRollbackActorList;
    if (bRollbackGenericClientActors)
    {
      GenericRollbackActorList = GatherGenericRollbackActors();
      BufferGenericRollbackActorStates(GenericRollbackActorList, EActorRollbackContext::LocalClientPawnReplayingMove);
    }

    Client_PreReplay();

    for (auto& Move : Client_MoveQueue)
    {
      if (bRollbackClientPawns)
      {
        // Roll back all other pawns for move execution.
        RollbackPawns(Move.Timestamp, RollbackPawnList, ESimulatedContext::RollingBackClientPawn);
      }

      if (bRollbackGenericClientActors)
      {
        // Roll back all generic actors for move execution.
        RollbackGenericActors(
          Move.Timestamp,
          Move.DeltaTime,
          Move,
          GenericRollbackActorList,
          EActorRollbackContext::LocalClientPawnReplayingMove
        );
      }

      FillMoveWithData(Move, FMove::EStateVars::Input);
      ExecuteMove(Move, EImmediateContext::LocalClientPawnReplayingMove);
      FillMoveWithData(Move, FMove::EStateVars::Output);
      Client_QuantizePawnStateFrom(Move);
      Client_PostReplayMoveExecution(Move);
      DEBUG_LOG_REPLAY_CLIENT_REPLAY_LOOP
    }

    Client_OnMovesReplayed();

    if (bRollbackClientPawns)
    {
      // Restore the states of all pawns that were rolled back for move execution.
      RestoreRolledBackPawns(RollbackPawnList);
    }

    if (bRollbackGenericClientActors)
    {
      // Restore the states of all generic actors that were rolled back for move execution.
      RestoreRolledBackGenericActors(GenericRollbackActorList, EActorRollbackContext::RestoringRolledBackClientActors);
    }

    Client_bIsReplaying = false;
    Client_ApplyBufferedLocalState();
  }
  DEBUG_LOG_REPLAY_CLIENT_STATE_AFTER_REPLAY
}

bool UGenMovementReplicationComponent::Client_ShouldReplay(const FMove& SourceMove) const
{
  SCOPE_CYCLE_COUNTER(STAT_Client_ShouldReplay)

  checkGMC(bUseClientPrediction)
  checkGMC(IsGMCEnabled())
  checkGMC(SourceMove.IsValid())
  checkGMC(ServerState_AutonomousProxy().IsValid())

  if (bAlwaysReplay)
  {
    checkGMC(ServerState_AutonomousProxy().bContainsFullRepBatch)
    return true;
  }

  if (!ServerState_AutonomousProxy().bContainsFullRepBatch)
  {
    // Our state is valid because the server didn't send all the replay data, but the input mode, bound data or the velocity could still
    // deviate since these are not checked on the server (because the client doesn't send this data for comparison).
    // @attention Replays from valid source moves are always executed and are not affected by @see Client_IsAllowedToReplay.
    if (!Client_IsInputModeValid(SourceMove))
    {
      // The input mode is out of sync with the server, we need to replay. This will most likely be a noticeable correction.
      GMC_LOG(
        VeryVerbose,
        TEXT("Client move with timestamp %f was valid, but the input mode deviates: ")
        TEXT("ServerState.InputMode = %s | SourceMove.OutInputMode = %s"),
        SourceMove.Timestamp,
        *GetInputModeAsString(ServerState_AutonomousProxy().InputMode),
        *GetInputModeAsString(SourceMove.OutInputMode)
      )
      GMC_LOG(
        Verbose,
        TEXT("Starting replay from valid source move with timestamp %f (input mode result deviates)."),
        SourceMove.Timestamp
      )
      return true;
    }
    if (!Client_IsBoundDataValid(SourceMove))
    {
      // Some bound variable is out of sync with the server, we need to replay. How frequently this can occur and whether this will be a
      // noticeable correction depends on the implementation.
      GMC_LOG(
        Verbose,
        TEXT("Starting replay from valid source move with timestamp %f (bound data result deviates)."),
        SourceMove.Timestamp
      )
      return true;
    }
    // When simulating physics we skip velocity validation because we can't accurately sync the velocity with prediction anyway.
    const auto RootComponent = PawnOwner->GetRootComponent();
    const bool bIsSimulatingPhysics = RootComponent && RootComponent->IsSimulatingPhysics();
    if (!bIsSimulatingPhysics && !Client_IsVelocityValid(SourceMove))
    {
      // Replay when the client move was valid but the velocity deviates. This replay will almost certainly go completely unnoticed by the
      // player since a slight correction in velocity usually doesn't have a big impact on the pawn location.
      GMC_LOG(
        VeryVerbose,
        TEXT("Client move with timestamp %f was valid, but velocity deviates by %f > %f (tolerance): ")
        TEXT("ServerState.Velocity = %13.6f, %13.6f, %13.6f | SourceMove.OutVelocity = %13.6f, %13.6f, %13.6f"),
        SourceMove.Timestamp,
        (GetValidVector(ServerState_AutonomousProxy().Velocity, FVector{0}) - GetValidVector(SourceMove.OutVelocity, FVector{0})).Size(),
        MaxVelocityError,
        ServerState_AutonomousProxy().Velocity.X,
        ServerState_AutonomousProxy().Velocity.Y,
        ServerState_AutonomousProxy().Velocity.Z,
        SourceMove.OutVelocity.X,
        SourceMove.OutVelocity.Y,
        SourceMove.OutVelocity.Z
      )
      GMC_LOG(
        Verbose,
        TEXT("Starting replay from valid source move with timestamp %f (velocity result deviates)."),
        SourceMove.Timestamp
      )
      return true;
    }
    // Our state is in sync with the server.
    return false;
  }

  checkGMC(ServerState_AutonomousProxy().bContainsFullRepBatch)

  // The client move was marked as not valid by the server, but that doesn't necessarily mean we have to replay. Check if our current state
  // is actually out of sync with the server.
  const auto RootComponent = PawnOwner->GetRootComponent();
  const bool bIsSimulatingPhysics = RootComponent && RootComponent->IsSimulatingPhysics();
  const bool bVelocityIsValid = bIsSimulatingPhysics ? true : Client_IsVelocityValid(SourceMove);
  const bool bLocationIsValid = bUseClientLocation ? true : Client_IsLocationValid(SourceMove);
  const bool bRotationIsValid = bUseClientRotation ? true : Client_IsRotationValid(SourceMove);
  const bool bControlRotationIsValid = bUseClientControlRotation ? true : Client_IsControlRotationValid(SourceMove);
  const bool bInputModeIsValid = Client_IsInputModeValid(SourceMove);
  const bool bBoundDataIsValid = Client_IsBoundDataValid(SourceMove);
  const bool bClientStateIsValid =
    bVelocityIsValid
    && bLocationIsValid
    && bRotationIsValid
    && bControlRotationIsValid
    && bInputModeIsValid
    && bBoundDataIsValid;

  GMC_CLOG(
    !bVelocityIsValid,
    VeryVerbose,
    TEXT("Client velocity deviates by %f > %f (tolerance): ")
    TEXT("SourceMove.Timestamp = %f | ")
    TEXT("ServerState.Velocity = %13.6f, %13.6f, %13.6f | SourceMove.OutVelocity = %13.6f, %13.6f, %13.6f"),
    (GetValidVector(ServerState_AutonomousProxy().Velocity, FVector{0}) - GetValidVector(SourceMove.OutVelocity, FVector{0})).Size(),
    MaxVelocityError,
    SourceMove.Timestamp,
    ServerState_AutonomousProxy().Velocity.X,
    ServerState_AutonomousProxy().Velocity.Y,
    ServerState_AutonomousProxy().Velocity.Z,
    SourceMove.OutVelocity.X,
    SourceMove.OutVelocity.Y,
    SourceMove.OutVelocity.Z
  )
  GMC_CLOG(
    !bLocationIsValid,
    VeryVerbose,
    TEXT("Client location deviates by %f > %f (tolerance): ")
    TEXT("SourceMove.Timestamp = %f | ")
    TEXT("ServerState.Location = %13.6f, %13.6f, %13.6f | SourceMove.OutLocation = %13.6f, %13.6f, %13.6f"),
    (GetValidVector(ServerState_AutonomousProxy().Location, FVector{0}) - GetValidVector(SourceMove.OutLocation, FVector{0})).Size(),
    MaxLocationError,
    SourceMove.Timestamp,
    ServerState_AutonomousProxy().Location.X,
    ServerState_AutonomousProxy().Location.Y,
    ServerState_AutonomousProxy().Location.Z,
    SourceMove.OutLocation.X,
    SourceMove.OutLocation.Y,
    SourceMove.OutLocation.Z
  )
  GMC_CLOG(
    !bRotationIsValid,
    VeryVerbose,
    TEXT("Client rotation deviates by %f > %f (tolerance): ")
    TEXT("SourceMove.Timestamp = %f | ")
    TEXT("ServerState.Rotation = %13.6f, %13.6f, %13.6f | SourceMove.OutRotation = %13.6f, %13.6f, %13.6f"),
    (GetValidRotator(ServerState_AutonomousProxy().Rotation, FRotator{0}).GetDenormalized() - GetValidRotator(SourceMove.OutRotation,
      FRotator{0}).GetDenormalized()).Euler().Size(),
    MaxRotationError,
    SourceMove.Timestamp,
    ServerState_AutonomousProxy().Rotation.Roll,
    ServerState_AutonomousProxy().Rotation.Pitch,
    ServerState_AutonomousProxy().Rotation.Yaw,
    SourceMove.OutRotation.Roll,
    SourceMove.OutRotation.Pitch,
    SourceMove.OutRotation.Yaw
  )
  GMC_CLOG(
    !bControlRotationIsValid,
    VeryVerbose,
    TEXT("Client control rotation deviates by %f > %f (tolerance): ")
    TEXT("SourceMove.Timestamp = %f | ")
    TEXT("ServerState.ControlRotation = %13.6f, %13.6f, %13.6f | SourceMove.OutControlRotation = %13.6f, %13.6f, %13.6f"),
    (GetValidRotator(ServerState_AutonomousProxy().ControlRotation, FRotator{0}).GetDenormalized() - GetValidRotator(SourceMove.
      OutControlRotation, FRotator{0}).GetDenormalized()).Euler().Size(),
    MaxControlRotationError,
    SourceMove.Timestamp,
    ServerState_AutonomousProxy().ControlRotation.Roll,
    ServerState_AutonomousProxy().ControlRotation.Pitch,
    ServerState_AutonomousProxy().ControlRotation.Yaw,
    SourceMove.OutControlRotation.Roll,
    SourceMove.OutControlRotation.Pitch,
    SourceMove.OutControlRotation.Yaw
  )
  GMC_CLOG(
    !bInputModeIsValid,
    VeryVerbose,
    TEXT("Client input mode deviates: ")
    TEXT("SourceMove.Timestamp = %f | ")
    TEXT("ServerState.InputMode = %s | SourceMove.InputMode = %s"),
    SourceMove.Timestamp,
    *GetInputModeAsString(ServerState_AutonomousProxy().InputMode),
    *GetInputModeAsString(SourceMove.OutInputMode)
  )

  if (bClientStateIsValid)
  {
    // Even though the move was marked invalid the client state is still in sync with the server, no need to replay.
    return false;
  }
  if (!Client_IsAllowedToReplay())
  {
    GMC_LOG(Verbose, TEXT("Client state is deviating but a replay from an invalid source move is currently not allowed."))
    return false;
  }

  GMC_CLOG(
    !bVelocityIsValid,
    Verbose,
    TEXT("Starting replay from invalid source move with timestamp %f (velocity result deviates)."),
    SourceMove.Timestamp
  )
  GMC_CLOG(
    !bLocationIsValid,
    Verbose,
    TEXT("Starting replay from invalid source move with timestamp %f (location result deviates)."),
    SourceMove.Timestamp
  )
  GMC_CLOG(
    !bRotationIsValid,
    Verbose,
    TEXT("Starting replay from invalid source move with timestamp %f (rotation result deviates)."),
    SourceMove.Timestamp
  )
  GMC_CLOG(
    !bControlRotationIsValid,
    Verbose,
    TEXT("Starting replay from invalid source move with timestamp %f (control rotation result deviates)."),
    SourceMove.Timestamp
  )
  GMC_CLOG(
    !bInputModeIsValid,
    Verbose,
    TEXT("Starting replay from invalid source move with timestamp %f (input mode deviates)."),
    SourceMove.Timestamp
  )
  GMC_CLOG(
    !bBoundDataIsValid,
    Verbose,
    TEXT("Starting replay from invalid source move with timestamp %f (bound data deviates)."),
    SourceMove.Timestamp
  )

  // The client state is invalid and we are allowed to replay.
  return true;
}

bool UGenMovementReplicationComponent::Client_IsAllowedToReplay() const
{
  // Currently only one optional restriction is implemented which is that we only allow a replay when we are exceeding a certain velocity.
  // This can make corrections less noticeable but could let us rest in an invalid state.
  if (bOnlyReplayWhenMoving)
  {
    if (GetVelocity().SizeSquared() < FMath::Square(ReplaySpeedThreshold))
    {
      // We are currently not exceeding the velocity threshold so we are not allowed to replay.
      return false;
    }
  }
  return true;
}

void UGenMovementReplicationComponent::Client_AdoptServerStateForReplay(bool bContainsFullRepBatch, const FMove& SourceMove)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_AdoptServerStateForReplay)

  // Values that were not replicated are filled in with the current state and remain unchanged after the replay.
  SetVelocity(GetValidVelocity(ServerState_AutonomousProxy().Velocity));
  Client_LoadInputModeForReplay(SourceMove);
  Client_LoadBoundDataForReplay(SourceMove);
  constexpr ETeleportType TeleportPhysics = ETeleportType::TeleportPhysics;
  if (!bContainsFullRepBatch)
  {
    // The move was valid and only the velocity deviates, the other state values can be taken from the source move.
    PawnOwner->SetActorLocation(GetValidActorLocation(SourceMove.OutLocation), false, nullptr, TeleportPhysics);
    PawnOwner->SetActorRotation(GetValidActorRotation(SourceMove.OutRotation), TeleportPhysics);
    if (const auto Controller = PawnOwner->GetController())
    {
      Controller->SetControlRotation(GetValidControlRotation(SourceMove.OutControlRotation));
    }
  }
  else
  {
    // The move was not valid because the server state contains all the replicated data, so set all values from the received server state.
    PawnOwner->SetActorLocation(GetValidActorLocation(ServerState_AutonomousProxy().Location), false, nullptr, TeleportPhysics);
    PawnOwner->SetActorRotation(GetValidActorRotation(ServerState_AutonomousProxy().Rotation), TeleportPhysics);
    if (const auto Controller = PawnOwner->GetController())
    {
      Controller->SetControlRotation(GetValidControlRotation(ServerState_AutonomousProxy().ControlRotation));
    }
  }
  Client_OnServerStateAdopted(SourceMove, ServerState_AutonomousProxy());
  // @attention We don't need to call the @see OnImmediateStateLoaded event here as it is going to get called during replay.
}

void UGenMovementReplicationComponent::Client_AdoptServerStateDirectly()
{
  // Use a move struct to temporarily hold the current values of bound data and the input mode. These values will be used for non-replicated
  // bound members. Using @see Client_AdoptServerStateForReplay for this is currently the most convenient way to set the pawn state directly
  // from the server state while still considering the serialization/replication settings saved in the server state. We are not actually
  // going to execute a replay after this, it's just a trick.
  FMove StandInMove;
  StandInMove.OutInputMode = GenPawnOwner->GetInputMode();
  SaveBoundDataToMove(StandInMove, FMove::EStateVars::Output);
  Client_AdoptServerStateForReplay(true, StandInMove);
  OnImmediateStateLoaded(EImmediateContext::LocalClientAdoptingServerStateDirectly);
}

bool UGenMovementReplicationComponent::Client_ShouldUseSmoothCorrections() const
{
  return bUseSmoothCorrections && !bAlwaysReplay && IsAutonomousProxy();
}

void UGenMovementReplicationComponent::Client_HandleSmoothCorrectionOnWorldTickStart()
{
  checkGMC(IsAutonomousProxy())

  // Restore the actual pawn transform, the smoothed transform is only for display.
  Client_SwapCorrectionTransform(Client_CorrectionBuffer.TransformBuffer, true);

  if (Client_CorrectionBuffer.AccumulatedTime >= SmoothCorrectionTime)
  {
    // The target transform has been reached (i.e. the interpolation ratio is 1), reset all correction data.
    Client_StartCorrectionTransform = Client_TargetCorrectionTransform = FReplayCorrectionTransform{};
    Client_CorrectionBuffer = FReplayCorrectionBuffer{};
    checkGMC(Client_CorrectionBuffer.AccumulatedTime == -1.f)
  }
}

void UGenMovementReplicationComponent::Client_HandleSmoothCorrectionOnWorldTickEnd(float DeltaTime, ELevelTick TickType)
{
  checkGMC(IsAutonomousProxy())
  checkGMC(DeltaTime >= MIN_DELTA_TIME)

  // @attention The delta time must necessarily be greater than 0 here so that @see FReplayCorrectionBuffer::HasBegunCorrection returns true
  // on the next frame to swap the buffer back.
  Client_CorrectionBuffer.AccumulatedTime += FMath::Clamp(DeltaTime, MIN_DELTA_TIME, BIG_NUMBER);

  // The target transform is the actual current transform of the pawn.
  Client_SaveCorrectionTransform(Client_TargetCorrectionTransform);

  // Calculate the interpolated values.
  Client_InterpolateCorrectionTransform(
    Client_CorrectionBuffer,
    Client_StartCorrectionTransform,
    Client_TargetCorrectionTransform,
    SmoothCorrectionTime
  );

  // Set the pawn to the smoothed transform for display.
  Client_SwapCorrectionTransform(Client_CorrectionBuffer.TransformBuffer, true);

  if (const auto GenController = Cast<AGenPlayerController>(PawnOwner->GetController()))
  {
    checkGMC(GenController->WasAutonomousProxyCameraManagerUpdateDeferred())
    GenController->Client_UpdateCameraForCorrection(DeltaTime);
  }
}

void UGenMovementReplicationComponent::Client_SaveCorrectionTransform(FReplayCorrectionTransform& Transform)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_SaveCorrectionTransform)

  checkGMC(SimulatedRootComponent)
  Transform.Location = PawnOwner->GetActorLocation();
  Transform.Rotation = PawnOwner->GetActorRotation();
  Transform.ControlRotation = PawnOwner->GetControlRotation();
  Transform.SimulatedRootLocation = SimulatedRootComponent->GetComponentLocation();
  Transform.SimulatedRootRotation = SimulatedRootComponent->GetComponentRotation();
}

void UGenMovementReplicationComponent::Client_SwapCorrectionTransform(FReplayCorrectionTransform& SwapTransform, bool bSimulate)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_SwapCorrectionTransform)

  checkGMC(SimulatedRootComponent)
  const FVector SwapActorLocation = PawnOwner->GetActorLocation();
  const FRotator SwapActorRotation = PawnOwner->GetActorRotation();
  const FVector SwapSimulatedRootLocation = SimulatedRootComponent->GetComponentLocation();
  const FRotator SwapSimulatedRootRotation = SimulatedRootComponent->GetComponentRotation();
  const FRotator SwapControlRotation = PawnOwner->GetControlRotation();
  if (!bSimulate)
  {
    PawnOwner->SetActorLocation(SwapTransform.Location, false, nullptr, ETeleportType::TeleportPhysics);
    PawnOwner->SetActorRotation(SwapTransform.Rotation, ETeleportType::TeleportPhysics);
  }
  SimulatedRootComponent->SetWorldLocation(SwapTransform.SimulatedRootLocation, false, nullptr, ETeleportType::TeleportPhysics);
  SimulatedRootComponent->SetWorldRotation(SwapTransform.SimulatedRootRotation, false, nullptr, ETeleportType::TeleportPhysics);
  if (const auto Controller = PawnOwner->GetController()) Controller->SetControlRotation(SwapTransform.ControlRotation);
  SwapTransform.Location = SwapActorLocation;
  SwapTransform.Rotation = SwapActorRotation;
  SwapTransform.SimulatedRootLocation = SwapSimulatedRootLocation;
  SwapTransform.SimulatedRootRotation = SwapSimulatedRootRotation;
  SwapTransform.ControlRotation = SwapControlRotation;
}

void UGenMovementReplicationComponent::Client_InterpolateCorrectionTransform(
  FReplayCorrectionBuffer& Buffer,
  const FReplayCorrectionTransform& StartTransform,
  const FReplayCorrectionTransform& TargetTransform,
  float TotalDuration
) const
{
  SCOPE_CYCLE_COUNTER(STAT_Client_InterpolateCorrectionTransform)

  checkGMC(TotalDuration >= 0.01f)
  const float InterpolationRatio = FMath::Clamp(Buffer.AccumulatedTime / FMath::Clamp(TotalDuration, 0.01f, BIG_NUMBER), 0.f, 1.f);
  auto& TransformBuffer = Buffer.TransformBuffer;

  TransformBuffer.Location =
    bUseClientLocation ? TargetTransform.Location : FMath::LerpStable(
      StartTransform.Location,
      TargetTransform.Location,
      InterpolationRatio
    );

  TransformBuffer.SimulatedRootLocation =
    bUseClientLocation ? TargetTransform.SimulatedRootLocation : FMath::LerpStable(
      StartTransform.SimulatedRootLocation,
      TargetTransform.SimulatedRootLocation,
      InterpolationRatio
    );

  TransformBuffer.Rotation =
    bUseClientRotation ? TargetTransform.Rotation : FQuat::Slerp(
      StartTransform.Rotation.Quaternion(),
      TargetTransform.Rotation.Quaternion(),
      InterpolationRatio
    ).Rotator();

  TransformBuffer.SimulatedRootRotation =
    bUseClientRotation ? TargetTransform.SimulatedRootRotation : FQuat::Slerp(
      StartTransform.SimulatedRootRotation.Quaternion(),
      TargetTransform.SimulatedRootRotation.Quaternion(),
      InterpolationRatio
    ).Rotator();

  TransformBuffer.ControlRotation =
    bUseClientControlRotation ? TargetTransform.ControlRotation : FQuat::Slerp(
      StartTransform.ControlRotation.Quaternion(),
      TargetTransform.ControlRotation.Quaternion(),
      InterpolationRatio
    ).Rotator();
}

FMove UGenMovementReplicationComponent::Client_ClearAcknowledgedMoves(float ReceivedTimestamp)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_ClearAcknowledgedMoves)

  TArray<FMove> TempMoveQueue;
  TempMoveQueue.Reserve(MoveQueueMaxSize);
  FMove SourceMove;
  for (const auto& Move : Client_MoveQueue)
  {
    if (Move.Timestamp > ReceivedTimestamp)
    {
      TempMoveQueue.Emplace(Move);
    }
    else if (FMath::IsNearlyEqual(Move.Timestamp, ReceivedTimestamp, 0.000001f))
    {
      SourceMove = Move;
      // Mark values that are not replicated from the server in the source move as well.
      auto& SSAP = ServerState_AutonomousProxy();
      if (!SSAP.bSerializeVelocity)             Rep_SetInvalid(SourceMove.OutVelocity);
      if (!SSAP.bSerializeLocation)             Rep_SetInvalid(SourceMove.OutLocation);
      if (!SSAP.bSerializeRotationRoll)         Rep_SetInvalid(SourceMove.OutRotation.Roll);
      if (!SSAP.bSerializeRotationPitch)        Rep_SetInvalid(SourceMove.OutRotation.Pitch);
      if (!SSAP.bSerializeRotationYaw)          Rep_SetInvalid(SourceMove.OutRotation.Yaw);
      if (!SSAP.bSerializeControlRotationRoll)  Rep_SetInvalid(SourceMove.OutControlRotation.Roll);
      if (!SSAP.bSerializeControlRotationPitch) Rep_SetInvalid(SourceMove.OutControlRotation.Pitch);
      if (!SSAP.bSerializeControlRotationYaw)   Rep_SetInvalid(SourceMove.OutControlRotation.Yaw);

      // Fill out the input flags of the received state from the source move because they are not replicated from the server.
      SSAP.bInputFlag1  = SourceMove.bInputFlag1;
      SSAP.bInputFlag2  = SourceMove.bInputFlag2;
      SSAP.bInputFlag3  = SourceMove.bInputFlag3;
      SSAP.bInputFlag4  = SourceMove.bInputFlag4;
      SSAP.bInputFlag5  = SourceMove.bInputFlag5;
      SSAP.bInputFlag6  = SourceMove.bInputFlag6;
      SSAP.bInputFlag7  = SourceMove.bInputFlag7;
      SSAP.bInputFlag8  = SourceMove.bInputFlag8;
      SSAP.bInputFlag9  = SourceMove.bInputFlag9;
      SSAP.bInputFlag10 = SourceMove.bInputFlag10;
      SSAP.bInputFlag11 = SourceMove.bInputFlag11;
      SSAP.bInputFlag12 = SourceMove.bInputFlag12;
      SSAP.bInputFlag13 = SourceMove.bInputFlag13;
      SSAP.bInputFlag14 = SourceMove.bInputFlag14;
      SSAP.bInputFlag15 = SourceMove.bInputFlag15;
      SSAP.bInputFlag16 = SourceMove.bInputFlag16;

      // Client-authoritative values do not need to be replicated from the server and can be taken from the source move as well. This is
      // also less error-prone than relying on a replicated value as server packets can get lost.
      if (bUseClientLocation) SSAP.Location = SourceMove.OutLocation;
      if (bUseClientRotation) SSAP.Rotation = SourceMove.OutRotation;
      if (bUseClientControlRotation) SSAP.ControlRotation = SourceMove.OutControlRotation;
    }
  }
  Client_MoveQueue = MoveTemp(TempMoveQueue);

  // An empty move queue after clearing should theoretically not be possible, but it can still happen in practice due to inconsistent
  // timestamps.
  GMC_CLOG(Client_MoveQueue.Num() == 0, Verbose, TEXT("Client move queue is empty after clearing acknowledged moves."))

  // Not finding a source move should only happen very rarely (e.g. during lag spikes).
  if (!SourceMove.IsValid())
  {
    GMC_LOG(
      Verbose,
      TEXT("No source move found while clearing acknowledged moves: ")
      TEXT("oldest timestamp in the queue is %s, received server state timestamp is %f ")
      TEXT("(the received server state may have been based on a proxy move)."),
      Client_MoveQueue.Num() > 0 ? *FString::SanitizeFloat(Client_MoveQueue[0].Timestamp) : TEXT("<queue is empty>"),
      ReceivedTimestamp
    )
  }

  return SourceMove;
}

bool UGenMovementReplicationComponent::Client_AddToMoveQueue(const FMove& NewMove, bool& bOutMoveQueueFull)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_AddToMoveQueue)

  bOutMoveQueueFull = false;
  if (Client_MoveQueue.Num() == MoveQueueMaxSize)
  {
    // Should only ever happen if there is a severe lag spike. If this happens during good network conditions, you should either increase
    // the move queue max size or increase the net update frequency for the autonomous proxy (will have the effect that the move queue gets
    // cleared more often which will prevent moves from accumulating).
    GMC_LOG(
      Verbose,
      TEXT("Max move queue size of %d moves reached. Move queue max size may need to be increased if this occurs repeatedly."),
      MoveQueueMaxSize
    )
    bOutMoveQueueFull = true;
    return false;
  }
  // We don't need to check for things like a valid timestamp here anymore because those things have been checked already when it was
  // determined that the move should be enqueued.
  Client_MoveQueue.Emplace(NewMove);
  if (Client_MoveQueue.Num() == MoveQueueMaxSize)
  {
    // The move queue might be full after enqueueing the new move, which is not a problem (yet) if the queue gets cleared before the next
    // local move is created that cannot be combined.
    bOutMoveQueueFull = true;
  }
  return true;
}

void UGenMovementReplicationComponent::Client_QuantizeInputValues(FMove& Move)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_QuantizeInputValues)

  Move.QuantizeInputVector();
  Move.QuantizeRotationInput();

  if (bUseClientLocation)
  {
    Move.QuantizeInLocation();
  }
  if (bUseClientRotation)
  {
    Move.InRotation.Normalize();
    Move.QuantizeInRotation();
  }
  if (bUseClientControlRotation)
  {
    Move.InControlRotation.Normalize();
    Move.QuantizeInControlRotation();
  }
  // @attention If client-authoritative values are used the pawn will be set to the quantized state when the move is loaded during move
  // execution so we don't need to do that here already.
}

void UGenMovementReplicationComponent::Client_QuantizePawnStateFrom(FMove& Move)
{
  SCOPE_CYCLE_COUNTER(STAT_Client_QuantizePawnStateFrom)

  // We need to enforce the same quantization level for the velocity across server and client to remain synced and to avoid triggering
  // replays (which cost performance and can cause teleports for the client). Keep in mind that the the client's velocity won't actually be
  // replicated to the server. The server calculates the velocity from the client move with its local in-velocity, quantizes the result, and
  // sends it back to the client. If we didn't quantize our velocity locally as well on the client, the replicated (rounded) velocity would
  // differ from the local velocity (which would be non-rounded in this case). They would also differ if they were quantized with a
  // different level, which is why we ensure they are the same here.
  Move.OutVelocityQuantize = ServerState_AutonomousProxy().VelocityQuantize;
  Move.QuantizeOutVelocity();
  SetVelocity(Move.OutVelocity);
  GMC_CLOG(
    bUseClientLocation && !Move.OutLocation.Equals(Move.InLocation, 0.01f),
    Warning,
    TEXT("<bUseClientLocation> is true but the pawn's location changed during move execution on the client: ")
    TEXT("PreMoveExecutionLocation = %s | PostMoveExecutionLocation = %s"),
    *GetValidActorLocation(Move.InLocation).ToString(),
    *Move.OutLocation.ToString()
  )
  checkGMC(bUseClientLocation ? Move.OutLocation.Equals(Move.InLocation, 0.01f) : true)
  if (!bUseClientLocation)
  {
    Move.QuantizeOutLocation();
    PawnOwner->SetActorLocation(Move.OutLocation, false, nullptr, ETeleportType::TeleportPhysics);
  }
  Move.OutRotation.Normalize();
  GMC_CLOG(
    bUseClientRotation && !Move.OutRotation.Equals(Move.InRotation, 0.1f),
    Warning,
    TEXT("<bUseClientRotation> is true but the pawn's rotation changed during move execution on the client: ")
    TEXT("PreMoveExecutionRotation = %s | PostMoveExecutionRotation = %s"),
    *GetValidActorRotation(Move.InRotation).ToString(),
    *Move.OutRotation.ToString()
  )
  checkGMC(bUseClientRotation ? Move.OutRotation.Equals(Move.InRotation, 0.1f) : true)
  if (!bUseClientRotation)
  {
    Move.QuantizeOutRotation();
    PawnOwner->SetActorRotation(Move.OutRotation, ETeleportType::TeleportPhysics);
  }
  Move.OutControlRotation.Normalize();
  GMC_CLOG(
    bUseClientControlRotation && !Move.OutControlRotation.Equals(Move.InControlRotation, 0.1f),
    Warning,
    TEXT("<bUseClientControlRotation> is true but the pawn's control rotation changed during move execution on the client: ")
    TEXT("PreMoveExecutionControlRotation = %s | PostMoveExecutionControlRotation = %s"),
    *GetValidControlRotation(Move.InControlRotation).ToString(),
    *Move.OutControlRotation.ToString()
  )
  checkGMC(bUseClientControlRotation ? Move.OutControlRotation.Equals(Move.InControlRotation, 0.1f) : true)
  if (!bUseClientControlRotation)
  {
    Move.QuantizeOutControlRotation();
    if (const auto Controller = PawnOwner->GetController()) Controller->SetControlRotation(Move.OutControlRotation);
  }
}

void UGenMovementReplicationComponent::SmoothMovement(ESimulatedContext Context, TArray<int32>& OutSkippedStateIndices)
{
  SCOPE_CYCLE_COUNTER(STAT_SmoothMovement)

  checkGMC(InterpolationFunction)
  checkGMC(InterpolationMethod != EInterpolationMethod::None)
  checkGMC(IsSimulatedProxy() || IsSmoothedListenServerPawn() || IsAutonomousProxy() && !bUseClientPrediction)
  checkGMC(OutSkippedStateIndices.Num() == 0)

  DEBUG_LOG_STATE_QUEUE_DATA

  // Check if we have enough states available to do smoothing.
  if (StateQueue.Num() < 2)
  {
    // No start or target state for this iteration. If this happens sporadically during gameplay it might hint to a bad connection. In some
    // scenarios this is expected however. For example, if there is an idle pawn (no controller) or a pawn that does not replicate on the
    // server the state queue on the client will remain empty.
    CurrentStartStateIndex = CurrentTargetStateIndex = -1;
    return;
  }

  if (IsRemotelyControlledListenServerPawn() && !Server_bStateBufferInitialized)
  {
    checkGMC(bSmoothRemoteServerPawn)
    // If the state buffer is not initialized yet we should not apply any smoothing, otherwise the state buffer will initialize to the
    // smoothed state which can cause issues.
    CurrentStartStateIndex = CurrentTargetStateIndex = -1;
    return;
  }

  FState StartState;
  FState TargetState;
  float InterpolationRatio{-1.f};
  const float InterpolationTime = ComputeSmoothingInput(StartState, TargetState, InterpolationRatio);
  DEBUG_LOG_USING_EXTRAPOLATED_DATA

  // Reset the accumulated extrapolation time when returning to interpolation. Only relevant for fully simulated extrapolation.
  if (!bUsingExtrapolatedData) AccumulatedFullSimulatedExtrapolationTime = 0.f;

  if (LastValidInterpolationTime >= InterpolationTime)
  {
    GMC_LOG(
      VeryVerbose,
      TEXT("Invalid interpolation time: current interpolation time = %f | last valid interpolation time = %f"),
      InterpolationTime,
      LastValidInterpolationTime
    )
    CurrentStartStateIndex = CurrentTargetStateIndex = -1;
    return;
  }
  LastValidInterpolationTime = InterpolationTime;

  // Determine states that may have been skipped by the interpolation algorithm.
  if (bDetermineSkippedSmoothingStates) DetermineSkippedStates(OutSkippedStateIndices);

  FState& SmoothState = bUsingExtrapolatedData ? ExtrapolatedState : InterpolatedState;

  if (
    bAllowExtrapolation
    && InterpolationRatio > 1.f
    && (ExtrapolationMode == EExtrapolationMode::SemiSimulated || ExtrapolationMode == EExtrapolationMode::FullSimulated)
  )
  {
    // We are going to simulate the actual movement logic to get an extrapolated state to set the pawn to.
    ExtrapolateSimulated(InterpolationTime, StartState, TargetState, SmoothState, OutSkippedStateIndices);
  }
  else
  {
    // Initialize the smooth state to the start or target state so it reflects all persistent values (like replication settings) correctly.
    // We choose the state that is closest to the interpolation time as initialization state, so the smoothed state holds the most accurate
    // data for any properties that are not interpolated.
    SmoothState = CreateInitializationState(InterpolationTime, StartState, TargetState);
    checkGMC(InterpolationRatio >= 0.f)
    checkGMC(StartState.IsValid())
    checkGMC(TargetState.IsValid())
    checkGMC(SmoothState.IsValid())

    // When we interpolate the resulting values would ideally be exactly the same as those we would have received in a server update at time
    // "InterpolationTime". If we extrapolate the resulting values will most likely be much less accurate. The extrapolation state can be
    // used once we have a new server state available again as the starting point for interpolation. That way we can at least get a
    // relatively smooth correction to the actual server position instead of a teleport if the connection recovered quickly enough.
    ComputeInterpolatedState(InterpolationRatio, StartState, TargetState, SmoothState);
  }

  // Set the pawn to its new state.
  SetReplicatedPawnState(SmoothState, StartState, TargetState, bSkipBoneUpdateForSmoothing, Context);

  DEBUG_LOG_SMOOTHING_INTERPOLATION_DATA
}

void UGenMovementReplicationComponent::SMLSaveTimestamps(const FState& ServerState)
{
  SCOPE_CYCLE_COUNTER(STAT_SMLSaveTimestamps)

  checkGMC(ServerState.IsValid())

  // Save the current state of the pawn as the start state of the interpolation.
  SaveLocalPawnState(SMLStartState);

  // Set to the current server state for the first update.
  if (SMLUpdateTime == 0.f)
  {
    SMLSimulationTime = SMLTargetStateTimestamp = ServerState.Timestamp;
    SMLTargetDelta = 0.f;
    SMLUpdateTime = GetTime();
    return;
  }

  SMLUpdateTime = GetTime();

  // Slow down if the pawn is already ahead of the previous target state.
  if (SMLSimulationTime > SMLTargetStateTimestamp)
  {
    SMLSimulationTime = FMath::LerpStable(SMLTargetStateTimestamp, SMLSimulationTime, 0.5f);
  }

  // Calculate the time between the last two updates based on the original timestamps (meaning this will be unaffected by network latency
  // fluctuations).
  const float PrevTargetStateTimestamp = SMLTargetStateTimestamp;
  SMLTargetStateTimestamp = ServerState.Timestamp;
  const float ElapsedTime = SMLTargetStateTimestamp - PrevTargetStateTimestamp;

  // Do not exceed the target and do not fall behind by more than "Delta".
  const float MaxDelta = FMath::Min(MatchLatestMaxDeltaTime, SMLTargetStateTimestamp);
  const float MinDelta = FMath::Min(MatchLatestMinDeltaTime, MaxDelta);
  const float Delta = FMath::Clamp(ElapsedTime * SML_MAX_TIME_BEHIND_PERCENT, MinDelta, MaxDelta);
  SMLSimulationTime = FMath::Clamp(SMLSimulationTime, SMLTargetStateTimestamp - Delta, SMLTargetStateTimestamp);

  // Determine the delta between the new target state and the current pawn simulation.
  SMLTargetDelta = SMLTargetStateTimestamp - SMLSimulationTime;
}

void UGenMovementReplicationComponent::SetPawnStateNoInterpolation(ESimulatedContext Context, TArray<int32>& OutSkippedStateIndices)
{
  checkGMC(OutSkippedStateIndices.Num() == 0)

  // Even though we are not interpolating the smoothing info should contain meaningful values.
  const auto& LatestState = StateQueue.Last();
  if (IsSimulatedProxy()) SmoothedControlRotation = ControlRotationToLocal(GetValidControlRotation(LatestState.ControlRotation));
  CurrentStartStateIndex = CurrentTargetStateIndex = StateQueue.Num() - 1;
  InterpolatedState = ExtrapolatedState = LatestState;
  bUsingExtrapolatedData = false;
  AccumulatedFullSimulatedExtrapolationTime = 0.f;
  LastValidInterpolationTime = LatestState.Timestamp;
  checkGMC(LastValidInterpolationTime >= 0.f)

  if (bDetermineSkippedSmoothingStates)
  {
    DetermineSkippedStates(OutSkippedStateIndices);
  }

  // Update all pawn data from the latest state.
  SetReplicatedPawnState(LatestState, LatestState, LatestState, bSkipBoneUpdateForSmoothing, Context);
}

void UGenMovementReplicationComponent::AddSimulatedRootComponent()
{
  checkGMC(!SimulatedRootComponent)
  const auto RootComponent = PawnOwner->GetRootComponent();
  check(RootComponent)
  TArray<USceneComponent*> RootChildrenExclusive;
  RootComponent->GetChildrenComponents(false, RootChildrenExclusive);
  SimulatedRootComponent = NewObject<USceneComponent>(PawnOwner, USceneComponent::StaticClass(), TEXT("SimulatedRoot"));
  PawnOwner->AddInstanceComponent(SimulatedRootComponent);
  SimulatedRootComponent->RegisterComponent();
  PawnOwner->Modify();
  SimulatedRootComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
  for (USceneComponent* Child : RootChildrenExclusive)
  {
    Child->AttachToComponent(SimulatedRootComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
  }
  // Since the component will exist on every pawn and is stably named, we can mark it as net addressable.
  SimulatedRootComponent->SetNetAddressable();
}

void UGenMovementReplicationComponent::RemoveSimulatedRootComponent()
{
  check(SimulatedRootComponent)
  SimulatedRootComponent->UnregisterComponent();
  SimulatedRootComponent->DestroyComponent(false/*do not promote children*/);
  SimulatedRootComponent = nullptr;
}

float UGenMovementReplicationComponent::ComputeSmoothingInput(FState& OutStartState, FState& OutTargetState, float& OutInterpolationRatio)
{
  SCOPE_CYCLE_COUNTER(STAT_ComputeSmoothingInput)

  checkGMC(StateQueue.Num() >= 2)

  // The configured max extrapolation ratio is only relevant for simple extrapolation. If simulated extrapolation is used we just set an
  // arbitrary high number as max ratio (effectively disabling the ratio limit) because simulated extrapolation is capped by a time value
  // (@see MaxExtrapolationDeltaTime).
  const float MaxExtrapolationRatioToUse = ExtrapolationMode == EExtrapolationMode::Simple ? MaxExtrapolationRatio : 1000000.f;

  if (InterpolationMode == EInterpolationMode::MatchLatest)
  {
    SMLSimulationTime = FMath::Clamp(SMLSimulationTime, 0.f, BIG_NUMBER);

    // Although we will try to match the most recent server state from @see SMLStartState, the output start state should always be the state
    // prior to the target state. This will produce better results for bound data when creating the initialization state.
    const int32 StateQueueSize = StateQueue.Num();
    CurrentStartStateIndex = StateQueueSize - 2;
    OutStartState = StateQueue[CurrentStartStateIndex];
    CurrentTargetStateIndex = StateQueueSize - 1;
    OutTargetState = StateQueue[CurrentTargetStateIndex];
    OutInterpolationRatio = 1.f;

    if (auto World = GetWorld())
    {
      // Advance the simulation by delta time seconds.
      SMLSimulationTime += World->GetDeltaSeconds();
    }

    if (SMLTargetDelta > KINDA_SMALL_NUMBER)
    {
      // Prevent the pawn from getting too far ahead of the target state.
      checkGMC(MaxExtrapolationRatioToUse >= 1.f)
      const float MaxTimeAhead = SMLTargetDelta * (bAllowExtrapolation ? MaxExtrapolationRatioToUse - 1.f : 0.f);
      SMLSimulationTime = FMath::Min(SMLSimulationTime, SMLTargetStateTimestamp + MaxTimeAhead);

      // Compute the interpolation ratio for this iteration.
      const float RemainingTime = SMLTargetStateTimestamp - SMLSimulationTime;
      const float TargetTime = SMLTargetDelta - RemainingTime;
      OutInterpolationRatio = TargetTime / SMLTargetDelta;
      checkGMC(OutInterpolationRatio <= MaxExtrapolationRatioToUse + KINDA_SMALL_NUMBER)
    }

    checkGMC(SMLTargetStateTimestamp == StateQueue.Last().Timestamp)
    bUsingExtrapolatedData = SMLSimulationTime > SMLTargetStateTimestamp;

    return SMLSimulationTime;
  }

  checkGMC(InterpolationMode == EInterpolationMode::FixedDelay || InterpolationMode == EInterpolationMode::AdaptiveDelay)

  // The time in the past at which the simulated pawn is going to be displayed.
  float Time = FMath::Clamp(GetTime() - SimulationDelay, 0.f, BIG_NUMBER);

  // Interpolate if we have a recent enough state available, extrapolate if the newest state in the queue is too old.
  if (StateQueue.Last().Timestamp > Time)
  {
    // Find the correct starting state which is the latest state that was received before (or exactly at) the targeted interpolation time.
    // If none is found our state queue does not contain any state old enough, most likely because the buffer size was set too small and
    // should be increased.
    for (int32 Index = StateQueue.Num() - 2; Index >= 0; --Index)
    {
      if (StateQueue[Index].Timestamp <= Time)
      {
        // To avoid teleports we only stop using extrapolation data once we have a more up-to-date starting state available again.
        if (bUsingExtrapolatedData && ExtrapolatedState.Timestamp < StateQueue[Index].Timestamp)
        {
          bUsingExtrapolatedData = false;
        }

        // Set the starting state to the determined index and the target state to the state that is one timestamp later than the start
        // state. These are the two states we want to interpolate between because those states are the closest to the targeted delay. The
        // interpolation ratio tells us how far in between the two states we should be based on the interpolation time.
        CurrentStartStateIndex = Index;
        OutStartState = bUsingExtrapolatedData && ExtrapolatedState.IsValid() ? ExtrapolatedState : StateQueue[CurrentStartStateIndex];
        CurrentTargetStateIndex = Index + 1;
        OutTargetState = StateQueue[CurrentTargetStateIndex];
        OutInterpolationRatio =
          (Time - OutStartState.Timestamp) / FMath::Max(OutTargetState.Timestamp - OutStartState.Timestamp, MIN_DELTA_TIME);
        return Time;
      }
    }
    GMC_LOG(
      Verbose,
      TEXT("No state old enough to interpolate found. State queue max size may need to be increased if this occurs repeatedly.")
    )
    GMC_LOG(
      VeryVerbose,
      TEXT("Oldest available state has timestamp %f, interpolation time is %f, current time is %f."),
      StateQueue[0].Timestamp,
      Time,
      GetTime()
    )
    // The least intrusive way to handle this situation is to use the oldest state that we have as a stopgap.
    CurrentStartStateIndex = CurrentTargetStateIndex = 0;
    OutStartState = OutTargetState = StateQueue[0];
    OutInterpolationRatio = 1.f;
  }
  else
  {
    if (bAllowExtrapolation)
    {
      // We extrapolate based on the two newest states received.
      const int32 StateQueueSize = StateQueue.Num();
      CurrentStartStateIndex = StateQueueSize - 2;
      OutStartState = StateQueue[CurrentStartStateIndex];
      CurrentTargetStateIndex = StateQueueSize - 1;
      OutTargetState = StateQueue[CurrentTargetStateIndex];
      OutInterpolationRatio = FMath::Clamp(
        (Time - OutStartState.Timestamp) / FMath::Max(OutTargetState.Timestamp - OutStartState.Timestamp, MIN_DELTA_TIME),
        1.f, MaxExtrapolationRatioToUse
      );
    }
    else
    {
      GMC_LOG(
        Verbose,
        TEXT("No state recent enough state to interpolate found (extrapolation disabled). ")
        TEXT("Simulation delay may need to be increased if this occurs repeatedly.")
      )
      GMC_LOG(
        VeryVerbose,
        TEXT("Newest available state has timestamp %f, interpolation time is %f, current time is %f."),
        StateQueue.Last().Timestamp,
        Time,
        GetTime()
      )
      CurrentStartStateIndex = CurrentTargetStateIndex = StateQueue.Num() - 1;
      OutStartState = OutTargetState = StateQueue.Last();
      OutInterpolationRatio = 1.f;
    }
    // Even if extrapolation is disabled we still want to use the timestamped extrapolated state to remain consistent with the simulation
    // delay once interpolation is resumed.
    bUsingExtrapolatedData = true;
  }

  return Time;
}

void UGenMovementReplicationComponent::ComputeInterpolatedState(
  float InterpolationRatio,
  const FState& StartState,
  const FState& TargetState,
  FState& SmoothState
) const
{
  SCOPE_CYCLE_COUNTER(STAT_ComputeInterpolatedState)

  FState InterpolationResult =
    (this->*InterpolationFunction)(GetValidInterpolationState(StartState), GetValidInterpolationState(TargetState), InterpolationRatio);
  SmoothState.Velocity = InterpolationResult.Velocity;
  SmoothState.Location = InterpolationResult.Location;
  SmoothState.Rotation = InterpolationResult.Rotation;
  SmoothState.ControlRotation = InterpolationResult.ControlRotation;
  InterpolatedStatePreserveNaN(SmoothState, TargetState);
}

void UGenMovementReplicationComponent::ExtrapolateSimulated(
  float ExtrapolationTime,
  const FState& StartState,
  const FState& TargetState,
  FState& ExtrapolationState,
  const TArray<int32>& SkippedStateIndices
)
{
  SCOPE_CYCLE_COUNTER(STAT_ExtrapolateSimulated)

  checkGMC(bAllowExtrapolation)
  checkGMC(bUsingExtrapolatedData)
  checkGMC(ExtrapolationMode == EExtrapolationMode::SemiSimulated || ExtrapolationMode == EExtrapolationMode::FullSimulated)
  checkGMC(ExtrapolationTime > TargetState.Timestamp)
  checkGMC(ExtrapolationTime > ExtrapolationState.Timestamp)

  const bool bUsingFullSimulationMode = ExtrapolationMode == EExtrapolationMode::FullSimulated;
  if (bUsingFullSimulationMode && HasJustStartedExtrapolation())
  {
    // Extrapolation has just started, begin simulation from the target state for fully simulated extrapolation.
    // @attention When using fully simulated extrapolation the pawn state won't be synchronised with received data again until we can resume
    // interpolation, causing the pawn state to deviate more and more the longer the extrapolation continues.
    ExtrapolationState = TargetState;

    // Make sure the accumulated extrapolation time gets reset.
    AccumulatedFullSimulatedExtrapolationTime = 0.f;
  }

  // "SemiSimulated" always extrapolates based on the latest received state. "FullSimulated" starts simulation from the latest state when
  // extrapolation has just begun, but afterwards the simulation will continue to run on its own until we can interpolate again.
  const auto& InitState = !bUsingFullSimulationMode ? TargetState : ExtrapolationState;

  // Set the pawn to the current extrapolation state.
  SetSimulatedExtrapolationPawnState(InitState);

  // Construct the input move for simulation. For "SemiSimulated" we always extrapolate from the target state (i.e. the latest state) by
  // simulating the whole delta time between the latest state timestamp and the current extrapolation time. For "FullSimulated" we only take
  // the delta time between the last simulation and the current extrapolation time.
  FMove ExtrapolationMove = BuildSimulatedExtrapolationMove(ExtrapolationTime, InitState, TargetState);

  bool bIsExceedingExtrapolationTime = false;
  if (bUsingFullSimulationMode)
  {
    // The accumulated extrapolation time is required to enforce @see MaxExtrapolationDeltaTime for fully simulated extrapolation.
    AccumulatedFullSimulatedExtrapolationTime += ExtrapolationMove.DeltaTime;

    if (AccumulatedFullSimulatedExtrapolationTime > MaxExtrapolationDeltaTime)
    {
      AccumulatedFullSimulatedExtrapolationTime = FMath::Clamp(AccumulatedFullSimulatedExtrapolationTime, 0.f, MaxExtrapolationDeltaTime);
      bIsExceedingExtrapolationTime = true;
    }
  }

  // Only simulate movement if we are not exceeding the max allowed extrapolation time. Only relevant for fully simulated extrapolation.
  if (!bIsExceedingExtrapolationTime)
  {
    // Allows the user to change the input move and make custom adjustments to the pawn state before move execution.
    ExtrapolationMove = PreSimulateMovementForExtrapolation(
      ExtrapolationMove,
      ExtrapolationTime,
      StartState,
      TargetState,
      ExtrapolationState,
      SkippedStateIndices
    );

    // Simulate the movement.
    ExecuteMoveSimulated(ExtrapolationMove);

    // Save the output of the movement simulation to the extrapolated state.
    ExtrapolationState.Timestamp = ExtrapolationTime;
    ExtrapolationState.Velocity = GetVelocity();
    ExtrapolationState.Location = PawnOwner->GetActorLocation();
    ExtrapolationState.Rotation = PawnOwner->GetActorRotation();
    ExtrapolationState.ControlRotation = PawnOwner->GetControlRotation();
    ExtrapolationState.InputMode = GenPawnOwner->GetInputMode();
    SaveBoundInputFlagsToState(ExtrapolationState);
    SaveBoundDataToState(ExtrapolationState);
    InterpolatedStatePreserveNaN(ExtrapolationState, TargetState);

    // Allows the user to the change the extrapolated state (which is what will be displayed) after move execution.
    ExtrapolationState = PostSimulateMovementForExtrapolation(
      ExtrapolationMove,
      ExtrapolationTime,
      StartState,
      TargetState,
      ExtrapolationState,
      SkippedStateIndices
    );
  }
  else
  {
    // Even though we did not simulate anything the pawn state still could have been changed in the simulated tick by the user last frame.
    ExtrapolationState.Timestamp = ExtrapolationTime;
    ExtrapolationState.Velocity = GetVelocity();
    ExtrapolationState.Location = PawnOwner->GetActorLocation();
    ExtrapolationState.Rotation = PawnOwner->GetActorRotation();
    ExtrapolationState.ControlRotation = PawnOwner->GetControlRotation();
    ExtrapolationState.InputMode = GenPawnOwner->GetInputMode();
    SaveBoundInputFlagsToState(ExtrapolationState);
    SaveBoundDataToState(ExtrapolationState);
    InterpolatedStatePreserveNaN(ExtrapolationState, TargetState);
  }
}

void UGenMovementReplicationComponent::SetSimulatedExtrapolationPawnState(const FState& InitState)
{
  SetVelocity(GetValidVelocity(InitState.Velocity));
  PawnOwner->SetActorLocation(GetValidActorLocation(InitState.Location), false, nullptr, ETeleportType::TeleportPhysics);
  PawnOwner->SetActorRotation(GetValidActorRotation(InitState.Rotation), ETeleportType::TeleportPhysics);
  if (SimulatedRootComponent)
  {
    // Reset any offset of the simulated root for the simulation.
    SimulatedRootComponent->SetRelativeLocationAndRotation(
      FVector::ZeroVector,
      FRotator::ZeroRotator,
      false,
      nullptr,
      ETeleportType::TeleportPhysics
    );
  }
  if (const auto Controller = PawnOwner->GetController())
  {
    Controller->SetControlRotation(GetValidControlRotation(InitState.ControlRotation));
  }
  LoadInputModeFromState(InitState);
  LoadBoundInputFlagsFromState(InitState);
  LoadBoundDataFromState(InitState);
  OnImmediateStateLoaded(EImmediateContext::SimulatingMovementForExtrapolation);
}

FMove UGenMovementReplicationComponent::BuildSimulatedExtrapolationMove(
  float ExtrapolationTime,
  const FState& InitState,
  const FState& TargetState
) const
{
  FMove Move;
  Move.Timestamp = ExtrapolationTime;

  checkGMC(ExtrapolationTime > InitState.Timestamp)
  const float DeltaTime = ExtrapolationTime - InitState.Timestamp;
  if (ExtrapolationMode == EExtrapolationMode::SemiSimulated)
  {
    // For semi-simulated extrapolation we always extrapolate from the current target state and can clamp the delta time directly.
    Move.DeltaTime = FMath::Clamp(DeltaTime, MIN_DELTA_TIME, MaxExtrapolationDeltaTime);
  }
  else
  {
    checkGMC(ExtrapolationMode == EExtrapolationMode::FullSimulated)
    checkGMC(AccumulatedFullSimulatedExtrapolationTime <= MaxExtrapolationDeltaTime)

    // For fully simulated extrapolation we extrapolate incrementally and have to track the time.
    Move.DeltaTime = FMath::Clamp(DeltaTime, MIN_DELTA_TIME, BIG_NUMBER);
    if (AccumulatedFullSimulatedExtrapolationTime + Move.DeltaTime > MaxExtrapolationDeltaTime)
    {
      // Clamp the move delta time to the remaining time available.
      Move.DeltaTime = FMath::Clamp(MaxExtrapolationDeltaTime - AccumulatedFullSimulatedExtrapolationTime, MIN_DELTA_TIME, BIG_NUMBER);
    }
  }

  // Save the state values.
  Move.InVelocity = GetValidVelocity(InitState.Velocity);
  Move.InLocation = GetValidActorLocation(InitState.Location);
  Move.InRotation = GetValidActorRotation(InitState.Rotation);
  Move.InControlRotation = GetValidControlRotation(InitState.ControlRotation);
  Move.InInputMode = InitState.InputMode;
  SaveBoundDataToMove(Move, FMove::EStateVars::Input);

  // Save the move input values from the last known player input (saved in the target state).
  // @note "InputVector" and "RotationInput" are not implemented yet for FState.
  //Move.InputVector   = TargetState.InputVector;
  //Move.RotationInput = TargetState.RotationInput;
  Move.bInputFlag1   = TargetState.bInputFlag1;
  Move.bInputFlag2   = TargetState.bInputFlag2;
  Move.bInputFlag3   = TargetState.bInputFlag3;
  Move.bInputFlag4   = TargetState.bInputFlag4;
  Move.bInputFlag5   = TargetState.bInputFlag5;
  Move.bInputFlag6   = TargetState.bInputFlag6;
  Move.bInputFlag7   = TargetState.bInputFlag7;
  Move.bInputFlag8   = TargetState.bInputFlag8;
  Move.bInputFlag9   = TargetState.bInputFlag9;
  Move.bInputFlag10  = TargetState.bInputFlag10;
  Move.bInputFlag11  = TargetState.bInputFlag11;
  Move.bInputFlag12  = TargetState.bInputFlag12;
  Move.bInputFlag13  = TargetState.bInputFlag13;
  Move.bInputFlag14  = TargetState.bInputFlag14;
  Move.bInputFlag15  = TargetState.bInputFlag15;
  Move.bInputFlag16  = TargetState.bInputFlag16;

  return Move;
}

void UGenMovementReplicationComponent::ExecuteMoveSimulated(const FMove& Move)
{
  SCOPE_CYCLE_COUNTER(STAT_ExecuteMoveSimulated)

  checkGMC(!bIsPerformingSimulatedExtrapolation)
  bIsPerformingSimulatedExtrapolation = true;

  FMove IterationMove{Move};
  int32 Iterations{0};
  float RemainingTime{IterationMove.DeltaTime};
  float& SubDeltaTime{IterationMove.DeltaTime};
  while (RemainingTime >= MIN_DELTA_TIME)
  {
    ++Iterations;
    SubDeltaTime = CalculateSubDeltaTime(Iterations, RemainingTime, MaxTimeStepSimulated, MaxIterationsSimulated);
    RemainingTime -= SubDeltaTime;
    ReplicatedTick(IterationMove, Iterations, RemainingTime >= MIN_DELTA_TIME);
    UpdateComponentVelocity();
  }

  bIsPerformingSimulatedExtrapolation = false;
}

FState UGenMovementReplicationComponent::InterpolateLinear(
  const FState& StartState,
  const FState& TargetState,
  float InterpolationRatio
) const
{
  const FState& InterpStartState =
    InterpolationMode == EInterpolationMode::MatchLatest ? GetValidInterpolationState(SMLStartState) : StartState;

  FState Result;
  Result.Location = FMath::LerpStable(InterpStartState.Location, TargetState.Location, InterpolationRatio);
  Result.Rotation = FQuat::Slerp(InterpStartState.Rotation.Quaternion(), TargetState.Rotation.Quaternion(), InterpolationRatio).Rotator();
  Result.ControlRotation = FQuat::Slerp(
    InterpStartState.ControlRotation.Quaternion(),
    TargetState.ControlRotation.Quaternion(),
    InterpolationRatio
  ).Rotator();
  Result.Velocity = FMath::LerpStable(InterpStartState.Velocity, TargetState.Velocity, InterpolationRatio);
  return Result;
}

FState UGenMovementReplicationComponent::InterpolateCubic(
  const FState& StartState,
  const FState& TargetState,
  float InterpolationRatio
) const
{
  const FState& InterpStartState =
    InterpolationMode == EInterpolationMode::MatchLatest ? GetValidInterpolationState(SMLStartState) : StartState;

  const float dt = FMath::Max(TargetState.Timestamp - InterpStartState.Timestamp, MIN_DELTA_TIME);
  const FVector StartDerivative = InterpStartState.Velocity * dt;
  const FVector TargetDerivative = TargetState.Velocity * dt;
  const FVector NewDerivative = FMath::CubicInterpDerivative(
    InterpStartState.Location,
    StartDerivative,
    TargetState.Location,
    TargetDerivative,
    InterpolationRatio
  );
  FState Result;
  Result.Location =
    FMath::CubicInterp(InterpStartState.Location, StartDerivative, TargetState.Location, TargetDerivative, InterpolationRatio);
  Result.Rotation = FQuat::Slerp(InterpStartState.Rotation.Quaternion(), TargetState.Rotation.Quaternion(), InterpolationRatio).Rotator();
  Result.Velocity = NewDerivative / dt;
  return Result;
}

FState UGenMovementReplicationComponent::InterpolateCustom1_Implementation(
  const FState& StartState,
  const FState& TargetState,
  float InterpolationRatio
) const
{
  GMC_LOG(Error, TEXT("Custom interpolation function is not implemented."))
  return TargetState;
}

FState UGenMovementReplicationComponent::InterpolateCustom2_Implementation(
  const FState& StartState,
  const FState& TargetState,
  float InterpolationRatio
) const
{
  GMC_LOG(Error, TEXT("Custom interpolation function is not implemented."))
  return TargetState;
}

FState UGenMovementReplicationComponent::InterpolateCustom3_Implementation(
  const FState& StartState,
  const FState& TargetState,
  float InterpolationRatio
) const
{
  GMC_LOG(Error, TEXT("Custom interpolation function is not implemented."))
  return TargetState;
}

FState UGenMovementReplicationComponent::InterpolateCustom4_Implementation(
  const FState& StartState,
  const FState& TargetState,
  float InterpolationRatio
) const
{
  GMC_LOG(Error, TEXT("Custom interpolation function is not implemented."))
  return TargetState;
}

FState UGenMovementReplicationComponent::MakeFState(
  const FVector& FinalLocation,
  const FVector& FinalVelocity,
  const FRotator& FinalRotation,
  const FRotator& FinalControlRotation
) const
{
  FState Result;
  Result.Location = FinalLocation;
  Result.Velocity = FinalVelocity;
  Result.Rotation = FinalRotation;
  Result.ControlRotation = FinalControlRotation;
  return Result;
}

void UGenMovementReplicationComponent::SetPawnState(const FState& State, uint8 ValuesToUpdate, bool bSkipBoneUpdate)
{
  SCOPE_CYCLE_COUNTER(STAT_SetPawnState)

  // If the pawn has a skeletal mesh component we may want to skip updating kinematic bones as it can be very expensive.
  auto SkeletalMesh = PawnOwner->FindComponentByClass<USkeletalMeshComponent>();
  EKinematicBonesUpdateToPhysics::Type SavedBoneUpdateSetting{EKinematicBonesUpdateToPhysics::Type::SkipSimulatingBones};
  const bool bShouldSkipBoneUpdate = bSkipBoneUpdate && SkeletalMesh;
  if (bShouldSkipBoneUpdate)
  {
    SavedBoneUpdateSetting = SkeletalMesh->KinematicBonesUpdateType;
    SkeletalMesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::Type::SkipAllBones;
  }

  // @attention There might be NaN values in the passed state so we always convert to valid values before setting any state.

  if (ValuesToUpdate & UpdateVelocity)
  {
    SetVelocity(GetValidVelocity(State.Velocity));
  }

  // Do not overwrite client-authoritative values of non-predicted autonomous proxies.
  const bool bIsAutonomousProxy = IsAutonomousProxy();
  checkGMC(bIsAutonomousProxy ? !bUseClientPrediction : true)

  // Teleport physics when setting the pawn transform.
  constexpr ETeleportType TeleportPhysics = ETeleportType::TeleportPhysics;

  if (!(bIsAutonomousProxy && bUseClientLocation))
  {
    if (ValuesToUpdate & UpdateLocationRoot && ValuesToUpdate & UpdateLocationSimulatedRoot)
    {
      check(SimulatedRootComponent)
      PawnOwner->SetActorLocation(GetValidActorLocation(State.Location), false, nullptr, TeleportPhysics);
      SimulatedRootComponent->SetWorldLocation(GetValidActorLocation(State.Location), false, nullptr, TeleportPhysics);
    }
    else if (ValuesToUpdate & UpdateLocationRoot)
    {
      check(SimulatedRootComponent)
      const FVector OriginalLocation = PawnOwner->GetActorLocation();
      PawnOwner->SetActorLocation(GetValidActorLocation(State.Location), false, nullptr, TeleportPhysics);
      SimulatedRootComponent->SetWorldLocation(OriginalLocation, false, nullptr, TeleportPhysics);
    }
    else if (ValuesToUpdate & UpdateLocationSimulatedRoot)
    {
      check(SimulatedRootComponent)
      SimulatedRootComponent->SetWorldLocation(GetValidActorLocation(State.Location), false, nullptr, TeleportPhysics);
    }
  }

  if (!(bIsAutonomousProxy && bUseClientRotation))
  {
    if (ValuesToUpdate & UpdateRotationRoot && ValuesToUpdate & UpdateRotationSimulatedRoot)
    {
      check(SimulatedRootComponent)
      PawnOwner->SetActorRotation(GetValidActorRotation(State.Rotation), TeleportPhysics);
      SimulatedRootComponent->SetWorldRotation(GetValidActorRotation(State.Rotation), false, nullptr, TeleportPhysics);
    }
    else if (ValuesToUpdate & UpdateRotationRoot)
    {
      check(SimulatedRootComponent)
      const FRotator OriginalRotation = PawnOwner->GetActorRotation();
      PawnOwner->SetActorRotation(GetValidActorRotation(State.Rotation), TeleportPhysics);
      SimulatedRootComponent->SetWorldRotation(OriginalRotation, false, nullptr, TeleportPhysics);
    }
    else if (ValuesToUpdate & UpdateRotationSimulatedRoot)
    {
      check(SimulatedRootComponent)
      SimulatedRootComponent->SetWorldRotation(GetValidActorRotation(State.Rotation), false, nullptr, TeleportPhysics);
    }
  }

  if (!(bIsAutonomousProxy && bUseClientControlRotation))
  {
    if (ValuesToUpdate & UpdateControlRotation)
    {
      if (const auto Controller = PawnOwner->GetController())
      {
        // Simulated proxies do not have a controller, but all server pawns do.
        checkGMC(!IsSimulatedProxy())
        Controller->SetControlRotation(GetValidControlRotation(State.ControlRotation));
      }
    }
  }

  if (bShouldSkipBoneUpdate)
  {
    // Restore the original update flag.
    SkeletalMesh->KinematicBonesUpdateType = SavedBoneUpdateSetting;
  }
}

void UGenMovementReplicationComponent::DetermineSkippedStates(TArray<int32>& OutSkippedStateIndices) const
{
  SCOPE_CYCLE_COUNTER(STAT_DetermineSkippedStates)

  OutSkippedStateIndices.Reset();
  if (
    !IsValidStateQueueIndex(CurrentStartStateIndex)
    || !IsValidStateQueueIndex(CurrentTargetStateIndex)
    || PreviousInterpolationTargetStateTimestamp < 0.f)
  {
    // The states that we need for checking are not valid.
    return;
  }

  if (StateQueue[CurrentTargetStateIndex].Timestamp == PreviousInterpolationTargetStateTimestamp)
  {
    // We are still interpolating between the same two states as during the last tick which means we cannot have skipped any states.
    return;
  }
  if (StateQueue[CurrentStartStateIndex].Timestamp != PreviousInterpolationTargetStateTimestamp)
  {
    // We may have skipped a state.
    if (CurrentStartStateIndex == 0)
    {
      // We don't have any older states available to check (should not happen usually).
      return;
    }
    if (StateQueue[CurrentStartStateIndex - 1].Timestamp == PreviousInterpolationTargetStateTimestamp)
    {
      // We have not interpolated between the current start state and the previous target state but we didn't skip any state.
      return;
    }
    // We skipped one or more states.
    int32 CheckIndex = CurrentStartStateIndex - 2;
    while (IsValidStateQueueIndex(CheckIndex))
    {
      // We skipped the state at index "CheckIndex + 1".
      OutSkippedStateIndices.Emplace(CheckIndex + 1);
      if (StateQueue[CheckIndex].Timestamp == PreviousInterpolationTargetStateTimestamp)
      {
        // We found the last target state and return the indices of the skipped states, which are all states with a timestamp larger than
        // the previous target state but smaller than the current start state.
        GMC_LOG(VeryVerbose, TEXT("%d states were skipped during interpolation."), OutSkippedStateIndices.Num())
        return;
      }
      --CheckIndex;
    }
    // The oldest state in the queue is newer than the previous target state. This can happen during bad latency spikes or if the configured
    // state queue max size is too small.
    checkGMC(CheckIndex + 1 == 0)
    OutSkippedStateIndices.Emplace(0);
    GMC_LOG(VeryVerbose, TEXT("Previous target state was already deleted from the state queue."))
  }
}

FRotator UGenMovementReplicationComponent::GetControlRotationLocalSpace() const
{
  // PawnOwner can cause a nullptr exception in the editor if this function is called in the actor's anim blueprint.
  if (PawnOwner)
  {
    if (IsSimulatedProxy())
    {
      return SmoothedControlRotation;
    }
    else if (PawnOwner->Controller)
    {
      return ControlRotationToLocal(PawnOwner->GetControlRotation());
    }
  }
  return FRotator{0};
}

FRotator UGenMovementReplicationComponent::ControlRotationToLocal(const FRotator& WorldControlRotation) const
{
  // PawnOwner can be nullptr in the editor (e.g. if this function is called in an anim blueprint).
  if (PawnOwner)
  {
    if (const auto RootComponent = PawnOwner->GetRootComponent())
    {
      const FTransform& RootCollisionTransform = RootComponent->GetComponentTransform();
      const FQuat WorldControlRotationQuat = WorldControlRotation.Quaternion().GetNormalized();
      if (!WorldControlRotationQuat.IsIdentity(KINDA_SMALL_NUMBER))
      {
        return RootCollisionTransform.InverseTransformRotation(WorldControlRotationQuat).Rotator().GetNormalized();
      }
    }
  }
  return FRotator{0};
}

FState UGenMovementReplicationComponent::CreateInitializationState(
  float Time,
  const FState& StartState,
  const FState& TargetState
) const
{
  SCOPE_CYCLE_COUNTER(STAT_CreateInitializationState)

  // By taking the absolute value of the timestamp difference this will also work correctly when extrapolating.
  FState InitializationState;
  if (FMath::Abs(Time - StartState.Timestamp) < FMath::Abs(TargetState.Timestamp - Time))
  {
    InitializationState = StartState;
    // Make sure we don't skip any important values by choosing the start state as initialization state. We do this by looking at the two
    // next states in the queue. Example for a boolean value when the start state was chosen as initialization state: The start state has
    // the value "false". We look at the next state (the target state), and check if it has a different value than the start state (which
    // would be "true" in our example). If it does, we look at the next state after the target state, and check if this state has the same
    // value as the start state (i.e. "false" in our example). If it does, we overwrite the bool value of the initialization state (which
    // currently has the value of the start state) with the value of the target state, otherwise we would risk skipping the "true" value of
    // the current target state in the smooth state. This is only done for data that is configured to force a net update upon changing. It
    // is still possible to miss values with this strategy if they change frequently (e.g. sequential states alternating between "true" and
    // "false"), but usually you wouldn't force net updates for properties behaving this way.
    AddTargetStateInputFlagsToInitState(InitializationState, TargetState);
    AddTargetStateBoundDataToInitState(InitializationState, TargetState);
  }
  else
  {
    InitializationState = TargetState;
    // Analogous to what is described in the first branch but we look at the two previous states in the queue.
    AddStartStateInputFlagsToInitState(InitializationState, StartState);
    AddStartStateBoundDataToInitState(InitializationState, StartState);
  }
  InitializationState.Timestamp = Time;
  return InitializationState;
}

void UGenMovementReplicationComponent::SetInterpolationMethod(EInterpolationMethod NewInterpolationMethod)
{
  InterpolationMethod = NewInterpolationMethod;
  switch (InterpolationMethod)
  {
    case EInterpolationMethod::None:
      InterpolationFunction = nullptr;
      return;
    case EInterpolationMethod::Linear:
      InterpolationFunction = &UGenMovementReplicationComponent::InterpolateLinear;
      return;
    case EInterpolationMethod::Cubic:
      InterpolationFunction = &UGenMovementReplicationComponent::InterpolateCubic;
      return;
    case EInterpolationMethod::Custom1:
      InterpolationFunction = &UGenMovementReplicationComponent::InterpolateCustom1;
      return;
    case EInterpolationMethod::Custom2:
      InterpolationFunction = &UGenMovementReplicationComponent::InterpolateCustom2;
      return;
    case EInterpolationMethod::Custom3:
      InterpolationFunction = &UGenMovementReplicationComponent::InterpolateCustom3;
      return;
    case EInterpolationMethod::Custom4:
      InterpolationFunction = &UGenMovementReplicationComponent::InterpolateCustom4;
      return;
    default: checkNoEntryGMC();
  }
  checkNoEntryGMC()
}

FState UGenMovementReplicationComponent::GetValidInterpolationState(const FState& State) const
{
  FState ValidState = State;
  ValidState.Velocity = GetValidVector(State.Velocity, FVector{0});
  ValidState.Location = GetValidVector(State.Location, FVector{0});
  ValidState.Rotation = GetValidRotator(State.Rotation, FRotator{0});
  ValidState.ControlRotation = GetValidRotator(State.ControlRotation, FRotator{0});
  return ValidState;
}

void UGenMovementReplicationComponent::InterpolatedStatePreserveNaN(FState& SmoothState, const FState& SourceState) const
{
  if (!Rep_IsValid(SourceState.Velocity))              Rep_SetInvalid(SmoothState.Velocity);
  if (!Rep_IsValid(SourceState.Location))              Rep_SetInvalid(SmoothState.Location);
  if (!Rep_IsValid(SourceState.Rotation.Roll))         Rep_SetInvalid(SmoothState.Rotation.Roll);
  if (!Rep_IsValid(SourceState.Rotation.Pitch))        Rep_SetInvalid(SmoothState.Rotation.Pitch);
  if (!Rep_IsValid(SourceState.Rotation.Yaw))          Rep_SetInvalid(SmoothState.Rotation.Yaw);
  if (!Rep_IsValid(SourceState.ControlRotation.Roll))  Rep_SetInvalid(SmoothState.ControlRotation.Roll);
  if (!Rep_IsValid(SourceState.ControlRotation.Pitch)) Rep_SetInvalid(SmoothState.ControlRotation.Pitch);
  if (!Rep_IsValid(SourceState.ControlRotation.Yaw))   Rep_SetInvalid(SmoothState.ControlRotation.Yaw);
}

bool UGenMovementReplicationComponent::AddToStateQueue(const FState& State)
{
  SCOPE_CYCLE_COUNTER(STAT_AddToStateQueue)

  if (StateQueue.Num() > 0 && State.Timestamp <= StateQueue.Last().Timestamp)
  {
    // This should usually not be happening since server to client replication is reliable (no out-of-order packets) and the client doesn't
    // send moves with inconsistent timestamps to the server (meaning we also cannot receive server states with inconsistent timestamps).
    // However, in some circumstances this can still occur e.g. when a client gets assigned a different pawn by the server.
    return false;
  }
  StateQueue.Emplace(State);
  // If the queue reached the desired size, we delete the oldest state in the buffer.
  checkGMC(StateQueue.Num() <= StateQueueMaxSize)
  if (StateQueue.Num() == StateQueueMaxSize)
  {
    StateQueue.RemoveAt(0, 1, false/*don't shrink*/);
  }
  return true;
}

void UGenMovementReplicationComponent::SaveLocalStateBeforeRollback(const APawn* PawnToSave) const
{
  SCOPE_CYCLE_COUNTER(STAT_SaveLocalStateBeforeRollback)

  const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(PawnToSave->GetMovementComponent());
  check(ReplicationComponent)
  ReplicationComponent->SaveLocalPawnState(ReplicationComponent->RollbackRestoreState);
  checkGMC(ReplicationComponent->RollbackRestoreState.IsValid())
  // We also need to store the transform of the simulated root as we may change its relative location/rotation through interpolation.
  ReplicationComponent->RollbackRestoreTransform_SimulatedRoot = FTransform{
    ReplicationComponent->SimulatedRootComponent->GetComponentQuat(),
    ReplicationComponent->SimulatedRootComponent->GetComponentLocation(),
    ReplicationComponent->SimulatedRootComponent->GetComponentScale()
  };
}

void UGenMovementReplicationComponent::RestoreOriginalStateAfterRollback(const APawn* PawnToRestore, EImmediateContext Context) const
{
  SCOPE_CYCLE_COUNTER(STAT_RestoreOriginalStateAfterRollback)

  const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(PawnToRestore->GetMovementComponent());
  check(ReplicationComponent)
  checkGMC(ReplicationComponent->RollbackRestoreState.IsValid())
  // @attention This will remove any relative offset of the simulated root component, we'll restore the original offset next.
  ReplicationComponent->SetLocalPawnState(
    ReplicationComponent->RollbackRestoreState,
    ReplicationComponent->bSkipBoneUpdateForRollback,
    Context
  );
  // Restore the original transform of the simulated root component of the other pawn.
  auto SimulatedRoot = ReplicationComponent->SimulatedRootComponent;
  SimulatedRoot->SetWorldLocation(
    ReplicationComponent->RollbackRestoreTransform_SimulatedRoot.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics
  );
  SimulatedRoot->SetWorldRotation(
    ReplicationComponent->RollbackRestoreTransform_SimulatedRoot.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics
  );
  SimulatedRoot->SetWorldScale3D(
    ReplicationComponent->RollbackRestoreTransform_SimulatedRoot.GetScale3D()
  );
}

TArray<AGenPawn*> UGenMovementReplicationComponent::GatherRollbackPawns() const
{
  SCOPE_CYCLE_COUNTER(STAT_GatherRollbackPawns)

  TArray<AGenPawn*> RollbackPawns;
  const auto World = GetWorld();
  if (!World) return RollbackPawns;

  if (IsServerPawn())
  {
    checkGMC(bRollbackServerPawns)
    // Gather pawns for server rollback.
    for (auto Iterator = World->GetControllerIterator(); Iterator; ++Iterator)
    {
      check(Iterator->IsValid())
      const auto Controller = Iterator->Get();
      check(Controller)
      const auto GenPawn = Cast<AGenPawn>(Controller->GetPawn());
      if (!ShouldBeRolledBack(GenPawn))
      {
        continue;
      }
      RollbackPawns.Emplace(GenPawn);

      // Buffer the state so we can restore it after the move execution.
      SaveLocalStateBeforeRollback(GenPawn);
    }
  }
  else
  {
    checkGMC(IsAutonomousProxy())
    checkGMC(bRollbackClientPawns)
    // Gather pawns for client rollback.
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenPawn::StaticClass(), Actors);
    for (const auto Actor : Actors)
    {
      const auto GenPawn = Cast<AGenPawn>(Actor);
      checkGMC(GenPawn)
      if (!ShouldBeRolledBack(GenPawn))
      {
        continue;
      }
      RollbackPawns.Emplace(GenPawn);

      // Buffer the state so we can restore it after the move execution.
      SaveLocalStateBeforeRollback(GenPawn);
    }
  }

  return RollbackPawns;
}

bool UGenMovementReplicationComponent::ShouldBeRolledBack(const APawn* PawnToTest) const
{
  SCOPE_CYCLE_COUNTER(STAT_ShouldBeRolledBack)

  checkGMC(bRollbackServerPawns || bRollbackClientPawns)

  if (!IsValid(PawnToTest))
  {
    // The considered pawn is pending kill.
    return false;
  }

  if (PawnToTest == PawnOwner)
  {
    // Do not roll back the pawn we are going to simulate the moves for.
    return false;
  }

  const auto GenPawnToTest = Cast<AGenPawn>(PawnToTest);
  if (!GenPawnToTest)
  {
    // Only pawns of type AGenPawn are supported.
    return false;
  }

  const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(GenPawnToTest->GetMovementComponent());
  if (!ReplicationComponent)
  {
    // The considered pawn needs a replication component.
    return false;
  }

  const auto& StateQueueOther = ReplicationComponent->StateQueue;
  const int32 StateQueueOtherSize = StateQueueOther.Num();
  if (StateQueueOtherSize == 0)
  {
    // The considered pawn's state queue is empty.
    return false;
  }

  if (
    ReplicationComponent->InterpolationMethod == EInterpolationMethod::None
    || ReplicationComponent->InterpolationMode == EInterpolationMode::MatchLatest
  )
  {
    // Rollback does not work without a consistent simulation delay.
    return false;
  }
  checkGMC(ReplicationComponent->InterpolationFunction)
  checkGMC(InterpolationMode == EInterpolationMode::FixedDelay || InterpolationMode == EInterpolationMode::AdaptiveDelay)

  const float ConsiderationRadius = IsServerPawn() ? ServerPawnRollbackRadius : ClientPawnRollbackRadius;
  if ((PawnToTest->GetActorLocation() - PawnOwner->GetActorLocation()).Size() > ConsiderationRadius)
  {
    // The considered pawn is further away than the set threshold.
    return false;
  }

  if (const auto Controller = PawnOwner->GetController())
  {
    FRotator ViewRotation{0};
    FVector ViewLocation{0};
    Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
    if (!GenPawnToTest->IsNetRelevantFor(Controller, PawnOwner, ViewLocation))
    {
      // The considered pawn is not net relevant to this pawn meaning there either won't be any current states in its state queue (on the
      // client) or none of the states in the queue will be marked as replicated (on the server).
      return false;
    }
  }

  // Client rollback specific checks.
  if (IsAutonomousProxy())
  {
    checkGMC(bRollbackClientPawns)

    if (!ReplicationComponent->IsSimulatedProxy())
    {
      // Only simulated proxies are rolled back for client replay.
      return false;
    }

    if (
      ReplicationComponent->bUsingExtrapolatedData ?
        !ReplicationComponent->ExtrapolatedState.IsValid() :
        !ReplicationComponent->InterpolatedState.IsValid()
    )
    {
      // Simulated proxies are always smoothed so we need valid interpolation data to set them back to their original states after the
      // rollback.
      return false;
    }
  }

  // The tested pawn should be rolled back.
  return true;
}

void UGenMovementReplicationComponent::RollbackPawns(float Time, const TArray<AGenPawn*>& PawnsToRollBack, ESimulatedContext Context) const
{
  SCOPE_CYCLE_COUNTER(STAT_RollbackPawns)

  checkGMC(bRollbackServerPawns || bRollbackClientPawns)
  const auto World = GetWorld();
  if (!World || PawnsToRollBack.Num() == 0) return;

  // We are interested in the states of the connection which owns the pawn that we are currently simulating (i.e. the client controller).
  const auto OwningConnection = Cast<APlayerController>(PawnOwner->GetController());
  checkGMC(OwningConnection)

  for (const auto GenPawn : PawnsToRollBack)
  {
    if (!IsValid(GenPawn))
    {
      // Pawn may have been destroyed within the replicated tick.
      continue;
    }
    const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent());
    check(ReplicationComponent)
    const auto& StateQueueOther = ReplicationComponent->StateQueue;
    const int32 StateQueueOtherSize = StateQueueOther.Num();
    checkGMC(StateQueueOtherSize > 0)

    float InterpolationTime = Time;
    if (ReplicationComponent->InterpolationMode == EInterpolationMode::FixedDelay)
    {
      // @attention "SimulationDelay" must have the same fixed value as on the client.
      InterpolationTime -= ReplicationComponent->SimulationDelay;
    }
    else
    {
      checkGMC(ReplicationComponent->InterpolationMode == EInterpolationMode::AdaptiveDelay)
      // When using an adaptive delay we must retrieve the simulation delay value that was used on the client when the move was executed.
      InterpolationTime -= ReplicationComponent->Server_SADGetSimulationDelayForRollback(Time, OwningConnection);
    }

    // Find the states that were used on the client during the original move execution.
    FState StartState;
    FState TargetState;
    float InterpolationRatio{-1.f};
    if (!ComputeRollbackInput(OwningConnection, InterpolationTime, StateQueueOther, StartState, TargetState, InterpolationRatio))
    {
      GMC_LOG(
        Verbose,
        TEXT("No states to roll back pawn %s (%s) found. ")
        TEXT("State queue max size of said pawn may need to be increased if this occurs repeatedly."),
        *GenPawn->GetName(),
        *DebugGetNetRoleAsString(GenPawn->GetLocalRole())
      )
      continue;
    }
    checkGMC(InterpolationRatio >= 0.f)
    checkGMC(InterpolationRatio <= 1.f + KINDA_SMALL_NUMBER)

    // Set the pawn back in time.
    FState RollbackState = ReplicationComponent->CreateInitializationState(InterpolationTime, StartState, TargetState);
    ReplicationComponent->ComputeInterpolatedState(InterpolationRatio, StartState, TargetState, RollbackState);
    ReplicationComponent->SetReplicatedPawnState(
      RollbackState,
      StartState,
      TargetState,
      ReplicationComponent->bSkipBoneUpdateForRollback,
      Context
    );

    // Event for custom logic.
    ReplicationComponent->OnPawnRolledBack(RollbackState, StartState, TargetState, Context);
  }
}

bool UGenMovementReplicationComponent::ComputeRollbackInput(
  APlayerController* Connection,
  float Time,
  const TArray<FState>& StateQueueToSearch,
  FState& OutStartState,
  FState& OutTargetState,
  float& OutInterpolationRatio
) const
{
  SCOPE_CYCLE_COUNTER(STAT_ComputeRollbackInput)

  checkGMC(IsServerPawn() || IsAutonomousProxy())
  OutStartState = FState();
  OutTargetState = FState();
  OutInterpolationRatio = -1.f;

  if (!IsValid(Connection))
  {
    return false;
  }

  const int32 QueueSize{StateQueueToSearch.Num()};
  int32 Index{0};

  // It is very likely that we need quite recent states so we go through the queue from newest to oldest.
  if (IsServerPawn())
  {
    // @attention This branch is usually run for regular move execution on the server (if @see bRollbackServerPawns is true) but it may also
    // be entered in an unknown context if the user called @see Server_RewindPawn.

    for (Index = QueueSize - 1; Index >= 0; --Index)
    {
      const FState& State = StateQueueToSearch[Index];
      if (!State.LastSerialized.Contains(Connection))
      {
        // This usually happens when a new client just (dis)connected.
        continue;
      }
      const float Timestamp = State.Timestamp;
      if (Timestamp > Time)
      {
        continue;
      }
      // Check if the currently considered state was actually replicated to the client.
      if (State.LastSerialized[Connection].bReplicatedToSimulatedProxy)
      {
        OutStartState = State;
        break;
      }
    }
  }
  else
  {
    checkGMC(IsAutonomousProxy())
    checkGMC(bRollbackClientPawns)
    for (Index = QueueSize - 1; Index >= 0; --Index)
    {
      const FState& State = StateQueueToSearch[Index];
      const float Timestamp = State.Timestamp;
      if (Timestamp > Time)
      {
        continue;
      }
      OutStartState = State;
      break;
    }
  }
  if (Index < 0)
  {
    // Start state was not found in the queue.
    checkGMC(!OutStartState.IsValid())
    checkGMC(!OutTargetState.IsValid())
    checkGMC(OutInterpolationRatio == -1.f)
    return false;
  }
  checkGMC(OutStartState.Timestamp >= 0)

  if (IsServerPawn())
  {
    // @attention This branch is usually run for regular move execution on the server (if @see bRollbackServerPawns is true) but it may also
    // be entered in an unknown context if the user called @see Server_RewindPawn.

    // We have found the start state, go back up through the queue and find the target state which is the next state with a greater
    // timestamp that was also replicated.
    while (++Index < QueueSize)
    {
      const FState& NextState = StateQueueToSearch[Index];
      if (!NextState.LastSerialized.Contains(Connection))
      {
        // This can happen if the controller just switched pawns.
        break;
      }
      if (NextState.LastSerialized[Connection].bReplicatedToSimulatedProxy)
      {
        OutTargetState = NextState;
        OutInterpolationRatio =
          (Time - OutStartState.Timestamp) / FMath::Max(OutTargetState.Timestamp - OutStartState.Timestamp, MIN_DELTA_TIME);
        checkGMC(OutInterpolationRatio >= 0.f)
        checkGMC(OutInterpolationRatio <= 1.f + KINDA_SMALL_NUMBER)
        return true;
      }
    }
  }
  else
  {
    checkGMC(IsAutonomousProxy())
    checkGMC(bRollbackClientPawns)
    // We have found the start state, the target state is the next state in the queue.
    if (++Index < QueueSize)
    {
      OutTargetState = StateQueueToSearch[Index];
      OutInterpolationRatio =
        (Time - OutStartState.Timestamp) / FMath::Max(OutTargetState.Timestamp - OutStartState.Timestamp, MIN_DELTA_TIME);
      checkGMC(OutInterpolationRatio >= 0.f)
      checkGMC(OutInterpolationRatio <= 1.f + KINDA_SMALL_NUMBER)
      return true;
    }
  }

  checkGMC(!OutTargetState.IsValid())
  checkGMC(OutInterpolationRatio == -1.f)
  return false;
}

void UGenMovementReplicationComponent::RestoreRolledBackPawns(const TArray<AGenPawn*>& PawnsToRestore) const
{
  SCOPE_CYCLE_COUNTER(STAT_RestoreRolledBackPawns)

  for (const auto GenPawn : PawnsToRestore)
  {
    if (!IsValid(GenPawn))
    {
      // Pawn may have been destroyed within the replicated tick.
      continue;
    }
    checkGMC(GenPawn != PawnOwner)
    const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(GenPawn->GetMovementComponent());
    check(ReplicationComponent)

    // Set the pawn back to its original state (before the rollback).
    if (IsServerPawn())
    {
      RestoreOriginalStateAfterRollback(GenPawn, EImmediateContext::RestoringRolledBackServerPawn);
      ReplicationComponent->Server_OnPawnRestoredAfterRollback(EImmediateContext::RestoringRolledBackServerPawn);
    }
    else
    {
      checkGMC(IsAutonomousProxy())
      checkGMC(GenPawn->GetLocalRole() == ROLE_SimulatedProxy)
      RestoreOriginalStateAfterRollback(GenPawn, EImmediateContext::RestoringRolledBackClientPawn);
      ReplicationComponent->Client_OnPawnRestoredAfterRollback(EImmediateContext::RestoringRolledBackClientPawn);
    }
  }
}

bool UGenMovementReplicationComponent::Server_RewindPawn(APlayerController* Connection, float Time, AGenPawn* Pawn, bool bSkipBoneUpdate)
{
  if (!IsValid(PawnOwner) || !IsValid(Pawn) || !IsValid(Connection))
  {
    return false;
  }

  if (Pawn->GetLocalRole() != ROLE_Authority)
  {
    GMC_LOG(Warning, TEXT("Rewind failed: only server pawns can be rewound."))
    return false;
  }

  if (Connection->IsLocalController())
  {
    GMC_LOG(Warning, TEXT("Rewind failed: the target controller must be a client."))
    return false;
  }

  if (Pawn->GetController() == Connection)
  {
    // This does not work because we only save simulated proxy server states and the controller in this case is the autonomous proxy
    // connection that owns the target pawn.
    GMC_LOG(Warning, TEXT("Rewind failed: cannot rewind a pawn for its owning connection."))
    return false;
  }

  const auto PawnReplicationComponent = Cast<UGenMovementReplicationComponent>(Pawn->GetMovementComponent());
  if (!PawnReplicationComponent)
  {
    GMC_LOG(Warning, TEXT("Rewind failed: the target pawn does not have a replication component."))
    return false;
  }

  if (
    PawnReplicationComponent->InterpolationMethod == EInterpolationMethod::None
    || PawnReplicationComponent->InterpolationMode == EInterpolationMode::MatchLatest
  )
  {
    // We need to have a consistent simulation delay to find the correct states to rewind.
    GMC_LOG(Warning, TEXT("Rewind failed: the target pawn's configured interpolation mode and/or method does not support rewind."))
    return false;
  }
  checkGMC(PawnReplicationComponent->InterpolationFunction)
  checkGMC(InterpolationMode == EInterpolationMode::FixedDelay || InterpolationMode == EInterpolationMode::AdaptiveDelay)

  const auto& PawnStateQueue = PawnReplicationComponent->StateQueue;
  if (PawnStateQueue.Num() == 0)
  {
    GMC_LOG(Warning, TEXT("Rewind failed: the target pawn's state queue is empty."))
    return false;
  }

  float InterpolationTime = Time;
  if (PawnReplicationComponent->InterpolationMode == EInterpolationMode::FixedDelay)
  {
    // @attention "SimulationDelay" must have the same fixed value as on the client.
    InterpolationTime -= PawnReplicationComponent->SimulationDelay;
  }
  else
  {
    // When using an adaptive delay we must retrieve the simulation delay value that was used on the client when the move was executed.
    InterpolationTime -= PawnReplicationComponent->Server_SADGetSimulationDelayForRollback(Time, Connection);
  }

  // Find the states that were used for interpolation during the original move execution on the client.
  FState StartState;
  FState TargetState;
  float InterpolationRatio{-1.f};
  if (!ComputeRollbackInput(Connection, InterpolationTime, PawnStateQueue, StartState, TargetState, InterpolationRatio))
  {
    return false;
  }
  checkGMC(InterpolationRatio >= 0.f)
  checkGMC(InterpolationRatio <= 1.f + KINDA_SMALL_NUMBER)

  // Save the current state so we can restore it afterwards.
  SaveLocalStateBeforeRollback(Pawn);

  // Set the pawn back in time.
  FState RewindState = PawnReplicationComponent->CreateInitializationState(InterpolationTime, StartState, TargetState);
  PawnReplicationComponent->ComputeInterpolatedState(InterpolationRatio, StartState, TargetState, RewindState);
  PawnReplicationComponent->SetReplicatedPawnState(
    RewindState,
    StartState,
    TargetState,
    bSkipBoneUpdate,
    ESimulatedContext::NoContextInformation
  );

  PawnReplicationComponent->Server_OnPawnRewound(RewindState, StartState, TargetState);

  return true;
}

bool UGenMovementReplicationComponent::Server_RestorePawnAfterRewind(AGenPawn* Pawn, bool bSkipBoneUpdate)
{
  if (!IsValid(PawnOwner) || !IsValid(Pawn))
  {
    return false;
  }

  if (Pawn->GetLocalRole() != ROLE_Authority)
  {
    return false;
  }

  const auto PawnReplicationComponent = Cast<UGenMovementReplicationComponent>(Pawn->GetMovementComponent());
  if (!PawnReplicationComponent)
  {
    return false;
  }

  if (!PawnReplicationComponent->RollbackRestoreState.IsValid())
  {
    return false;
  }

  // Restore the original pawn state.
  RestoreOriginalStateAfterRollback(Pawn, EImmediateContext::NoContextInformation);

  PawnReplicationComponent->Server_OnPawnRestoredAfterRewind();

  return true;
}

void UGenMovementReplicationComponent::TickGenericRollbackActors(
  float Time,
  float DeltaTime,
  const FMove& Move,
  EActorRollbackContext Context
) const
{
  SCOPE_CYCLE_COUNTER(STAT_TickGenericRollbackActors)

  checkGMC(PawnOwner->IsLocallyControlled())

  TArray<AGenRollbackActor*> RollbackActors;
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenRollbackActor::StaticClass(), Actors);
  for (const auto Actor : Actors)
  {
    const auto GenRollbackActor = Cast<AGenRollbackActor>(Actor);
    check(GenRollbackActor)
    GenRollbackActor->UpdateState(Time, DeltaTime, Move, Context);
    GenRollbackActor->SetTicked(true);
  }
}

TArray<class AGenRollbackActor*> UGenMovementReplicationComponent::GatherGenericRollbackActors() const
{
  SCOPE_CYCLE_COUNTER(STAT_GatherGenericRollbackActors)

  checkGMC(bRollbackGenericServerActors || bRollbackGenericClientActors)

  TArray<AGenRollbackActor*> RollbackActors;
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGenRollbackActor::StaticClass(), Actors);
  for (const auto Actor : Actors)
  {
    const auto GenRollbackActor = Cast<AGenRollbackActor>(Actor);
    checkGMC(GenRollbackActor)
    if (!ShouldBeRolledBack(GenRollbackActor))
    {
      continue;
    }
    RollbackActors.Emplace(GenRollbackActor);
  }
  return RollbackActors;
}

bool UGenMovementReplicationComponent::ShouldBeRolledBack(const AGenRollbackActor* ActorToTest) const
{
  checkGMC(bRollbackGenericServerActors || bRollbackGenericClientActors)

  if (!IsValid(ActorToTest))
  {
    // The considered actor is pending kill.
    return false;
  }

  const float ConsiderationRadius = IsServerPawn() ? GenericServerActorRollbackRadius : GenericClientActorRollbackRadius;
  if ((ActorToTest->GetActorLocation() - PawnOwner->GetActorLocation()).Size() > ConsiderationRadius)
  {
    // The considered actor is further away than the set threshold.
    return false;
  }

  // The tested actor should be rolled back.
  return true;
}

void UGenMovementReplicationComponent::BufferGenericRollbackActorStates(
  const TArray<AGenRollbackActor*>& RollbackActors,
  EActorRollbackContext Context
) const
{
  checkGMC(bRollbackGenericServerActors || bRollbackGenericClientActors)

  for (const auto RollbackActor : RollbackActors)
  {
    check(RollbackActor)
    RollbackActor->SaveState(Context);
  }
}

void UGenMovementReplicationComponent::RollbackGenericActors(
  float Time,
  float DeltaTime,
  const FMove& Move,
  const TArray<AGenRollbackActor*>& ActorsToRollBack,
  EActorRollbackContext Context
) const
{
  SCOPE_CYCLE_COUNTER(STAT_RollbackGenericActors)

  checkGMC(bRollbackGenericServerActors || bRollbackGenericClientActors)

  for (const auto RollbackActor : ActorsToRollBack)
  {
    check(RollbackActor)
    RollbackActor->UpdateState(Time, DeltaTime, Move, Context);
  }
}

void UGenMovementReplicationComponent::RestoreRolledBackGenericActors(
  const TArray<AGenRollbackActor*>& RollbackActors,
  EActorRollbackContext Context
) const
{
  SCOPE_CYCLE_COUNTER(STAT_RestoreRolledBackGenericActors)

  checkGMC(bRollbackGenericServerActors || bRollbackGenericClientActors)

  for (const auto RollbackActor : RollbackActors)
  {
    check(RollbackActor)
    RollbackActor->LoadState(Context);
  }
}

bool UGenMovementReplicationComponent::IsAutonomousProxy() const
{
  if (!PawnOwner) return false;
  return PawnOwner->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UGenMovementReplicationComponent::IsSimulatedProxy() const
{
  if (!PawnOwner) return false;
  return PawnOwner->GetLocalRole() == ROLE_SimulatedProxy;
}

bool UGenMovementReplicationComponent::IsLocallyControlledServerPawn() const
{
  if (!PawnOwner) return false;
  return PawnOwner->GetLocalRole() == ROLE_Authority && PawnOwner->IsLocallyControlled();
}

bool UGenMovementReplicationComponent::IsRemotelyControlledServerPawn() const
{
  if (!PawnOwner) return false;
  return PawnOwner->GetLocalRole() == ROLE_Authority && PawnOwner->GetController() && !PawnOwner->IsLocallyControlled();
}

bool UGenMovementReplicationComponent::IsLocallyControlledDedicatedServerPawn() const
{
  return IsNetMode(NM_DedicatedServer) && IsLocallyControlledServerPawn();
}

bool UGenMovementReplicationComponent::IsRemotelyControlledDedicatedServerPawn() const
{
  return IsNetMode(NM_DedicatedServer) && IsRemotelyControlledServerPawn();
}

bool UGenMovementReplicationComponent::IsLocallyControlledListenServerPawn() const
{
  return IsNetMode(NM_ListenServer) && IsLocallyControlledServerPawn();
}

bool UGenMovementReplicationComponent::IsRemotelyControlledListenServerPawn() const
{
  return IsNetMode(NM_ListenServer) && IsRemotelyControlledServerPawn();
}

bool UGenMovementReplicationComponent::IsSmoothedListenServerPawn() const
{
  return bSmoothRemoteServerPawn && IsRemotelyControlledListenServerPawn();
}

bool UGenMovementReplicationComponent::IsClientPawn() const
{
  if (!PawnOwner) return false;
  return PawnOwner->GetLocalRole() == ROLE_AutonomousProxy || PawnOwner->GetLocalRole() == ROLE_SimulatedProxy;
}

bool UGenMovementReplicationComponent::IsServerPawn() const
{
  if (!PawnOwner) return false;
  return PawnOwner->GetLocalRole() == ROLE_Authority;
}

bool UGenMovementReplicationComponent::IsServerBot() const
{
  if (!PawnOwner) return false;
  if (PawnOwner->GetLocalRole() == ROLE_Authority && PawnOwner->IsLocallyControlled() && Cast<AAIController>(PawnOwner->GetController()))
  {
    return true;
  }
  return false;
}

bool UGenMovementReplicationComponent::IsNetworkedServer() const
{
  return IsNetMode(NM_DedicatedServer) || IsNetMode(NM_ListenServer);
}

float UGenMovementReplicationComponent::GetTime() const
{
  SCOPE_CYCLE_COUNTER(STAT_GetTime)

  if (IsClientPawn())
  {
    if (UGameInstance* GameInstance = PawnOwner->GetGameInstance())
    {
      AGenPlayerController* LocalController = Cast<AGenPlayerController>(GameInstance->GetFirstLocalPlayerController());
      if (LocalController)
      {
        return LocalController->Client_GetSyncedWorldTimeSeconds();
      }
    }
    GMC_LOG(Warning, TEXT("Client was unable to retrieve the server world time."))
    return 0.f;
  }

  if (UWorld* World = GetWorld())
  {
    if (AGameStateBase* GameState = World->GetGameState())
    {
      return GameState->GetServerWorldTimeSeconds();
    }
  }
  GMC_LOG(Warning, TEXT("Server world time could not be retrieved."))
  return 0.f;
}

bool UGenMovementReplicationComponent::HasValueChanged(float CurrentValue, float& LastSentValue, float Tolerance)
{
  if (!(FMath::Abs(CurrentValue - LastSentValue) <= Tolerance))
  {
    LastSentValue = CurrentValue;
    return true;
  }
  return false;
}

bool UGenMovementReplicationComponent::HasValueChanged(const FVector& CurrentValue, FVector& LastSentValue, float Tolerance)
{
  if (!CurrentValue.Equals(LastSentValue, Tolerance))
  {
    LastSentValue = CurrentValue;
    return true;
  }
  return false;
}

float UGenMovementReplicationComponent::GetCompareTolerance(EDecimalQuantization QuantizationLevel)
{
  switch (QuantizationLevel)
  {
    case EDecimalQuantization::RoundWholeNumber: return 1.f;
    case EDecimalQuantization::RoundOneDecimal: return 0.1f;
    case EDecimalQuantization::RoundTwoDecimals: return 0.01f;
    case EDecimalQuantization::None: return 0.000001f;
    default: checkNoEntryGMC();
  }
  checkNoEntryGMC()
  return 0.f;
}

float UGenMovementReplicationComponent::GetCompareToleranceRotator(ESizeQuantization QuantizationLevel)
{
  switch (QuantizationLevel)
  {
    case ESizeQuantization::Byte: return 1.f;
    case ESizeQuantization::Short: return 0.01f;
    case ESizeQuantization::None: return 0.000001f;
    default: checkNoEntryGMC();
  }
  checkNoEntryGMC()
  return 0.f;
}

float UGenMovementReplicationComponent::GetCompareToleranceVectorMax1(ESizeQuantization QuantizationLevel)
{
  static_assert(MAX_DIRECTION_INPUT == 1, "Only applicable if all absolute component values are <= 1.");
  switch (QuantizationLevel)
  {
    case ESizeQuantization::Byte: return 0.01f;
    case ESizeQuantization::Short: return 0.0001f;
    case ESizeQuantization::None: return 0.000001f;
    default: checkNoEntryGMC();
  }
  checkNoEntryGMC()
  return 0.f;
}

float UGenMovementReplicationComponent::GetCompareToleranceRotatorMax200(ESizeQuantization QuantizationLevel)
{
  static_assert(MAX_ROTATION_INPUT == 200, "Only applicable if all absolute component values are <= 200.");
  switch (QuantizationLevel)
  {
    case ESizeQuantization::Byte: return 1.f;
    case ESizeQuantization::Short: return 0.01f;
    case ESizeQuantization::None: return 0.000001f;
    default: checkNoEntryGMC();
  }
  checkNoEntryGMC()
  return 0.f;
}

void UGenMovementReplicationComponent::K2_BindInputFlag(
  FName ActionName,
  bool& VariableToBind,
  bool bReplicateToSimulatedProxy,
  bool bNoMoveCombine
)
{
  BindInputFlag(ActionName, VariableToBind, bReplicateToSimulatedProxy, bNoMoveCombine);
}

void UGenMovementReplicationComponent::BindInputFlag(
  FName ActionName,
  bool& VariableToBind,
  bool bReplicateToSimulatedProxy,
  bool bNoMoveCombine
)
{
  const auto Owner = Cast<AGenPawn>(GetOwner());
  checkCodeGMC(if (!HasAnyFlags(NoBindingFlags)) check(Owner))
  if (!Owner) return;

  /* TODO: Handle for this block
  TArray<FInputActionKeyMapping> Mapping;
  UInputSettings::GetInputSettings()->GetActionMappingByName(ActionName, Mapping);
  if (Mapping.Num() == 0)
  {
    UE_CLOG(
      ActionName != "" && ActionName != "None",
      LogGMCReplication,
      Error,
      TEXT("Input could not be bound. No action with the name \"%s\" exists."), *ActionName.ToString()
    )
    return;
  }
  */

  // The input flags should never be replicated to the autonomous proxy. They represent the physical input of the player and are sent to
  // the server with every move. The server then executes the move with the received data meaning the input flags can never deviate
  // between server and client, and it would be a waste of bandwidth to replicate them back to the client.
  // @attention Replicated input flags always force a net update to simulated proxies.
  // @attention Input flags may be skipped while binding, e.g. input flags 1 and 3 could be bound, but not 2.
  if (Owner->ID_Action1 == ActionName)
  {
    if (!InputFlag1)
    {
      InputFlag1 = &VariableToBind;
      checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag1) if (bNoMoveCombine)
      {
        bNoMoveCombineInputFlag1 = true;
      }
      if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag1 = true; }
      return;
    }
    UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString())
    return;
  }
  if (Owner->ID_Action2 == ActionName)  { if (!InputFlag2)  { InputFlag2  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag2)  if (bNoMoveCombine) { bNoMoveCombineInputFlag2  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag2  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action3 == ActionName)  { if (!InputFlag3)  { InputFlag3  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag3)  if (bNoMoveCombine) { bNoMoveCombineInputFlag3  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag3  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action4 == ActionName)  { if (!InputFlag4)  { InputFlag4  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag4)  if (bNoMoveCombine) { bNoMoveCombineInputFlag4  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag4  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action5 == ActionName)  { if (!InputFlag5)  { InputFlag5  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag5)  if (bNoMoveCombine) { bNoMoveCombineInputFlag5  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag5  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action6 == ActionName)  { if (!InputFlag6)  { InputFlag6  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag6)  if (bNoMoveCombine) { bNoMoveCombineInputFlag6  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag6  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action7 == ActionName)  { if (!InputFlag7)  { InputFlag7  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag7)  if (bNoMoveCombine) { bNoMoveCombineInputFlag7  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag7  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action8 == ActionName)  { if (!InputFlag8)  { InputFlag8  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag8)  if (bNoMoveCombine) { bNoMoveCombineInputFlag8  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag8  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action9 == ActionName)  { if (!InputFlag9)  { InputFlag9  = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag9)  if (bNoMoveCombine) { bNoMoveCombineInputFlag9  = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag9  = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action10 == ActionName) { if (!InputFlag10) { InputFlag10 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag10) if (bNoMoveCombine) { bNoMoveCombineInputFlag10 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag10 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action11 == ActionName) { if (!InputFlag11) { InputFlag11 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag11) if (bNoMoveCombine) { bNoMoveCombineInputFlag11 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag11 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action12 == ActionName) { if (!InputFlag12) { InputFlag12 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag12) if (bNoMoveCombine) { bNoMoveCombineInputFlag12 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag12 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action13 == ActionName) { if (!InputFlag13) { InputFlag13 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag13) if (bNoMoveCombine) { bNoMoveCombineInputFlag13 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag13 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action14 == ActionName) { if (!InputFlag14) { InputFlag14 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag14) if (bNoMoveCombine) { bNoMoveCombineInputFlag14 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag14 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action15 == ActionName) { if (!InputFlag15) { InputFlag15 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag15) if (bNoMoveCombine) { bNoMoveCombineInputFlag15 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag15 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
  if (Owner->ID_Action16 == ActionName) { if (!InputFlag16) { InputFlag16 = &VariableToBind; checkGMC(!ServerState_AutonomousProxy().bReplicateInputFlag16) if (bNoMoveCombine) { bNoMoveCombineInputFlag16 = true; } if (bReplicateToSimulatedProxy) { ServerState_SimulatedProxy().bReplicateInputFlag16 = true; } return; } UE_LOG(LogGMCReplication, Error, TEXT("Action \"%s\" is already bound to a variable."), *ActionName.ToString()) return; }
}

void UGenMovementReplicationComponent::Server_SwapStateBufferBoundInputFlags()
{
  checkGMC(IsServerPawn())
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (InputFlag1)  { const auto SwapInputFlag1  = *InputFlag1;  *InputFlag1  = StateBuffer().bInputFlag1;  StateBuffer().bInputFlag1  = SwapInputFlag1;  }
  if (InputFlag2)  { const auto SwapInputFlag2  = *InputFlag2;  *InputFlag2  = StateBuffer().bInputFlag2;  StateBuffer().bInputFlag2  = SwapInputFlag2;  }
  if (InputFlag3)  { const auto SwapInputFlag3  = *InputFlag3;  *InputFlag3  = StateBuffer().bInputFlag3;  StateBuffer().bInputFlag3  = SwapInputFlag3;  }
  if (InputFlag4)  { const auto SwapInputFlag4  = *InputFlag4;  *InputFlag4  = StateBuffer().bInputFlag4;  StateBuffer().bInputFlag4  = SwapInputFlag4;  }
  if (InputFlag5)  { const auto SwapInputFlag5  = *InputFlag5;  *InputFlag5  = StateBuffer().bInputFlag5;  StateBuffer().bInputFlag5  = SwapInputFlag5;  }
  if (InputFlag6)  { const auto SwapInputFlag6  = *InputFlag6;  *InputFlag6  = StateBuffer().bInputFlag6;  StateBuffer().bInputFlag6  = SwapInputFlag6;  }
  if (InputFlag7)  { const auto SwapInputFlag7  = *InputFlag7;  *InputFlag7  = StateBuffer().bInputFlag7;  StateBuffer().bInputFlag7  = SwapInputFlag7;  }
  if (InputFlag8)  { const auto SwapInputFlag8  = *InputFlag8;  *InputFlag8  = StateBuffer().bInputFlag8;  StateBuffer().bInputFlag8  = SwapInputFlag8;  }
  if (InputFlag9)  { const auto SwapInputFlag9  = *InputFlag9;  *InputFlag9  = StateBuffer().bInputFlag9;  StateBuffer().bInputFlag9  = SwapInputFlag9;  }
  if (InputFlag10) { const auto SwapInputFlag10 = *InputFlag10; *InputFlag10 = StateBuffer().bInputFlag10; StateBuffer().bInputFlag10 = SwapInputFlag10; }
  if (InputFlag11) { const auto SwapInputFlag11 = *InputFlag11; *InputFlag11 = StateBuffer().bInputFlag11; StateBuffer().bInputFlag11 = SwapInputFlag11; }
  if (InputFlag12) { const auto SwapInputFlag12 = *InputFlag12; *InputFlag12 = StateBuffer().bInputFlag12; StateBuffer().bInputFlag12 = SwapInputFlag12; }
  if (InputFlag13) { const auto SwapInputFlag13 = *InputFlag13; *InputFlag13 = StateBuffer().bInputFlag13; StateBuffer().bInputFlag13 = SwapInputFlag13; }
  if (InputFlag14) { const auto SwapInputFlag14 = *InputFlag14; *InputFlag14 = StateBuffer().bInputFlag14; StateBuffer().bInputFlag14 = SwapInputFlag14; }
  if (InputFlag15) { const auto SwapInputFlag15 = *InputFlag15; *InputFlag15 = StateBuffer().bInputFlag15; StateBuffer().bInputFlag15 = SwapInputFlag15; }
  if (InputFlag16) { const auto SwapInputFlag16 = *InputFlag16; *InputFlag16 = StateBuffer().bInputFlag16; StateBuffer().bInputFlag16 = SwapInputFlag16; }
}

bool UGenMovementReplicationComponent::Server_ForceNetUpdateCheckBoundInputFlags()
{
  checkGMC(IsServerPawn())
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  bool bShouldForceNetUpdate = false;
  if (InputFlag1  && ServerState_SimulatedProxy().bReplicateInputFlag1  && !bLockedInputFlag1)  { if (ServerState_SimulatedProxy().bInputFlag1  != ServerState_SimulatedProxy_Buffered().bInputFlag1)  { bShouldForceNetUpdate = true; bLockedInputFlag1  = true; LockSetTimeInputFlag1  = GetTime(); } }
  if (InputFlag2  && ServerState_SimulatedProxy().bReplicateInputFlag2  && !bLockedInputFlag2)  { if (ServerState_SimulatedProxy().bInputFlag2  != ServerState_SimulatedProxy_Buffered().bInputFlag2)  { bShouldForceNetUpdate = true; bLockedInputFlag2  = true; LockSetTimeInputFlag2  = GetTime(); } }
  if (InputFlag3  && ServerState_SimulatedProxy().bReplicateInputFlag3  && !bLockedInputFlag3)  { if (ServerState_SimulatedProxy().bInputFlag3  != ServerState_SimulatedProxy_Buffered().bInputFlag3)  { bShouldForceNetUpdate = true; bLockedInputFlag3  = true; LockSetTimeInputFlag3  = GetTime(); } }
  if (InputFlag4  && ServerState_SimulatedProxy().bReplicateInputFlag4  && !bLockedInputFlag4)  { if (ServerState_SimulatedProxy().bInputFlag4  != ServerState_SimulatedProxy_Buffered().bInputFlag4)  { bShouldForceNetUpdate = true; bLockedInputFlag4  = true; LockSetTimeInputFlag4  = GetTime(); } }
  if (InputFlag5  && ServerState_SimulatedProxy().bReplicateInputFlag5  && !bLockedInputFlag5)  { if (ServerState_SimulatedProxy().bInputFlag5  != ServerState_SimulatedProxy_Buffered().bInputFlag5)  { bShouldForceNetUpdate = true; bLockedInputFlag5  = true; LockSetTimeInputFlag5  = GetTime(); } }
  if (InputFlag6  && ServerState_SimulatedProxy().bReplicateInputFlag6  && !bLockedInputFlag6)  { if (ServerState_SimulatedProxy().bInputFlag6  != ServerState_SimulatedProxy_Buffered().bInputFlag6)  { bShouldForceNetUpdate = true; bLockedInputFlag6  = true; LockSetTimeInputFlag6  = GetTime(); } }
  if (InputFlag7  && ServerState_SimulatedProxy().bReplicateInputFlag7  && !bLockedInputFlag7)  { if (ServerState_SimulatedProxy().bInputFlag7  != ServerState_SimulatedProxy_Buffered().bInputFlag7)  { bShouldForceNetUpdate = true; bLockedInputFlag7  = true; LockSetTimeInputFlag7  = GetTime(); } }
  if (InputFlag8  && ServerState_SimulatedProxy().bReplicateInputFlag8  && !bLockedInputFlag8)  { if (ServerState_SimulatedProxy().bInputFlag8  != ServerState_SimulatedProxy_Buffered().bInputFlag8)  { bShouldForceNetUpdate = true; bLockedInputFlag8  = true; LockSetTimeInputFlag8  = GetTime(); } }
  if (InputFlag9  && ServerState_SimulatedProxy().bReplicateInputFlag9  && !bLockedInputFlag9)  { if (ServerState_SimulatedProxy().bInputFlag9  != ServerState_SimulatedProxy_Buffered().bInputFlag9)  { bShouldForceNetUpdate = true; bLockedInputFlag9  = true; LockSetTimeInputFlag9  = GetTime(); } }
  if (InputFlag10 && ServerState_SimulatedProxy().bReplicateInputFlag10 && !bLockedInputFlag10) { if (ServerState_SimulatedProxy().bInputFlag10 != ServerState_SimulatedProxy_Buffered().bInputFlag10) { bShouldForceNetUpdate = true; bLockedInputFlag10 = true; LockSetTimeInputFlag10 = GetTime(); } }
  if (InputFlag11 && ServerState_SimulatedProxy().bReplicateInputFlag11 && !bLockedInputFlag11) { if (ServerState_SimulatedProxy().bInputFlag11 != ServerState_SimulatedProxy_Buffered().bInputFlag11) { bShouldForceNetUpdate = true; bLockedInputFlag11 = true; LockSetTimeInputFlag11 = GetTime(); } }
  if (InputFlag12 && ServerState_SimulatedProxy().bReplicateInputFlag12 && !bLockedInputFlag12) { if (ServerState_SimulatedProxy().bInputFlag12 != ServerState_SimulatedProxy_Buffered().bInputFlag12) { bShouldForceNetUpdate = true; bLockedInputFlag12 = true; LockSetTimeInputFlag12 = GetTime(); } }
  if (InputFlag13 && ServerState_SimulatedProxy().bReplicateInputFlag13 && !bLockedInputFlag13) { if (ServerState_SimulatedProxy().bInputFlag13 != ServerState_SimulatedProxy_Buffered().bInputFlag13) { bShouldForceNetUpdate = true; bLockedInputFlag13 = true; LockSetTimeInputFlag13 = GetTime(); } }
  if (InputFlag14 && ServerState_SimulatedProxy().bReplicateInputFlag14 && !bLockedInputFlag14) { if (ServerState_SimulatedProxy().bInputFlag14 != ServerState_SimulatedProxy_Buffered().bInputFlag14) { bShouldForceNetUpdate = true; bLockedInputFlag14 = true; LockSetTimeInputFlag14 = GetTime(); } }
  if (InputFlag15 && ServerState_SimulatedProxy().bReplicateInputFlag15 && !bLockedInputFlag15) { if (ServerState_SimulatedProxy().bInputFlag15 != ServerState_SimulatedProxy_Buffered().bInputFlag15) { bShouldForceNetUpdate = true; bLockedInputFlag15 = true; LockSetTimeInputFlag15 = GetTime(); } }
  if (InputFlag16 && ServerState_SimulatedProxy().bReplicateInputFlag16 && !bLockedInputFlag16) { if (ServerState_SimulatedProxy().bInputFlag16 != ServerState_SimulatedProxy_Buffered().bInputFlag16) { bShouldForceNetUpdate = true; bLockedInputFlag16 = true; LockSetTimeInputFlag16 = GetTime(); } }
  return bShouldForceNetUpdate;
}

void UGenMovementReplicationComponent::Server_SaveBoundInputFlagsToServerState(FState& ServerState, const FMove& SourceMove) const
{
  checkGMC(IsServerPawn())
  // Should only be called for the simulated proxy server state, never for the autonomous proxy server state.
  checkGMC(ServerState.RecipientRole == ROLE_SimulatedProxy)
  if (InputFlag1  && !bLockedInputFlag1)  ServerState.bInputFlag1  = SourceMove.bInputFlag1;
  if (InputFlag2  && !bLockedInputFlag2)  ServerState.bInputFlag2  = SourceMove.bInputFlag2;
  if (InputFlag3  && !bLockedInputFlag3)  ServerState.bInputFlag3  = SourceMove.bInputFlag3;
  if (InputFlag4  && !bLockedInputFlag4)  ServerState.bInputFlag4  = SourceMove.bInputFlag4;
  if (InputFlag5  && !bLockedInputFlag5)  ServerState.bInputFlag5  = SourceMove.bInputFlag5;
  if (InputFlag6  && !bLockedInputFlag6)  ServerState.bInputFlag6  = SourceMove.bInputFlag6;
  if (InputFlag7  && !bLockedInputFlag7)  ServerState.bInputFlag7  = SourceMove.bInputFlag7;
  if (InputFlag8  && !bLockedInputFlag8)  ServerState.bInputFlag8  = SourceMove.bInputFlag8;
  if (InputFlag9  && !bLockedInputFlag9)  ServerState.bInputFlag9  = SourceMove.bInputFlag9;
  if (InputFlag10 && !bLockedInputFlag10) ServerState.bInputFlag10 = SourceMove.bInputFlag10;
  if (InputFlag11 && !bLockedInputFlag11) ServerState.bInputFlag11 = SourceMove.bInputFlag11;
  if (InputFlag12 && !bLockedInputFlag12) ServerState.bInputFlag12 = SourceMove.bInputFlag12;
  if (InputFlag13 && !bLockedInputFlag13) ServerState.bInputFlag13 = SourceMove.bInputFlag13;
  if (InputFlag14 && !bLockedInputFlag14) ServerState.bInputFlag14 = SourceMove.bInputFlag14;
  if (InputFlag15 && !bLockedInputFlag15) ServerState.bInputFlag15 = SourceMove.bInputFlag15;
  if (InputFlag16 && !bLockedInputFlag16) ServerState.bInputFlag16 = SourceMove.bInputFlag16;
}

void UGenMovementReplicationComponent::Server_ResetLockedBoundInputFlags()
{
  checkGMC(IsServerPawn())
  const float CurrentTime = GetTime();
  if (bLockedInputFlag1)  { ResetLockTimerInputFlag1  = CurrentTime - LockSetTimeInputFlag1;  if (ResetLockTimerInputFlag1  > MinRepHoldTime) { bLockedInputFlag1  = false; ResetLockTimerInputFlag1  = 0.f; } }
  if (bLockedInputFlag2)  { ResetLockTimerInputFlag2  = CurrentTime - LockSetTimeInputFlag2;  if (ResetLockTimerInputFlag2  > MinRepHoldTime) { bLockedInputFlag2  = false; ResetLockTimerInputFlag2  = 0.f; } }
  if (bLockedInputFlag3)  { ResetLockTimerInputFlag3  = CurrentTime - LockSetTimeInputFlag3;  if (ResetLockTimerInputFlag3  > MinRepHoldTime) { bLockedInputFlag3  = false; ResetLockTimerInputFlag3  = 0.f; } }
  if (bLockedInputFlag4)  { ResetLockTimerInputFlag4  = CurrentTime - LockSetTimeInputFlag4;  if (ResetLockTimerInputFlag4  > MinRepHoldTime) { bLockedInputFlag4  = false; ResetLockTimerInputFlag4  = 0.f; } }
  if (bLockedInputFlag5)  { ResetLockTimerInputFlag5  = CurrentTime - LockSetTimeInputFlag5;  if (ResetLockTimerInputFlag5  > MinRepHoldTime) { bLockedInputFlag5  = false; ResetLockTimerInputFlag5  = 0.f; } }
  if (bLockedInputFlag6)  { ResetLockTimerInputFlag6  = CurrentTime - LockSetTimeInputFlag6;  if (ResetLockTimerInputFlag6  > MinRepHoldTime) { bLockedInputFlag6  = false; ResetLockTimerInputFlag6  = 0.f; } }
  if (bLockedInputFlag7)  { ResetLockTimerInputFlag7  = CurrentTime - LockSetTimeInputFlag7;  if (ResetLockTimerInputFlag7  > MinRepHoldTime) { bLockedInputFlag7  = false; ResetLockTimerInputFlag7  = 0.f; } }
  if (bLockedInputFlag8)  { ResetLockTimerInputFlag8  = CurrentTime - LockSetTimeInputFlag8;  if (ResetLockTimerInputFlag8  > MinRepHoldTime) { bLockedInputFlag8  = false; ResetLockTimerInputFlag8  = 0.f; } }
  if (bLockedInputFlag9)  { ResetLockTimerInputFlag9  = CurrentTime - LockSetTimeInputFlag9;  if (ResetLockTimerInputFlag9  > MinRepHoldTime) { bLockedInputFlag9  = false; ResetLockTimerInputFlag9  = 0.f; } }
  if (bLockedInputFlag10) { ResetLockTimerInputFlag10 = CurrentTime - LockSetTimeInputFlag10; if (ResetLockTimerInputFlag10 > MinRepHoldTime) { bLockedInputFlag10 = false; ResetLockTimerInputFlag10 = 0.f; } }
  if (bLockedInputFlag11) { ResetLockTimerInputFlag11 = CurrentTime - LockSetTimeInputFlag11; if (ResetLockTimerInputFlag11 > MinRepHoldTime) { bLockedInputFlag11 = false; ResetLockTimerInputFlag11 = 0.f; } }
  if (bLockedInputFlag12) { ResetLockTimerInputFlag12 = CurrentTime - LockSetTimeInputFlag12; if (ResetLockTimerInputFlag12 > MinRepHoldTime) { bLockedInputFlag12 = false; ResetLockTimerInputFlag12 = 0.f; } }
  if (bLockedInputFlag13) { ResetLockTimerInputFlag13 = CurrentTime - LockSetTimeInputFlag13; if (ResetLockTimerInputFlag13 > MinRepHoldTime) { bLockedInputFlag13 = false; ResetLockTimerInputFlag13 = 0.f; } }
  if (bLockedInputFlag14) { ResetLockTimerInputFlag14 = CurrentTime - LockSetTimeInputFlag14; if (ResetLockTimerInputFlag14 > MinRepHoldTime) { bLockedInputFlag14 = false; ResetLockTimerInputFlag14 = 0.f; } }
  if (bLockedInputFlag15) { ResetLockTimerInputFlag15 = CurrentTime - LockSetTimeInputFlag15; if (ResetLockTimerInputFlag15 > MinRepHoldTime) { bLockedInputFlag15 = false; ResetLockTimerInputFlag15 = 0.f; } }
  if (bLockedInputFlag16) { ResetLockTimerInputFlag16 = CurrentTime - LockSetTimeInputFlag16; if (ResetLockTimerInputFlag16 > MinRepHoldTime) { bLockedInputFlag16 = false; ResetLockTimerInputFlag16 = 0.f; } }
}

bool UGenMovementReplicationComponent::Client_EnqueueMoveCheckBoundInputFlags(const FMove& CurrentMove) const
{
  checkGMC(IsAutonomousProxy())
  // An input action causes a new move to be enqueued whenever it changes its value, or when it is activated and was bound with the
  // no-move-combine-flag enabled.
  // @attention Input flags can be skipped during binding so we don't return false when we encounter the first variable that is nullptr.
  if (InputFlag1)  { if (CurrentMove.bInputFlag1  != Client_LastSignificantMove.bInputFlag1  || CurrentMove.bInputFlag1  && bNoMoveCombineInputFlag1)  return true; }
  if (InputFlag2)  { if (CurrentMove.bInputFlag2  != Client_LastSignificantMove.bInputFlag2  || CurrentMove.bInputFlag2  && bNoMoveCombineInputFlag2)  return true; }
  if (InputFlag3)  { if (CurrentMove.bInputFlag3  != Client_LastSignificantMove.bInputFlag3  || CurrentMove.bInputFlag3  && bNoMoveCombineInputFlag3)  return true; }
  if (InputFlag4)  { if (CurrentMove.bInputFlag4  != Client_LastSignificantMove.bInputFlag4  || CurrentMove.bInputFlag4  && bNoMoveCombineInputFlag4)  return true; }
  if (InputFlag5)  { if (CurrentMove.bInputFlag5  != Client_LastSignificantMove.bInputFlag5  || CurrentMove.bInputFlag5  && bNoMoveCombineInputFlag5)  return true; }
  if (InputFlag6)  { if (CurrentMove.bInputFlag6  != Client_LastSignificantMove.bInputFlag6  || CurrentMove.bInputFlag6  && bNoMoveCombineInputFlag6)  return true; }
  if (InputFlag7)  { if (CurrentMove.bInputFlag7  != Client_LastSignificantMove.bInputFlag7  || CurrentMove.bInputFlag7  && bNoMoveCombineInputFlag7)  return true; }
  if (InputFlag8)  { if (CurrentMove.bInputFlag8  != Client_LastSignificantMove.bInputFlag8  || CurrentMove.bInputFlag8  && bNoMoveCombineInputFlag8)  return true; }
  if (InputFlag9)  { if (CurrentMove.bInputFlag9  != Client_LastSignificantMove.bInputFlag9  || CurrentMove.bInputFlag9  && bNoMoveCombineInputFlag9)  return true; }
  if (InputFlag10) { if (CurrentMove.bInputFlag10 != Client_LastSignificantMove.bInputFlag10 || CurrentMove.bInputFlag10 && bNoMoveCombineInputFlag10) return true; }
  if (InputFlag11) { if (CurrentMove.bInputFlag11 != Client_LastSignificantMove.bInputFlag11 || CurrentMove.bInputFlag11 && bNoMoveCombineInputFlag11) return true; }
  if (InputFlag12) { if (CurrentMove.bInputFlag12 != Client_LastSignificantMove.bInputFlag12 || CurrentMove.bInputFlag12 && bNoMoveCombineInputFlag12) return true; }
  if (InputFlag13) { if (CurrentMove.bInputFlag13 != Client_LastSignificantMove.bInputFlag13 || CurrentMove.bInputFlag13 && bNoMoveCombineInputFlag13) return true; }
  if (InputFlag14) { if (CurrentMove.bInputFlag14 != Client_LastSignificantMove.bInputFlag14 || CurrentMove.bInputFlag14 && bNoMoveCombineInputFlag14) return true; }
  if (InputFlag15) { if (CurrentMove.bInputFlag15 != Client_LastSignificantMove.bInputFlag15 || CurrentMove.bInputFlag15 && bNoMoveCombineInputFlag15) return true; }
  if (InputFlag16) { if (CurrentMove.bInputFlag16 != Client_LastSignificantMove.bInputFlag16 || CurrentMove.bInputFlag16 && bNoMoveCombineInputFlag16) return true; }
  return false;
}

void UGenMovementReplicationComponent::SaveBoundInputFlagsToMove(FMove& Move) const
{
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (InputFlag1)  Move.bInputFlag1  = *InputFlag1;
  if (InputFlag2)  Move.bInputFlag2  = *InputFlag2;
  if (InputFlag3)  Move.bInputFlag3  = *InputFlag3;
  if (InputFlag4)  Move.bInputFlag4  = *InputFlag4;
  if (InputFlag5)  Move.bInputFlag5  = *InputFlag5;
  if (InputFlag6)  Move.bInputFlag6  = *InputFlag6;
  if (InputFlag7)  Move.bInputFlag7  = *InputFlag7;
  if (InputFlag8)  Move.bInputFlag8  = *InputFlag8;
  if (InputFlag9)  Move.bInputFlag9  = *InputFlag9;
  if (InputFlag10) Move.bInputFlag10 = *InputFlag10;
  if (InputFlag11) Move.bInputFlag11 = *InputFlag11;
  if (InputFlag12) Move.bInputFlag12 = *InputFlag12;
  if (InputFlag13) Move.bInputFlag13 = *InputFlag13;
  if (InputFlag14) Move.bInputFlag14 = *InputFlag14;
  if (InputFlag15) Move.bInputFlag15 = *InputFlag15;
  if (InputFlag16) Move.bInputFlag16 = *InputFlag16;
}

void UGenMovementReplicationComponent::SaveBoundInputFlagsToState(FState& State) const
{
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (InputFlag1)  State.bInputFlag1  = *InputFlag1;
  if (InputFlag2)  State.bInputFlag2  = *InputFlag2;
  if (InputFlag3)  State.bInputFlag3  = *InputFlag3;
  if (InputFlag4)  State.bInputFlag4  = *InputFlag4;
  if (InputFlag5)  State.bInputFlag5  = *InputFlag5;
  if (InputFlag6)  State.bInputFlag6  = *InputFlag6;
  if (InputFlag7)  State.bInputFlag7  = *InputFlag7;
  if (InputFlag8)  State.bInputFlag8  = *InputFlag8;
  if (InputFlag9)  State.bInputFlag9  = *InputFlag9;
  if (InputFlag10) State.bInputFlag10 = *InputFlag10;
  if (InputFlag11) State.bInputFlag11 = *InputFlag11;
  if (InputFlag12) State.bInputFlag12 = *InputFlag12;
  if (InputFlag13) State.bInputFlag13 = *InputFlag13;
  if (InputFlag14) State.bInputFlag14 = *InputFlag14;
  if (InputFlag15) State.bInputFlag15 = *InputFlag15;
  if (InputFlag16) State.bInputFlag16 = *InputFlag16;
}

void UGenMovementReplicationComponent::LoadBoundInputFlagsFromMove(const FMove& Move) const
{
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (InputFlag1)  *InputFlag1  = Move.bInputFlag1;
  if (InputFlag2)  *InputFlag2  = Move.bInputFlag2;
  if (InputFlag3)  *InputFlag3  = Move.bInputFlag3;
  if (InputFlag4)  *InputFlag4  = Move.bInputFlag4;
  if (InputFlag5)  *InputFlag5  = Move.bInputFlag5;
  if (InputFlag6)  *InputFlag6  = Move.bInputFlag6;
  if (InputFlag7)  *InputFlag7  = Move.bInputFlag7;
  if (InputFlag8)  *InputFlag8  = Move.bInputFlag8;
  if (InputFlag9)  *InputFlag9  = Move.bInputFlag9;
  if (InputFlag10) *InputFlag10 = Move.bInputFlag10;
  if (InputFlag11) *InputFlag11 = Move.bInputFlag11;
  if (InputFlag12) *InputFlag12 = Move.bInputFlag12;
  if (InputFlag13) *InputFlag13 = Move.bInputFlag13;
  if (InputFlag14) *InputFlag14 = Move.bInputFlag14;
  if (InputFlag15) *InputFlag15 = Move.bInputFlag15;
  if (InputFlag16) *InputFlag16 = Move.bInputFlag16;
}

void UGenMovementReplicationComponent::LoadBoundInputFlagsFromState(const FState& State) const
{
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (InputFlag1)  *InputFlag1  = State.bInputFlag1;
  if (InputFlag2)  *InputFlag2  = State.bInputFlag2;
  if (InputFlag3)  *InputFlag3  = State.bInputFlag3;
  if (InputFlag4)  *InputFlag4  = State.bInputFlag4;
  if (InputFlag5)  *InputFlag5  = State.bInputFlag5;
  if (InputFlag6)  *InputFlag6  = State.bInputFlag6;
  if (InputFlag7)  *InputFlag7  = State.bInputFlag7;
  if (InputFlag8)  *InputFlag8  = State.bInputFlag8;
  if (InputFlag9)  *InputFlag9  = State.bInputFlag9;
  if (InputFlag10) *InputFlag10 = State.bInputFlag10;
  if (InputFlag11) *InputFlag11 = State.bInputFlag11;
  if (InputFlag12) *InputFlag12 = State.bInputFlag12;
  if (InputFlag13) *InputFlag13 = State.bInputFlag13;
  if (InputFlag14) *InputFlag14 = State.bInputFlag14;
  if (InputFlag15) *InputFlag15 = State.bInputFlag15;
  if (InputFlag16) *InputFlag16 = State.bInputFlag16;
}

void UGenMovementReplicationComponent::LoadReplicatedBoundInputFlagsFromState(const FState& State) const
{
  // Only update bound input flags that are actually replicated.
  checkGMC(State.RecipientRole == ROLE_SimulatedProxy ?
       ServerState_SimulatedProxy().bReplicateInputFlag1  == State.bReplicateInputFlag1
    && ServerState_SimulatedProxy().bReplicateInputFlag2  == State.bReplicateInputFlag2
    && ServerState_SimulatedProxy().bReplicateInputFlag3  == State.bReplicateInputFlag3
    && ServerState_SimulatedProxy().bReplicateInputFlag4  == State.bReplicateInputFlag4
    && ServerState_SimulatedProxy().bReplicateInputFlag5  == State.bReplicateInputFlag5
    && ServerState_SimulatedProxy().bReplicateInputFlag6  == State.bReplicateInputFlag6
    && ServerState_SimulatedProxy().bReplicateInputFlag7  == State.bReplicateInputFlag7
    && ServerState_SimulatedProxy().bReplicateInputFlag8  == State.bReplicateInputFlag8
    && ServerState_SimulatedProxy().bReplicateInputFlag9  == State.bReplicateInputFlag9
    && ServerState_SimulatedProxy().bReplicateInputFlag10 == State.bReplicateInputFlag10
    && ServerState_SimulatedProxy().bReplicateInputFlag11 == State.bReplicateInputFlag11
    && ServerState_SimulatedProxy().bReplicateInputFlag12 == State.bReplicateInputFlag12
    && ServerState_SimulatedProxy().bReplicateInputFlag13 == State.bReplicateInputFlag13
    && ServerState_SimulatedProxy().bReplicateInputFlag14 == State.bReplicateInputFlag14
    && ServerState_SimulatedProxy().bReplicateInputFlag15 == State.bReplicateInputFlag15
    && ServerState_SimulatedProxy().bReplicateInputFlag16 == State.bReplicateInputFlag16
    : true
  )
  checkGMC(State.RecipientRole == ROLE_AutonomousProxy ?
       ServerState_AutonomousProxy().bReplicateInputFlag1  == State.bReplicateInputFlag1
    && ServerState_AutonomousProxy().bReplicateInputFlag2  == State.bReplicateInputFlag2
    && ServerState_AutonomousProxy().bReplicateInputFlag3  == State.bReplicateInputFlag3
    && ServerState_AutonomousProxy().bReplicateInputFlag4  == State.bReplicateInputFlag4
    && ServerState_AutonomousProxy().bReplicateInputFlag5  == State.bReplicateInputFlag5
    && ServerState_AutonomousProxy().bReplicateInputFlag6  == State.bReplicateInputFlag6
    && ServerState_AutonomousProxy().bReplicateInputFlag7  == State.bReplicateInputFlag7
    && ServerState_AutonomousProxy().bReplicateInputFlag8  == State.bReplicateInputFlag8
    && ServerState_AutonomousProxy().bReplicateInputFlag9  == State.bReplicateInputFlag9
    && ServerState_AutonomousProxy().bReplicateInputFlag10 == State.bReplicateInputFlag10
    && ServerState_AutonomousProxy().bReplicateInputFlag11 == State.bReplicateInputFlag11
    && ServerState_AutonomousProxy().bReplicateInputFlag12 == State.bReplicateInputFlag12
    && ServerState_AutonomousProxy().bReplicateInputFlag13 == State.bReplicateInputFlag13
    && ServerState_AutonomousProxy().bReplicateInputFlag14 == State.bReplicateInputFlag14
    && ServerState_AutonomousProxy().bReplicateInputFlag15 == State.bReplicateInputFlag15
    && ServerState_AutonomousProxy().bReplicateInputFlag16 == State.bReplicateInputFlag16
    : true
  )
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (InputFlag1  && State.bReplicateInputFlag1)  *InputFlag1  = State.bInputFlag1;
  if (InputFlag2  && State.bReplicateInputFlag2)  *InputFlag2  = State.bInputFlag2;
  if (InputFlag3  && State.bReplicateInputFlag3)  *InputFlag3  = State.bInputFlag3;
  if (InputFlag4  && State.bReplicateInputFlag4)  *InputFlag4  = State.bInputFlag4;
  if (InputFlag5  && State.bReplicateInputFlag5)  *InputFlag5  = State.bInputFlag5;
  if (InputFlag6  && State.bReplicateInputFlag6)  *InputFlag6  = State.bInputFlag6;
  if (InputFlag7  && State.bReplicateInputFlag7)  *InputFlag7  = State.bInputFlag7;
  if (InputFlag8  && State.bReplicateInputFlag8)  *InputFlag8  = State.bInputFlag8;
  if (InputFlag9  && State.bReplicateInputFlag9)  *InputFlag9  = State.bInputFlag9;
  if (InputFlag10 && State.bReplicateInputFlag10) *InputFlag10 = State.bInputFlag10;
  if (InputFlag11 && State.bReplicateInputFlag11) *InputFlag11 = State.bInputFlag11;
  if (InputFlag12 && State.bReplicateInputFlag12) *InputFlag12 = State.bInputFlag12;
  if (InputFlag13 && State.bReplicateInputFlag13) *InputFlag13 = State.bInputFlag13;
  if (InputFlag14 && State.bReplicateInputFlag14) *InputFlag14 = State.bInputFlag14;
  if (InputFlag15 && State.bReplicateInputFlag15) *InputFlag15 = State.bInputFlag15;
  if (InputFlag16 && State.bReplicateInputFlag16) *InputFlag16 = State.bInputFlag16;
}

void UGenMovementReplicationComponent::AddTargetStateInputFlagsToInitState(FState& InitializationState, const FState& TargetState) const
{
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (IsValidStateQueueIndex(CurrentTargetStateIndex + 1)) {
    if (InputFlag1)  { if (InitializationState.bInputFlag1  != TargetState.bInputFlag1  && InitializationState.bInputFlag1  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag1)  InitializationState.bInputFlag1  = TargetState.bInputFlag1;  }
    if (InputFlag2)  { if (InitializationState.bInputFlag2  != TargetState.bInputFlag2  && InitializationState.bInputFlag2  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag2)  InitializationState.bInputFlag2  = TargetState.bInputFlag2;  }
    if (InputFlag3)  { if (InitializationState.bInputFlag3  != TargetState.bInputFlag3  && InitializationState.bInputFlag3  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag3)  InitializationState.bInputFlag3  = TargetState.bInputFlag3;  }
    if (InputFlag4)  { if (InitializationState.bInputFlag4  != TargetState.bInputFlag4  && InitializationState.bInputFlag4  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag4)  InitializationState.bInputFlag4  = TargetState.bInputFlag4;  }
    if (InputFlag5)  { if (InitializationState.bInputFlag5  != TargetState.bInputFlag5  && InitializationState.bInputFlag5  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag5)  InitializationState.bInputFlag5  = TargetState.bInputFlag5;  }
    if (InputFlag6)  { if (InitializationState.bInputFlag6  != TargetState.bInputFlag6  && InitializationState.bInputFlag6  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag6)  InitializationState.bInputFlag6  = TargetState.bInputFlag6;  }
    if (InputFlag7)  { if (InitializationState.bInputFlag7  != TargetState.bInputFlag7  && InitializationState.bInputFlag7  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag7)  InitializationState.bInputFlag7  = TargetState.bInputFlag7;  }
    if (InputFlag8)  { if (InitializationState.bInputFlag8  != TargetState.bInputFlag8  && InitializationState.bInputFlag8  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag8)  InitializationState.bInputFlag8  = TargetState.bInputFlag8;  }
    if (InputFlag9)  { if (InitializationState.bInputFlag9  != TargetState.bInputFlag9  && InitializationState.bInputFlag9  == StateQueue[CurrentTargetStateIndex + 1].bInputFlag9)  InitializationState.bInputFlag9  = TargetState.bInputFlag9;  }
    if (InputFlag10) { if (InitializationState.bInputFlag10 != TargetState.bInputFlag10 && InitializationState.bInputFlag10 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag10) InitializationState.bInputFlag10 = TargetState.bInputFlag10; }
    if (InputFlag11) { if (InitializationState.bInputFlag11 != TargetState.bInputFlag11 && InitializationState.bInputFlag11 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag11) InitializationState.bInputFlag11 = TargetState.bInputFlag11; }
    if (InputFlag12) { if (InitializationState.bInputFlag12 != TargetState.bInputFlag12 && InitializationState.bInputFlag12 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag12) InitializationState.bInputFlag12 = TargetState.bInputFlag12; }
    if (InputFlag13) { if (InitializationState.bInputFlag13 != TargetState.bInputFlag13 && InitializationState.bInputFlag13 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag13) InitializationState.bInputFlag13 = TargetState.bInputFlag13; }
    if (InputFlag14) { if (InitializationState.bInputFlag14 != TargetState.bInputFlag14 && InitializationState.bInputFlag14 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag14) InitializationState.bInputFlag14 = TargetState.bInputFlag14; }
    if (InputFlag15) { if (InitializationState.bInputFlag15 != TargetState.bInputFlag15 && InitializationState.bInputFlag15 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag15) InitializationState.bInputFlag15 = TargetState.bInputFlag15; }
    if (InputFlag16) { if (InitializationState.bInputFlag16 != TargetState.bInputFlag16 && InitializationState.bInputFlag16 == StateQueue[CurrentTargetStateIndex + 1].bInputFlag16) InitializationState.bInputFlag16 = TargetState.bInputFlag16; }
  }
}

void UGenMovementReplicationComponent::AddStartStateInputFlagsToInitState(FState& InitializationState, const FState& StartState) const
{
  // @attention Input flags can be skipped during binding so we don't return when we encounter the first variable that is nullptr.
  if (IsValidStateQueueIndex(CurrentStartStateIndex - 1)) {
    if (InputFlag1)  { if (InitializationState.bInputFlag1  != StartState.bInputFlag1  && InitializationState.bInputFlag1  == StateQueue[CurrentStartStateIndex - 1].bInputFlag1)  InitializationState.bInputFlag1  = StartState.bInputFlag1;  }
    if (InputFlag2)  { if (InitializationState.bInputFlag2  != StartState.bInputFlag2  && InitializationState.bInputFlag2  == StateQueue[CurrentStartStateIndex - 1].bInputFlag2)  InitializationState.bInputFlag2  = StartState.bInputFlag2;  }
    if (InputFlag3)  { if (InitializationState.bInputFlag3  != StartState.bInputFlag3  && InitializationState.bInputFlag3  == StateQueue[CurrentStartStateIndex - 1].bInputFlag3)  InitializationState.bInputFlag3  = StartState.bInputFlag3;  }
    if (InputFlag4)  { if (InitializationState.bInputFlag4  != StartState.bInputFlag4  && InitializationState.bInputFlag4  == StateQueue[CurrentStartStateIndex - 1].bInputFlag4)  InitializationState.bInputFlag4  = StartState.bInputFlag4;  }
    if (InputFlag5)  { if (InitializationState.bInputFlag5  != StartState.bInputFlag5  && InitializationState.bInputFlag5  == StateQueue[CurrentStartStateIndex - 1].bInputFlag5)  InitializationState.bInputFlag5  = StartState.bInputFlag5;  }
    if (InputFlag6)  { if (InitializationState.bInputFlag6  != StartState.bInputFlag6  && InitializationState.bInputFlag6  == StateQueue[CurrentStartStateIndex - 1].bInputFlag6)  InitializationState.bInputFlag6  = StartState.bInputFlag6;  }
    if (InputFlag7)  { if (InitializationState.bInputFlag7  != StartState.bInputFlag7  && InitializationState.bInputFlag7  == StateQueue[CurrentStartStateIndex - 1].bInputFlag7)  InitializationState.bInputFlag7  = StartState.bInputFlag7;  }
    if (InputFlag8)  { if (InitializationState.bInputFlag8  != StartState.bInputFlag8  && InitializationState.bInputFlag8  == StateQueue[CurrentStartStateIndex - 1].bInputFlag8)  InitializationState.bInputFlag8  = StartState.bInputFlag8;  }
    if (InputFlag9)  { if (InitializationState.bInputFlag9  != StartState.bInputFlag9  && InitializationState.bInputFlag9  == StateQueue[CurrentStartStateIndex - 1].bInputFlag9)  InitializationState.bInputFlag9  = StartState.bInputFlag9;  }
    if (InputFlag10) { if (InitializationState.bInputFlag10 != StartState.bInputFlag10 && InitializationState.bInputFlag10 == StateQueue[CurrentStartStateIndex - 1].bInputFlag10) InitializationState.bInputFlag10 = StartState.bInputFlag10; }
    if (InputFlag11) { if (InitializationState.bInputFlag11 != StartState.bInputFlag11 && InitializationState.bInputFlag11 == StateQueue[CurrentStartStateIndex - 1].bInputFlag11) InitializationState.bInputFlag11 = StartState.bInputFlag11; }
    if (InputFlag12) { if (InitializationState.bInputFlag12 != StartState.bInputFlag12 && InitializationState.bInputFlag12 == StateQueue[CurrentStartStateIndex - 1].bInputFlag12) InitializationState.bInputFlag12 = StartState.bInputFlag12; }
    if (InputFlag13) { if (InitializationState.bInputFlag13 != StartState.bInputFlag13 && InitializationState.bInputFlag13 == StateQueue[CurrentStartStateIndex - 1].bInputFlag13) InitializationState.bInputFlag13 = StartState.bInputFlag13; }
    if (InputFlag14) { if (InitializationState.bInputFlag14 != StartState.bInputFlag14 && InitializationState.bInputFlag14 == StateQueue[CurrentStartStateIndex - 1].bInputFlag14) InitializationState.bInputFlag14 = StartState.bInputFlag14; }
    if (InputFlag15) { if (InitializationState.bInputFlag15 != StartState.bInputFlag15 && InitializationState.bInputFlag15 == StateQueue[CurrentStartStateIndex - 1].bInputFlag15) InitializationState.bInputFlag15 = StartState.bInputFlag15; }
    if (InputFlag16) { if (InitializationState.bInputFlag16 != StartState.bInputFlag16 && InitializationState.bInputFlag16 == StateQueue[CurrentStartStateIndex - 1].bInputFlag16) InitializationState.bInputFlag16 = StartState.bInputFlag16; }
  }
}

void UGenMovementReplicationComponent::Server_SwapStateBufferBoundData()
{
  checkGMC(IsServerPawn())
  SwapStateBufferBoundData_IMPLEMENTATION()
}

bool UGenMovementReplicationComponent::Server_ForceNetUpdateCheckBoundData(FState& ServerState, ENetRole RecipientRole)
{
  checkGMC(IsServerPawn())
  bool bShouldForceNetUpdate = false;
  ForceNetUpdateCheckBoundData_IMPLEMENTATION()
  return bShouldForceNetUpdate;
}

void UGenMovementReplicationComponent::Server_SaveBoundDataToServerState(
  FState& ServerState,
  ENetRole RecipientRole,
  bool bUsingClientPrediction
) const
{
  SCOPE_CYCLE_COUNTER(STAT_Server_SaveBoundDataToServerState)

  checkGMC(IsServerPawn())
  checkGMC(RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy)
  SaveBoundDataToServerState_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::Server_ResetLockedBoundData()
{
  checkGMC(IsServerPawn())
  ResetLockedBoundData_IMPLEMENTATION()
}

bool UGenMovementReplicationComponent::Client_IsBoundDataValid(const FMove& SourceMove) const
{
  checkGMC(IsAutonomousProxy())
  IsBoundDataValid_IMPLEMENTATION()
  // @attention We can return from inside the macro so this section of the function is not always reached.
  return true;
}

void UGenMovementReplicationComponent::Client_UnpackBoundData(FState& ServerState) const
{
  checkGMC(IsAutonomousProxy() || IsSimulatedProxy())
  UnpackBoundData_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::Client_LoadBoundDataForReplay(const FMove& SourceMove)
{
  checkGMC(IsAutonomousProxy())
  LoadBoundDataForReplay_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::SaveBoundDataToMove(FMove& Move, FMove::EStateVars VarsToSave) const
{
  SCOPE_CYCLE_COUNTER(STAT_SaveBoundDataToMove)

  if (VarsToSave == FMove::EStateVars::Input)
  {
    SaveInBoundDataToMove_IMPLEMENTATION()
  }
  else if (VarsToSave == FMove::EStateVars::Output)
  {
    SaveOutBoundDataToMove_IMPLEMENTATION()
  }
}

void UGenMovementReplicationComponent::SaveBoundDataToState(FState& State) const
{
  SaveBoundDataToState_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::LoadInBoundDataFromMove(const FMove& Move) const
{
  LoadInBoundDataFromMove_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::LoadBoundDataFromState(const FState& State) const
{
  LoadBoundDataFromState_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::LoadReplicatedBoundDataFromState(const FState& State) const
{
  LoadReplicatedBoundDataFromState_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::AddTargetStateBoundDataToInitState(FState& InitializationState, const FState& TargetState) const
{
  AddTargetStateBoundDataToInitState_IMPLEMENTATION()
}

void UGenMovementReplicationComponent::AddStartStateBoundDataToInitState(FState& InitializationState, const FState& StartState) const
{
  AddStartStateBoundDataToInitState_IMPLEMENTATION()
}

bool UGenMovementReplicationComponent::Client_IsValidFloat(const FMove& SourceMove) const
{
  checkGMC(IsAutonomousProxy())
  const auto& ServerState = ServerState_AutonomousProxy();
  constexpr float COMPARE_TOLERANCE = 0.000001f;
  if (Float1)  { if (ServerState.bReplicateFloat1  && !FMath::IsNearlyEqual(ServerState.Float1,  SourceMove.OutFloat1,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float1  (%f) != SourceMove.OutFloat1  (%f)"), ServerState.Float1,  SourceMove.OutFloat1)  return false; } } else return true;
  if (Float2)  { if (ServerState.bReplicateFloat2  && !FMath::IsNearlyEqual(ServerState.Float2,  SourceMove.OutFloat2,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float2  (%f) != SourceMove.OutFloat2  (%f)"), ServerState.Float2,  SourceMove.OutFloat2)  return false; } } else return true;
  if (Float3)  { if (ServerState.bReplicateFloat3  && !FMath::IsNearlyEqual(ServerState.Float3,  SourceMove.OutFloat3,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float3  (%f) != SourceMove.OutFloat3  (%f)"), ServerState.Float3,  SourceMove.OutFloat3)  return false; } } else return true;
  if (Float4)  { if (ServerState.bReplicateFloat4  && !FMath::IsNearlyEqual(ServerState.Float4,  SourceMove.OutFloat4,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float4  (%f) != SourceMove.OutFloat4  (%f)"), ServerState.Float4,  SourceMove.OutFloat4)  return false; } } else return true;
  if (Float5)  { if (ServerState.bReplicateFloat5  && !FMath::IsNearlyEqual(ServerState.Float5,  SourceMove.OutFloat5,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float5  (%f) != SourceMove.OutFloat5  (%f)"), ServerState.Float5,  SourceMove.OutFloat5)  return false; } } else return true;
  if (Float6)  { if (ServerState.bReplicateFloat6  && !FMath::IsNearlyEqual(ServerState.Float6,  SourceMove.OutFloat6,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float6  (%f) != SourceMove.OutFloat6  (%f)"), ServerState.Float6,  SourceMove.OutFloat6)  return false; } } else return true;
  if (Float7)  { if (ServerState.bReplicateFloat7  && !FMath::IsNearlyEqual(ServerState.Float7,  SourceMove.OutFloat7,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float7  (%f) != SourceMove.OutFloat7  (%f)"), ServerState.Float7,  SourceMove.OutFloat7)  return false; } } else return true;
  if (Float8)  { if (ServerState.bReplicateFloat8  && !FMath::IsNearlyEqual(ServerState.Float8,  SourceMove.OutFloat8,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float8  (%f) != SourceMove.OutFloat8  (%f)"), ServerState.Float8,  SourceMove.OutFloat8)  return false; } } else return true;
  if (Float9)  { if (ServerState.bReplicateFloat9  && !FMath::IsNearlyEqual(ServerState.Float9,  SourceMove.OutFloat9,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float9  (%f) != SourceMove.OutFloat9  (%f)"), ServerState.Float9,  SourceMove.OutFloat9)  return false; } } else return true;
  if (Float10) { if (ServerState.bReplicateFloat10 && !FMath::IsNearlyEqual(ServerState.Float10, SourceMove.OutFloat10, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float10 (%f) != SourceMove.OutFloat10 (%f)"), ServerState.Float10, SourceMove.OutFloat10) return false; } } else return true;
  if (Float11) { if (ServerState.bReplicateFloat11 && !FMath::IsNearlyEqual(ServerState.Float11, SourceMove.OutFloat11, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float11 (%f) != SourceMove.OutFloat11 (%f)"), ServerState.Float11, SourceMove.OutFloat11) return false; } } else return true;
  if (Float12) { if (ServerState.bReplicateFloat12 && !FMath::IsNearlyEqual(ServerState.Float12, SourceMove.OutFloat12, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float12 (%f) != SourceMove.OutFloat12 (%f)"), ServerState.Float12, SourceMove.OutFloat12) return false; } } else return true;
  if (Float13) { if (ServerState.bReplicateFloat13 && !FMath::IsNearlyEqual(ServerState.Float13, SourceMove.OutFloat13, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float13 (%f) != SourceMove.OutFloat13 (%f)"), ServerState.Float13, SourceMove.OutFloat13) return false; } } else return true;
  if (Float14) { if (ServerState.bReplicateFloat14 && !FMath::IsNearlyEqual(ServerState.Float14, SourceMove.OutFloat14, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float14 (%f) != SourceMove.OutFloat14 (%f)"), ServerState.Float14, SourceMove.OutFloat14) return false; } } else return true;
  if (Float15) { if (ServerState.bReplicateFloat15 && !FMath::IsNearlyEqual(ServerState.Float15, SourceMove.OutFloat15, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float15 (%f) != SourceMove.OutFloat15 (%f)"), ServerState.Float15, SourceMove.OutFloat15) return false; } } else return true;
  if (Float16) { if (ServerState.bReplicateFloat16 && !FMath::IsNearlyEqual(ServerState.Float16, SourceMove.OutFloat16, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Float16 (%f) != SourceMove.OutFloat16 (%f)"), ServerState.Float16, SourceMove.OutFloat16) return false; } } else return true;
  return true;
}

bool UGenMovementReplicationComponent::Client_IsValidDouble(const FMove& SourceMove) const
{
  checkGMC(IsAutonomousProxy())
  const auto& ServerState = ServerState_AutonomousProxy();
  constexpr double COMPARE_TOLERANCE = 1e-15;
  if (Double1)  { if (ServerState.bReplicateDouble1  && !FMath::IsNearlyEqual(ServerState.Double1,  SourceMove.OutDouble1,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double1  (%.15f) != SourceMove.OutDouble1  (%.15f)"), ServerState.Double1,  SourceMove.OutDouble1)  return false; } } else return true;
  if (Double2)  { if (ServerState.bReplicateDouble2  && !FMath::IsNearlyEqual(ServerState.Double2,  SourceMove.OutDouble2,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double2  (%.15f) != SourceMove.OutDouble2  (%.15f)"), ServerState.Double2,  SourceMove.OutDouble2)  return false; } } else return true;
  if (Double3)  { if (ServerState.bReplicateDouble3  && !FMath::IsNearlyEqual(ServerState.Double3,  SourceMove.OutDouble3,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double3  (%.15f) != SourceMove.OutDouble3  (%.15f)"), ServerState.Double3,  SourceMove.OutDouble3)  return false; } } else return true;
  if (Double4)  { if (ServerState.bReplicateDouble4  && !FMath::IsNearlyEqual(ServerState.Double4,  SourceMove.OutDouble4,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double4  (%.15f) != SourceMove.OutDouble4  (%.15f)"), ServerState.Double4,  SourceMove.OutDouble4)  return false; } } else return true;
  if (Double5)  { if (ServerState.bReplicateDouble5  && !FMath::IsNearlyEqual(ServerState.Double5,  SourceMove.OutDouble5,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double5  (%.15f) != SourceMove.OutDouble5  (%.15f)"), ServerState.Double5,  SourceMove.OutDouble5)  return false; } } else return true;
  if (Double6)  { if (ServerState.bReplicateDouble6  && !FMath::IsNearlyEqual(ServerState.Double6,  SourceMove.OutDouble6,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double6  (%.15f) != SourceMove.OutDouble6  (%.15f)"), ServerState.Double6,  SourceMove.OutDouble6)  return false; } } else return true;
  if (Double7)  { if (ServerState.bReplicateDouble7  && !FMath::IsNearlyEqual(ServerState.Double7,  SourceMove.OutDouble7,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double7  (%.15f) != SourceMove.OutDouble7  (%.15f)"), ServerState.Double7,  SourceMove.OutDouble7)  return false; } } else return true;
  if (Double8)  { if (ServerState.bReplicateDouble8  && !FMath::IsNearlyEqual(ServerState.Double8,  SourceMove.OutDouble8,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double8  (%.15f) != SourceMove.OutDouble8  (%.15f)"), ServerState.Double8,  SourceMove.OutDouble8)  return false; } } else return true;
  if (Double9)  { if (ServerState.bReplicateDouble9  && !FMath::IsNearlyEqual(ServerState.Double9,  SourceMove.OutDouble9,  COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double9  (%.15f) != SourceMove.OutDouble9  (%.15f)"), ServerState.Double9,  SourceMove.OutDouble9)  return false; } } else return true;
  if (Double10) { if (ServerState.bReplicateDouble10 && !FMath::IsNearlyEqual(ServerState.Double10, SourceMove.OutDouble10, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double10 (%.15f) != SourceMove.OutDouble10 (%.15f)"), ServerState.Double10, SourceMove.OutDouble10) return false; } } else return true;
  if (Double11) { if (ServerState.bReplicateDouble11 && !FMath::IsNearlyEqual(ServerState.Double11, SourceMove.OutDouble11, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double11 (%.15f) != SourceMove.OutDouble11 (%.15f)"), ServerState.Double11, SourceMove.OutDouble11) return false; } } else return true;
  if (Double12) { if (ServerState.bReplicateDouble12 && !FMath::IsNearlyEqual(ServerState.Double12, SourceMove.OutDouble12, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double12 (%.15f) != SourceMove.OutDouble12 (%.15f)"), ServerState.Double12, SourceMove.OutDouble12) return false; } } else return true;
  if (Double13) { if (ServerState.bReplicateDouble13 && !FMath::IsNearlyEqual(ServerState.Double13, SourceMove.OutDouble13, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double13 (%.15f) != SourceMove.OutDouble13 (%.15f)"), ServerState.Double13, SourceMove.OutDouble13) return false; } } else return true;
  if (Double14) { if (ServerState.bReplicateDouble14 && !FMath::IsNearlyEqual(ServerState.Double14, SourceMove.OutDouble14, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double14 (%.15f) != SourceMove.OutDouble14 (%.15f)"), ServerState.Double14, SourceMove.OutDouble14) return false; } } else return true;
  if (Double15) { if (ServerState.bReplicateDouble15 && !FMath::IsNearlyEqual(ServerState.Double15, SourceMove.OutDouble15, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double15 (%.15f) != SourceMove.OutDouble15 (%.15f)"), ServerState.Double15, SourceMove.OutDouble15) return false; } } else return true;
  if (Double16) { if (ServerState.bReplicateDouble16 && !FMath::IsNearlyEqual(ServerState.Double16, SourceMove.OutDouble16, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Double16 (%.15f) != SourceMove.OutDouble16 (%.15f)"), ServerState.Double16, SourceMove.OutDouble16) return false; } } else return true;
  return true;
}

bool UGenMovementReplicationComponent::Client_IsValidVector(const FMove& SourceMove) const
{
  checkGMC(IsAutonomousProxy())
  const auto& ServerState = ServerState_AutonomousProxy();
  constexpr float COMPARE_TOLERANCE = 0.01f; // @see SerializeVectorTypes
  if (Vector1)  { if (ServerState.bReplicateVector1  && !ServerState.Vector1.Equals(SourceMove.OutVector1,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector1  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector1  (X: %f, Y: %f, Z: %f)"), ServerState.Vector1.X,  ServerState.Vector1.Y,  ServerState.Vector1.Z,  SourceMove.OutVector1.X,  SourceMove.OutVector1.Y,  SourceMove.OutVector1.Z)  return false; } } else return true;
  if (Vector2)  { if (ServerState.bReplicateVector2  && !ServerState.Vector2.Equals(SourceMove.OutVector2,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector2  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector2  (X: %f, Y: %f, Z: %f)"), ServerState.Vector2.X,  ServerState.Vector2.Y,  ServerState.Vector2.Z,  SourceMove.OutVector2.X,  SourceMove.OutVector2.Y,  SourceMove.OutVector2.Z)  return false; } } else return true;
  if (Vector3)  { if (ServerState.bReplicateVector3  && !ServerState.Vector3.Equals(SourceMove.OutVector3,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector3  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector3  (X: %f, Y: %f, Z: %f)"), ServerState.Vector3.X,  ServerState.Vector3.Y,  ServerState.Vector3.Z,  SourceMove.OutVector3.X,  SourceMove.OutVector3.Y,  SourceMove.OutVector3.Z)  return false; } } else return true;
  if (Vector4)  { if (ServerState.bReplicateVector4  && !ServerState.Vector4.Equals(SourceMove.OutVector4,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector4  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector4  (X: %f, Y: %f, Z: %f)"), ServerState.Vector4.X,  ServerState.Vector4.Y,  ServerState.Vector4.Z,  SourceMove.OutVector4.X,  SourceMove.OutVector4.Y,  SourceMove.OutVector4.Z)  return false; } } else return true;
  if (Vector5)  { if (ServerState.bReplicateVector5  && !ServerState.Vector5.Equals(SourceMove.OutVector5,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector5  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector5  (X: %f, Y: %f, Z: %f)"), ServerState.Vector5.X,  ServerState.Vector5.Y,  ServerState.Vector5.Z,  SourceMove.OutVector5.X,  SourceMove.OutVector5.Y,  SourceMove.OutVector5.Z)  return false; } } else return true;
  if (Vector6)  { if (ServerState.bReplicateVector6  && !ServerState.Vector6.Equals(SourceMove.OutVector6,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector6  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector6  (X: %f, Y: %f, Z: %f)"), ServerState.Vector6.X,  ServerState.Vector6.Y,  ServerState.Vector6.Z,  SourceMove.OutVector6.X,  SourceMove.OutVector6.Y,  SourceMove.OutVector6.Z)  return false; } } else return true;
  if (Vector7)  { if (ServerState.bReplicateVector7  && !ServerState.Vector7.Equals(SourceMove.OutVector7,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector7  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector7  (X: %f, Y: %f, Z: %f)"), ServerState.Vector7.X,  ServerState.Vector7.Y,  ServerState.Vector7.Z,  SourceMove.OutVector7.X,  SourceMove.OutVector7.Y,  SourceMove.OutVector7.Z)  return false; } } else return true;
  if (Vector8)  { if (ServerState.bReplicateVector8  && !ServerState.Vector8.Equals(SourceMove.OutVector8,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector8  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector8  (X: %f, Y: %f, Z: %f)"), ServerState.Vector8.X,  ServerState.Vector8.Y,  ServerState.Vector8.Z,  SourceMove.OutVector8.X,  SourceMove.OutVector8.Y,  SourceMove.OutVector8.Z)  return false; } } else return true;
  if (Vector9)  { if (ServerState.bReplicateVector9  && !ServerState.Vector9.Equals(SourceMove.OutVector9,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector9  (X: %f, Y: %f, Z: %f) != SourceMove.OutVector9  (X: %f, Y: %f, Z: %f)"), ServerState.Vector9.X,  ServerState.Vector9.Y,  ServerState.Vector9.Z,  SourceMove.OutVector9.X,  SourceMove.OutVector9.Y,  SourceMove.OutVector9.Z)  return false; } } else return true;
  if (Vector10) { if (ServerState.bReplicateVector10 && !ServerState.Vector10.Equals(SourceMove.OutVector10, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector10 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector10 (X: %f, Y: %f, Z: %f)"), ServerState.Vector10.X, ServerState.Vector10.Y, ServerState.Vector10.Z, SourceMove.OutVector10.X, SourceMove.OutVector10.Y, SourceMove.OutVector10.Z) return false; } } else return true;
  if (Vector11) { if (ServerState.bReplicateVector11 && !ServerState.Vector11.Equals(SourceMove.OutVector11, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector11 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector11 (X: %f, Y: %f, Z: %f)"), ServerState.Vector11.X, ServerState.Vector11.Y, ServerState.Vector11.Z, SourceMove.OutVector11.X, SourceMove.OutVector11.Y, SourceMove.OutVector11.Z) return false; } } else return true;
  if (Vector12) { if (ServerState.bReplicateVector12 && !ServerState.Vector12.Equals(SourceMove.OutVector12, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector12 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector12 (X: %f, Y: %f, Z: %f)"), ServerState.Vector12.X, ServerState.Vector12.Y, ServerState.Vector12.Z, SourceMove.OutVector12.X, SourceMove.OutVector12.Y, SourceMove.OutVector12.Z) return false; } } else return true;
  if (Vector13) { if (ServerState.bReplicateVector13 && !ServerState.Vector13.Equals(SourceMove.OutVector13, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector13 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector13 (X: %f, Y: %f, Z: %f)"), ServerState.Vector13.X, ServerState.Vector13.Y, ServerState.Vector13.Z, SourceMove.OutVector13.X, SourceMove.OutVector13.Y, SourceMove.OutVector13.Z) return false; } } else return true;
  if (Vector14) { if (ServerState.bReplicateVector14 && !ServerState.Vector14.Equals(SourceMove.OutVector14, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector14 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector14 (X: %f, Y: %f, Z: %f)"), ServerState.Vector14.X, ServerState.Vector14.Y, ServerState.Vector14.Z, SourceMove.OutVector14.X, SourceMove.OutVector14.Y, SourceMove.OutVector14.Z) return false; } } else return true;
  if (Vector15) { if (ServerState.bReplicateVector15 && !ServerState.Vector15.Equals(SourceMove.OutVector15, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector15 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector15 (X: %f, Y: %f, Z: %f)"), ServerState.Vector15.X, ServerState.Vector15.Y, ServerState.Vector15.Z, SourceMove.OutVector15.X, SourceMove.OutVector15.Y, SourceMove.OutVector15.Z) return false; } } else return true;
  if (Vector16) { if (ServerState.bReplicateVector16 && !ServerState.Vector16.Equals(SourceMove.OutVector16, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Vector16 (X: %f, Y: %f, Z: %f) != SourceMove.OutVector16 (X: %f, Y: %f, Z: %f)"), ServerState.Vector16.X, ServerState.Vector16.Y, ServerState.Vector16.Z, SourceMove.OutVector16.X, SourceMove.OutVector16.Y, SourceMove.OutVector16.Z) return false; } } else return true;
  return true;
}

bool UGenMovementReplicationComponent::Client_IsValidNormal(const FMove& SourceMove) const
{
  checkGMC(IsAutonomousProxy())
  const auto& ServerState = ServerState_AutonomousProxy();
  constexpr float COMPARE_TOLERANCE = 0.0001f; // @see SerializeNormalTypes
  if (Normal1)  { if (ServerState.bReplicateNormal1  && !ServerState.Normal1.Equals(SourceMove.OutNormal1,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal1  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal1  (X: %f, Y: %f, Z: %f)"), ServerState.Normal1.X,  ServerState.Normal1.Y,  ServerState.Normal1.Z,  SourceMove.OutNormal1.X,  SourceMove.OutNormal1.Y,  SourceMove.OutNormal1.Z)  return false; } } else return true;
  if (Normal2)  { if (ServerState.bReplicateNormal2  && !ServerState.Normal2.Equals(SourceMove.OutNormal2,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal2  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal2  (X: %f, Y: %f, Z: %f)"), ServerState.Normal2.X,  ServerState.Normal2.Y,  ServerState.Normal2.Z,  SourceMove.OutNormal2.X,  SourceMove.OutNormal2.Y,  SourceMove.OutNormal2.Z)  return false; } } else return true;
  if (Normal3)  { if (ServerState.bReplicateNormal3  && !ServerState.Normal3.Equals(SourceMove.OutNormal3,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal3  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal3  (X: %f, Y: %f, Z: %f)"), ServerState.Normal3.X,  ServerState.Normal3.Y,  ServerState.Normal3.Z,  SourceMove.OutNormal3.X,  SourceMove.OutNormal3.Y,  SourceMove.OutNormal3.Z)  return false; } } else return true;
  if (Normal4)  { if (ServerState.bReplicateNormal4  && !ServerState.Normal4.Equals(SourceMove.OutNormal4,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal4  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal4  (X: %f, Y: %f, Z: %f)"), ServerState.Normal4.X,  ServerState.Normal4.Y,  ServerState.Normal4.Z,  SourceMove.OutNormal4.X,  SourceMove.OutNormal4.Y,  SourceMove.OutNormal4.Z)  return false; } } else return true;
  if (Normal5)  { if (ServerState.bReplicateNormal5  && !ServerState.Normal5.Equals(SourceMove.OutNormal5,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal5  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal5  (X: %f, Y: %f, Z: %f)"), ServerState.Normal5.X,  ServerState.Normal5.Y,  ServerState.Normal5.Z,  SourceMove.OutNormal5.X,  SourceMove.OutNormal5.Y,  SourceMove.OutNormal5.Z)  return false; } } else return true;
  if (Normal6)  { if (ServerState.bReplicateNormal6  && !ServerState.Normal6.Equals(SourceMove.OutNormal6,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal6  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal6  (X: %f, Y: %f, Z: %f)"), ServerState.Normal6.X,  ServerState.Normal6.Y,  ServerState.Normal6.Z,  SourceMove.OutNormal6.X,  SourceMove.OutNormal6.Y,  SourceMove.OutNormal6.Z)  return false; } } else return true;
  if (Normal7)  { if (ServerState.bReplicateNormal7  && !ServerState.Normal7.Equals(SourceMove.OutNormal7,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal7  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal7  (X: %f, Y: %f, Z: %f)"), ServerState.Normal7.X,  ServerState.Normal7.Y,  ServerState.Normal7.Z,  SourceMove.OutNormal7.X,  SourceMove.OutNormal7.Y,  SourceMove.OutNormal7.Z)  return false; } } else return true;
  if (Normal8)  { if (ServerState.bReplicateNormal8  && !ServerState.Normal8.Equals(SourceMove.OutNormal8,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal8  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal8  (X: %f, Y: %f, Z: %f)"), ServerState.Normal8.X,  ServerState.Normal8.Y,  ServerState.Normal8.Z,  SourceMove.OutNormal8.X,  SourceMove.OutNormal8.Y,  SourceMove.OutNormal8.Z)  return false; } } else return true;
  if (Normal9)  { if (ServerState.bReplicateNormal9  && !ServerState.Normal9.Equals(SourceMove.OutNormal9,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal9  (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal9  (X: %f, Y: %f, Z: %f)"), ServerState.Normal9.X,  ServerState.Normal9.Y,  ServerState.Normal9.Z,  SourceMove.OutNormal9.X,  SourceMove.OutNormal9.Y,  SourceMove.OutNormal9.Z)  return false; } } else return true;
  if (Normal10) { if (ServerState.bReplicateNormal10 && !ServerState.Normal10.Equals(SourceMove.OutNormal10, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal10 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal10 (X: %f, Y: %f, Z: %f)"), ServerState.Normal10.X, ServerState.Normal10.Y, ServerState.Normal10.Z, SourceMove.OutNormal10.X, SourceMove.OutNormal10.Y, SourceMove.OutNormal10.Z) return false; } } else return true;
  if (Normal11) { if (ServerState.bReplicateNormal11 && !ServerState.Normal11.Equals(SourceMove.OutNormal11, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal11 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal11 (X: %f, Y: %f, Z: %f)"), ServerState.Normal11.X, ServerState.Normal11.Y, ServerState.Normal11.Z, SourceMove.OutNormal11.X, SourceMove.OutNormal11.Y, SourceMove.OutNormal11.Z) return false; } } else return true;
  if (Normal12) { if (ServerState.bReplicateNormal12 && !ServerState.Normal12.Equals(SourceMove.OutNormal12, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal12 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal12 (X: %f, Y: %f, Z: %f)"), ServerState.Normal12.X, ServerState.Normal12.Y, ServerState.Normal12.Z, SourceMove.OutNormal12.X, SourceMove.OutNormal12.Y, SourceMove.OutNormal12.Z) return false; } } else return true;
  if (Normal13) { if (ServerState.bReplicateNormal13 && !ServerState.Normal13.Equals(SourceMove.OutNormal13, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal13 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal13 (X: %f, Y: %f, Z: %f)"), ServerState.Normal13.X, ServerState.Normal13.Y, ServerState.Normal13.Z, SourceMove.OutNormal13.X, SourceMove.OutNormal13.Y, SourceMove.OutNormal13.Z) return false; } } else return true;
  if (Normal14) { if (ServerState.bReplicateNormal14 && !ServerState.Normal14.Equals(SourceMove.OutNormal14, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal14 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal14 (X: %f, Y: %f, Z: %f)"), ServerState.Normal14.X, ServerState.Normal14.Y, ServerState.Normal14.Z, SourceMove.OutNormal14.X, SourceMove.OutNormal14.Y, SourceMove.OutNormal14.Z) return false; } } else return true;
  if (Normal15) { if (ServerState.bReplicateNormal15 && !ServerState.Normal15.Equals(SourceMove.OutNormal15, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal15 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal15 (X: %f, Y: %f, Z: %f)"), ServerState.Normal15.X, ServerState.Normal15.Y, ServerState.Normal15.Z, SourceMove.OutNormal15.X, SourceMove.OutNormal15.Y, SourceMove.OutNormal15.Z) return false; } } else return true;
  if (Normal16) { if (ServerState.bReplicateNormal16 && !ServerState.Normal16.Equals(SourceMove.OutNormal16, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Normal16 (X: %f, Y: %f, Z: %f) != SourceMove.OutNormal16 (X: %f, Y: %f, Z: %f)"), ServerState.Normal16.X, ServerState.Normal16.Y, ServerState.Normal16.Z, SourceMove.OutNormal16.X, SourceMove.OutNormal16.Y, SourceMove.OutNormal16.Z) return false; } } else return true;
  return true;
}

bool UGenMovementReplicationComponent::Client_IsValidRotator(const FMove& SourceMove) const
{
  checkGMC(IsAutonomousProxy())
  const auto& ServerState = ServerState_AutonomousProxy();
  constexpr float COMPARE_TOLERANCE = 0.01f; // @see SerializeRotatorTypes
  if (Rotator1)  { if (ServerState.bReplicateRotator1  && !ServerState.Rotator1.Equals(SourceMove.OutRotator1,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator1  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator1  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator1.Roll,  ServerState.Rotator1.Pitch,  ServerState.Rotator1.Yaw,  SourceMove.OutRotator1.Roll,  SourceMove.OutRotator1.Pitch,  SourceMove.OutRotator1.Yaw)  return false; } } else return true;
  if (Rotator2)  { if (ServerState.bReplicateRotator2  && !ServerState.Rotator2.Equals(SourceMove.OutRotator2,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator2  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator2  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator2.Roll,  ServerState.Rotator2.Pitch,  ServerState.Rotator2.Yaw,  SourceMove.OutRotator2.Roll,  SourceMove.OutRotator2.Pitch,  SourceMove.OutRotator2.Yaw)  return false; } } else return true;
  if (Rotator3)  { if (ServerState.bReplicateRotator3  && !ServerState.Rotator3.Equals(SourceMove.OutRotator3,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator3  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator3  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator3.Roll,  ServerState.Rotator3.Pitch,  ServerState.Rotator3.Yaw,  SourceMove.OutRotator3.Roll,  SourceMove.OutRotator3.Pitch,  SourceMove.OutRotator3.Yaw)  return false; } } else return true;
  if (Rotator4)  { if (ServerState.bReplicateRotator4  && !ServerState.Rotator4.Equals(SourceMove.OutRotator4,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator4  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator4  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator4.Roll,  ServerState.Rotator4.Pitch,  ServerState.Rotator4.Yaw,  SourceMove.OutRotator4.Roll,  SourceMove.OutRotator4.Pitch,  SourceMove.OutRotator4.Yaw)  return false; } } else return true;
  if (Rotator5)  { if (ServerState.bReplicateRotator5  && !ServerState.Rotator5.Equals(SourceMove.OutRotator5,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator5  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator5  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator5.Roll,  ServerState.Rotator5.Pitch,  ServerState.Rotator5.Yaw,  SourceMove.OutRotator5.Roll,  SourceMove.OutRotator5.Pitch,  SourceMove.OutRotator5.Yaw)  return false; } } else return true;
  if (Rotator6)  { if (ServerState.bReplicateRotator6  && !ServerState.Rotator6.Equals(SourceMove.OutRotator6,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator6  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator6  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator6.Roll,  ServerState.Rotator6.Pitch,  ServerState.Rotator6.Yaw,  SourceMove.OutRotator6.Roll,  SourceMove.OutRotator6.Pitch,  SourceMove.OutRotator6.Yaw)  return false; } } else return true;
  if (Rotator7)  { if (ServerState.bReplicateRotator7  && !ServerState.Rotator7.Equals(SourceMove.OutRotator7,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator7  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator7  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator7.Roll,  ServerState.Rotator7.Pitch,  ServerState.Rotator7.Yaw,  SourceMove.OutRotator7.Roll,  SourceMove.OutRotator7.Pitch,  SourceMove.OutRotator7.Yaw)  return false; } } else return true;
  if (Rotator8)  { if (ServerState.bReplicateRotator8  && !ServerState.Rotator8.Equals(SourceMove.OutRotator8,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator8  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator8  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator8.Roll,  ServerState.Rotator8.Pitch,  ServerState.Rotator8.Yaw,  SourceMove.OutRotator8.Roll,  SourceMove.OutRotator8.Pitch,  SourceMove.OutRotator8.Yaw)  return false; } } else return true;
  if (Rotator9)  { if (ServerState.bReplicateRotator9  && !ServerState.Rotator9.Equals(SourceMove.OutRotator9,   COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator9  (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator9  (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator9.Roll,  ServerState.Rotator9.Pitch,  ServerState.Rotator9.Yaw,  SourceMove.OutRotator9.Roll,  SourceMove.OutRotator9.Pitch,  SourceMove.OutRotator9.Yaw)  return false; } } else return true;
  if (Rotator10) { if (ServerState.bReplicateRotator10 && !ServerState.Rotator10.Equals(SourceMove.OutRotator10, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator10 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator10 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator10.Roll, ServerState.Rotator10.Pitch, ServerState.Rotator10.Yaw, SourceMove.OutRotator10.Roll, SourceMove.OutRotator10.Pitch, SourceMove.OutRotator10.Yaw) return false; } } else return true;
  if (Rotator11) { if (ServerState.bReplicateRotator11 && !ServerState.Rotator11.Equals(SourceMove.OutRotator11, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator11 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator11 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator11.Roll, ServerState.Rotator11.Pitch, ServerState.Rotator11.Yaw, SourceMove.OutRotator11.Roll, SourceMove.OutRotator11.Pitch, SourceMove.OutRotator11.Yaw) return false; } } else return true;
  if (Rotator12) { if (ServerState.bReplicateRotator12 && !ServerState.Rotator12.Equals(SourceMove.OutRotator12, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator12 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator12 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator12.Roll, ServerState.Rotator12.Pitch, ServerState.Rotator12.Yaw, SourceMove.OutRotator12.Roll, SourceMove.OutRotator12.Pitch, SourceMove.OutRotator12.Yaw) return false; } } else return true;
  if (Rotator13) { if (ServerState.bReplicateRotator13 && !ServerState.Rotator13.Equals(SourceMove.OutRotator13, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator13 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator13 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator13.Roll, ServerState.Rotator13.Pitch, ServerState.Rotator13.Yaw, SourceMove.OutRotator13.Roll, SourceMove.OutRotator13.Pitch, SourceMove.OutRotator13.Yaw) return false; } } else return true;
  if (Rotator14) { if (ServerState.bReplicateRotator14 && !ServerState.Rotator14.Equals(SourceMove.OutRotator14, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator14 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator14 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator14.Roll, ServerState.Rotator14.Pitch, ServerState.Rotator14.Yaw, SourceMove.OutRotator14.Roll, SourceMove.OutRotator14.Pitch, SourceMove.OutRotator14.Yaw) return false; } } else return true;
  if (Rotator15) { if (ServerState.bReplicateRotator15 && !ServerState.Rotator15.Equals(SourceMove.OutRotator15, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator15 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator15 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator15.Roll, ServerState.Rotator15.Pitch, ServerState.Rotator15.Yaw, SourceMove.OutRotator15.Roll, SourceMove.OutRotator15.Pitch, SourceMove.OutRotator15.Yaw) return false; } } else return true;
  if (Rotator16) { if (ServerState.bReplicateRotator16 && !ServerState.Rotator16.Equals(SourceMove.OutRotator16, COMPARE_TOLERANCE)) { GMC_LOG(Verbose, TEXT("Bound data deviates: ServerState_AutonomousProxy.Rotator16 (Roll: %f, Pitch: %f, Yaw: %f) != SourceMove.OutRotator16 (Roll: %f, Pitch: %f, Yaw: %f)"), ServerState.Rotator16.Roll, ServerState.Rotator16.Pitch, ServerState.Rotator16.Yaw, SourceMove.OutRotator16.Roll, SourceMove.OutRotator16.Pitch, SourceMove.OutRotator16.Yaw) return false; } } else return true;
  return true;
}

FMove::FMove(
  float Timestamp,
  float DeltaTime,
  FVector InputVector,
  FRotator RotationInput,
  bool bInputFlag1,
  bool bInputFlag2,
  bool bInputFlag3,
  bool bInputFlag4,
  bool bInputFlag5,
  bool bInputFlag6,
  bool bInputFlag7,
  bool bInputFlag8,
  bool bInputFlag9,
  bool bInputFlag10,
  bool bInputFlag11,
  bool bInputFlag12,
  bool bInputFlag13,
  bool bInputFlag14,
  bool bInputFlag15,
  bool bInputFlag16,
  FVector InVelocity,
  FVector InLocation,
  FRotator InRotation,
  FRotator InControlRotation,
  EInputMode InInputMode
)
  : Timestamp(Timestamp),
    DeltaTime(DeltaTime),
    InputVector(InputVector),
    RotationInput(RotationInput),
    bInputFlag1(bInputFlag1),
    bInputFlag2(bInputFlag2),
    bInputFlag3(bInputFlag3),
    bInputFlag4(bInputFlag4),
    bInputFlag5(bInputFlag5),
    bInputFlag6(bInputFlag6),
    bInputFlag7(bInputFlag7),
    bInputFlag8(bInputFlag8),
    bInputFlag9(bInputFlag9),
    bInputFlag10(bInputFlag10),
    bInputFlag11(bInputFlag11),
    bInputFlag12(bInputFlag12),
    bInputFlag13(bInputFlag13),
    bInputFlag14(bInputFlag14),
    bInputFlag15(bInputFlag15),
    bInputFlag16(bInputFlag16),
    InVelocity(InVelocity),
    InLocation(InLocation),
    InRotation(InRotation),
    InControlRotation(InControlRotation),
    InInputMode(InInputMode)
{}

bool FMove::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
  bOutSuccess = true;
  Ar << Timestamp;
  bOutSuccess &= SerializeInputVector(Ar);
  bOutSuccess &= SerializeRotationInput(Ar);
  SerializeInputFlags(Ar);
  bOutSuccess &= SerializeOutVelocity(Ar);
  bOutSuccess &= SerializeOutLocation(Ar);
  SerializeOutRotation(Ar);
  SerializeOutControlRotation(Ar);
  UE_CLOG(!bOutSuccess, LogGMCReplication, Error, TEXT("FMove net serialization returned with bOutSuccess = false."))
  return true;
}

bool FMove::SerializeInputVector(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bOutSuccess = true;

  switch (uint8 B = 0; InputVectorQuantize)
  {
    case ESizeQuantization::Byte:
    {
      if (bArIsSaving)
      {
        if (bSerializeInputVectorX)
        {
          B = bHasNewInputVectorX;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 8>(InputVector.X, Ar);
          }
        }
        if (bSerializeInputVectorY)
        {
          B = bHasNewInputVectorY;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 8>(InputVector.Y, Ar);
          }
        }
        if (bSerializeInputVectorZ)
        {
          B = bHasNewInputVectorZ;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 8>(InputVector.Z, Ar);
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeInputVectorX)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 8>(InputVector.X, Ar);
            bHasNewInputVectorX = true;
          }
        }
        if (bSerializeInputVectorY)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 8>(InputVector.Y, Ar);
            bHasNewInputVectorY = true;
          }
        }
        if (bSerializeInputVectorZ)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 8>(InputVector.Z, Ar);
            bHasNewInputVectorZ = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::Short:
    {
      if (bArIsSaving)
      {
        if (bSerializeInputVectorX)
        {
          B = bHasNewInputVectorX;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 16>(InputVector.X, Ar);
          }
        }
        if (bSerializeInputVectorY)
        {
          B = bHasNewInputVectorY;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 16>(InputVector.Y, Ar);
          }
        }
        if (bSerializeInputVectorZ)
        {
          B = bHasNewInputVectorZ;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 16>(InputVector.Z, Ar);
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeInputVectorX)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 16>(InputVector.X, Ar);
            bHasNewInputVectorX = true;
          }
        }
        if (bSerializeInputVectorY)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 16>(InputVector.Y, Ar);
            bHasNewInputVectorY = true;
          }
        }
        if (bSerializeInputVectorZ)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_DIRECTION_INPUT, 16>(InputVector.Z, Ar);
            bHasNewInputVectorZ = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::None:
    {
      if (bSerializeInputVectorX)
      {
        if (bArIsSaving) B = bHasNewInputVectorX;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << InputVector.X;
          if (bArIsLoading) bHasNewInputVectorX = true;
        }
      }
      if (bSerializeInputVectorY)
      {
        if (bArIsSaving) B = bHasNewInputVectorY;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << InputVector.Y;
          if (bArIsLoading) bHasNewInputVectorY = true;
        }
      }
      if (bSerializeInputVectorZ)
      {
        if (bArIsSaving) B = bHasNewInputVectorZ;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << InputVector.Z;
          if (bArIsLoading) bHasNewInputVectorZ = true;
        }
      }
      break;
    }
    default: checkNoEntryGMC();
  }
  return bOutSuccess;
}

bool FMove::SerializeRotationInput(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bOutSuccess = true;

  switch (uint8 B = 0; RotationInputQuantize)
  {
    case ESizeQuantization::Byte:
    {
      if (bArIsSaving)
      {
        if (bSerializeRotationInputRoll)
        {
          B = bHasNewRotationInputRoll;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 8>(RotationInput.Roll, Ar);
          }
        }
        if (bSerializeRotationInputPitch)
        {
          B = bHasNewRotationInputPitch;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 8>(RotationInput.Pitch, Ar);
          }
        }
        if (bSerializeRotationInputYaw)
        {
          B = bHasNewRotationInputYaw;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 8>(RotationInput.Yaw, Ar);
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeRotationInputRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 8>(RotationInput.Roll, Ar);
            bHasNewRotationInputRoll = true;
          }
        }
        if (bSerializeRotationInputPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 8>(RotationInput.Pitch, Ar);
            bHasNewRotationInputPitch = true;
          }
        }
        if (bSerializeRotationInputYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 8>(RotationInput.Yaw, Ar);
            bHasNewRotationInputYaw = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::Short:
    {
      if (bArIsSaving)
      {
        if (bSerializeRotationInputRoll)
        {
          B = bHasNewRotationInputRoll;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 16>(RotationInput.Roll, Ar);
          }
        }
        if (bSerializeRotationInputPitch)
        {
          B = bHasNewRotationInputPitch;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 16>(RotationInput.Pitch, Ar);
          }
        }
        if (bSerializeRotationInputYaw)
        {
          B = bHasNewRotationInputYaw;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            bOutSuccess &= WriteFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 16>(RotationInput.Yaw, Ar);
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeRotationInputRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 16>(RotationInput.Roll, Ar);
            bHasNewRotationInputRoll = true;
          }
        }
        if (bSerializeRotationInputPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 16>(RotationInput.Pitch, Ar);
            bHasNewRotationInputPitch = true;
          }
        }
        if (bSerializeRotationInputYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            ReadFixedCompressedFloat<UGenMovementReplicationComponent::MAX_ROTATION_INPUT, 16>(RotationInput.Yaw, Ar);
            bHasNewRotationInputYaw = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::None:
    {
      if (bSerializeRotationInputRoll)
      {
        if (bArIsSaving) B = bHasNewRotationInputRoll;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << RotationInput.Roll;
          if (bArIsLoading) bHasNewRotationInputRoll = true;
        }
      }
      if (bSerializeRotationInputPitch)
      {
        if (bArIsSaving) B = bHasNewRotationInputPitch;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << RotationInput.Pitch;
          if (bArIsLoading) bHasNewRotationInputPitch = true;
        }
      }
      if (bSerializeRotationInputYaw)
      {
        if (bArIsSaving) B = bHasNewRotationInputYaw;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << RotationInput.Yaw;
          if (bArIsLoading) bHasNewRotationInputYaw = true;
        }
      }
      break;
    }
    default: checkNoEntryGMC();
  }
  return bOutSuccess;
}

void FMove::SerializeInputFlags(FArchive& Ar)
{
  checkGMC(!(NumSerializedInputFlags < 0 || NumSerializedInputFlags > 16))
  if (NumSerializedInputFlags == 0)
  {
    // Input flag serialization is disabled.
    return;
  }
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  uint16 Flags = 0;

  if (bArIsSaving)
  {
    Flags =
        (bInputFlag1  <<  0)
      | (bInputFlag2  <<  1)
      | (bInputFlag3  <<  2)
      | (bInputFlag4  <<  3)
      | (bInputFlag5  <<  4)
      | (bInputFlag6  <<  5)
      | (bInputFlag7  <<  6)
      | (bInputFlag8  <<  7)
      | (bInputFlag9  <<  8)
      | (bInputFlag10 <<  9)
      | (bInputFlag11 << 10)
      | (bInputFlag12 << 11)
      | (bInputFlag13 << 12)
      | (bInputFlag14 << 13)
      | (bInputFlag15 << 14)
      | (bInputFlag16 << 15);
  }
  Ar.SerializeBits(&Flags, NumSerializedInputFlags);
  if (bArIsLoading)
  {
    bInputFlag1  = (Flags & (1 << 0))  ? 1 : 0;
    bInputFlag2  = (Flags & (1 << 1))  ? 1 : 0;
    bInputFlag3  = (Flags & (1 << 2))  ? 1 : 0;
    bInputFlag4  = (Flags & (1 << 3))  ? 1 : 0;
    bInputFlag5  = (Flags & (1 << 4))  ? 1 : 0;
    bInputFlag6  = (Flags & (1 << 5))  ? 1 : 0;
    bInputFlag7  = (Flags & (1 << 6))  ? 1 : 0;
    bInputFlag8  = (Flags & (1 << 7))  ? 1 : 0;
    bInputFlag9  = (Flags & (1 << 8))  ? 1 : 0;
    bInputFlag10 = (Flags & (1 << 9))  ? 1 : 0;
    bInputFlag11 = (Flags & (1 << 10)) ? 1 : 0;
    bInputFlag12 = (Flags & (1 << 11)) ? 1 : 0;
    bInputFlag13 = (Flags & (1 << 12)) ? 1 : 0;
    bInputFlag14 = (Flags & (1 << 13)) ? 1 : 0;
    bInputFlag15 = (Flags & (1 << 14)) ? 1 : 0;
    bInputFlag16 = (Flags & (1 << 15)) ? 1 : 0;
  }
}

bool FMove::SerializeOutVelocity(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bOutSuccess = true;

  if (bSerializeOutVelocity)
  {
    uint8 B = 0;
    if (bArIsSaving) B = bHasNewOutVelocity;
    Ar.Serialize(&B, 1);
    if (B)
    {
      switch (OutVelocityQuantize)
      {
        case EDecimalQuantization::RoundWholeNumber:
          bOutSuccess &= SerializePackedVector<1, 24>(OutVelocity, Ar);
          break;
        case EDecimalQuantization::RoundOneDecimal:
          bOutSuccess &= SerializePackedVector<10, 27>(OutVelocity, Ar);
          break;
        case EDecimalQuantization::RoundTwoDecimals:
          bOutSuccess &= SerializePackedVector<100, 30>(OutVelocity, Ar);
          break;
        case EDecimalQuantization::None:
          Ar << OutVelocity;
          break;
        default: checkNoEntryGMC();
      }
      if (bArIsLoading) bHasNewOutVelocity = true;
    }
    else
    {
      if (bArIsLoading) bHasNewOutVelocity = false;
    }
  }
  return bOutSuccess;
}

bool FMove::SerializeOutLocation(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bOutSuccess = true;

  if (bSerializeOutLocation)
  {
    uint8 B = 0;
    if (bArIsSaving) B = bHasNewOutLocation;
    Ar.Serialize(&B, 1);
    if (B)
    {
      switch (OutLocationQuantize)
      {
        case EDecimalQuantization::RoundWholeNumber:
          bOutSuccess &= SerializePackedVector<1, 24>(OutLocation, Ar);
          break;
        case EDecimalQuantization::RoundOneDecimal:
          bOutSuccess &= SerializePackedVector<10, 27>(OutLocation, Ar);
          break;
        case EDecimalQuantization::RoundTwoDecimals:
          bOutSuccess &= SerializePackedVector<100, 30>(OutLocation, Ar);
          break;
        case EDecimalQuantization::None:
          Ar << OutLocation;
          break;
        default: checkNoEntryGMC();
      }
      if (bArIsLoading) bHasNewOutLocation = true;
    }
    else
    {
      if (bArIsLoading) bHasNewOutLocation = false;
    }
  }
  return bOutSuccess;
}

void FMove::SerializeOutRotation(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();

  switch (uint8 B = 0; OutRotationQuantize)
  {
    case ESizeQuantization::Byte:
    {
      // 1 byte components.
      uint8 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        if (bSerializeOutRotationRoll)
        {
          B = bHasNewOutRotationRoll;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToByte(OutRotation.Roll);
            Ar << Roll;
          }
        }
        if (bSerializeOutRotationPitch)
        {
          B = bHasNewOutRotationPitch;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToByte(OutRotation.Pitch);
            Ar << Pitch;
          }
        }
        if (bSerializeOutRotationYaw)
        {
          B = bHasNewOutRotationYaw;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToByte(OutRotation.Yaw);
            Ar << Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeOutRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            OutRotation.Roll = FRotator::DecompressAxisFromByte(Roll);
            bHasNewOutRotationRoll = true;
          }
        }
        if (bSerializeOutRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            OutRotation.Pitch = FRotator::DecompressAxisFromByte(Pitch);
            bHasNewOutRotationPitch = true;
          }
        }
        if (bSerializeOutRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            OutRotation.Yaw = FRotator::DecompressAxisFromByte(Yaw);
            bHasNewOutRotationYaw = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::Short:
    {
      // 2 byte components.
      uint16 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        if (bSerializeOutRotationRoll)
        {
          B = bHasNewOutRotationRoll;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToShort(OutRotation.Roll);
            Ar << Roll;
          }
        }
        if (bSerializeOutRotationPitch)
        {
          B = bHasNewOutRotationPitch;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToShort(OutRotation.Pitch);
            Ar << Pitch;
          }
        }
        if (bSerializeOutRotationYaw)
        {
          B = bHasNewOutRotationYaw;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToShort(OutRotation.Yaw);
            Ar << Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeOutRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            OutRotation.Roll = FRotator::DecompressAxisFromShort(Roll);
            bHasNewOutRotationRoll = true;
          }
        }
        if (bSerializeOutRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            OutRotation.Pitch = FRotator::DecompressAxisFromShort(Pitch);
            bHasNewOutRotationPitch = true;
          }
        }
        if (bSerializeOutRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            OutRotation.Yaw = FRotator::DecompressAxisFromShort(Yaw);
            bHasNewOutRotationYaw = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::None:
    {
      if (bSerializeOutRotationRoll)
      {
        if (bArIsSaving) B = bHasNewOutRotationRoll;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << OutRotation.Roll;
          if (bArIsLoading) bHasNewOutRotationRoll = true;
        }
      }
      if (bSerializeOutRotationPitch)
      {
        if (bArIsSaving) B = bHasNewOutRotationPitch;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << OutRotation.Pitch;
          if (bArIsLoading) bHasNewOutRotationPitch = true;
        }
      }
      if (bSerializeOutRotationYaw)
      {
        if (bArIsSaving) B = bHasNewOutRotationYaw;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << OutRotation.Yaw;
          if (bArIsLoading) bHasNewOutRotationYaw = true;
        }
      }
      break;
    }
    default: checkNoEntryGMC();
  }
}

void FMove::SerializeOutControlRotation(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();

  switch (uint8 B = 0; OutControlRotationQuantize)
  {
    case ESizeQuantization::Byte:
    {
      // 1 byte components.
      uint8 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        if (bSerializeOutControlRotationRoll)
        {
          B = bHasNewOutControlRotationRoll;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToByte(OutControlRotation.Roll);
            Ar << Roll;
          }
        }
        if (bSerializeOutControlRotationPitch)
        {
          B = bHasNewOutControlRotationPitch;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToByte(OutControlRotation.Pitch);
            Ar << Pitch;
          }
        }
        if (bSerializeOutControlRotationYaw)
        {
          B = bHasNewOutControlRotationYaw;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToByte(OutControlRotation.Yaw);
            Ar << Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeOutControlRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            OutControlRotation.Roll = FRotator::DecompressAxisFromByte(Roll);
            bHasNewOutControlRotationRoll = true;
          }
        }
        if (bSerializeOutControlRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            OutControlRotation.Pitch = FRotator::DecompressAxisFromByte(Pitch);
            bHasNewOutControlRotationPitch = true;
          }
        }
        if (bSerializeOutControlRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            OutControlRotation.Yaw = FRotator::DecompressAxisFromByte(Yaw);
            bHasNewOutControlRotationYaw = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::Short:
    {
      // 2 byte components.
      uint16 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        if (bSerializeOutControlRotationRoll)
        {
          B = bHasNewOutControlRotationRoll;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToShort(OutControlRotation.Roll);
            Ar << Roll;
          }
        }
        if (bSerializeOutControlRotationPitch)
        {
          B = bHasNewOutControlRotationPitch;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToShort(OutControlRotation.Pitch);
            Ar << Pitch;
          }
        }
        if (bSerializeOutControlRotationYaw)
        {
          B = bHasNewOutControlRotationYaw;
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToShort(OutControlRotation.Yaw);
            Ar << Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeOutControlRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            OutControlRotation.Roll = FRotator::DecompressAxisFromShort(Roll);
            bHasNewOutControlRotationRoll = true;
          }
        }
        if (bSerializeOutControlRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            OutControlRotation.Pitch = FRotator::DecompressAxisFromShort(Pitch);
            bHasNewOutControlRotationPitch = true;
          }
        }
        if (bSerializeOutControlRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            OutControlRotation.Yaw = FRotator::DecompressAxisFromShort(Yaw);
            bHasNewOutControlRotationYaw = true;
          }
        }
      }
      break;
    }
    case ESizeQuantization::None:
    {
      if (bSerializeOutRotationRoll)
      {
        if (bArIsSaving) B = bHasNewOutControlRotationRoll;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << OutControlRotation.Roll;
          if (bArIsLoading) bHasNewOutControlRotationRoll = true;
        }
      }
      if (bSerializeOutControlRotationPitch)
      {
        if (bArIsSaving) B = bHasNewOutControlRotationPitch;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << OutControlRotation.Pitch;
          if (bArIsLoading) bHasNewOutControlRotationPitch = true;
        }
      }
      if (bSerializeOutControlRotationYaw)
      {
        if (bArIsSaving) B = bHasNewOutControlRotationYaw;
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << OutControlRotation.Yaw;
          if (bArIsLoading) bHasNewOutControlRotationYaw = true;
        }
      }
      break;
    }
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeInputVector()
{
  static_assert(UGenMovementReplicationComponent::MAX_DIRECTION_INPUT == 1, "Only applicable if all absolute component values are <= 1.");
  switch (InputVectorQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 4 decimal places of precision.
      if (bSerializeInputVectorX && UGenMovementReplicationComponent::Rep_IsValid(InputVector.X))
      {
        InputVector.X = FMath::RoundToFloat(InputVector.X * 10000.f) / 10000.f;
      }
      if (bSerializeInputVectorY && UGenMovementReplicationComponent::Rep_IsValid(InputVector.Y))
      {
        InputVector.Y = FMath::RoundToFloat(InputVector.Y * 10000.f) / 10000.f;
      }
      if (bSerializeInputVectorZ && UGenMovementReplicationComponent::Rep_IsValid(InputVector.Z))
      {
        InputVector.Z = FMath::RoundToFloat(InputVector.Z * 10000.f) / 10000.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // 2 decimal places of precision.
      // @attention Byte compression is inaccurate (even when rounding to 1 decimal place), don't use.
      if (bSerializeInputVectorX && UGenMovementReplicationComponent::Rep_IsValid(InputVector.X))
      {
        InputVector.X = FMath::RoundToFloat(InputVector.X * 100.f) / 100.f;
      }
      if (bSerializeInputVectorY && UGenMovementReplicationComponent::Rep_IsValid(InputVector.Y))
      {
        InputVector.Y = FMath::RoundToFloat(InputVector.Y * 100.f) / 100.f;
      }
      if (bSerializeInputVectorZ && UGenMovementReplicationComponent::Rep_IsValid(InputVector.Z))
      {
        InputVector.Z = FMath::RoundToFloat(InputVector.Z * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeRotationInput()
{
  static_assert(UGenMovementReplicationComponent::MAX_ROTATION_INPUT == 200);
  switch (RotationInputQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (bSerializeRotationInputRoll && UGenMovementReplicationComponent::Rep_IsValid(RotationInput.Roll))
      {
        RotationInput.Roll = FMath::RoundToFloat(RotationInput.Roll * 100.f) / 100.f;
      }
      if (bSerializeRotationInputPitch && UGenMovementReplicationComponent::Rep_IsValid(RotationInput.Pitch))
      {
        RotationInput.Pitch = FMath::RoundToFloat(RotationInput.Pitch * 100.f) / 100.f;
      }
      if (bSerializeRotationInputYaw && UGenMovementReplicationComponent::Rep_IsValid(RotationInput.Yaw))
      {
        RotationInput.Yaw = FMath::RoundToFloat(RotationInput.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      // @attention Byte compression is inaccurate, don't use.
      if (bSerializeRotationInputRoll && UGenMovementReplicationComponent::Rep_IsValid(RotationInput.Roll))
      {
        RotationInput.Roll = FMath::RoundToFloat(RotationInput.Roll);
      }
      if (bSerializeRotationInputPitch && UGenMovementReplicationComponent::Rep_IsValid(RotationInput.Pitch))
      {
        RotationInput.Pitch = FMath::RoundToFloat(RotationInput.Pitch);
      }
      if (bSerializeRotationInputYaw && UGenMovementReplicationComponent::Rep_IsValid(RotationInput.Yaw))
      {
        RotationInput.Yaw = FMath::RoundToFloat(RotationInput.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeInLocation()
{
  // @attention For simplicity, we just use the same quantization level here that we usually use for the out-value when not replicating
  // client-authoritatively.
  switch (OutLocationQuantize)
  {
    case EDecimalQuantization::RoundTwoDecimals:
    {
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.X))
      {
        InLocation.X = FMath::RoundToFloat(InLocation.X * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.Y))
      {
        InLocation.Y = FMath::RoundToFloat(InLocation.Y * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.Z))
      {
        InLocation.Z = FMath::RoundToFloat(InLocation.Z * 100.f) / 100.f;
      }
      return;
    }
    case EDecimalQuantization::RoundOneDecimal:
    {
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.X))
      {
        InLocation.X = FMath::RoundToFloat(InLocation.X * 10.f) / 10.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.Y))
      {
        InLocation.Y = FMath::RoundToFloat(InLocation.Y * 10.f) / 10.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.Z))
      {
        InLocation.Z = FMath::RoundToFloat(InLocation.Z * 10.f) / 10.f;
      }
      return;
    }
    case EDecimalQuantization::RoundWholeNumber:
    {
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.X))
      {
        InLocation.X = FMath::RoundToFloat(InLocation.X);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.Y))
      {
        InLocation.Y = FMath::RoundToFloat(InLocation.Y);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InLocation.Z))
      {
        InLocation.Z = FMath::RoundToFloat(InLocation.Z);
      }
      return;
    }
    case EDecimalQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeInRotation()
{
  // @attention For simplicity, we just use the same quantization level here that we usually use for the out-value when not replicating
  // client-authoritatively.
  switch (OutRotationQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (UGenMovementReplicationComponent::Rep_IsValid(InRotation.Roll))
      {
        InRotation.Roll = FMath::RoundToFloat(InRotation.Roll * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InRotation.Pitch))
      {
        InRotation.Pitch = FMath::RoundToFloat(InRotation.Pitch * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InRotation.Yaw))
      {
        InRotation.Yaw = FMath::RoundToFloat(InRotation.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      // @attention Byte compression is inaccurate, only use with client-authoritative replication (bUseClientRotation = true).
      if (UGenMovementReplicationComponent::Rep_IsValid(InRotation.Roll))
      {
        InRotation.Roll = FMath::RoundToFloat(InRotation.Roll);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InRotation.Pitch))
      {
        InRotation.Pitch = FMath::RoundToFloat(InRotation.Pitch);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InRotation.Yaw))
      {
        InRotation.Yaw = FMath::RoundToFloat(InRotation.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeInControlRotation()
{
  // @attention For simplicity, we just use the same quantization level here that we usually use for the out-value when not replicating
  // client-authoritatively.
  switch (OutControlRotationQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (UGenMovementReplicationComponent::Rep_IsValid(InControlRotation.Roll))
      {
        InControlRotation.Roll = FMath::RoundToFloat(InControlRotation.Roll * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InControlRotation.Pitch))
      {
        InControlRotation.Pitch = FMath::RoundToFloat(InControlRotation.Pitch * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InControlRotation.Yaw))
      {
        InControlRotation.Yaw = FMath::RoundToFloat(InControlRotation.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      // @attention Byte compression is inaccurate, only use with client-authoritative replication (bUseClientControlRotation = true).
      if (UGenMovementReplicationComponent::Rep_IsValid(InControlRotation.Roll))
      {
        InControlRotation.Roll = FMath::RoundToFloat(InControlRotation.Roll);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InControlRotation.Pitch))
      {
        InControlRotation.Pitch = FMath::RoundToFloat(InControlRotation.Pitch);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(InControlRotation.Yaw))
      {
        InControlRotation.Yaw = FMath::RoundToFloat(InControlRotation.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeOutLocation()
{
  switch (OutLocationQuantize)
  {
    case EDecimalQuantization::RoundTwoDecimals:
    {
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.X))
      {
        OutLocation.X = FMath::RoundToFloat(OutLocation.X * 100.f) / 100.f;
      }
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.Y))
      {
        OutLocation.Y = FMath::RoundToFloat(OutLocation.Y * 100.f) / 100.f;
      }
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.Z))
      {
        OutLocation.Z = FMath::RoundToFloat(OutLocation.Z * 100.f) / 100.f;
      }
      return;
    }
    case EDecimalQuantization::RoundOneDecimal:
    {
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.X))
      {
        OutLocation.X = FMath::RoundToFloat(OutLocation.X * 10.f) / 10.f;
      }
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.Y))
      {
        OutLocation.Y = FMath::RoundToFloat(OutLocation.Y * 10.f) / 10.f;
      }
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.Z))
      {
        OutLocation.Z = FMath::RoundToFloat(OutLocation.Z * 10.f) / 10.f;
      }
      return;
    }
    case EDecimalQuantization::RoundWholeNumber:
    {
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.X))
      {
        OutLocation.X = FMath::RoundToFloat(OutLocation.X);
      }
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.Y))
      {
        OutLocation.Y = FMath::RoundToFloat(OutLocation.Y);
      }
      if (bSerializeOutLocation && UGenMovementReplicationComponent::Rep_IsValid(OutLocation.Z))
      {
        OutLocation.Z = FMath::RoundToFloat(OutLocation.Z);
      }
      return;
    }
    case EDecimalQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeOutRotation()
{
  switch (OutRotationQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (bSerializeOutRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(OutRotation.Roll))
      {
        OutRotation.Roll = FMath::RoundToFloat(OutRotation.Roll * 100.f) / 100.f;
      }
      if (bSerializeOutRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(OutRotation.Pitch))
      {
        OutRotation.Pitch = FMath::RoundToFloat(OutRotation.Pitch * 100.f) / 100.f;
      }
      if (bSerializeOutRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(OutRotation.Yaw))
      {
        OutRotation.Yaw = FMath::RoundToFloat(OutRotation.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      // @attention Byte compression is inaccurate, only use with client-authoritative replication (bUseClientRotation = true).
      if (bSerializeOutRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(OutRotation.Roll))
      {
        OutRotation.Roll = FMath::RoundToFloat(OutRotation.Roll);
      }
      if (bSerializeOutRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(OutRotation.Pitch))
      {
        OutRotation.Pitch = FMath::RoundToFloat(OutRotation.Pitch);
      }
      if (bSerializeOutRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(OutRotation.Yaw))
      {
        OutRotation.Yaw = FMath::RoundToFloat(OutRotation.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeOutControlRotation()
{
  switch (OutControlRotationQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (bSerializeOutControlRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(OutControlRotation.Roll))
      {
        OutControlRotation.Roll = FMath::RoundToFloat(OutControlRotation.Roll * 100.f) / 100.f;
      }
      if (bSerializeOutControlRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(OutControlRotation.Pitch))
      {
        OutControlRotation.Pitch = FMath::RoundToFloat(OutControlRotation.Pitch * 100.f) / 100.f;
      }
      if (bSerializeOutControlRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(OutControlRotation.Yaw))
      {
        OutControlRotation.Yaw = FMath::RoundToFloat(OutControlRotation.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      // @attention Byte compression is inaccurate, only use with client-authoritative replication (bUseClientControlRotation = true).
      if (bSerializeOutControlRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(OutControlRotation.Roll))
      {
        OutControlRotation.Roll = FMath::RoundToFloat(OutControlRotation.Roll);
      }
      if (bSerializeOutControlRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(OutControlRotation.Pitch))
      {
        OutControlRotation.Pitch = FMath::RoundToFloat(OutControlRotation.Pitch);
      }
      if (bSerializeOutControlRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(OutControlRotation.Yaw))
      {
        OutControlRotation.Yaw = FMath::RoundToFloat(OutControlRotation.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FMove::QuantizeOutVelocity()
{
  // @attention Unlike the other move properties, the velocity will always be quantized upon calling this function (even if it is not
  // replicated). To ensure that the server ends up with the same velocity as the client, it is imperative that the velocity quantization
  // level of the client moves is the same as that of the autonomous proxy server state.
  const float StartVelocityX = OutVelocity.X;
  const float StartVelocityY = OutVelocity.Y;
  const float StartVelocityZ = OutVelocity.Z;
  switch (OutVelocityQuantize)
  {
    case EDecimalQuantization::RoundTwoDecimals:
    {
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.X))
      {
        OutVelocity.X = FMath::RoundToFloat(OutVelocity.X * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.Y))
      {
        OutVelocity.Y = FMath::RoundToFloat(OutVelocity.Y * 100.f) / 100.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.Z))
      {
        OutVelocity.Z = FMath::RoundToFloat(OutVelocity.Z * 100.f) / 100.f;
      }
      break;
    }
    case EDecimalQuantization::RoundOneDecimal:
    {
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.X))
      {
        OutVelocity.X = FMath::RoundToFloat(OutVelocity.X * 10.f) / 10.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.Y))
      {
        OutVelocity.Y = FMath::RoundToFloat(OutVelocity.Y * 10.f) / 10.f;
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.Z))
      {
        OutVelocity.Z = FMath::RoundToFloat(OutVelocity.Z * 10.f) / 10.f;
      }
      break;
    }
    case EDecimalQuantization::RoundWholeNumber:
    {
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.X))
      {
        OutVelocity.X = FMath::RoundToFloat(OutVelocity.X);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.Y))
      {
        OutVelocity.Y = FMath::RoundToFloat(OutVelocity.Y);
      }
      if (UGenMovementReplicationComponent::Rep_IsValid(OutVelocity.Z))
      {
        OutVelocity.Z = FMath::RoundToFloat(OutVelocity.Z);
      }
      break;
    }
    case EDecimalQuantization::None: break;
    default: checkNoEntryGMC();
  }

  // The quantization level may be too high for how the velocity is calculated. E.g. if we round to 1 decimal place and a velocity vector
  // component is 0.04 it will be rounded to 0. If this happens when the pawn tries to accelerate from a resting position, no movement would
  // ever occur and you would have to lower the quantization level of the velocity (more likely if you are using very low acceleration
  // values). Keep in mind that this is strongly affected by the delta time so always test your movement with very high frame rates as well.
  UE_CLOG(
    StartVelocityX != 0.f && OutVelocity.X == 0.f,
    LogGMCReplication,
    VeryVerbose,
    TEXT("Client velocity component (OutVelocity.X = %f) was quantized to zero (quantization level is %d)."),
    StartVelocityX,
    (uint8)OutVelocityQuantize
  )
  UE_CLOG(
    StartVelocityY != 0.f && OutVelocity.Y == 0.f,
    LogGMCReplication,
    VeryVerbose,
    TEXT("Client velocity component (OutVelocity.Y = %f) was quantized to zero (quantization level is %d)."),
    StartVelocityY,
    (uint8)OutVelocityQuantize
  );
  UE_CLOG(
    StartVelocityZ != 0.f && OutVelocity.Z == 0.f,
    LogGMCReplication,
    VeryVerbose,
    TEXT("Client velocity component (OutVelocity.Z = %f) was quantized to zero (quantization level is %d)."),
    StartVelocityZ,
    (uint8)OutVelocityQuantize
  );
}

bool FState::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
  checkGMC(RecipientRole == ROLE_AutonomousProxy || RecipientRole == ROLE_SimulatedProxy)

  if (Ar.IsSaving())
  {
    // Server only logic for managing the serialization map.
    const auto TargetConnection = Cast<UPackageMapClient>(Map)->GetConnection()->OwningActor;
    CurrentTargetConnection = Cast<APlayerController>(TargetConnection);
    checkGMC(CurrentTargetConnection)
    checkGMC(LastSerialized.Contains(CurrentTargetConnection))
    if (Owner && RecipientRole == ROLE_SimulatedProxy)
    {
      const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(Owner->GetMovementComponent());
      check(ReplicationComponent)
      // This state will be replicated to a simulated proxy, set the appropriate flag within the state queue of the owning pawn.
      ReplicationComponent->Server_SetReplicationFlag(CurrentTargetConnection);
    }
  }

  // (De)serialization of replication data.
  bOutSuccess = true;
  if (bSerializeTimestamp)
  {
    Ar << Timestamp;
    Ar.SerializeBits(&bIsUsingServerAuthPhysicsReplication, 1);
    if (bIsUsingServerAuthPhysicsReplication && RecipientRole == ROLE_AutonomousProxy)
    {
      Ar << ServerAuthPhysicsAckTimestamp;
    }
  }
  // Data that the client does not send to the server is always serialized for the autonomous proxy server state as well, because the server
  // cannot verify them. The client checks them locally from the replicated values every time a replication update is received and replays
  // if necessary.
  SerializeMoveValidation(Ar);
  bOutSuccess &= SerializeVelocity(Ar);
  SerializeInputMode(Ar);
  SerializeBoundData(Ar);
  if (bContainsFullRepBatch)
  {
    // "bContainsFullRepBatch" may be true or false for the autonomous proxy server state, but will always be true for the simulated proxy.
    bOutSuccess &= SerializeLocation(Ar);
    SerializeRotation(Ar);
    SerializeControlRotation(Ar);
    // The input flags will never be serialized for the autonomous proxy server state.
    SerializeInputFlags(Ar);
  }
  else if (Ar.IsLoading())
  {
    // If the client move was valid, the location, rotation and control rotation won't be deserialized, meaning the read-new flags won't be
    // updated. We need to ensure they are all set to false in this case so the unpacking of client moves still works correctly and the last
    // serialized server state is saved with the correct values.
    // @attention This branch should only ever be entered when deserializing for the autonomous proxy.
    checkGMC(RecipientRole == ROLE_AutonomousProxy)
    bReadNewLocation = false;
    bReadNewRotationRoll = false;
    bReadNewRotationPitch = false;
    bReadNewRotationYaw = false;
    bReadNewControlRotationRoll = false;
    bReadNewControlRotationPitch = false;
    bReadNewControlRotationYaw = false;
  }

  if (Ar.IsSaving())
  {
    // Server only: Reset the flag to force full serialization, this should have happened within this call.
    LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate = false;
  }

  UE_CLOG(!bOutSuccess, LogGMCReplication, Error, TEXT("FState net serialization returned with bOutSuccess = false."))
  return true;
}

bool FState::SerializeLocation(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  const float CompareTolerance = UGenMovementReplicationComponent::GetCompareTolerance(LocationQuantize);
  bool bOutSuccess = true;
  checkCodeGMC(if (bArIsSaving) check(LastSerialized.Contains(CurrentTargetConnection)))

  if (bSerializeLocation)
  {
    uint8 B = 0;
    if (bArIsSaving)
    {
      const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
      B = bForceFullSerialization ? 1 : !Location.Equals(LastSerialized[CurrentTargetConnection].Location, CompareTolerance);
    }
    Ar.Serialize(&B, 1);
    if (B)
    {
      switch (LocationQuantize)
      {
        case EDecimalQuantization::RoundWholeNumber:
          bOutSuccess &= SerializePackedVector<1, 24>(Location, Ar);
          break;
        case EDecimalQuantization::RoundOneDecimal:
          bOutSuccess &= SerializePackedVector<10, 27>(Location, Ar);
          break;
        case EDecimalQuantization::RoundTwoDecimals:
          bOutSuccess &= SerializePackedVector<100, 30>(Location, Ar);
          break;
        case EDecimalQuantization::None:
          Ar << Location;
          break;
        default: checkNoEntryGMC();
      }
      if (bArIsSaving) LastSerialized[CurrentTargetConnection].Location = Location;
      if (bArIsLoading) bReadNewLocation = true;
    }
    else
    {
      if (bArIsLoading) bReadNewLocation = false;
    }
  }
  return bOutSuccess;
}

bool FState::SerializeVelocity(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  const float CompareTolerance = UGenMovementReplicationComponent::GetCompareTolerance(VelocityQuantize);
  bool bOutSuccess = true;
  checkCodeGMC(if (bArIsSaving) check(LastSerialized.Contains(CurrentTargetConnection)))

  if (bSerializeVelocity)
  {
    uint8 B = 0;
    if (bArIsSaving)
    {
      const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
      B = bForceFullSerialization ? 1 : !Velocity.Equals(LastSerialized[CurrentTargetConnection].Velocity, CompareTolerance);
    }
    Ar.Serialize(&B, 1);
    if (B)
    {
      switch (VelocityQuantize)
      {
        case EDecimalQuantization::RoundWholeNumber:
          bOutSuccess &= SerializePackedVector<1, 24>(Velocity, Ar);
          break;
        case EDecimalQuantization::RoundOneDecimal:
          bOutSuccess &= SerializePackedVector<10, 27>(Velocity, Ar);
          break;
        case EDecimalQuantization::RoundTwoDecimals:
          bOutSuccess &= SerializePackedVector<100, 30>(Velocity, Ar);
          break;
        case EDecimalQuantization::None:
          Ar << Velocity;
          break;
        default: checkNoEntryGMC();
      }
      if (bArIsSaving) LastSerialized[CurrentTargetConnection].Velocity = Velocity;
      if (bArIsLoading) bReadNewVelocity = true;
    }
    else
    {
      if (bArIsLoading) bReadNewVelocity = false;
    }
  }
  return bOutSuccess;
}

void FState::SerializeRotation(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  const float CompareTolerance = UGenMovementReplicationComponent::GetCompareToleranceRotator(RotationQuantize);
  checkCodeGMC(if (bArIsSaving) check(LastSerialized.Contains(CurrentTargetConnection)))

  switch (uint8 B = 0; RotationQuantize)
  {
    case ESizeQuantization::Byte:
    {
      // 1 byte components.
      uint8 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
        if (bSerializeRotationRoll)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Roll, LastSerialized[CurrentTargetConnection].RotationRoll, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToByte(Rotation.Roll);
            Ar << Roll;
            LastSerialized[CurrentTargetConnection].RotationRoll = Rotation.Roll;
          }
        }
        if (bSerializeRotationPitch)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Pitch, LastSerialized[CurrentTargetConnection].RotationPitch, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToByte(Rotation.Pitch);
            Ar << Pitch;
            LastSerialized[CurrentTargetConnection].RotationPitch = Rotation.Pitch;
          }
        }
        if (bSerializeRotationYaw)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Yaw, LastSerialized[CurrentTargetConnection].RotationYaw, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToByte(Rotation.Yaw);
            Ar << Yaw;
            LastSerialized[CurrentTargetConnection].RotationYaw = Rotation.Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            Rotation.Roll = FRotator::DecompressAxisFromByte(Roll);
            bReadNewRotationRoll = true;
          }
          else
          {
            bReadNewRotationRoll = false;
          }
        }
        if (bSerializeRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            Rotation.Pitch = FRotator::DecompressAxisFromByte(Pitch);
            bReadNewRotationPitch = true;
          }
          else
          {
            bReadNewRotationPitch = false;
          }
        }
        if (bSerializeRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            Rotation.Yaw = FRotator::DecompressAxisFromByte(Yaw);
            bReadNewRotationYaw = true;
          }
          else
          {
            bReadNewRotationYaw = false;
          }
        }
      }
      break;
    }
    case ESizeQuantization::Short:
    {
      // 2 byte components.
      uint16 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
        if (bSerializeRotationRoll)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Roll, LastSerialized[CurrentTargetConnection].RotationRoll, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToShort(Rotation.Roll);
            Ar << Roll;
            LastSerialized[CurrentTargetConnection].RotationRoll = Rotation.Roll;
          }
        }
        if (bSerializeRotationPitch)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Pitch, LastSerialized[CurrentTargetConnection].RotationPitch, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToShort(Rotation.Pitch);
            Ar << Pitch;
            LastSerialized[CurrentTargetConnection].RotationPitch = Rotation.Pitch;
          }
        }
        if (bSerializeRotationYaw)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Yaw, LastSerialized[CurrentTargetConnection].RotationYaw, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToShort(Rotation.Yaw);
            Ar << Yaw;
            LastSerialized[CurrentTargetConnection].RotationYaw = Rotation.Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            Rotation.Roll = FRotator::DecompressAxisFromShort(Roll);
            bReadNewRotationRoll = true;
          }
          else
          {
            bReadNewRotationRoll = false;
          }
        }
        if (bSerializeRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            Rotation.Pitch = FRotator::DecompressAxisFromShort(Pitch);
            bReadNewRotationPitch = true;
          }
          else
          {
            bReadNewRotationPitch = false;
          }
        }
        if (bSerializeRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            Rotation.Yaw = FRotator::DecompressAxisFromShort(Yaw);
            bReadNewRotationYaw = true;
          }
          else
          {
            bReadNewRotationYaw = false;
          }
        }
      }
      break;
    }
    case ESizeQuantization::None:
    {
      if (bSerializeRotationRoll)
      {
        if (bArIsSaving)
        {
          const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Roll, LastSerialized[CurrentTargetConnection].RotationRoll, CompareTolerance);
        }
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << Rotation.Roll;
          if (bArIsSaving) LastSerialized[CurrentTargetConnection].RotationRoll = Rotation.Roll;
          if (bArIsLoading) bReadNewRotationRoll = true;
        }
        else
        {
          if (bArIsLoading) bReadNewRotationRoll = false;
        }
      }
      if (bSerializeRotationPitch)
      {
        if (bArIsSaving)
        {
          const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Pitch, LastSerialized[CurrentTargetConnection].RotationPitch, CompareTolerance);
        }
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << Rotation.Pitch;
          if (bArIsSaving) LastSerialized[CurrentTargetConnection].RotationPitch = Rotation.Pitch;
          if (bArIsLoading) bReadNewRotationPitch = true;
        }
        else
        {
          if (bArIsLoading) bReadNewRotationPitch = false;
        }
      }
      if (bSerializeRotationYaw)
      {
        if (bArIsSaving)
        {
          const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(Rotation.Yaw, LastSerialized[CurrentTargetConnection].RotationYaw, CompareTolerance);
        }
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << Rotation.Yaw;
          if (bArIsSaving) LastSerialized[CurrentTargetConnection].RotationYaw = Rotation.Yaw;
          if (bArIsLoading) bReadNewRotationYaw = true;
        }
        else
        {
          if (bArIsLoading) bReadNewRotationYaw = false;
        }
      }
      break;
    }
    default: checkNoEntryGMC();
  }
}

void FState::SerializeControlRotation(FArchive& Ar)
{
  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  const float CompareTolerance = UGenMovementReplicationComponent::GetCompareToleranceRotator(ControlRotationQuantize);
  checkCodeGMC(if (bArIsSaving) check(LastSerialized.Contains(CurrentTargetConnection)))

  switch (uint8 B = 0; ControlRotationQuantize)
  {
    case ESizeQuantization::Byte:
    {
      // 1 byte components.
      uint8 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
        if (bSerializeControlRotationRoll)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Roll, LastSerialized[CurrentTargetConnection].ControlRotationRoll, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToByte(ControlRotation.Roll);
            Ar << Roll;
            LastSerialized[CurrentTargetConnection].ControlRotationRoll = ControlRotation.Roll;
          }
        }
        if (bSerializeControlRotationPitch)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Pitch, LastSerialized[CurrentTargetConnection].ControlRotationPitch, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToByte(ControlRotation.Pitch);
            Ar << Pitch;
            LastSerialized[CurrentTargetConnection].ControlRotationPitch = ControlRotation.Pitch;
          }
        }
        if (bSerializeControlRotationYaw)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Yaw, LastSerialized[CurrentTargetConnection].ControlRotationYaw, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToByte(ControlRotation.Yaw);
            Ar << Yaw;
            LastSerialized[CurrentTargetConnection].ControlRotationYaw = ControlRotation.Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeControlRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            ControlRotation.Roll = FRotator::DecompressAxisFromByte(Roll);
            bReadNewControlRotationRoll = true;
          }
          else
          {
            bReadNewControlRotationRoll = false;
          }
        }
        if (bSerializeControlRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            ControlRotation.Pitch = FRotator::DecompressAxisFromByte(Pitch);
            bReadNewControlRotationPitch = true;
          }
          else
          {
            bReadNewControlRotationPitch = false;
          }
        }
        if (bSerializeControlRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            ControlRotation.Yaw = FRotator::DecompressAxisFromByte(Yaw);
            bReadNewControlRotationYaw = true;
          }
          else
          {
            bReadNewControlRotationYaw = false;
          }
        }
      }
      break;
    }
    case ESizeQuantization::Short:
    {
      // 2 byte components.
      uint16 Pitch = 0, Yaw = 0, Roll = 0;
      if (bArIsSaving)
      {
        const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
        if (bSerializeControlRotationRoll)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Roll, LastSerialized[CurrentTargetConnection].ControlRotationRoll, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Roll = FRotator::CompressAxisToShort(ControlRotation.Roll);
            Ar << Roll;
            LastSerialized[CurrentTargetConnection].ControlRotationRoll = ControlRotation.Roll;
          }
        }
        if (bSerializeControlRotationPitch)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Pitch, LastSerialized[CurrentTargetConnection].ControlRotationPitch, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Pitch = FRotator::CompressAxisToShort(ControlRotation.Pitch);
            Ar << Pitch;
            LastSerialized[CurrentTargetConnection].ControlRotationPitch = ControlRotation.Pitch;
          }
        }
        if (bSerializeControlRotationYaw)
        {
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Yaw, LastSerialized[CurrentTargetConnection].ControlRotationYaw, CompareTolerance);
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Yaw = FRotator::CompressAxisToShort(ControlRotation.Yaw);
            Ar << Yaw;
            LastSerialized[CurrentTargetConnection].ControlRotationYaw = ControlRotation.Yaw;
          }
        }
      }
      else if (bArIsLoading)
      {
        if (bSerializeControlRotationRoll)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Roll;
            ControlRotation.Roll = FRotator::DecompressAxisFromShort(Roll);
            bReadNewControlRotationRoll = true;
          }
          else
          {
            bReadNewControlRotationRoll = false;
          }
        }
        if (bSerializeControlRotationPitch)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Pitch;
            ControlRotation.Pitch = FRotator::DecompressAxisFromShort(Pitch);
            bReadNewControlRotationPitch = true;
          }
          else
          {
            bReadNewControlRotationPitch = false;
          }
        }
        if (bSerializeControlRotationYaw)
        {
          Ar.SerializeBits(&B, 1);
          if (B)
          {
            Ar << Yaw;
            ControlRotation.Yaw = FRotator::DecompressAxisFromShort(Yaw);
            bReadNewControlRotationYaw = true;
          }
          else
          {
            bReadNewControlRotationYaw = false;
          }
        }
      }
      break;
    }
    case ESizeQuantization::None:
    {
      if (bSerializeControlRotationRoll)
      {
        if (bArIsSaving)
        {
          const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Roll, LastSerialized[CurrentTargetConnection].ControlRotationRoll, CompareTolerance);
        }
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << ControlRotation.Roll;
          if (bArIsSaving) LastSerialized[CurrentTargetConnection].ControlRotationRoll = ControlRotation.Roll;
          if (bArIsLoading) bReadNewControlRotationRoll = true;
        }
        else
        {
          if (bArIsLoading) bReadNewControlRotationRoll = false;
        }
      }
      if (bSerializeControlRotationPitch)
      {
        if (bArIsSaving)
        {
          const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Pitch, LastSerialized[CurrentTargetConnection].ControlRotationPitch, CompareTolerance);
        }
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << ControlRotation.Pitch;
          if (bArIsSaving) LastSerialized[CurrentTargetConnection].ControlRotationPitch = ControlRotation.Pitch;
          if (bArIsLoading) bReadNewControlRotationPitch = true;
        }
        else
        {
          if (bArIsLoading) bReadNewControlRotationPitch = false;
        }
      }
      if (bSerializeControlRotationYaw)
      {
        if (bArIsSaving)
        {
          const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
          B = bForceFullSerialization ? 1 : !FMath::IsNearlyEqual(ControlRotation.Yaw, LastSerialized[CurrentTargetConnection].ControlRotationYaw, CompareTolerance);
        }
        Ar.SerializeBits(&B, 1);
        if (B)
        {
          Ar << ControlRotation.Yaw;
          if (bArIsSaving) LastSerialized[CurrentTargetConnection].ControlRotationYaw = ControlRotation.Yaw;
          if (bArIsLoading) bReadNewControlRotationYaw = true;
        }
        else
        {
          if (bArIsLoading) bReadNewControlRotationYaw = false;
        }
      }
      break;
    }
    default: checkNoEntryGMC();
  }
}

void FState::SerializeInputFlags(FArchive& Ar)
{
  if (bSerializeBoundData)
  {
    // The bReplicateInputFlag variables will always be false for the autonomous proxy server state.
    if (bReplicateInputFlag1) Ar.SerializeBits(&bInputFlag1, 1);
    if (bReplicateInputFlag2) Ar.SerializeBits(&bInputFlag2, 1);
    if (bReplicateInputFlag3) Ar.SerializeBits(&bInputFlag3, 1);
    if (bReplicateInputFlag4) Ar.SerializeBits(&bInputFlag4, 1);
    if (bReplicateInputFlag5) Ar.SerializeBits(&bInputFlag5, 1);
    if (bReplicateInputFlag6) Ar.SerializeBits(&bInputFlag6, 1);
    if (bReplicateInputFlag7) Ar.SerializeBits(&bInputFlag7, 1);
    if (bReplicateInputFlag8) Ar.SerializeBits(&bInputFlag8, 1);
    if (bReplicateInputFlag9) Ar.SerializeBits(&bInputFlag9, 1);
    if (bReplicateInputFlag10) Ar.SerializeBits(&bInputFlag10, 1);
    if (bReplicateInputFlag11) Ar.SerializeBits(&bInputFlag11, 1);
    if (bReplicateInputFlag12) Ar.SerializeBits(&bInputFlag12, 1);
    if (bReplicateInputFlag13) Ar.SerializeBits(&bInputFlag13, 1);
    if (bReplicateInputFlag14) Ar.SerializeBits(&bInputFlag14, 1);
    if (bReplicateInputFlag15) Ar.SerializeBits(&bInputFlag15, 1);
    if (bReplicateInputFlag16) Ar.SerializeBits(&bInputFlag16, 1);
  }
}

void FState::SerializeBoundData(FArchive& Ar)
{
  if (bSerializeBoundData)
  {
    SerializeBoundData_IMPLEMENTATION()
  }
}

void FState::SerializeMoveValidation(FArchive& Ar)
{
  if (bSerializeMoveValidation)
  {
    Ar.SerializeBits(&bContainsFullRepBatch, 1);
  }
}

void FState::SerializeInputMode(FArchive& Ar)
{
  uint8 B = 0;
  checkCodeGMC(if (Ar.IsSaving()) check(LastSerialized.Contains(CurrentTargetConnection)))

  if (bSerializeInputMode)
  {
    if (Ar.IsSaving())
    {
      const bool bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
      B = bForceFullSerialization ? 1 : InputMode != LastSerialized[CurrentTargetConnection].InputMode;
      Ar.SerializeBits(&B, 1);
      if (B)
      {
        Ar.SerializeBits(&InputMode, 3);
        LastSerialized[CurrentTargetConnection].InputMode = InputMode;
      }
    }
    else if (Ar.IsLoading())
    {
      Ar.SerializeBits(&B, 1);
      if (B)
      {
        Ar.SerializeBits(&InputMode, 3);
        bReadNewInputMode = true;
      }
      else
      {
        bReadNewInputMode = false;
      }
    }
  }
}

void FState::QuantizeVelocity()
{
  switch (VelocityQuantize)
  {
    case EDecimalQuantization::RoundTwoDecimals:
    {
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.X))
      {
        Velocity.X = FMath::RoundToFloat(Velocity.X * 100.f) / 100.f;
      }
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.Y))
      {
        Velocity.Y = FMath::RoundToFloat(Velocity.Y * 100.f) / 100.f;
      }
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.Z))
      {
        Velocity.Z = FMath::RoundToFloat(Velocity.Z * 100.f) / 100.f;
      }
      return;
    }
    case EDecimalQuantization::RoundOneDecimal:
    {
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.X))
      {
        Velocity.X = FMath::RoundToFloat(Velocity.X * 10.f) / 10.f;
      }
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.Y))
      {
        Velocity.Y = FMath::RoundToFloat(Velocity.Y * 10.f) / 10.f;
      }
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.Z))
      {
        Velocity.Z = FMath::RoundToFloat(Velocity.Z * 10.f) / 10.f;
      }
      return;
    }
    case EDecimalQuantization::RoundWholeNumber:
    {
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.X))
      {
        Velocity.X = FMath::RoundToFloat(Velocity.X);
      }
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.Y))
      {
        Velocity.Y = FMath::RoundToFloat(Velocity.Y);
      }
      if (bSerializeVelocity && UGenMovementReplicationComponent::Rep_IsValid(Velocity.Z))
      {
        Velocity.Z = FMath::RoundToFloat(Velocity.Z);
      }
      return;
    }
    case EDecimalQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FState::QuantizeLocation()
{
  switch (LocationQuantize)
  {
    case EDecimalQuantization::RoundTwoDecimals:
    {
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.X))
      {
        Location.X = FMath::RoundToFloat(Location.X * 100.f) / 100.f;
      }
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.Y))
      {
        Location.Y = FMath::RoundToFloat(Location.Y * 100.f) / 100.f;
      }
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.Z))
      {
        Location.Z = FMath::RoundToFloat(Location.Z * 100.f) / 100.f;
      }
      return;
    }
    case EDecimalQuantization::RoundOneDecimal:
    {
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.X))
      {
        Location.X = FMath::RoundToFloat(Location.X * 10.f) / 10.f;
      }
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.Y))
      {
        Location.Y = FMath::RoundToFloat(Location.Y * 10.f) / 10.f;
      }
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.Z))
      {
        Location.Z = FMath::RoundToFloat(Location.Z * 10.f) / 10.f;
      }
      return;
    }
    case EDecimalQuantization::RoundWholeNumber:
    {
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.X))
      {
        Location.X = FMath::RoundToFloat(Location.X);
      }
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.Y))
      {
        Location.Y = FMath::RoundToFloat(Location.Y);
      }
      if (bSerializeLocation && UGenMovementReplicationComponent::Rep_IsValid(Location.Z))
      {
        Location.Z = FMath::RoundToFloat(Location.Z);
      }
      return;
    }
    case EDecimalQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FState::QuantizeRotation()
{
  switch (RotationQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (bSerializeRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(Rotation.Roll))
      {
        Rotation.Roll = FMath::RoundToFloat(Rotation.Roll * 100.f) / 100.f;
      }
      if (bSerializeRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(Rotation.Pitch))
      {
        Rotation.Pitch = FMath::RoundToFloat(Rotation.Pitch * 100.f) / 100.f;
      }
      if (bSerializeRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(Rotation.Yaw))
      {
        Rotation.Yaw = FMath::RoundToFloat(Rotation.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      if (bSerializeRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(Rotation.Roll))
      {
        Rotation.Roll = FMath::RoundToFloat(Rotation.Roll);
      }
      if (bSerializeRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(Rotation.Pitch))
      {
        Rotation.Pitch = FMath::RoundToFloat(Rotation.Pitch);
      }
      if (bSerializeRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(Rotation.Yaw))
      {
        Rotation.Yaw = FMath::RoundToFloat(Rotation.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FState::QuantizeControlRotation()
{
  switch (ControlRotationQuantize)
  {
    case ESizeQuantization::Short:
    {
      // 2 decimal places of precision.
      if (bSerializeControlRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(ControlRotation.Roll))
      {
        ControlRotation.Roll = FMath::RoundToFloat(ControlRotation.Roll * 100.f) / 100.f;
      }
      if (bSerializeControlRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(ControlRotation.Pitch))
      {
        ControlRotation.Pitch = FMath::RoundToFloat(ControlRotation.Pitch * 100.f) / 100.f;
      }
      if (bSerializeControlRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(ControlRotation.Yaw))
      {
        ControlRotation.Yaw = FMath::RoundToFloat(ControlRotation.Yaw * 100.f) / 100.f;
      }
      return;
    }
    case ESizeQuantization::Byte:
    {
      // Round whole number.
      if (bSerializeControlRotationRoll && UGenMovementReplicationComponent::Rep_IsValid(ControlRotation.Roll))
      {
        ControlRotation.Roll = FMath::RoundToFloat(ControlRotation.Roll);
      }
      if (bSerializeControlRotationPitch && UGenMovementReplicationComponent::Rep_IsValid(ControlRotation.Pitch))
      {
        ControlRotation.Pitch = FMath::RoundToFloat(ControlRotation.Pitch);
      }
      if (bSerializeControlRotationYaw && UGenMovementReplicationComponent::Rep_IsValid(ControlRotation.Yaw))
      {
        ControlRotation.Yaw = FMath::RoundToFloat(ControlRotation.Yaw);
      }
      return;
    }
    case ESizeQuantization::None: return;
    default: checkNoEntryGMC();
  }
}

void FState::SerializeBoolTypes(FArchive& Ar)
{
  // Booleans are serialized as uint32 with the << operator (which is very wasteful) so we serialize the bits manually one at a time.
  // Additionally, since booleans only hold 1 bit of information, there is no point in using the usual optimization strategy of only sending
  // 1 bit when the value hasn't changed.

  if (bReplicateBool1) Ar.SerializeBits(&Bool1, 1);
  if (bReplicateBool2) Ar.SerializeBits(&Bool2, 1);
  if (bReplicateBool3) Ar.SerializeBits(&Bool3, 1);
  if (bReplicateBool4) Ar.SerializeBits(&Bool4, 1);
  if (bReplicateBool5) Ar.SerializeBits(&Bool5, 1);
  if (bReplicateBool6) Ar.SerializeBits(&Bool6, 1);
  if (bReplicateBool7) Ar.SerializeBits(&Bool7, 1);
  if (bReplicateBool8) Ar.SerializeBits(&Bool8, 1);
  if (bReplicateBool9) Ar.SerializeBits(&Bool9, 1);
  if (bReplicateBool10) Ar.SerializeBits(&Bool10, 1);
  if (bReplicateBool11) Ar.SerializeBits(&Bool11, 1);
  if (bReplicateBool12) Ar.SerializeBits(&Bool12, 1);
  if (bReplicateBool13) Ar.SerializeBits(&Bool13, 1);
  if (bReplicateBool14) Ar.SerializeBits(&Bool14, 1);
  if (bReplicateBool15) Ar.SerializeBits(&Bool15, 1);
  if (bReplicateBool16) Ar.SerializeBits(&Bool16, 1);
}

void FState::SerializeHalfByteTypes(FArchive& Ar)
{
  // A half byte can carry 4 bit of information. It is useful for replicating integer values that only need to be able to hold a few
  // different states (mostly enums).

  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bForceFullSerialization = false;
  if (bArIsSaving)
  {
    check(LastSerialized.Contains(CurrentTargetConnection))
    bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
  }
  uint8 B = 0;
  if (bArIsSaving) { if (bReplicateHalfByte1)  { B = bForceFullSerialization ? 1 : HalfByte1  != LastSerialized[CurrentTargetConnection].HalfByte1;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte1, 4);  LastSerialized[CurrentTargetConnection].HalfByte1  = HalfByte1;  } } } else if (bArIsLoading) { if (bReplicateHalfByte1)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte1, 4);  bReadNewHalfByte1  = true; } else { bReadNewHalfByte1  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte2)  { B = bForceFullSerialization ? 1 : HalfByte2  != LastSerialized[CurrentTargetConnection].HalfByte2;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte2, 4);  LastSerialized[CurrentTargetConnection].HalfByte2  = HalfByte2;  } } } else if (bArIsLoading) { if (bReplicateHalfByte2)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte2, 4);  bReadNewHalfByte2  = true; } else { bReadNewHalfByte2  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte3)  { B = bForceFullSerialization ? 1 : HalfByte3  != LastSerialized[CurrentTargetConnection].HalfByte3;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte3, 4);  LastSerialized[CurrentTargetConnection].HalfByte3  = HalfByte3;  } } } else if (bArIsLoading) { if (bReplicateHalfByte3)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte3, 4);  bReadNewHalfByte3  = true; } else { bReadNewHalfByte3  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte4)  { B = bForceFullSerialization ? 1 : HalfByte4  != LastSerialized[CurrentTargetConnection].HalfByte4;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte4, 4);  LastSerialized[CurrentTargetConnection].HalfByte4  = HalfByte4;  } } } else if (bArIsLoading) { if (bReplicateHalfByte4)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte4, 4);  bReadNewHalfByte4  = true; } else { bReadNewHalfByte4  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte5)  { B = bForceFullSerialization ? 1 : HalfByte5  != LastSerialized[CurrentTargetConnection].HalfByte5;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte5, 4);  LastSerialized[CurrentTargetConnection].HalfByte5  = HalfByte5;  } } } else if (bArIsLoading) { if (bReplicateHalfByte5)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte5, 4);  bReadNewHalfByte5  = true; } else { bReadNewHalfByte5  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte6)  { B = bForceFullSerialization ? 1 : HalfByte6  != LastSerialized[CurrentTargetConnection].HalfByte6;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte6, 4);  LastSerialized[CurrentTargetConnection].HalfByte6  = HalfByte6;  } } } else if (bArIsLoading) { if (bReplicateHalfByte6)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte6, 4);  bReadNewHalfByte6  = true; } else { bReadNewHalfByte6  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte7)  { B = bForceFullSerialization ? 1 : HalfByte7  != LastSerialized[CurrentTargetConnection].HalfByte7;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte7, 4);  LastSerialized[CurrentTargetConnection].HalfByte7  = HalfByte7;  } } } else if (bArIsLoading) { if (bReplicateHalfByte7)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte7, 4);  bReadNewHalfByte7  = true; } else { bReadNewHalfByte7  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte8)  { B = bForceFullSerialization ? 1 : HalfByte8  != LastSerialized[CurrentTargetConnection].HalfByte8;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte8, 4);  LastSerialized[CurrentTargetConnection].HalfByte8  = HalfByte8;  } } } else if (bArIsLoading) { if (bReplicateHalfByte8)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte8, 4);  bReadNewHalfByte8  = true; } else { bReadNewHalfByte8  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte9)  { B = bForceFullSerialization ? 1 : HalfByte9  != LastSerialized[CurrentTargetConnection].HalfByte9;  Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte9, 4);  LastSerialized[CurrentTargetConnection].HalfByte9  = HalfByte9;  } } } else if (bArIsLoading) { if (bReplicateHalfByte9)  { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte9, 4);  bReadNewHalfByte9  = true; } else { bReadNewHalfByte9  = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte10) { B = bForceFullSerialization ? 1 : HalfByte10 != LastSerialized[CurrentTargetConnection].HalfByte10; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte10, 4); LastSerialized[CurrentTargetConnection].HalfByte10 = HalfByte10; } } } else if (bArIsLoading) { if (bReplicateHalfByte10) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte10, 4); bReadNewHalfByte10 = true; } else { bReadNewHalfByte10 = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte11) { B = bForceFullSerialization ? 1 : HalfByte11 != LastSerialized[CurrentTargetConnection].HalfByte11; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte11, 4); LastSerialized[CurrentTargetConnection].HalfByte11 = HalfByte11; } } } else if (bArIsLoading) { if (bReplicateHalfByte11) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte11, 4); bReadNewHalfByte11 = true; } else { bReadNewHalfByte11 = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte12) { B = bForceFullSerialization ? 1 : HalfByte12 != LastSerialized[CurrentTargetConnection].HalfByte12; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte12, 4); LastSerialized[CurrentTargetConnection].HalfByte12 = HalfByte12; } } } else if (bArIsLoading) { if (bReplicateHalfByte12) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte12, 4); bReadNewHalfByte12 = true; } else { bReadNewHalfByte12 = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte13) { B = bForceFullSerialization ? 1 : HalfByte13 != LastSerialized[CurrentTargetConnection].HalfByte13; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte13, 4); LastSerialized[CurrentTargetConnection].HalfByte13 = HalfByte13; } } } else if (bArIsLoading) { if (bReplicateHalfByte13) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte13, 4); bReadNewHalfByte13 = true; } else { bReadNewHalfByte13 = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte14) { B = bForceFullSerialization ? 1 : HalfByte14 != LastSerialized[CurrentTargetConnection].HalfByte14; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte14, 4); LastSerialized[CurrentTargetConnection].HalfByte14 = HalfByte14; } } } else if (bArIsLoading) { if (bReplicateHalfByte14) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte14, 4); bReadNewHalfByte14 = true; } else { bReadNewHalfByte14 = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte15) { B = bForceFullSerialization ? 1 : HalfByte15 != LastSerialized[CurrentTargetConnection].HalfByte15; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte15, 4); LastSerialized[CurrentTargetConnection].HalfByte15 = HalfByte15; } } } else if (bArIsLoading) { if (bReplicateHalfByte15) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte15, 4); bReadNewHalfByte15 = true; } else { bReadNewHalfByte15 = false; } } }
  if (bArIsSaving) { if (bReplicateHalfByte16) { B = bForceFullSerialization ? 1 : HalfByte16 != LastSerialized[CurrentTargetConnection].HalfByte16; Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte16, 4); LastSerialized[CurrentTargetConnection].HalfByte16 = HalfByte16; } } } else if (bArIsLoading) { if (bReplicateHalfByte16) { Ar.SerializeBits(&B, 1); if (B) { Ar.SerializeBits(&HalfByte16, 4); bReadNewHalfByte16 = true; } else { bReadNewHalfByte16 = false; } } }
}

void FState::SerializeVectorTypes(FArchive& Ar)
{
  // Vectors are rounded to 2 decimal places.

  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bForceFullSerialization = false;
  if (bArIsSaving)
  {
    check(LastSerialized.Contains(CurrentTargetConnection))
    bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
  }
  constexpr float COMPARE_TOLERANCE = 0.01f;
  uint8 B = 0;
  if (bArIsSaving) { if (bReplicateVector1)  { B = bForceFullSerialization ? 1 : !Vector1.Equals(LastSerialized[CurrentTargetConnection].Vector1, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector1, Ar);  LastSerialized[CurrentTargetConnection].Vector1  = Vector1;  } } } else if (bArIsLoading) { if (bReplicateVector1)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector1, Ar);  bReadNewVector1  = true; } else { bReadNewVector1  = false; } } }
  if (bArIsSaving) { if (bReplicateVector2)  { B = bForceFullSerialization ? 1 : !Vector2.Equals(LastSerialized[CurrentTargetConnection].Vector2, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector2, Ar);  LastSerialized[CurrentTargetConnection].Vector2  = Vector2;  } } } else if (bArIsLoading) { if (bReplicateVector2)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector2, Ar);  bReadNewVector2  = true; } else { bReadNewVector2  = false; } } }
  if (bArIsSaving) { if (bReplicateVector3)  { B = bForceFullSerialization ? 1 : !Vector3.Equals(LastSerialized[CurrentTargetConnection].Vector3, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector3, Ar);  LastSerialized[CurrentTargetConnection].Vector3  = Vector3;  } } } else if (bArIsLoading) { if (bReplicateVector3)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector3, Ar);  bReadNewVector3  = true; } else { bReadNewVector3  = false; } } }
  if (bArIsSaving) { if (bReplicateVector4)  { B = bForceFullSerialization ? 1 : !Vector4.Equals(LastSerialized[CurrentTargetConnection].Vector4, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector4, Ar);  LastSerialized[CurrentTargetConnection].Vector4  = Vector4;  } } } else if (bArIsLoading) { if (bReplicateVector4)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector4, Ar);  bReadNewVector4  = true; } else { bReadNewVector4  = false; } } }
  if (bArIsSaving) { if (bReplicateVector5)  { B = bForceFullSerialization ? 1 : !Vector5.Equals(LastSerialized[CurrentTargetConnection].Vector5, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector5, Ar);  LastSerialized[CurrentTargetConnection].Vector5  = Vector5;  } } } else if (bArIsLoading) { if (bReplicateVector5)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector5, Ar);  bReadNewVector5  = true; } else { bReadNewVector5  = false; } } }
  if (bArIsSaving) { if (bReplicateVector6)  { B = bForceFullSerialization ? 1 : !Vector6.Equals(LastSerialized[CurrentTargetConnection].Vector6, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector6, Ar);  LastSerialized[CurrentTargetConnection].Vector6  = Vector6;  } } } else if (bArIsLoading) { if (bReplicateVector6)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector6, Ar);  bReadNewVector6  = true; } else { bReadNewVector6  = false; } } }
  if (bArIsSaving) { if (bReplicateVector7)  { B = bForceFullSerialization ? 1 : !Vector7.Equals(LastSerialized[CurrentTargetConnection].Vector7, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector7, Ar);  LastSerialized[CurrentTargetConnection].Vector7  = Vector7;  } } } else if (bArIsLoading) { if (bReplicateVector7)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector7, Ar);  bReadNewVector7  = true; } else { bReadNewVector7  = false; } } }
  if (bArIsSaving) { if (bReplicateVector8)  { B = bForceFullSerialization ? 1 : !Vector8.Equals(LastSerialized[CurrentTargetConnection].Vector8, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector8, Ar);  LastSerialized[CurrentTargetConnection].Vector8  = Vector8;  } } } else if (bArIsLoading) { if (bReplicateVector8)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector8, Ar);  bReadNewVector8  = true; } else { bReadNewVector8  = false; } } }
  if (bArIsSaving) { if (bReplicateVector9)  { B = bForceFullSerialization ? 1 : !Vector9.Equals(LastSerialized[CurrentTargetConnection].Vector9, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector9, Ar);  LastSerialized[CurrentTargetConnection].Vector9  = Vector9;  } } } else if (bArIsLoading) { if (bReplicateVector9)  { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector9, Ar);  bReadNewVector9  = true; } else { bReadNewVector9  = false; } } }
  if (bArIsSaving) { if (bReplicateVector10) { B = bForceFullSerialization ? 1 : !Vector10.Equals(LastSerialized[CurrentTargetConnection].Vector10, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector10, Ar); LastSerialized[CurrentTargetConnection].Vector10 = Vector10; } } } else if (bArIsLoading) { if (bReplicateVector10) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector10, Ar); bReadNewVector10 = true; } else { bReadNewVector10 = false; } } }
  if (bArIsSaving) { if (bReplicateVector11) { B = bForceFullSerialization ? 1 : !Vector11.Equals(LastSerialized[CurrentTargetConnection].Vector11, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector11, Ar); LastSerialized[CurrentTargetConnection].Vector11 = Vector11; } } } else if (bArIsLoading) { if (bReplicateVector11) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector11, Ar); bReadNewVector11 = true; } else { bReadNewVector11 = false; } } }
  if (bArIsSaving) { if (bReplicateVector12) { B = bForceFullSerialization ? 1 : !Vector12.Equals(LastSerialized[CurrentTargetConnection].Vector12, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector12, Ar); LastSerialized[CurrentTargetConnection].Vector12 = Vector12; } } } else if (bArIsLoading) { if (bReplicateVector12) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector12, Ar); bReadNewVector12 = true; } else { bReadNewVector12 = false; } } }
  if (bArIsSaving) { if (bReplicateVector13) { B = bForceFullSerialization ? 1 : !Vector13.Equals(LastSerialized[CurrentTargetConnection].Vector13, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector13, Ar); LastSerialized[CurrentTargetConnection].Vector13 = Vector13; } } } else if (bArIsLoading) { if (bReplicateVector13) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector13, Ar); bReadNewVector13 = true; } else { bReadNewVector13 = false; } } }
  if (bArIsSaving) { if (bReplicateVector14) { B = bForceFullSerialization ? 1 : !Vector14.Equals(LastSerialized[CurrentTargetConnection].Vector14, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector14, Ar); LastSerialized[CurrentTargetConnection].Vector14 = Vector14; } } } else if (bArIsLoading) { if (bReplicateVector14) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector14, Ar); bReadNewVector14 = true; } else { bReadNewVector14 = false; } } }
  if (bArIsSaving) { if (bReplicateVector15) { B = bForceFullSerialization ? 1 : !Vector15.Equals(LastSerialized[CurrentTargetConnection].Vector15, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector15, Ar); LastSerialized[CurrentTargetConnection].Vector15 = Vector15; } } } else if (bArIsLoading) { if (bReplicateVector15) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector15, Ar); bReadNewVector15 = true; } else { bReadNewVector15 = false; } } }
  if (bArIsSaving) { if (bReplicateVector16) { B = bForceFullSerialization ? 1 : !Vector16.Equals(LastSerialized[CurrentTargetConnection].Vector16, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector16, Ar); LastSerialized[CurrentTargetConnection].Vector16 = Vector16; } } } else if (bArIsLoading) { if (bReplicateVector16) { Ar.SerializeBits(&B, 1); if (B) { SerializePackedVector<100, 30>(Vector16, Ar); bReadNewVector16 = true; } else { bReadNewVector16 = false; } } }
}

void FState::SerializeNormalTypes(FArchive& Ar)
{
  // Normals must not exceed the max value of 1 per component. This allows us to compress the vector to 16 bit components while retaining
  // a precision of 4 decimal places.

  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bForceFullSerialization = false;
  if (bArIsSaving)
  {
    check(LastSerialized.Contains(CurrentTargetConnection))
    bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
  }
  constexpr float COMPARE_TOLERANCE = 0.0001f;
  uint8 B = 0;
  if (bArIsSaving) { if (bReplicateNormal1)  { B = bForceFullSerialization ? 1 : !Normal1.Equals(LastSerialized[CurrentTargetConnection].Normal1, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal1, Ar);  LastSerialized[CurrentTargetConnection].Normal1  = Normal1;  } } } else if (bArIsLoading) { if (bReplicateNormal1)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal1, Ar);  bReadNewNormal1  = true; } else { bReadNewNormal1  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal2)  { B = bForceFullSerialization ? 1 : !Normal2.Equals(LastSerialized[CurrentTargetConnection].Normal2, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal2, Ar);  LastSerialized[CurrentTargetConnection].Normal2  = Normal2;  } } } else if (bArIsLoading) { if (bReplicateNormal2)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal2, Ar);  bReadNewNormal2  = true; } else { bReadNewNormal2  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal3)  { B = bForceFullSerialization ? 1 : !Normal3.Equals(LastSerialized[CurrentTargetConnection].Normal3, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal3, Ar);  LastSerialized[CurrentTargetConnection].Normal3  = Normal3;  } } } else if (bArIsLoading) { if (bReplicateNormal3)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal3, Ar);  bReadNewNormal3  = true; } else { bReadNewNormal3  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal4)  { B = bForceFullSerialization ? 1 : !Normal4.Equals(LastSerialized[CurrentTargetConnection].Normal4, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal4, Ar);  LastSerialized[CurrentTargetConnection].Normal4  = Normal4;  } } } else if (bArIsLoading) { if (bReplicateNormal4)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal4, Ar);  bReadNewNormal4  = true; } else { bReadNewNormal4  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal5)  { B = bForceFullSerialization ? 1 : !Normal5.Equals(LastSerialized[CurrentTargetConnection].Normal5, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal5, Ar);  LastSerialized[CurrentTargetConnection].Normal5  = Normal5;  } } } else if (bArIsLoading) { if (bReplicateNormal5)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal5, Ar);  bReadNewNormal5  = true; } else { bReadNewNormal5  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal6)  { B = bForceFullSerialization ? 1 : !Normal6.Equals(LastSerialized[CurrentTargetConnection].Normal6, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal6, Ar);  LastSerialized[CurrentTargetConnection].Normal6  = Normal6;  } } } else if (bArIsLoading) { if (bReplicateNormal6)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal6, Ar);  bReadNewNormal6  = true; } else { bReadNewNormal6  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal7)  { B = bForceFullSerialization ? 1 : !Normal7.Equals(LastSerialized[CurrentTargetConnection].Normal7, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal7, Ar);  LastSerialized[CurrentTargetConnection].Normal7  = Normal7;  } } } else if (bArIsLoading) { if (bReplicateNormal7)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal7, Ar);  bReadNewNormal7  = true; } else { bReadNewNormal7  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal8)  { B = bForceFullSerialization ? 1 : !Normal8.Equals(LastSerialized[CurrentTargetConnection].Normal8, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal8, Ar);  LastSerialized[CurrentTargetConnection].Normal8  = Normal8;  } } } else if (bArIsLoading) { if (bReplicateNormal8)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal8, Ar);  bReadNewNormal8  = true; } else { bReadNewNormal8  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal9)  { B = bForceFullSerialization ? 1 : !Normal9.Equals(LastSerialized[CurrentTargetConnection].Normal9, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal9, Ar);  LastSerialized[CurrentTargetConnection].Normal9  = Normal9;  } } } else if (bArIsLoading) { if (bReplicateNormal9)  { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal9, Ar);  bReadNewNormal9  = true; } else { bReadNewNormal9  = false; } } }
  if (bArIsSaving) { if (bReplicateNormal10) { B = bForceFullSerialization ? 1 : !Normal10.Equals(LastSerialized[CurrentTargetConnection].Normal10, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal10, Ar); LastSerialized[CurrentTargetConnection].Normal10 = Normal10; } } } else if (bArIsLoading) { if (bReplicateNormal10) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal10, Ar); bReadNewNormal10 = true; } else { bReadNewNormal10 = false; } } }
  if (bArIsSaving) { if (bReplicateNormal11) { B = bForceFullSerialization ? 1 : !Normal11.Equals(LastSerialized[CurrentTargetConnection].Normal11, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal11, Ar); LastSerialized[CurrentTargetConnection].Normal11 = Normal11; } } } else if (bArIsLoading) { if (bReplicateNormal11) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal11, Ar); bReadNewNormal11 = true; } else { bReadNewNormal11 = false; } } }
  if (bArIsSaving) { if (bReplicateNormal12) { B = bForceFullSerialization ? 1 : !Normal12.Equals(LastSerialized[CurrentTargetConnection].Normal12, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal12, Ar); LastSerialized[CurrentTargetConnection].Normal12 = Normal12; } } } else if (bArIsLoading) { if (bReplicateNormal12) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal12, Ar); bReadNewNormal12 = true; } else { bReadNewNormal12 = false; } } }
  if (bArIsSaving) { if (bReplicateNormal13) { B = bForceFullSerialization ? 1 : !Normal13.Equals(LastSerialized[CurrentTargetConnection].Normal13, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal13, Ar); LastSerialized[CurrentTargetConnection].Normal13 = Normal13; } } } else if (bArIsLoading) { if (bReplicateNormal13) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal13, Ar); bReadNewNormal13 = true; } else { bReadNewNormal13 = false; } } }
  if (bArIsSaving) { if (bReplicateNormal14) { B = bForceFullSerialization ? 1 : !Normal14.Equals(LastSerialized[CurrentTargetConnection].Normal14, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal14, Ar); LastSerialized[CurrentTargetConnection].Normal14 = Normal14; } } } else if (bArIsLoading) { if (bReplicateNormal14) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal14, Ar); bReadNewNormal14 = true; } else { bReadNewNormal14 = false; } } }
  if (bArIsSaving) { if (bReplicateNormal15) { B = bForceFullSerialization ? 1 : !Normal15.Equals(LastSerialized[CurrentTargetConnection].Normal15, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal15, Ar); LastSerialized[CurrentTargetConnection].Normal15 = Normal15; } } } else if (bArIsLoading) { if (bReplicateNormal15) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal15, Ar); bReadNewNormal15 = true; } else { bReadNewNormal15 = false; } } }
  if (bArIsSaving) { if (bReplicateNormal16) { B = bForceFullSerialization ? 1 : !Normal16.Equals(LastSerialized[CurrentTargetConnection].Normal16, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal16, Ar); LastSerialized[CurrentTargetConnection].Normal16 = Normal16; } } } else if (bArIsLoading) { if (bReplicateNormal16) { Ar.SerializeBits(&B, 1); if (B) { SerializeFixedVector<1, 16>(Normal16, Ar); bReadNewNormal16 = true; } else { bReadNewNormal16 = false; } } }
}

void FState::SerializeRotatorTypes(FArchive& Ar)
{
  // Rotators are compressed to 16 bit components with a precision of 2 decimal places.

  const bool bArIsSaving = Ar.IsSaving();
  const bool bArIsLoading = Ar.IsLoading();
  bool bForceFullSerialization = false;
  if (bArIsSaving)
  {
    check(LastSerialized.Contains(CurrentTargetConnection))
    bForceFullSerialization = !bOptimizeTraffic || LastSerialized[CurrentTargetConnection].bForceFullSerializationOnNextUpdate;
  }
  constexpr float COMPARE_TOLERANCE = 0.01f;
  uint8 B = 0;
  if (bArIsSaving) { if (bReplicateRotator1)  { B = bForceFullSerialization ? 1 : !Rotator1.Equals(LastSerialized[CurrentTargetConnection].Rotator1, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator1.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator1  = Rotator1;  } } } else if (bArIsLoading) { if (bReplicateRotator1)  { Ar.SerializeBits(&B, 1); if (B) { Rotator1.SerializeCompressedShort(Ar);  bReadNewRotator1  = true; } else { bReadNewRotator1  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator2)  { B = bForceFullSerialization ? 1 : !Rotator2.Equals(LastSerialized[CurrentTargetConnection].Rotator2, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator2.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator2  = Rotator2;  } } } else if (bArIsLoading) { if (bReplicateRotator2)  { Ar.SerializeBits(&B, 1); if (B) { Rotator2.SerializeCompressedShort(Ar);  bReadNewRotator2  = true; } else { bReadNewRotator2  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator3)  { B = bForceFullSerialization ? 1 : !Rotator3.Equals(LastSerialized[CurrentTargetConnection].Rotator3, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator3.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator3  = Rotator3;  } } } else if (bArIsLoading) { if (bReplicateRotator3)  { Ar.SerializeBits(&B, 1); if (B) { Rotator3.SerializeCompressedShort(Ar);  bReadNewRotator3  = true; } else { bReadNewRotator3  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator4)  { B = bForceFullSerialization ? 1 : !Rotator4.Equals(LastSerialized[CurrentTargetConnection].Rotator4, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator4.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator4  = Rotator4;  } } } else if (bArIsLoading) { if (bReplicateRotator4)  { Ar.SerializeBits(&B, 1); if (B) { Rotator4.SerializeCompressedShort(Ar);  bReadNewRotator4  = true; } else { bReadNewRotator4  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator5)  { B = bForceFullSerialization ? 1 : !Rotator5.Equals(LastSerialized[CurrentTargetConnection].Rotator5, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator5.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator5  = Rotator5;  } } } else if (bArIsLoading) { if (bReplicateRotator5)  { Ar.SerializeBits(&B, 1); if (B) { Rotator5.SerializeCompressedShort(Ar);  bReadNewRotator5  = true; } else { bReadNewRotator5  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator6)  { B = bForceFullSerialization ? 1 : !Rotator6.Equals(LastSerialized[CurrentTargetConnection].Rotator6, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator6.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator6  = Rotator6;  } } } else if (bArIsLoading) { if (bReplicateRotator6)  { Ar.SerializeBits(&B, 1); if (B) { Rotator6.SerializeCompressedShort(Ar);  bReadNewRotator6  = true; } else { bReadNewRotator6  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator7)  { B = bForceFullSerialization ? 1 : !Rotator7.Equals(LastSerialized[CurrentTargetConnection].Rotator7, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator7.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator7  = Rotator7;  } } } else if (bArIsLoading) { if (bReplicateRotator7)  { Ar.SerializeBits(&B, 1); if (B) { Rotator7.SerializeCompressedShort(Ar);  bReadNewRotator7  = true; } else { bReadNewRotator7  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator8)  { B = bForceFullSerialization ? 1 : !Rotator8.Equals(LastSerialized[CurrentTargetConnection].Rotator8, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator8.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator8  = Rotator8;  } } } else if (bArIsLoading) { if (bReplicateRotator8)  { Ar.SerializeBits(&B, 1); if (B) { Rotator8.SerializeCompressedShort(Ar);  bReadNewRotator8  = true; } else { bReadNewRotator8  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator9)  { B = bForceFullSerialization ? 1 : !Rotator9.Equals(LastSerialized[CurrentTargetConnection].Rotator9, COMPARE_TOLERANCE);   Ar.SerializeBits(&B, 1); if (B) { Rotator9.SerializeCompressedShort(Ar);  LastSerialized[CurrentTargetConnection].Rotator9  = Rotator9;  } } } else if (bArIsLoading) { if (bReplicateRotator9)  { Ar.SerializeBits(&B, 1); if (B) { Rotator9.SerializeCompressedShort(Ar);  bReadNewRotator9  = true; } else { bReadNewRotator9  = false; } } }
  if (bArIsSaving) { if (bReplicateRotator10) { B = bForceFullSerialization ? 1 : !Rotator10.Equals(LastSerialized[CurrentTargetConnection].Rotator10, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator10.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator10 = Rotator10; } } } else if (bArIsLoading) { if (bReplicateRotator10) { Ar.SerializeBits(&B, 1); if (B) { Rotator10.SerializeCompressedShort(Ar); bReadNewRotator10 = true; } else { bReadNewRotator10 = false; } } }
  if (bArIsSaving) { if (bReplicateRotator11) { B = bForceFullSerialization ? 1 : !Rotator11.Equals(LastSerialized[CurrentTargetConnection].Rotator11, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator11.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator11 = Rotator11; } } } else if (bArIsLoading) { if (bReplicateRotator11) { Ar.SerializeBits(&B, 1); if (B) { Rotator11.SerializeCompressedShort(Ar); bReadNewRotator11 = true; } else { bReadNewRotator11 = false; } } }
  if (bArIsSaving) { if (bReplicateRotator12) { B = bForceFullSerialization ? 1 : !Rotator12.Equals(LastSerialized[CurrentTargetConnection].Rotator12, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator12.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator12 = Rotator12; } } } else if (bArIsLoading) { if (bReplicateRotator12) { Ar.SerializeBits(&B, 1); if (B) { Rotator12.SerializeCompressedShort(Ar); bReadNewRotator12 = true; } else { bReadNewRotator12 = false; } } }
  if (bArIsSaving) { if (bReplicateRotator13) { B = bForceFullSerialization ? 1 : !Rotator13.Equals(LastSerialized[CurrentTargetConnection].Rotator13, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator13.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator13 = Rotator13; } } } else if (bArIsLoading) { if (bReplicateRotator13) { Ar.SerializeBits(&B, 1); if (B) { Rotator13.SerializeCompressedShort(Ar); bReadNewRotator13 = true; } else { bReadNewRotator13 = false; } } }
  if (bArIsSaving) { if (bReplicateRotator14) { B = bForceFullSerialization ? 1 : !Rotator14.Equals(LastSerialized[CurrentTargetConnection].Rotator14, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator14.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator14 = Rotator14; } } } else if (bArIsLoading) { if (bReplicateRotator14) { Ar.SerializeBits(&B, 1); if (B) { Rotator14.SerializeCompressedShort(Ar); bReadNewRotator14 = true; } else { bReadNewRotator14 = false; } } }
  if (bArIsSaving) { if (bReplicateRotator15) { B = bForceFullSerialization ? 1 : !Rotator15.Equals(LastSerialized[CurrentTargetConnection].Rotator15, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator15.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator15 = Rotator15; } } } else if (bArIsLoading) { if (bReplicateRotator15) { Ar.SerializeBits(&B, 1); if (B) { Rotator15.SerializeCompressedShort(Ar); bReadNewRotator15 = true; } else { bReadNewRotator15 = false; } } }
  if (bArIsSaving) { if (bReplicateRotator16) { B = bForceFullSerialization ? 1 : !Rotator16.Equals(LastSerialized[CurrentTargetConnection].Rotator16, COMPARE_TOLERANCE); Ar.SerializeBits(&B, 1); if (B) { Rotator16.SerializeCompressedShort(Ar); LastSerialized[CurrentTargetConnection].Rotator16 = Rotator16; } } } else if (bArIsLoading) { if (bReplicateRotator16) { Ar.SerializeBits(&B, 1); if (B) { Rotator16.SerializeCompressedShort(Ar); bReadNewRotator16 = true; } else { bReadNewRotator16 = false; } } }
}

void FServerAuthPhysicsSettings::SaveSettings(UGenMovementReplicationComponent* ReplicationComponent)
{
  bUseClientPrediction = ReplicationComponent->bUseClientPrediction;
  bSmoothRemoteServerPawn = ReplicationComponent->bSmoothRemoteServerPawn;
  InterpolationMode = ReplicationComponent->InterpolationMode;
  InterpolationMethod = ReplicationComponent->InterpolationMethod;
  ExtrapolationMode = ReplicationComponent->ExtrapolationMode;
}

void FServerAuthPhysicsSettings::ApplySettings(UGenMovementReplicationComponent* ReplicationComponent)
{
  ReplicationComponent->bUseClientPrediction = bUseClientPrediction;
  ReplicationComponent->bSmoothRemoteServerPawn = bSmoothRemoteServerPawn;
  ReplicationComponent->InterpolationMode = InterpolationMode;
  ReplicationComponent->InterpolationMethod = InterpolationMethod;
  ReplicationComponent->ExtrapolationMode = ExtrapolationMode;
}

bool FServerAuthPhysicsSettings::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
  static_assert((uint8)EInterpolationMode::MAX < 16, "Number of serialized bits need to be increased");
  static_assert((uint8)EInterpolationMethod::MAX < 16, "Number of serialized bits need to be increased");
  static_assert((uint8)EExtrapolationMode::MAX < 16, "Number of serialized bits need to be increased");

  if (Ar.IsSaving())
  {
    uint8 InterpolationModeByte = static_cast<uint8>(InterpolationMode);
    Ar.SerializeBits(&InterpolationModeByte, 4);
    uint8 InterpolationMethodByte = static_cast<uint8>(InterpolationMethod);
    Ar.SerializeBits(&InterpolationMethodByte, 4);
    uint8 ExtrapolationModeByte = static_cast<uint8>(ExtrapolationMode);
    Ar.SerializeBits(&ExtrapolationModeByte, 4);
  }
  else if (Ar.IsLoading())
  {
    Ar.SerializeBits(&InterpolationMode, 4);
    Ar.SerializeBits(&InterpolationMethod, 4);
    Ar.SerializeBits(&ExtrapolationMode, 4);
  }

  return (bOutSuccess = true);
}

void FClientAuthPhysicsSettings::SaveSettings(UGenMovementReplicationComponent* ReplicationComponent)
{
  bAssumeClientState = ReplicationComponent->bAssumeClientState;
  bUseClientLocation = ReplicationComponent->bUseClientLocation;
  bUseClientRotation = ReplicationComponent->bUseClientRotation;
  bUseClientControlRotation = ReplicationComponent->bUseClientControlRotation;
  bRollbackServerPawns = ReplicationComponent->bRollbackServerPawns;
  bRollbackClientPawns = ReplicationComponent->bRollbackClientPawns;
  bRollbackGenericServerActors = ReplicationComponent->bRollbackGenericServerActors;
  bRollbackGenericClientActors = ReplicationComponent->bRollbackGenericClientActors;
}

void FClientAuthPhysicsSettings::ApplySettings(UGenMovementReplicationComponent* ReplicationComponent)
{
  ReplicationComponent->bAssumeClientState = bAssumeClientState;
  ReplicationComponent->bUseClientLocation = bUseClientLocation;
  ReplicationComponent->bUseClientRotation = bUseClientRotation;
  ReplicationComponent->bUseClientControlRotation = bUseClientControlRotation;
  ReplicationComponent->bRollbackServerPawns = bRollbackServerPawns;
  ReplicationComponent->bRollbackClientPawns = bRollbackClientPawns;
  ReplicationComponent->bRollbackGenericServerActors = bRollbackGenericServerActors;
  ReplicationComponent->bRollbackGenericClientActors = bRollbackGenericClientActors;
}

bool FClientAuthPhysicsSettings::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
  Ar.SerializeBits(&bRollbackServerPawns, 1);
  Ar.SerializeBits(&bRollbackClientPawns, 1);
  Ar.SerializeBits(&bRollbackGenericServerActors, 1);
  Ar.SerializeBits(&bRollbackGenericClientActors, 1);

  return (bOutSuccess = true);
}
