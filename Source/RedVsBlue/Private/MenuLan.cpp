// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuLan.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

class UButton;

void UMenuLan::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IsValid(Button_Hostear))
	{
		Button_Hostear->OnClicked.AddDynamic(this, &UMenuLan::OnButtonHostearClicked);
	}

	if (IsValid(Button_Unirse))
	{
		Button_Unirse->OnClicked.AddDynamic(this, &UMenuLan::OnButtonUnirseClicked);
	}

	if (IsValid(Button_Back))
	{
		Button_Back->OnClicked.AddDynamic(this, &UMenuLan::OnButtonBackClicked);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;

		GetOwningPlayer()->SetInputMode(InputMode);
		GetOwningPlayer()->SetShowMouseCursor(true);
	}

}

void UMenuLan::OnButtonHostearClicked()
{

	FInputModeGameOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(false);

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Level, true, TEXT("listen"));
}

void UMenuLan::OnButtonUnirseClicked()
{
	FInputModeGameOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(false);

	const FString IP = EditableTextBox_DireccionIP->GetText().ToString();
	UGameplayStatics::OpenLevel(this, *IP);
}

void UMenuLan::OnButtonBackClicked()
{
	OnBackRequested.Broadcast();
	SetVisibility(ESlateVisibility::Collapsed);
}
