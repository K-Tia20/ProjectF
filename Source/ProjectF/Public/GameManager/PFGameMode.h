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
class APFBaseItem;

UCLASS()
class PROJECTF_API APFGameMode : public AGameModeBase
{
	GENERATED_BODY()
    
public:
    
	APFGameMode();
    
	void OnMonsterKilled(AActor* KilledMonster);
    
	void ProceedToNextLevel();
    
	// 게임오버 이동
	void GameOver();
	
	UFUNCTION(BlueprintCallable, Category = "Item|Drop")
	void HandleMonsterDeath(const FVector& SpawnLocation);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LoadingWidgetClass;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Classes")
	TSubclassOf<APFBaseItem> HealingItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Classes")
	TSubclassOf<APFBaseItem> PistolAmmoClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Classes")
	TSubclassOf<APFBaseItem> RifleAmmoClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Classes")
	TSubclassOf<APFBaseItem> GrenadeItemClass;

	// 기본 드랍 확률 (20%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|DropSettings")
	float OverallDropChance = 0.2f;
	
	void RegisterLevelPortal(class APFLevelPortal* Portal) { LevelPortalInstance = Portal; }
	
protected:
	
	virtual void BeginPlay() override;
    
	void TimerCountDown();
	void StartWave();
	void WaveClear();
	void LevelClear();
	
    
    
private:
    
	void SpawnRandomItem(const FVector& SpawnLocation, int32 CurrentLevel);
	
	UPROPERTY(VisibleAnywhere, Category = "Game Data")
	APFGameState* PFGameState;
    
	FTimerHandle PFTimerHandle;
    
	UPROPERTY()
	class APFWaveManager* PFWaveManager;
	UPROPERTY()
	class APFSpawnManager* PFSpawnManager;
    
	UPROPERTY(EditAnywhere, Category = "Wave Data")
	UDataTable* WaveDataTableAsset;
    
	class APFLevelPortal* LevelPortalInstance;
};