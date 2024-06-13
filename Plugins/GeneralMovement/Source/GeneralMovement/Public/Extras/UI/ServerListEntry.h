// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerListEntry.generated.h"

/// Provides the bindings for a widget Blueprint which represents an entry in the server browser list created by @see UMainMenu.
UCLASS()
class GENERALMOVEMENT_API UServerListEntry : public UUserWidget
{
  GENERATED_BODY()

public:

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  /// Name of the server for this entry.
  class UTextBlock* ServerName{nullptr};

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  /// Username of the host for this server.
  UTextBlock* HostName{nullptr};

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  /// Number of players currently joined.
  UTextBlock* NumCurrentPlayers{nullptr};

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  /// Maximum number of players for this server.
  UTextBlock* NumMaxPlayers{nullptr};

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  /// Ping to the session (not provided by the Steam API).
  UTextBlock* Ping{nullptr};

  UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (BindWidgetOptional))
  /// Makes the server list entry clickable.
  class UButton* ServerButton{nullptr};

  void SetEntryIndex(UUserWidget* InParent, uint32 InIndex);

private:

  /// Index in the server list for this entry.
  uint32 Index{0};

  UFUNCTION()
  void OnServerButtonClicked();

  UPROPERTY()
  UUserWidget* Parent{nullptr};
};
