/* Copyright Next Genium Studio. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ItemsLibrary.generated.h"

class UItemObject;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnContainerItemOperationSignature, UItemObject*)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemOperationSignature, const FString&)

UENUM(BlueprintType)
enum class EDragDropOperationResult : uint8
{
	Remove,
	Subtract
};

USTRUCT(BlueprintType)
struct FItemParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ClampMin = "1"))
	uint32 ItemId = 1;
	
	UPROPERTY(EditAnywhere, Category = "Item", meta = (ClampMin = "1"))
	uint16 Amount = 1;
	
	UPROPERTY(EditAnywhere, Category = "Item", meta = (ClampMin = "0.0", ForceUnits = "%"))
	float Condition = 100.0f;
};

USTRUCT(BlueprintType)
struct FArmorParams
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct FWeaponParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<TSubclassOf<UItemObject>> AmmoClasses;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	int MagElapsed = 0;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	bool bAutomatic = false;
};

USTRUCT(BlueprintType, Blueprintable)
struct FItemData
{
	GENERATED_USTRUCT_BODY()

	FDataTableRowHandle ItemRow;
	FItemParams ItemParams;
	
	bool IsValid() const
	{
		return !ItemRow.IsNull();
	}
};

USTRUCT(BlueprintType)
struct FTableRowItems : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Class")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<class AItemActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<UItemObject> ObjectClass;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Description")
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditAnywhere, Category = "Properties")
	TArray<TSubclassOf<UGameplayEffect>> ItemEffects;
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	FIntPoint Size = {0, 0};

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bUsable = false;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bDroppable = true;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bStackable = false;
	
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (ClampMin = "1"))
	uint32 StackAmount = 1;
	
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (ClampMin = "1"))
	float Weight = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> PreviewMesh;
};

USTRUCT(BlueprintType)
struct FTableRowArmor : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Armor")
	TSoftObjectPtr<USkeletalMesh> Visual;
};

USTRUCT(BlueprintType)
struct FTableRowAmmo : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct FTableRowMedicament : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct FTableRowFood : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct FTableRowWeapon : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<TSubclassOf<UItemObject>> AmmoClasses;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = "1"))
	int MagSize = 1;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bAutomatic = false;
};
