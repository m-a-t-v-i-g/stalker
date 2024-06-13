// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "GenPlayerController.h"

GAME_STATE_CLASS::GAME_STATE_CLASS()
{
  ServerWorldTimeSecondsUpdateFrequency = 0.1f;
}

float GAME_STATE_CLASS::GetLastReplicatedServerWorldTimeSeconds() const
{
#if UE_VERSION_OLDER_THAN(5, 2, 0)
  return ReplicatedWorldTimeSeconds;
#else
  return ReplicatedWorldTimeSecondsDouble;
#endif
}

#if UE_VERSION_OLDER_THAN(5, 2, 0)
void GAME_STATE_CLASS::OnRep_ReplicatedWorldTimeSeconds()
#else
void GAME_STATE_CLASS::OnRep_ReplicatedWorldTimeSecondsDouble()
#endif
{
#if UE_VERSION_OLDER_THAN(5, 2, 0)
  Super::OnRep_ReplicatedWorldTimeSeconds();
#else
  Super::OnRep_ReplicatedWorldTimeSecondsDouble();
#endif

  if (UGameInstance* GameInstance = GetGameInstance())
  {
    AGenPlayerController* LocalController = Cast<AGenPlayerController>(GameInstance->GetFirstLocalPlayerController());
    if (LocalController)
    {
      LocalController->Client_SyncWithServerTime();
    }
  }
}

void GAME_STATE_CLASS::UpdateServerTimeSeconds()
{
  if (UWorld* World = GetWorld())
  {
#if UE_VERSION_OLDER_THAN(5, 2, 0)
    ReplicatedWorldTimeSeconds = World->GetTimeSeconds();
#else
    ReplicatedWorldTimeSecondsDouble = World->GetTimeSeconds();
#endif

    // @attention We need to replicate the server time as quickly as possible to the client to keep the discrepancy not caused by network
    // latency minimal.
    ForceNetUpdate();
  }
}
