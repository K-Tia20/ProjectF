// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PFGameMode.generated.h"

/**
 * 
 */
class APFGameState;
class APFLevelPortal;

UCLASS()
class PROJECTF_API APFGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	APFGameMode();
	
	void OnMonsterKilled();
	
	void ProceedToNextLevel();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LoadingWidgetClass;
	
protected:
	
	virtual void BeginPlay() override;
	
	void TimerCountDown();
	void StartWave();
	void WaveClear();
	void LevelClear();
	void GameOver();
	
	
private:
	
	UPROPERTY(VisibleAnywhere, Category = "Game Data")
	APFGameState* PFGameState;
	
	FTimerHandle PFTimerHandle;
	
	UPROPERTY()
	class APFWaveManager* PFWaveManager;
	UPROPERTY()
	class APFSpawnManager* PFSpawnManager;
	
	UPROPERTY(EditAnywhere, Category = "Wave Data")
	UDataTable* WaveDataTableAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APFLevelPortal> LevelPortalClass;
	
	
};
