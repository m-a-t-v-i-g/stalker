// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GenOrganicMovementComponent.h"
#include "GenGoldSrcMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EJumpMode : uint8
{
  Manual UMETA(DisplayName = "Manual", ToolTip = "The player can only jump if the jump button is pressed while already on the ground."),
  SemiAuto UMETA(DisplayName = "SemiAuto", ToolTip = "The player can jump by holding the jump button, but the button must be released between jumps."),
  FullAuto UMETA(DisplayName = "FullAuto", ToolTip = "The player can continuously jump by holding the jump button."),
  MAX UMETA(Hidden)
};

/// Movement component loosely based on the movement physics found in classic shooter games such as Quake and Half-Life.
UCLASS(ClassGroup = "Movement", HideCategories = ("Movement|Tempo"), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class GENERALMOVEMENT_API UGenGoldSrcMovementComponent : public UGenOrganicMovementComponent
{
  GENERATED_BODY()
  friend class UGoldSrcConfigMenu;

public:

  UGenGoldSrcMovementComponent();

  ///~ Begin AActor Interface
  void BeginPlay() override;
  ///~ End AActor Interface

  ///~ Begin UMovementComponent Interface
  void HaltMovement() override;
  FVector ComputeSlideVector(const FVector& Delta, float Time, const FVector& Normal, const FHitResult& Hit) const override;
  float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact = false) override;
  void TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const override;
  ///~ End UMovementComponent Interface

  /// Whether the pawn has just jumped. Only intended as animation trigger, should not be used for movement logic (use @see OnJumped
  /// instead).
  ///
  /// @returns      bool    Whether the pawn has just jumped.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool HasJustJumped() const;

  /// Whether the pawn has just landed on the ground. Only intended as animation trigger, should not be used for movement logic (use
  /// @see OnLanded or @see HasJustLanded_Internal instead).
  ///
  /// @returns      bool    Whether the pawn has just landed.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool HasJustLanded() const;

  /// Whether the pawn is currently sprinting.
  /// @see bIsSprinting
  ///
  /// @returns      bool    Whether the pawn is sprinting.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool IsSprinting() const;

  /// Returns the default half height of the pawn's root collision.
  ///
  /// @returns      float    The half height the root collision when the pawn spawned.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  float GetDefaultHalfHeight() const;

  /// Returns the half height of the pawn's root collision when fully crouched.
  ///
  /// @returns      float    The half height the root collision when the pawn is fully crouched.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  float GetCrouchedHalfHeight() const;

  /// Whether the pawn is fully crouched. Only returns true if the root collision's half height is equal to @see CrouchedHalfHeight.
  ///
  /// @returns      bool    True if the pawn is currently fully crouched, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool IsFullyCrouched() const;

  /// Whether the pawn is currently in the process of crouching down. Only returns true if the root collision's half height is smaller than
  /// @see DefaultHalfHeight and greater than @see CrouchedHalfHeight, and the crouch button (@see bWantsToCrouch) is being pressed.
  ///
  /// @returns      bool    True if the pawn is currently in the process of crouching down, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool IsCrouchingDown() const;

  /// Whether the pawn is currently in the process of uncrouching. Only returns true if the root collision's half height is greater than
  /// @see CrouchedHalfHeight and smaller than @see DefaultHalfHeight, and the crouch button (@see bWantsToCrouch) is not being pressed.
  ///
  /// @returns      bool    True if the pawn is currently in the process of uncrouching, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool IsUncrouching() const;

  /// Whether the pawn is using fly mode.
  ///
  /// @returns      bool    True when currently flying, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool IsUsingFlyMode() const;

  /// Whether the pawn is using noclip mode.
  ///
  /// @returns      bool    True when currently using noclip, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool IsUsingNoClipMode() const;

protected:

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedBool JustJumpedAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedBool JustLandedAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedBool IsSprintingAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedFloat MaxSpeedAccessor{};

  ///~ Begin UGenMovementReplicationComponent Interface
  void BindReplicationData_Implementation() override;
  ///~ End UGenMovementReplicationComponent Interface

  ///~ Begin UGenOrganicMovementComponent Interface
  void PreMovementUpdate_Implementation(float DeltaSeconds) override;
  void ClampToValidValues() override;
  void OnMovementModeChanged_Implementation(EGenMovementMode PreviousMovementMode) override;
  void MovementUpdate_Implementation(float DeltaSeconds) override;
  bool UpdateMovementModeDynamic_Implementation(const FFloorParams& Floor, float DeltaSeconds) override;
  void PostPhysicsUpdate_Implementation(float DeltaSeconds) override;
  void PhysicsCustom_Implementation(float DeltaSeconds) override;
  FVector PreProcessInputVector_Implementation(FVector RawInputVector) override;
  void CalculateVelocity(float DeltaSeconds) override;
  void ApplyInputVelocity(float DeltaSeconds) override;
  void ApplyDeceleration(float DeltaSeconds) override;
  void LimitSpeed(float DeltaSeconds) override;
  float GetInputAcceleration() const override;
  float GetBrakingDeceleration() const override;
  float GetOverMaxSpeedDeceleration() const override;
  float GetMaxSpeed() const override;
  void AdjustVelocityFromHitAirborne_Implementation(const FHitResult& Hit, float DeltaSeconds) override;
  void ProcessLanded(const FHitResult& Hit, float DeltaSeconds, bool bUpdateFloor = true) override;
  void OnLanded_Implementation() override;
  void PostMovementUpdate_Implementation(float DeltaSeconds) override;
  float CalculatePathFollowingBrakingDistance_Implementation(float InMaxSpeed, float DeltaSeconds) const;
  ///~ End UGenOrganicMovementComponent Interface

  /// Custom movement mode aliases.
  constexpr EGenMovementMode MovementModeFlying() const { return EGenMovementMode::Custom1; }
  constexpr EGenMovementMode MovementModeNoClip() const { return EGenMovementMode::Custom2; }

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// The half height of the pawn's root collision when just spawned.
  float DefaultHalfHeight{0.f};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// The collision type that the root component had when it just spawned.
  TEnumAsByte<ECollisionEnabled::Type> DefaultCollisionType;

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// True while the user is pressing the sprint button, false otherwise.
  bool bWantsToSprint{false};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// True while the user is pressing the crouch button, false otherwise.
  bool bWantsToCrouch{false};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// True while the user is pressing the jump button, false otherwise.
  bool bWantsToJump{false};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Flag to determine whether a jump is currently allowed.
  bool bCanJump{false};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Flag set when the pawn is sprinting.
  bool bIsSprinting{false};

  /// Whether the pawn has just jumped. Only intended as animation trigger, should not be used for logic (use @see OnJumped instead).
  bool bJustJumped{false};

  /// Whether the pawn has just landed. Since @see OnLanded can be called from multiple places we need more than just one variable to track
  /// the state accurately. These are set and cleared at different times during move execution. Use @see HasJustLanded for querying the
  /// state with respect to the current context.
  /// @attention "bJustLandedPrePhysics" and "bJustLandedPostPhysicsLock" do not need to be bound.
  bool bJustLandedPrePhysics{false};
  bool bJustLandedPostPhysics{false};
  bool bJustLandedPostPhysicsLock{false};
  bool bJustLanded{false};

  /// Implements movement physics similar to those used in GoldSource games.
  ///
  /// @param        WishMove        The directional input of the player.
  /// @param        WishVelocity    The velocity generated from the directional input.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void PhysicsGoldSrc(const FVector& WishMove, FVector WishVelocity, float DeltaSeconds);

  /// Calculates a new velocity for the pawn when on the ground.
  ///
  /// @param        WishDirection    The direction the player wants to move in.
  /// @param        WishSpeed        The speed the player wants to have based on the input.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void WalkMove(FVector WishDirection, float WishSpeed, float DeltaSeconds);

  /// Calculates a new velocity for the pawn when in the air.
  ///
  /// @param        WishDirection    The direction the player wants to move in.
  /// @param        WishSpeed        The speed the player wants to have based on the input.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void AirMove(FVector WishDirection, float WishSpeed, float DeltaSeconds);

  /// Calculates a new velocity for the pawn when in water.
  ///
  /// @param        WishDirection    The direction the player wants to move in.
  /// @param        WishSpeed        The speed the player wants to have based on the input.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void WaterMove(FVector WishDirection, float WishSpeed, float DeltaSeconds);

  /// Applies acceleration to calculate a new velocity for the pawn.
  ///
  /// @param        WishDirection    The direction the player wants to move in.
  /// @param        WishSpeed        The speed the player wants to have based on the input.
  /// @param        CurrentSpeed     The current speed of the pawn.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void Accelerate(FVector WishDirection, float WishSpeed, float CurrentSpeed, float DeltaSeconds);

  /// Applies friction while moving on the ground.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      float           The new speed of the pawn.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float ApplyFloorFriction(float DeltaSeconds);

  /// Returns the friction value to use when moving on the ground.
  /// @see FloorFriction
  /// @see EdgeFrictionScale
  ///
  /// @returns      float    The floor friction to use.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetFloorFriction() const;

  /// Applies friction while moving in a fluid.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      float           The new speed of the pawn.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float ApplyWaterFriction(float DeltaSeconds);

  /// Makes the pawn slide along a usually walkable surface if its velocity is high enough.
  ///
  /// @param        MinRequiredZ    The minimum Z of the projected velocity required to slide.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void Slide(float MinRequiredZ, float DeltaSeconds);

  /// Whether the pawn has just landed on the ground. Can be used for movement logic. For simulated pawns use @see HasJustLanded.
  ///
  /// @returns      bool    Whether the pawn has just landed.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool HasJustLanded_Internal() const;

  /// Increases the movement speed of the pawn to @see SprintSpeed if desired.
  ///
  /// @param        bRquestedSprint    Whether the player has pressed the sprint button.
  /// @param        DeltaSeconds       The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void Sprint(bool bRequestedSprint, float DeltaSeconds);
  virtual void Sprint_Implementation(bool bRequestedSprint, float DeltaSeconds);

  /// Whether the pawn is currently allowed to sprint (irrespective of whether the player wants to sprint or not).
  ///
  /// @returns      bool    True if the pawn can currently sprint, false otherwise.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  bool CanSprint() const;
  virtual bool CanSprint_Implementation() const;

  /// Smoothly shrinks the collision half height towards @see CrouchedHalfHeight or increases it towards @see DefaultHalfHeight.
  ///
  /// @param        bRequestedCrouch    Whether the player has pressed the crouch button.
  /// @param        DeltaSeconds        The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void Crouch(bool bRequestedCrouch, float DeltaSeconds);
  virtual void Crouch_Implementation(bool bRequestedCrouch, float DeltaSeconds);

  /// Whether the pawn is currently allowed to crouch (irrespective of whether the player wants to crouch or not).
  ///
  /// @returns      bool    True if the pawn can currently crouch, false otherwise.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  bool CanCrouch() const;
  virtual bool CanCrouch_Implementation() const;

  /// Handles the jumping logic.
  ///
  /// @param        bRequestedJump    Whether the player has pressed the jump button.
  /// @param        DeltaSeconds      The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void Jump(bool bRequestedJump, float DeltaSeconds);
  virtual void Jump_Implementation(bool bRequestedJump, float DeltaSeconds);

  /// Whether the pawn is currently allowed to jump (irrespective of whether the player wants to jump or not).
  ///
  /// @returns      bool    True if the pawn can currently jump, false otherwise.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  bool CanJump() const;
  virtual bool CanJump_Implementation() const;

  /// Handles the logic for jumping out of water.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void WaterJump(float DeltaSeconds);
  virtual void WaterJump_Implementation(float DeltaSeconds);

  /// Calculates and applies the jump speed boost.
  ///
  /// @param        DeltaSeconds      The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void ApplyJumpSpeedBoost(float DeltaSeconds);
  virtual void ApplyJumpSpeedBoost_Implementation(float DeltaSeconds);

  /// Called when the pawn is just about to jump. The movement mode at this point is still grounded but will will be changed to airborne in
  /// the next frame.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void OnJumped();
  virtual void OnJumped_Implementation() {}

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "1", UIMax = "1000"))
  /// Acceleration of the pawn when not in the air.
  float Acceleration{10.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "100", UIMax = "2000"))
  /// Soft cap on the movement speed of the pawn.
  float MaxSpeed{600.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "500"))
  /// How quickly the pawn comes to a stop when there is no directional input.
  float StopSpeed{200.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "100", UIMax = "5000"))
  /// The speed the pawn should have when sprinting.
  float SprintSpeed{900.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "100", UIMax = "2000"))
  /// How fast the pawn can walk when crouching while grounded.
  float CrouchWalkSpeed{400.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "100", UIMax = "1000"))
  /// How fast the pawn can (un)crouch.
  float CrouchInterpSpeed{500.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0.0001", UIMin = "1"))
  /// What the half height of the pawn's root collision should be when fully crouched. Crouching is only possible if the root collision is
  /// a vertical capsule or a box. Will be clamped if larger than the default root collision half height.
  float CrouchedHalfHeight{60.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "1", UIMax = "10"))
  /// The amount of friction applied when moving on the ground.
  float FloorFriction{5.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "1", UIMax = "5"))
  /// Multiplier applied to the floor friction when the pawn is close to a drop-off.
  float EdgeFrictionScale{2.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "10", UIMax = "100"))
  /// How high a ledge must be at the least for edge friction to be applied.
  float EdgeFrictionMinRequiredHeight{50.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground")
  /// If false the edge friction multiplier will be applied as soon as the outer edge of the collision shape is over the ledge. If true the
  /// edge friction multiplier will be applied only after the center of the pawn's collision shape is already over the ledge.
  bool bUseAltEdgeFriction{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground")
  /// Determines the behaviour of the jumping mechanic.
  EJumpMode JumpMode{EJumpMode::SemiAuto};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "300", UIMax = "2000"))
  /// The strength of the pawn's jump.
  float JumpForce{500.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground")
  /// Whether the pawn should receive a forward speed boost when jumping. Allows gaining speed through "Accelerated Back Hopping" but also
  /// prevents strafe jumping.
  bool bApplyJumpSpeedBoost{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "0.01", UIMax = "1", EditCondition = "bApplyJumpSpeedBoost"))
  /// The percentage of the max speed applied as jump speed boost when the pawn is sprinting or fully crouched.
  float SmallJumpSpeedBoostFactor{0.1f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|On Ground", meta =
    (ClampMin = "0", UIMin = "0.01", UIMax = "1", EditCondition = "bApplyJumpSpeedBoost"))
  /// The percentage of the max speed applied as jump speed boost when the pawn is neither sprinting nor fully crouched.
  float LargeJumpSpeedBoostFactor{0.5f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Air", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "1000"))
  /// Acceleration of the pawn when in the air.
  float AirAcceleration{20.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Air", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "1"))
  /// Multiplier limiting the pawn's speed while in the air.
  float AirControl{0.1f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Water", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "1"))
  /// Multiplier limiting the pawn's speed while in water.
  float WaterControl{0.5f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Water", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "500"))
  /// How quickly the pawn sinks in water when there is no directional input.
  float SinkSpeed{150.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Water", meta =
    (ClampMin = "0", UIMin = "300", UIMax = "2000"))
  /// The strength of the pawn's jump when trying to jump out of water.
  float WaterJumpForce{700.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Water", meta =
    (ClampMin = "0", UIMin = "0", UIMax = "2000"))
  /// The strength of the forward push when the pawn's trying to jump out of water.
  float WaterJumpPush{600.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|In Water")
  /// If false swimming controls with use an "AbsoluteZ" input scheme. If true swimming controls will be fully relative to the player's view
  /// rotation.
  bool bUseAltSwimControls{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay, meta =
    (ClampMin = "0", UIMin = "0"))
  /// The absolute max speed that the pawn can achieve under any circumstances. Setting this to 0 disables the hard cap.
  float SpeedHardCap{10000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// If true the configured mass of the pawn will affect the height of the jump.
  bool bConsiderMassOnJump{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|Action Mappings")
  /// The ID for the mapped sprint action.
  FName ID_Sprint{"Sprint"};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|Action Mappings")
  /// The ID for the mapped crouch action.
  FName ID_Crouch{"Crouch"};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|GoldSrc|Action Mappings")
  /// The ID for the mapped jump action.
  FName ID_Jump{"Jump"};
};

FORCEINLINE bool UGenGoldSrcMovementComponent::HasJustJumped() const
{
  return bJustJumped;
}

FORCEINLINE bool UGenGoldSrcMovementComponent::HasJustLanded() const
{
  return bJustLanded;
}

FORCEINLINE bool UGenGoldSrcMovementComponent::HasJustLanded_Internal() const
{
  return bJustLandedPrePhysics || bJustLandedPostPhysics;
}

FORCEINLINE bool UGenGoldSrcMovementComponent::IsSprinting() const
{
  return bIsSprinting;
}

FORCEINLINE float UGenGoldSrcMovementComponent::GetDefaultHalfHeight() const
{
  return DefaultHalfHeight;
}

FORCEINLINE float UGenGoldSrcMovementComponent::GetCrouchedHalfHeight() const
{
  return CrouchedHalfHeight;
}

FORCEINLINE bool UGenGoldSrcMovementComponent::IsUsingFlyMode() const
{
  return MovementMode == static_cast<uint8>(MovementModeFlying());
}

FORCEINLINE bool UGenGoldSrcMovementComponent::IsUsingNoClipMode() const
{
  return MovementMode == static_cast<uint8>(MovementModeNoClip());
}

FORCEINLINE bool UGenGoldSrcMovementComponent::IsFullyCrouched() const
{
  return FMath::IsNearlyEqual(GetRootCollisionHalfHeight(), CrouchedHalfHeight, 0.1f);
}

FORCEINLINE bool UGenGoldSrcMovementComponent::IsCrouchingDown() const
{
  const float CurrentHalfHeight = GetRootCollisionHalfHeight();
  return bWantsToCrouch && CurrentHalfHeight < DefaultHalfHeight && CurrentHalfHeight > CrouchedHalfHeight;
}

FORCEINLINE bool UGenGoldSrcMovementComponent::IsUncrouching() const
{
  const float CurrentHalfHeight = GetRootCollisionHalfHeight();
  return !bWantsToCrouch && CurrentHalfHeight > CrouchedHalfHeight && CurrentHalfHeight < DefaultHalfHeight;
}
