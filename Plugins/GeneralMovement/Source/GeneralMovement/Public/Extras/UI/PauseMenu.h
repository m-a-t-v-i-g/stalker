// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

/// Provides the bindings for a widget Blueprint which displays a very simple pause menu for sessions created with @see USteamGameInstance.
UCLASS()
class GENERALMOVEMENT_API UPauseMenu : public UUserWidget
{
  GENERATED_BODY()

protected:

  bool Initialize() override;

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  class UButton* ExitButton{nullptr};

  UFUNCTION()
  void OnExitButtonClicked();
};
