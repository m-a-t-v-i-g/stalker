// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_RangedWeaponFire.h"
#include "AbilitySystemComponent.h"
#include "BulletBase.h"
#include "CharacterWeaponComponent.h"
#include "StalkerCharacter.h"
#include "StalkerGameplayTags.h"
#include "Ammo/AmmoObject.h"
#include "Weapons/WeaponObject.h"

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	return Dir.GetSafeNormal();
}

UStalkerGameplayAbility_RangedWeaponFire::UStalkerGameplayAbility_RangedWeaponFire(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	ActivationPolicy = EAbilityActivationPolicy::WhileInputActive;
}

UWeaponObject* UStalkerGameplayAbility_RangedWeaponFire::GetWeaponObject() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UWeaponObject>(Spec->SourceObject.Get());
	}
	return nullptr;
}

UCharacterWeaponComponent* UStalkerGameplayAbility_RangedWeaponFire::GetWeaponComponent() const
{
	if (AStalkerCharacter* StalkerPawn = Cast<AStalkerCharacter>(GetAvatarActorFromActorInfo()))
	{
		return StalkerPawn->GetWeaponComponent();
	}
	return nullptr;
}

void UStalkerGameplayAbility_RangedWeaponFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                               const FGameplayAbilityActorInfo* ActorInfo,
                                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                                               const FGameplayEventData* TriggerEventData)
{
	UWeaponObject* WeaponObject = GetWeaponObject();
	check(WeaponObject);

	if (WeaponObject->IsMagEmpty())
	{
		K2_CancelAbility();
		return;
	}

	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = AbilityComponent->AbilityTargetDataSetDelegate(
		CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (IsLocallyControlled())
	{
		StartWeaponTargeting();
	}

	FTimerDelegate EndAbilityDelegate;
	EndAbilityDelegate.BindLambda([&, this]
	{
		K2_EndAbility();
	});

	float FireRate = GetWeaponObject()->GetFireRate();
	GetWorld()->GetTimerManager().SetTimer(EndAbilityTimer, EndAbilityDelegate, FireRate, false);
}

void UStalkerGameplayAbility_RangedWeaponFire::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                          const FGameplayAbilityActorInfo* ActorInfo,
                                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                                          bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo,
			                                                      ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UWeaponObject* WeaponObject = GetWeaponObject();
		check(WeaponObject);

		UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(AbilityComponent);

		AbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle,
		                                               CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		AbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle,
		                                                    CurrentActivationInfo.GetActivationPredictionKey());

		if (!bWasCancelled)
		{
			WeaponObject->StopFire();
		}

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UStalkerGameplayAbility_RangedWeaponFire::StartWeaponTargeting()
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);

	FScopedPredictionWindow ScopedPrediction(AbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(FoundHits);

	FGameplayAbilityTargetDataHandle TargetData;
	if (FoundHits.Num() > 0)
	{
		for (const FHitResult& FoundHit : FoundHits)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;

			TargetData.Add(NewTargetData);
		}
	}

	OnTargetDataReadyCallback(TargetData, FGameplayTag::EmptyTag);
}

void UStalkerGameplayAbility_RangedWeaponFire::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	UWeaponObject* WeaponObject = GetWeaponObject();

	if (AvatarPawn && AvatarPawn->IsLocallyControlled() && WeaponObject)
	{
		FRangedWeaponFiringInput InputData;
		InputData.WeaponObject = WeaponObject;

		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, EWeaponTargetingSource::CameraTowardsFocus);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();
		InputData.EndAim = InputData.StartTrace + InputData.AimDir * 10000.0f;

		TraceBulletsInCartridge(InputData, OutHits);
	}
}

void UStalkerGameplayAbility_RangedWeaponFire::TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData,
                                                                       TArray<FHitResult>& OutHits)
{
	UWeaponObject* WeaponObject = InputData.WeaponObject;
	
	check(WeaponObject);
	check(GetWeaponComponent());

	TArray<UAmmoObject*> Cartridge = WeaponObject->GetRounds();
	if (Cartridge.IsEmpty())
	{
		return;
	}

	UAmmoObject* AmmoObject = Cartridge[0];
	const uint8 BulletsPerCartridge = AmmoObject->GetBulletsPerCartridge();

	for (int32 i = 0; i < BulletsPerCartridge; ++i)
	{
		const float SpreadAngle = GetWeaponComponent()->GetCalculatedSpreadAngle();
		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(SpreadAngle * 0.5f);
		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians,
		                                                      WeaponObject->GetSpreadExponent());
		const FVector EndTrace = InputData.StartTrace + BulletDir * 10000.0f;

		TArray<FHitResult> AllImpacts;
		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, AmmoObject->GetBulletSweepRadius(),
		                                        false, AllImpacts);

		if (Impact.GetActor())
		{
			DrawDebugPoint(GetWorld(), Impact.ImpactPoint, 18.0f, FColor::Green, false, 3.0f);

			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}
		}
		else if (!Impact.bBlockingHit)
		{
			Impact.Location = EndTrace;
			Impact.ImpactPoint = EndTrace;
			OutHits.Add(Impact);
		}
	}
}

FHitResult UStalkerGameplayAbility_RangedWeaponFire::DoSingleBulletTrace(
	const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated,
	TArray<FHitResult>& OutHits) const
{
	FHitResult Impact;

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = BulletTrace(StartTrace, EndTrace, /* SweepRadius */ 0.0f, bIsSimulated, OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = BulletTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, SweepHits);

			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

FHitResult UStalkerGameplayAbility_RangedWeaponFire::BulletTrace(const FVector& StartTrace, const FVector& EndTrace,
                                                                 float SweepRadius, bool bIsSimulated,
                                                                 TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	//AddAdditionalTraceIgnoreActors(TraceParams); TODO

	constexpr ECollisionChannel TraceChannel = ECC_Visibility;

	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel,
		                                FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
}

int32 UStalkerGameplayAbility_RangedWeaponFire::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			return Idx;
		}

		AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor != nullptr && HitActor->GetAttachParentActor() != nullptr && Cast<APawn>(
			HitActor->GetAttachParentActor()) != nullptr)
		{
			return Idx;
		}
	}

	return INDEX_NONE;
}

FTransform UStalkerGameplayAbility_RangedWeaponFire::GetTargetingTransform(APawn* SourcePawn, EWeaponTargetingSource Source) const
{
	check(SourcePawn);

	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->Controller;
	FVector SourceLoc;

	if (Controller != nullptr && Source == EWeaponTargetingSource::CameraTowardsFocus)
	{
		float FocalDistance = 1024.0f;
		FVector CamLoc;
		FRotator CamRot;

		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC != nullptr)
		{
			PC->GetPlayerViewPoint(CamLoc, CamRot);
		}

		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FVector FocalLoc = CamLoc + AimDir * FocalDistance;

		if (PC)
		{
			const FVector WeaponLoc = SourcePawn->GetActorLocation();
			CamLoc = FocalLoc + (WeaponLoc - FocalLoc | AimDir) * AimDir;
			FocalLoc = CamLoc + AimDir * FocalDistance;
		}

		return FTransform(CamRot, CamLoc);
	}

	return FTransform(AimQuat, SourceLoc);
}

void UStalkerGameplayAbility_RangedWeaponFire::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
                                                                         FGameplayTag ApplicationTag)
{
	UWeaponObject* WeaponObject = GetWeaponObject();
	check(WeaponObject);

	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = AbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow ScopedPrediction(AbilityComponent);
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		if (CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
		{
			AbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,
			                                                    CurrentActivationInfo.GetActivationPredictionKey(),
			                                                    LocalTargetDataHandle, ApplicationTag,
			                                                    AbilityComponent->ScopedPredictionKey);
		}

		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			if (CurrentActorInfo->IsNetAuthority())
			{
				SpawnBullets(LocalTargetDataHandle);
			}

			WeaponObject->StartFire();

			FGameplayCueParameters CueParams;
			CueParams.Instigator = GetAvatarActorFromActorInfo();
			CueParams.SourceObject = WeaponObject;

			K2_ExecuteGameplayCueWithParams(
				CueTag.IsValid() ? CueTag : FStalkerGameplayTags::GameplayCueTag_WeaponBurst, CueParams);
		}
		else
		{
			K2_EndAbility();
		}
	}

	AbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle,
	                                                    CurrentActivationInfo.GetActivationPredictionKey());
}

void UStalkerGameplayAbility_RangedWeaponFire::SpawnBullets(const FGameplayAbilityTargetDataHandle& InData)
{
	UWeaponObject* WeaponObject = GetWeaponObject();
	check(WeaponObject);

	TArray<UAmmoObject*> Cartridge = WeaponObject->GetRounds();
	if (Cartridge.IsEmpty())
	{
		return;
	}

	UAmmoObject* AmmoObject = Cartridge[0];
	const uint8 BulletsPerCartridge = AmmoObject->GetBulletsPerCartridge();

	if (InData.Num() == 0 || BulletsPerCartridge != InData.Num())
	{
		return;
	}

	for (int32 i = 0; i < BulletsPerCartridge; i++)
	{
		const FHitResult* HitResult = InData.Get(i)->GetHitResult();
		if (HitResult != nullptr)
		{
			APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
			FTransform SpawnTransform = GetTargetingTransform(AvatarPawn, EWeaponTargetingSource::CameraTowardsFocus);
			FRotator BulletRotation = FRotationMatrix::MakeFromX(HitResult->ImpactPoint - SpawnTransform.GetLocation()).Rotator();
			SpawnTransform.SetRotation(FQuat(BulletRotation));

			if (ABulletBase* Bullet = GetWorld()->SpawnActorDeferred<ABulletBase>(
				AmmoObject->GetBulletClass(), SpawnTransform, AvatarPawn, AvatarPawn,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				Bullet->SetupBullet(GetWeaponObject(), AmmoObject);
				Bullet->FinishSpawning(SpawnTransform);
			}
		}
	}
}
