// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RedVsBluePlayerController.generated.h"

class UInputMappingContext;
class URedVsBlueHUDWidget;
class UUserWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ARedVsBluePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<URedVsBlueHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<URedVsBlueHUDWidget> HUDWidget;

	FTimerHandle HUDRefreshTimer;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	UPROPERTY()
	bool bReturningToMenu = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	UFUNCTION()
	void RefreshHUD();

	UFUNCTION()
	void HandleReturnToMenu();

	UFUNCTION()
	void HandleSessionDestroyed(bool bWasSuccesful);

	void TravelToMainMenu();

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

private:
	bool bFinishedInputConfigured = false;

};
