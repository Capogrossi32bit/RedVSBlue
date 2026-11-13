// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/UserWidget.h"
#include "MenuLan.generated.h"


class UButton;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackRequested);

/**
 * 
 */
UCLASS()
class REDVSBLUE_API UMenuLan : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeOnInitialized() override;

	UPROPERTY(BlueprintReadWrite, Category = "Menu", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UUserWidget> MainMenuRef;

	UPROPERTY(BlueprintAssignable, Category = "Menu")
	FOnBackRequested OnBackRequested;

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Hostear;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Unirse;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_DireccionIP;;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> Level;

	UFUNCTION()
	void OnButtonHostearClicked();

	UFUNCTION()
	void OnButtonUnirseClicked();

	UFUNCTION()
	void OnButtonBackClicked();
};
