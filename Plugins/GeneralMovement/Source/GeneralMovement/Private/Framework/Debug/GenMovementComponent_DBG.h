// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#undef FLog
#undef CFLog

#if !NO_LOGGING

// Utility macros to quickly log messages from general movement component functions.
#define FLog(Verbosity, Format, ...)\
  GMC_LOG(Verbosity, TEXT("UGenMovementComponent::%s: ") TEXT(Format), *FString(__func__), ##__VA_ARGS__)
#define CFLog(Condition, Verbosity, Format, ...)\
  GMC_CLOG(Condition, Verbosity, TEXT("UGenMovementComponent::%s: ") TEXT(Format), *FString(__func__), ##__VA_ARGS__)

#else

#define FLog(Verbosity, Format, ...)
#define CFLog(Condition, Verbosity, Format, ...)

#endif

#if ALLOW_CONSOLE && !NO_LOGGING

// Visualize current velocity and acceleration of the pawn.
#define DEBUG_GMC_SHOW_MOVEMENT_VECTORS\
  if (GMCCVars::ShowMovementVectors != 0) {\
    if (const auto World = GetWorld()) {\
      const FVector CurrentLocation = PawnOwner->GetActorLocation();\
      constexpr float ArrowScale = 0.06f;\
      /*Velocity*/\
      DrawDebugDirectionalArrow(\
        World,\
        CurrentLocation,\
        CurrentLocation + (Velocity * ArrowScale),\
        100.f,\
        FColor::Blue,\
        false,\
        0.f,\
        ESceneDepthPriorityGroup::SDPG_Foreground,\
        2.5f\
      );\
      /*Acceleration*/\
      DrawDebugDirectionalArrow(\
        World,\
        CurrentLocation,\
        CurrentLocation + (Acceleration * ArrowScale),\
        100.f,\
        FColor::Red,\
        false,\
        0.f,\
        ESceneDepthPriorityGroup::SDPG_Foreground,\
        2.5f\
      );\
    }\
  }

#define DEBUG_GMC_LOG_AUTO_RESOLVE_PENETRATION_UP_SWEEP\
  if (Hit.bBlockingHit)\
  {\
    FLog(VeryVerbose, "Hit result upward sweep: "\
      "Hit.bBlockingHit = %s, "\
      "Hit.bStartPenetrating = %s, "\
      "Hit.Actor = %s, "\
      "Hit.Component = %s, "\
      "Hit.Distance = %f, "\
      "Hit.Time = %f, "\
      "Hit.PenetrationDepth = %f, "\
      "Hit.ImpactPoint = {%f, %f, %f}, "\
      "Hit.ImpactNormal = {%f, %f, %f}, "\
      "Hit.Location = {%f, %f, %f}, "\
      "Hit.Normal = {%f, %f, %f}, "\
      "Hit.TraceStart = {%f, %f, %f}, "\
      "Hit.TraceEnd = {%f, %f, %f}",\
      BOOL2STR(Hit.bBlockingHit),\
      BOOL2STR(Hit.bStartPenetrating),\
      Hit.GetActor() ? *Hit.GetActor()->GetHumanReadableName() : TEXT("nullptr"),\
      Hit.GetComponent() ? *Hit.GetComponent()->GetReadableName() : TEXT("nullptr"),\
      Hit.Distance,\
      Hit.Time,\
      Hit.PenetrationDepth,\
      Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z,\
      Hit.ImpactNormal.X, Hit.ImpactNormal.Y, Hit.ImpactNormal.Z,\
      Hit.Location.X, Hit.Location.Y, Hit.Location.Z,\
      Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z,\
      Hit.TraceStart.X, Hit.TraceStart.Y, Hit.TraceStart.Z,\
      Hit.TraceEnd.X, Hit.TraceEnd.Y, Hit.TraceEnd.Z\
    )\
  }

#define DEBUG_GMC_LOG_AUTO_RESOLVE_PENETRATION_DOWN_SWEEP\
  if (Hit.bBlockingHit)\
  {\
    FLog(VeryVerbose, "Hit result downward sweep: "\
      "Hit.bBlockingHit = %s, "\
      "Hit.bStartPenetrating = %s, "\
      "Hit.Actor = %s, "\
      "Hit.Component = %s, "\
      "Hit.Distance = %f, "\
      "Hit.Time = %f, "\
      "Hit.PenetrationDepth = %f, "\
      "Hit.ImpactPoint = {%f, %f, %f}, "\
      "Hit.ImpactNormal = {%f, %f, %f}, "\
      "Hit.Location = {%f, %f, %f}, "\
      "Hit.Normal = {%f, %f, %f}, "\
      "Hit.TraceStart = {%f, %f, %f}, "\
      "Hit.TraceEnd = {%f, %f, %f}",\
      BOOL2STR(Hit.bBlockingHit),\
      BOOL2STR(Hit.bStartPenetrating),\
      Hit.GetActor() ? *Hit.GetActor()->GetHumanReadableName() : TEXT("nullptr"),\
      Hit.GetComponent() ? *Hit.GetComponent()->GetReadableName() : TEXT("nullptr"),\
      Hit.Distance,\
      Hit.Time,\
      Hit.PenetrationDepth,\
      Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z,\
      Hit.ImpactNormal.X, Hit.ImpactNormal.Y, Hit.ImpactNormal.Z,\
      Hit.Location.X, Hit.Location.Y, Hit.Location.Z,\
      Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z,\
      Hit.TraceStart.X, Hit.TraceStart.Y, Hit.TraceStart.Z,\
      Hit.TraceEnd.X, Hit.TraceEnd.Y, Hit.TraceEnd.Z\
    )\
  }

namespace GMCCVars { extern int32 ShowFloorSweep; }
#define DEBUG_OMC_SHOW_FLOOR_SWEEP\
  if (GMCCVars::ShowFloorSweep != 0)\
  {\
    const FTransform& Transform = UpdatedComponent->GetComponentTransform();\
    const FVector ShapeTraceStart = Transform.GetLocation();\
    const FVector ShapeTraceEnd = ShapeTraceStart + FVector::DownVector * TraceLength;\
    const EGenCollisionShape CollisionShape = GetRootCollisionShape();\
    const FCollisionShape TraceShape = GetFrom(CollisionShape, GetRootCollisionExtent());\
    FQuat TraceRotation = AddGenCapsuleRotation(UpdatedComponent->GetComponentQuat());\
    /*Draw the line trace.*/\
    DrawDebugLine(\
      GetWorld(),\
      LineTraceStart,\
      LineHit.bBlockingHit ? LineHit.ImpactPoint : LineTraceEnd,\
      LineHit.bBlockingHit ? LineHit.bStartPenetrating ? FColor::Red : FColor::Green : FColor::Orange,\
      false,\
      0.f,\
      ESceneDepthPriorityGroup::SDPG_Foreground,\
      3.f\
    );\
    /*Draw the shape trace.*/\
    switch (CollisionShape)\
    {\
      case EGenCollisionShape::VerticalCapsule:\
      case EGenCollisionShape::HorizontalCapsule:\
        DrawDebugCapsule(\
          GetWorld(),\
          ShapeHit.bBlockingHit ? ShapeHit.Location : ShapeTraceEnd,\
          TraceShape.GetCapsuleHalfHeight(),\
          TraceShape.GetCapsuleRadius(),\
          TraceRotation,\
          ShapeHit.bBlockingHit ? ShapeHit.bStartPenetrating ? FColor::Red : FColor::Green : FColor::Orange,\
          false,\
          0.f,\
          ESceneDepthPriorityGroup::SDPG_Foreground\
        );\
        break;\
      case EGenCollisionShape::Box:\
        DrawDebugBox(\
          GetWorld(),\
          ShapeHit.bBlockingHit ? ShapeHit.Location : ShapeTraceEnd,\
          TraceShape.GetExtent(),\
          TraceRotation,\
          ShapeHit.bBlockingHit ? ShapeHit.bStartPenetrating ? FColor::Red : FColor::Green : FColor::Orange,\
          false,\
          0.f,\
          ESceneDepthPriorityGroup::SDPG_Foreground\
        );\
        break;\
      case EGenCollisionShape::Sphere:\
        DrawDebugSphere(\
          GetWorld(),\
          ShapeHit.bBlockingHit ? ShapeHit.Location : ShapeTraceEnd,\
          TraceShape.GetSphereRadius(),\
          64,\
          ShapeHit.bBlockingHit ? ShapeHit.bStartPenetrating ? FColor::Red : FColor::Green : FColor::Orange,\
          false,\
          0.f,\
          ESceneDepthPriorityGroup::SDPG_Foreground\
        );\
        break;\
      case EGenCollisionShape::Invalid:\
      case EGenCollisionShape::MAX:\
      default: checkNoEntry();\
    }\
    if (ShapeHit.bBlockingHit)\
    {\
      /*Shape hit impact point*/\
      DrawDebugPoint(\
        GetWorld(),\
        ShapeHit.ImpactPoint,\
        10.f,\
        ShapeHit.bStartPenetrating ? FColor::Red : FColor::Green,\
        false,\
        0.f,\
        ESceneDepthPriorityGroup::SDPG_Foreground\
      );\
    }\
    if (LineHit.bBlockingHit)\
    {\
      /*Line hit impact point*/\
      DrawDebugPoint(\
        GetWorld(),\
        LineHit.ImpactPoint,\
        10.f,\
        LineHit.bStartPenetrating ? FColor::Red : FColor::Green,\
        false,\
        0.f,\
        ESceneDepthPriorityGroup::SDPG_Foreground\
      );\
    }\
  }

#else

#define DEBUG_GMC_SHOW_MOVEMENT_VECTORS
#define DEBUG_GMC_LOG_AUTO_RESOLVE_PENETRATION_UP_SWEEP
#define DEBUG_GMC_LOG_AUTO_RESOLVE_PENETRATION_DOWN_SWEEP
#define DEBUG_OMC_SHOW_FLOOR_SWEEP

#endif
