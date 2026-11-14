// Copyright Epic Games, Inc. All Rights Reserved.


#include "RedVsBluePlayerController.h"
#include "RedVsBlueGameState.h"
#include "MatchPhase.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "RedVsBlueHUDWidget.h"
#include "../Plugins/MultiplayerSessionOW/Source/MultiplayerSessionOW/Public/MultiplayerSessionsSubsystem.h"
#include "Kismet/GameplayStatics.h"
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
		return;
	}

	HUDWidget = CreateWidget<URedVsBlueHUDWidget>(this, HUDWidgetClass);

	if (!IsValid(HUDWidget))
	{
		return;
	}

	HUDWidget->OnReturnToMenuRequested.AddDynamic(this, &ARedVsBluePlayerController::HandleReturnToMenu);
		
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
	if (!IsLocalController() || !IsValid(HUDWidget))
	{
		return;
	}

	ARedVsBlueGameState* RBGameState = GetWorld()->GetGameState<ARedVsBlueGameState>();

	if (!IsValid(RBGameState))
	{
		return;
	}

	if (RBGameState->GetMatchPhase() == EMatchPhase::Finished && !bFinishedInputConfigured)
	{
		bFinishedInputConfigured = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(HUDWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		SetInputMode(InputMode);
		SetShowMouseCursor(true);
	}
}

void ARedVsBluePlayerController::HandleReturnToMenu()
{


	if (bReturningToMenu || !IsLocalController())
	{
		return;
	}

	bReturningToMenu = true;

	GetWorldTimerManager().ClearTimer(HUDRefreshTimer);

	UGameInstance* GameInstance = GetGameInstance();

	if (!IsValid(GameInstance))
	{
		TravelToMainMenu();
		return;
	}

	UMultiplayerSessionsSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

	if (!IsValid(SessionSubsystem))
	{
		TravelToMainMenu();
		return;
	}

	SessionSubsystem->MultijugadorAlDestruirSesionCompletada.AddUniqueDynamic(this, &ARedVsBluePlayerController::HandleSessionDestroyed);
	SessionSubsystem->DestruirSesion();

}

void ARedVsBluePlayerController::HandleSessionDestroyed(bool bWasSuccesful)
{

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			SessionSubsystem->MultijugadorAlDestruirSesionCompletada.RemoveDynamic(this, &ARedVsBluePlayerController::HandleSessionDestroyed);
		}
	}

	if (!bWasSuccesful)
	{

		bReturningToMenu = false;

		if (IsValid(HUDWidget))
		{
			HUDWidget->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}
	TravelToMainMenu();
}

void ARedVsBluePlayerController::TravelToMainMenu()
{
	const FString MainMenuPath = TEXT("/Game/MainLevel");

	if (HasAuthority())
	{
		UGameplayStatics::OpenLevel(this, FName(*MainMenuPath));
	}
	else
	{
		ClientTravel(MainMenuPath, ETravelType::TRAVEL_Absolute);
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
