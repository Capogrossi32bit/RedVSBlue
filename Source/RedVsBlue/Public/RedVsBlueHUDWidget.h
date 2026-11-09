// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RedVsBlueHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class REDVSBLUE_API URedVsBlueHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshHUD();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshCombatHUD();

protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> BlueScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RedScoreText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PhaseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReloadText;
};
