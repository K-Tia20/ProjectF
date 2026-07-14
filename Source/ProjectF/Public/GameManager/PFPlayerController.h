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

	
	// UI 위젯 클래스 및 인스턴스 관리
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UUserWidget* HUDWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	class UUserWidget* MainMenuWidgetInstance;
		
	//  게임오버 위젯 분리용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;
	UPROPERTY(BlueprintReadOnly, Category = "UI")	
	class UUserWidget* GameOverWidgetInstance;
	
	
	UUserWidget* GetHUDWidget() const { return HUDWidgetInstance; }
	
	// UI 화면 전환 및 표시 제어 함수
	void ShowGameHUD();
	void ShowMainMenu(bool bIsRestart, bool bIsClear);
	void ShowGameOver(bool bIsClear);

	// HUD 위젯 갱신 함수
	void UpdatePlayerHP(float Health, float MaxHealth);
	void UpdateWaveText(int32 CurrentWave);
	void UpdateMonsterText(int32 RemainingMonsters);
	void UpdateTimer(int32 Min, int32 Sec, int32 Ms);
	
	
	void UpdateAmmoUI(int32 CurrentAmmo, int32 MaxAmmo);
	void UpdateDashCooldownUI(float DashCooldown);
	void UpdateGrenadeCountUI(int32 GrenadeCount);
	void UpdateWeaponNameUI(const FString& WeaponName);

	//  HUD 안에 들어있는 피격 & 상호작용 이벤트 다이렉트 호출용
	void ShowBloodEffect(float DamageAmount);
	void ShowInteractionPrompt(const FString& ActionText);
	void HideInteractionPrompt();
	void ShowItemNotification(const FString& ItemName, int32 Amount);
	void ShowKillNotification();
	void ShowHitMarker();
	
};
