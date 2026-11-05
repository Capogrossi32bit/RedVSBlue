// Copyright Epic Games, Inc. All Rights Reserved.


#include "RedVsBluePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "RedVsBlueHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "RedVsBlue.h"
#include "Widgets/Input/SVirtualJoystick.h"

void ARedVsBluePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogRedVsBlue, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	if (!IsLocalController())
	{
		return;
	}

	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("HUDWidgetClass is not assigned"));
		return;
	}

	HUDWidget = CreateWidget<URedVsBlueHUDWidget>(this, HUDWidgetClass);

	if (!IsValid(HUDWidget))
	{
		return;
	}

	HUDWidget->AddToViewport();

	RefreshHUD();

	GetWorldTimerManager().SetTimer(HUDRefreshTimer, this, &ARedVsBluePlayerController::RefreshHUD, 0.25f, true);

}

void ARedVsBluePlayerController::RefreshHUD()
{
	if (IsValid(HUDWidget))
	{
		HUDWidget->RefreshHUD();
	}
}

void ARedVsBluePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}
