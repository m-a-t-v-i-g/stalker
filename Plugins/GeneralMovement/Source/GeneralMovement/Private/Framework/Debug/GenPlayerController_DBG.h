// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#if ALLOW_CONSOLE && !NO_LOGGING

// Display the current ping to the server on a client machine.
#define DEBUG_STAT_PING\
    if (GMCCVars::StatPing != 0) {\
      if (GetLocalRole() == ROLE_AutonomousProxy) {\
        const UNetConnection* Connection = GetNetConnection();\
        float Ping = 0.f;\
        if (USE_PLAYER_STATE_PING) {\
          Ping = GetPingInMilliseconds();\
        }\
        else if (Connection) {\
          Ping = Connection->AvgLag * 1000.f;\
        }\
        DEBUG_PRINT_MSG(0, "ping: %.0f ms", Ping)\
        DEBUG_PRINT_MSG(0, "jitter: %.0f ms", Connection ? Connection->GetAverageJitterInMS() : 0.f)\
      }\
    }

// Log the current world time on the server and the synced world time on the client with a UTC timestamp for comparison.
#define DEBUG_LOG_NET_WORLD_TIME\
    if (GMCCVars::LogNetWorldTime != 0) {\
      if (GetLocalRole() == ROLE_Authority && IsLocalController()) {\
        if (const UWorld* World = GetWorld()) {\
          if (const AGameStateBase* GameState = World->GetGameState()) {\
            GMC_LOG(Log, TEXT("UtcNow: %s | ServerWorldTime = %12.6f s"),\
              *FDateTime::UtcNow().GetTimeOfDay().ToString(), GameState->GetServerWorldTimeSeconds())\
          }\
        }\
      }\
      if (GetLocalRole() == ROLE_AutonomousProxy) {\
        if (const UWorld* World = GetWorld()) {\
          if (const AGameStateBase* GameState = World->GetGameState()) {\
            float AvgRTT = NAN;\
            if (const UNetConnection* Connection = GetNetConnection()) {\
              AvgRTT = Connection->AvgLag;\
            }\
            GMC_LOG(Log, TEXT("UtcNow: %s | ClientWorldTime = %12.6f s | Ping = %4.0f ms"),\
              *FDateTime::UtcNow().GetTimeOfDay().ToString(), Client_SyncedWorldTime, AvgRTT / 2.f * 1000.f)\
          }\
        }\
      }\
    }

#else

  #define DEBUG_STAT_PING
  #define DEBUG_LOG_NET_WORLD_TIME

#endif
