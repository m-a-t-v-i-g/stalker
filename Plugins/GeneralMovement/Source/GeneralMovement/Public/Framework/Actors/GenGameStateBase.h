// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GenGameStateBase.generated.h"

/// Game state class which should be used in connection with @see AGenPlayerController to ensure accurate synchronisation of the server
/// world time on the client.
UCLASS()
class GENERALMOVEMENT_API AGenGameStateBase : public AGameStateBase
{
  GENERATED_BODY()

public:

  AGenGameStateBase();

  /// Returns the value of @see ReplicatedWorldTimeSeconds which is set periodically on the server. The frequency is determined by the value
  /// of @see AGameStateBase::ServerWorldTimeSecondsUpdateFrequency.
  ///
  /// @returns      float    The value of @see ReplicatedWorldTimeSeconds.
  UFUNCTION(BlueprintCallable, Category = "GameState")
  float GetLastReplicatedServerWorldTimeSeconds() const;

protected:

  /// Called when the client receives an updated value for @see ReplicatedWorldTimeSeconds which triggers the time synchronisation process
  /// (@see AGenPlayerController::Client_SyncWithServerTime).
  ///
  /// @returns      void
  void OnRep_ReplicatedWorldTimeSecondsDouble() override;

  /// Updates @see ReplicatedWorldTimeSeconds periodically on the server (@see AGameStateBase::ServerWorldTimeSecondsUpdateFrequency).
  ///
  /// @returns      void
  void UpdateServerTimeSeconds() override;
};
