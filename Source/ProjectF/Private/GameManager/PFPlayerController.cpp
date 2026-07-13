// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Misc/OutputDeviceNull.h"
#include "GameManager/PFGameState.h"




APFPlayerController::APFPlayerController()
	: HUDWidgetClass(nullptr), HUDWidgetInstance(nullptr),
	  MainMenuWidgetClass(nullptr), MainMenuWidgetInstance(nullptr)
{
}




void APFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains(TEXT("MainMenu")))
	{
		ShowMainMenu(false, false);
	}
	else
	{
		ShowGameHUD();
	}
}



void APFPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
		}
		if (APFGameState* PFGameState = GetWorld() ? GetWorld()->GetGameState<APFGameState>() : nullptr)
		{
			PFGameState->UpdateHUD();
		}
	}
}




void APFPlayerController::ShowMainMenu(bool bIsRestart, bool bIsClear)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			bShowMouseCursor = true;

			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
    
			SetInputMode(InputModeData);
		}

		UTextBlock* TitleText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("GameOverText")));
		UTextBlock* FinalRecordText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("FinalRecordText")));
		UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText")));

		if (bIsRestart)
		{
			if (ButtonText) ButtonText->SetText(FText::FromString(TEXT("Restart")));

			if (bIsClear)
			{
				if (TitleText) TitleText->SetText(FText::FromString(TEXT("Game Clear!")));
				if (FinalRecordText) FinalRecordText->SetText(FText::FromString(TEXT("Congratulations!")));
			}
			else
			{
				if (TitleText) TitleText->SetText(FText::FromString(TEXT("Game Over!")));

				if (APFGameState* PFGameState = GetWorld() ? GetWorld()->GetGameState<APFGameState>() : nullptr)
				{
					FString RecordStr = FString::Printf(TEXT("최종기록 : STAGE %d"), PFGameState->GetCurrentLevel());
					if (FinalRecordText) FinalRecordText->SetText(FText::FromString(RecordStr));
				}
			}
		}
		else
		{
			if (ButtonText) ButtonText->SetText(FText::FromString(TEXT("Start")));
		}
	}
}




void APFPlayerController::UpdatePlayerHP(float Health, float MaxHealth)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdatePlayerHP %f %f"), Health, MaxHealth);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::UpdateWaveText(int32 CurrentWave)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateWaveText %d"), CurrentWave);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::UpdateMonsterText(int32 RemainingMonsters)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateMonsterText %d"), RemainingMonsters);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::UpdateTimer(int32 Min, int32 Sec, int32 Ms)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateTimer %d %d %d"), Min, Sec, Ms);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}


void APFPlayerController::UpdateAmmoUI(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateAmmoUI %d %d"), CurrentAmmo, MaxAmmo);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::UpdateDashCooldownUI(float DashCooldown)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateDashCooldownUI %f"), DashCooldown);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::UpdateGrenadeCountUI(int32 GrenadeCount)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateGrenadeCountUI %d"), GrenadeCount);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::UpdateWeaponNameUI(const FString& WeaponName)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("UpdateWeaponNameUI \"%s\""), *WeaponName);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}

void APFPlayerController::ShowItemNotification(const FString& ItemName, int32 Amount)
{
	if (HUDWidgetInstance == nullptr) return;
	FOutputDeviceNull Ar;
	FString Command = FString::Printf(TEXT("ShowItemNotification \"%s\" %d"), *ItemName, Amount);
	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
}


void APFPlayerController::ShowGameOver(bool bIsClear)
{
	if (HUDWidgetInstance) { HUDWidgetInstance->RemoveFromParent(); HUDWidgetInstance = nullptr; }
	if (GameOverWidgetClass)
	{
		GameOverWidgetInstance = CreateWidget<UUserWidget>(this, GameOverWidgetClass);
		if (GameOverWidgetInstance)
		{
			GameOverWidgetInstance->AddToViewport(); 
			bShowMouseCursor = true; 
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(GameOverWidgetInstance->TakeWidget());
    
			SetInputMode(InputModeData);
		}
	}
}




void APFPlayerController::ShowBloodEffect(float DamageAmount)
{
	if (HUDWidgetInstance)
	{
		FOutputDeviceNull Ar;
		FString Command = FString::Printf(TEXT("PlayBloodAnim %f"), DamageAmount);
		HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
	}
}




void APFPlayerController::ShowInteractionPrompt(const FString& ActionText)
{
	if (HUDWidgetInstance)
	{
		FOutputDeviceNull Ar;
		FString Command = FString::Printf(TEXT("ShowPrompt %s"), *ActionText);
		HUDWidgetInstance->CallFunctionByNameWithArguments(*Command, Ar, nullptr, true);
	}
}




void APFPlayerController::HideInteractionPrompt()
{
	if (HUDWidgetInstance)
	{
		FOutputDeviceNull Ar;
		HUDWidgetInstance->CallFunctionByNameWithArguments(TEXT("HidePrompt"), Ar, nullptr, true);
	}
}




void APFPlayerController::ShowHitMarker()
{
	if (HUDWidgetInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] HUDWidgetInstance is null")
		);

		return;
	}

	FOutputDeviceNull Ar;
	FString Command = TEXT("PlayHitMarkerAnim");

	HUDWidgetInstance->CallFunctionByNameWithArguments(*Command,Ar,nullptr,true);

	UE_LOG(LogTemp, Log, TEXT("[HitMarker] PlayHitMarkerAnim called"));
}