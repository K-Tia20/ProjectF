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
	
	
	
	UUserWidget* GetHUDWidget() const { return HUDWidgetInstance; }
	void ShowGameHUD();
	void ShowMainMenu(bool bIsRestart, bool bIsClear);

	
	
	void UpdatePlayerHP(float Health, float MaxHealth);
	void UpdateWaveText(int32 CurrentWave);
	void UpdateMonsterText(int32 RemainingMonsters);
	void UpdateTimer(int32 Min, int32 Sec, int32 Ms);
	
	
	//  게임오버 위젯 분리용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;
	UPROPERTY(BlueprintReadOnly, Category = "UI")	
	class UUserWidget* GameOverWidgetInstance;
	
	void ShowGameOver(bool bIsClear);
	
	//  HUD 안에 들어있는 피격 & 상호작용 이벤트 다이렉트 호출용
	void ShowBloodEffect(float DamageAmount);
	void ShowInteractionPrompt(const FString& ActionText);
	void HideInteractionPrompt();
	
};
