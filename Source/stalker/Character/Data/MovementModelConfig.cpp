// Fill out your copyright notice in the Description page of Project Settings.

#include "MovementModelConfig.h"

FCharacterMovementModel UMovementModelConfig::GetMovementSettings(const ECharacterRotationMode RotationMode,
                                                                  const ECharacterStanceType Stance) const
{
	switch (RotationMode)
	{
	case ECharacterRotationMode::ControlDirection:
		{
			if (Stance == ECharacterStanceType::Standing)
			{
				return ControlDirection.Standing;
			}
			if (Stance == ECharacterStanceType::Crouching)
			{
				return ControlDirection.Crouching;
			}
		}
	case ECharacterRotationMode::LookingDirection:
		{
			if (Stance == ECharacterStanceType::Standing)
			{
				return LookingDirection.Standing;
			}
			if (Stance == ECharacterStanceType::Crouching)
			{
				return LookingDirection.Crouching;
			}
		}
	case ECharacterRotationMode::VelocityDirection:
		{
			if (Stance == ECharacterStanceType::Standing)
			{
				return VelocityDirection.Standing;
			}
			if (Stance == ECharacterStanceType::Crouching)
			{
				return VelocityDirection.Crouching;
			}
		}
	default: return VelocityDirection.Standing;
	}
}
