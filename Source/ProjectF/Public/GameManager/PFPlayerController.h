// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PFPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTF_API APFPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	
	APFPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UUserWidget* HUDWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UUserWidget* MainMenuWidgetInstance;
	
	void UpdateTimerText(const FString& TimeString);

	
	
	
	UUserWidget* GetHUDWidget() const { return HUDWidgetInstance; }
	void ShowGameHUD();
	void ShowMainMenu(bool bIsRestart, bool bIsClear);

	
	
	void UpdatePlayerHP(float Health, float MaxHealth);
	void UpdateWaveText(int32 CurrentWave);
	void UpdateMonsterText(int32 RemainingMonsters);
	void UpdateTimer(int32 Min, int32 Sec, int32 Ms);
};
