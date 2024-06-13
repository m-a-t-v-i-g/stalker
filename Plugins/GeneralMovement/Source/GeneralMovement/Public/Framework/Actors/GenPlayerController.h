// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenPlayerController.generated.h"

GENERALMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogGMCController, Log, All);

/// Controller class intended to be used with @see UGenMovementReplicationComponent.
UCLASS(BlueprintType, Blueprintable)
class GENERALMOVEMENT_API AGenPlayerController : public APlayerController
{
  GENERATED_BODY()

public:

  AGenPlayerController();
  void BeginPlay() override;
  void PlayerTick(float DeltaTime) override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  void AddRollInput(float Val) override;
  void AddPitchInput(float Val) override;
  void AddYawInput(float Val) override;
  void UpdateCameraManager(float DeltaSeconds) override;

#pragma region Console Commands

  UFUNCTION(Exec)
  /// Set the interpolation delay for remotely controlled pawns on a local machine.
  void SetInterpolationDelay(float Delay) const;

  UFUNCTION(Exec)
  /// Set the interpolation method for remotely controlled pawns on a local machine (@see EInterpolationMethod).
  void SetInterpolationMethod(int32 Method) const;

  UFUNCTION(Exec)
  /// Set whether extrapolation is allowed for remotely controlled pawns on a local machine when no recent enough states available.
  void SetExtrapolationAllowed(bool bAllowed) const;

  UFUNCTION(Exec)
  /// Set whether the root collision of remotely controlled pawns on a local machine should be smoothed.
  void SetSmoothCollision(bool bSmoothLocation, bool bSmoothRotation) const;

#pragma endregion

private:

  /// Whether we want to retrieve the ping from the player state or from the net connection.
  static constexpr bool USE_PLAYER_STATE_PING = true;

  /// Whether the camera manager update for the autonomous proxy was deferred.
  bool bDeferredAutonomousProxyCameraManagerUpdate{false};

public:

  /// Getter for @see bDeferredAutonomousProxyCameraManagerUpdate. The camera manager update may be deferred during a smoothed correction
  /// on the client.
  ///
  /// @returns      bool    True if the camera manager update was deferred, false otherwise.
  bool WasAutonomousProxyCameraManagerUpdateDeferred() const;

  /// Called after the correction buffer is swapped at the end of a frame for the autonomous proxy during a smooth correction. Used to
  /// perform additional setup on the camera before rendering.
  ///
  /// @param        DeltaTime    The current frame delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component",
    meta = (DisplayName = "Update Camera For Client Correction"))
  void Client_UpdateCameraForCorrection(float DeltaTime);
  virtual void Client_UpdateCameraForCorrection_Implementation(float DeltaTime);

  /// Routing RPC to inform a client pawn of new simulation parameters when using an adaptive simulation delay for interpolation
  /// (@see UGenMovementReplicationComponent::InterpolationMode). We need to route this call through the client's player controller because
  /// simulated proxies are not client-owned and therefore can't receive RPCs directly from the server. The actual implementation for this
  /// is in the replication component (@see UGenMovementReplicationComponent::Client_SADSendParameters).
  ///
  /// @param        TargetComponent    The replication component that is the target for this routing call.
  /// @param        NextDelayValue     The new client simulation delay.
  /// @param        NextSyncTime       The time at which to committ the new simulation delay.
  /// @returns      void
  UFUNCTION(Client, Reliable)
  void Client_SendParametersForAdaptiveSimulationDelay(
    UGenMovementReplicationComponent* TargetComponent,
    float NextDelayValue,
    float NextSyncTime
  ) const;
  void Client_SendParametersForAdaptiveSimulationDelay_Implementation(
    UGenMovementReplicationComponent* TargetComponent,
    float NextDelayValue,
    float NextSyncTime
  ) const;

#pragma region Rotation Input

  /// Returns the accumulated view rotation input for the current frame. The rotation input rotator is different from regular rotators in
  /// the sense that it is not interpreted as an orientation meaning revolutions and direction are meaningful (the rotation input should
  /// never be normalized). The absolute value of a component can theoretically be arbitrarily high but in reality the components are
  /// usually between 0 and +/- 100, but may be as large as +/- 500 with low frame rate and high mouse sensitivity. To get good results when
  /// compressing the components for replication we clamp their values to @see UGenMovementReplicationComponent::MAX_ROTATION_INPUT.
  /// @attention The returned value is only valid after the player tick has already run (otherwise it will still reflect the value from the
  /// previous frame).
  ///
  /// @returns      FRotator    The per axis rotation input for this frame.
  UFUNCTION(BlueprintCallable, Category = "Input")
  FRotator GetRotationInput() const;

private:

  /// Contains the saved rotation input for the current frame. The rotation input gets cleared after the actor tick has finished but we
  /// still want to use it in the replication component which ticks after the player controller.
  FRotator SavedRotationInput{0};

  /// Saves the rotation input for replication (@see SavedRotationInput).
  ///
  /// @returns      void
  virtual void SaveRotationInput();

#pragma endregion

#pragma region Client Time Syncronisation

public:

  /// Returns the synchronised server world time for a client. Considers network delay meaning the returned value will be nearly the same as
  /// the world time on the server at any given moment.
  ///
  /// @returns      float    The current synchronised server world time.
  float Client_GetSyncedWorldTimeSeconds() const;

  /// Returns exact ping from the player state if available (for local players or when running on the server) or the replicated compressed
  /// ping (converted back to milliseconds) otherwise.
  ///
  /// @returns      float    The current ping in milliseconds.
  float GetPingInMilliseconds() const;

  /// Queries and synchronises the client world time with the server world time if necessary (accounts for network delay).
  ///
  /// @returns      void
  void Client_SyncWithServerTime();
  void Client_SyncWithServerTime_DEPRECATED();

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Client Time Sync", meta =
    (ClampMin = "0.1", UIMin = "1", UIMax = "60"))
  /// DEPRECATED: Use GenGameState(Base) as game state class to use the new time synchronisation process.
  /// The interval in seconds at which the client should query the server world time. The client time will not be synchronised unless the
  /// difference exceeds the set threshold. Only used if the game state class is not of type GenGameState. If it is the frequency is
  /// determined by the "ServerWorldTimeSecondsUpdateFrequency" of the game state.
  float TimeSyncInterval{5.f};

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Client Time Sync", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "60"))
  /// DEPRECATED: Use GenGameState(Base) as game state class to use the new time synchronisation process.
  /// The delay before the first synchronisation with the server world time in seconds. Gives the net connection object some time to
  /// calculate an accurate average round trip time value. Only used if the game state class is not of type GenGameState.
  float InitialTimeSyncDelay{1.f};

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Client Time Sync", meta =
    (ClampMin = "0", ClampMax = "1", UIMin = "0.05", UIMax = "1"))
  /// The max ping that a client is expected to have (in seconds). This is not enforced but if a client has a higher ping than this the
  /// local world time will desync which can create all sorts of problems. Do not set this arbitrarily high or low, it is recommended to
  /// leave this at the default value.
  float MaxExpectedPing{1.f};

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Client Time Sync", meta =
    (ClampMin = "0", UIMin = "0.05", UIMax = "0.5"))
  /// The maximum acceptable difference in seconds between the local client world time and the authoritative server world time. Syncing
  /// (i.e. overwriting) the local time with the server time can create inconsistencies for consecutive timestamps which can cause stutter
  /// for the client. Note that the optimal value here strongly depends on "ServerWorldTimeSecondsUpdateFrequency" in the game state.
  float MaxClientTimeDifferenceHardLimit{0.15f};

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Client Time Sync", meta =
    (ClampMin = "0", UIMin = "0.01", UIMax = "0.1"))
  /// If the difference in seconds between the local client world time and the authoritative server world time exceeds this threshold the
  /// client time will be adjusted towards the server time by increasing or shortening the local delta time value for the next update. This
  /// will prevent inconsistencies in the client time while still reducing the time deviation. Regardless of the configured value, the lower
  /// bound for the soft limit is always the current frame delta time.
  float MaxClientTimeDifferenceSoftLimit{0.05f};

protected:

  /// Calculates the time sync adjustment value to use.
  ///
  /// @returns      float    The time sync adjustment value to use.
  virtual float Client_GetTimeSyncAdjustment() const;

private:

  /// Tracks the client world time by adding the current delta time to the world time every frame. The client time is synced at certain
  /// interval with the authoritative server world time (@see Client_SyncWithServerTime).
  ///
  /// @param        DeltaTime    The current delta time.
  /// @returns      void
  void Client_UpdateWorldTime(float DeltaTime);

  /// @deprecated The timer handle for world time syncing.
  FTimerHandle Client_TimeSyncHandle;

  /// The time in seconds since the client world was brought up. Synchronised with the server time.
  float Client_SyncedWorldTime{0.f};

  /// What the time was during the last frame so we can identify inconsistent timestamps (can happen after the time was just synced).
  float Client_SyncedTimeLastFrame{0.f};

  /// If set only a fraction of the delta time (@see TimeSyncAdjustment) will be added to the client world time during the next client time
  /// update (@see Client_UpdateWorldTime). This is done to preemptively bring the client time closer to the server time.
  bool Client_bSlowWorldTime{false};

  /// If set a higher delta time value (@see TimeSyncAdjustment) will be added to the client world time during the next client time update
  /// (@see Client_UpdateWorldTime). This is done to preemptively bring the client time closer to the server time.
  bool Client_bSpeedUpWorldTime{false};

  /// Flag to signal to the controller that the delta time should not be added to the synchronised world time this frame. Set when the
  /// server world time was received in which case @see AGenGameState::OnRep_ReplicatedWorldTimeSeconds is called. OnRep functions are
  /// processed before actors are ticked, and the received server world time is already the updated time for this frame.
  bool Client_bDoNotUpdateWorldTime{false};

#pragma endregion
};
