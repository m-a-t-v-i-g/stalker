// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "GenRollbackActor.h"
#include "GenPlayerController.h"
#include "GenPawn.h"
#define GMC_ROLLBACK_ACTOR_LOG
#include "GMC_LOG.h"

GENERALMOVEMENT_API DEFINE_LOG_CATEGORY(LogGMCRollbackActor);

AGenRollbackActor::AGenRollbackActor()
{
  PrimaryActorTick.bCanEverTick = true;
  bAllowTickBeforeBeginPlay = false;
}

void AGenRollbackActor::BeginPlay()
{
  Super::BeginPlay();

  BaseVelocity = Velocity;
  BaseAngularVelocity = AngularVelocity;
  BaseTransform = GetActorTransform();
}

void AGenRollbackActor::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (IsNetMode(NM_Client))
  {
    Client_ManagePrerequisiteTicks();
  }

  if (!HasTicked())
  {
    UpdateState(GetTime(), DeltaTime, FMove{}, EActorRollbackContext::RegularTickUpdate);
  }

  // Reset the flag. It will be set by the replication component of the local pawn during the next frame if applicable.
  SetTicked(false);
}

FVector AGenRollbackActor::ComputeTangentialVelocity(const FVector& WorldLocation) const
{
  if (AngularVelocity.IsNearlyZero(KINDA_SMALL_NUMBER)) return FVector{0};
  const FVector AngularVelocityRad = FMath::DegreesToRadians(FVector{AngularVelocity.Roll, AngularVelocity.Pitch, AngularVelocity.Yaw});
  const FVector RadialDistanceToActor = WorldLocation - GetActorLocation();
  return AngularVelocityRad ^ RadialDistanceToActor;
}

float AGenRollbackActor::GetTime() const
{
  if (IsNetMode(NM_Client))
  {
    if (UGameInstance* GameInstance = GetGameInstance())
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

void AGenRollbackActor::SaveState_Implementation(EActorRollbackContext Context)
{
  StateBuffer.SetComponents(GetActorRotation().Quaternion(), GetActorLocation(), GetActorScale());
}

void AGenRollbackActor::LoadState_Implementation(EActorRollbackContext Context)
{
  SetActorTransform(StateBuffer, false, nullptr, ETeleportType::TeleportPhysics);
}

void AGenRollbackActor::Client_ManagePrerequisiteTicks()
{
  checkGMC(IsNetMode(NM_Client))

  const auto GenLocalPlayerController = GEngine ? Cast<AGenPlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld())) : nullptr;
  if (GenLocalPlayerController)
  {
    // The local controller must tick first to update the world time.
    AddTickPrerequisiteActor(GenLocalPlayerController);

    // The locally controlled client should usually update all rollback actors through its replication component meaning the replication
    // component needs to tick before the rollback actor. If the replication component did not call @see UpdateState on the actor during
    // this frame (e.g. because it doesn't exist) the state will be updated in the actor's regular tick function.
    const auto LocalPawn = Cast<AGenPawn>(GenLocalPlayerController->GetPawn());
    checkGMC(LocalPawn->GetLocalRole() == ROLE_AutonomousProxy)
    if (const auto ReplicationComponent = Cast<UGenMovementReplicationComponent>(LocalPawn->GetMovementComponent()))
    {
      AddTickPrerequisiteComponent(ReplicationComponent);
    }
  }
}
