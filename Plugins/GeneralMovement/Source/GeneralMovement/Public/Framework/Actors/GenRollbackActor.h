// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenMovementReplicationComponent.h"
#include "GenRollbackActor.generated.h"

GENERALMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogGMCRollbackActor, Log, All);

/// Base class for dynamic actors that are spawned authoritatively on both client and server and can somehow affect pawn movement through
/// their position, scale and/or orientation. All actors of this type in a level can be rolled back to a previous state for move execution
/// by @see UGenMovementReplicationComponent.
UCLASS(ABSTRACT, ClassGroup = "Movement", BlueprintType, Blueprintable)
class GENERALMOVEMENT_API AGenRollbackActor : public AActor
{
  GENERATED_BODY()

public:

  AGenRollbackActor();
  void BeginPlay() override;
  void Tick(float DeltaTime) override;
  FVector GetVelocity() const override { return Velocity; }

  /// Returns the angular velocity of the actor in deg/sec.
  ///
  /// @returns      FRotator    The angular velocity of the actor.
  UFUNCTION(BlueprintCallable, Category = "Movement")
  FRotator GetAngularVelocity() const { return AngularVelocity; }

  /// Sets the linear velocity of the actor.
  ///
  /// @param        NewVelocity    The new velocity the actor should have.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "Movement")
  void SetVelocity(const FVector& NewVelocity) { Velocity = NewVelocity; }

  /// Sets the angular velocity of the rollback actor.
  ///
  /// @param        NewVelocity    The new angular velocity the actor should have.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "Movement")
  void SetAngularVelocity(const FRotator& NewVelocity) { AngularVelocity = NewVelocity; }

  /// Calculates the tangential velocity that would act on an object at the passed position. Only relevant if the actor has any angular
  /// velocity.
  ///
  /// @param        WorldLocation    The location to calculate the tangential velocity at in world space.
  /// @returns      FVector          The tangential velocity at the passed location.
  UFUNCTION(BlueprintCallable, Category = "Movement")
  virtual FVector ComputeTangentialVelocity(const FVector& WorldLocation) const;

  /// Returns the current synchronised world time i.e. when called on the client the network delay is already accounted for.
  ///
  /// @returns      float    The current world time.
  UFUNCTION(BlueprintCallable, Category = "Movement")
  float GetTime() const;

  /// Overridable function to update the state of the rollback actor (called every frame). This function should be implemented in a way that
  /// the state of the actor can be recreated solely based on the passed arguments.
  ///
  /// @param        Time         The actor should be set to the state that corresponds to the passed time.
  /// @param        DeltaTime    The delta time to use.
  /// @param        Move         The move that is about to be executed.
  /// @param        Context      The context in which the actor state is being updated.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "Movement")
  void UpdateState(float Time, float DeltaTime, const FMove& Move, EActorRollbackContext Context);
  virtual void UpdateState_Implementation(float Time, float DeltaTime, const FMove& Move, EActorRollbackContext Context) {}

  /// Save the current state of the rollback actor.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "Movement")
  void SaveState(EActorRollbackContext Context);
  virtual void SaveState_Implementation(EActorRollbackContext Context);

  /// Load the currently saved state of the rollback actor.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "Movement")
  void LoadState(EActorRollbackContext Context);
  virtual void LoadState_Implementation(EActorRollbackContext Context);

  /// Setter for @see bTicked.
  ///
  /// @param        bNewValue    The new value.
  /// @returns      void
  void SetTicked(bool bNewValue) { bTicked = bNewValue; }

  /// Checks whether @see UpdateState has already been called on this actor during the current frame.
  /// @see UGenMovementReplicationComponent::TickGenericRollbackActors
  /// @see bTicked
  ///
  /// @returns      bool    True if the rollback actor have already been ticked this frame, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "Movement")
  bool HasTicked() const { return bTicked; }

protected:

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  /// The linear velocity of the actor in cm/sec.
  FVector Velocity{0.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
  /// The angular velocity of the actor in deg/sec.
  FRotator AngularVelocity{0.f, 0.f, 0.f};

  UPROPERTY(BlueprintReadWrite, Category = "Movement")
  /// The velocity the actor started with when it was spawned.
  FVector BaseVelocity{0};

  UPROPERTY(BlueprintReadWrite, Category = "Movement")
  /// The angular velocity the actor started with when it was spawned.
  FRotator BaseAngularVelocity{0};

  UPROPERTY(BlueprintReadWrite, Category = "Movement")
  /// The transform the actor had when it was spawned.
  FTransform BaseTransform;

  UPROPERTY(BlueprintReadWrite, Category = "Movement")
  /// Used to buffer the state of the actor via @see SaveState.
  FTransform StateBuffer;

  /// Manages the prerequisite ticks for this actor on the client.
  ///
  /// @returns      void
  virtual void Client_ManagePrerequisiteTicks();

private:

  /// Whether this actor's state has already been updated this frame.
  bool bTicked{false};
};
