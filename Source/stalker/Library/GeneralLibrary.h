/* Copyright Next Genium Studio. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GeneralLibrary.generated.h"

USTRUCT(BlueprintType)
struct FTableRowDescriptions : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Description")
	TSoftObjectPtr<UTexture2D> Thumbnail;
};
