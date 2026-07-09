// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "PFGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTF_API APFGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	
public:
	APFGameState();
	
	
	void UpdateHUD();
	void OnMonsterKilled();
	
	void SetCurrentLevel(int32 NewLevel) { CurrentLevel = NewLevel; }
	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	int32 GetCurrentLevel() const { return CurrentLevel; }
	
	void SetCurrentWave(int32 NewWave) { CurrentWave = NewWave; }
	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	int32 GetCurrentWave() const { return CurrentWave; }
	
	void SetRemainingMonsters(int32 Amount) { RemainingMonsters = Amount; }
	void DecreaseMonsterCount() { RemainingMonsters = FMath::Max(0, RemainingMonsters - 1); }
	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	int32 GetRemainingMonsters() const { return RemainingMonsters; }
	
	void SetRemainingTime(float NewTime) { RemainingTime = NewTime; }
	UFUNCTION(BlueprintCallable, Category = "Game Progress")
	float GetRemainingTime() const { return RemainingTime; }
	

	
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Game Progress")
	int32 CurrentLevel;

	UPROPERTY(VisibleAnywhere, Category = "Game Progress")
	int32 CurrentWave;

	UPROPERTY(VisibleAnywhere, Category = "Game Progress")
	int32 RemainingMonsters;

	UPROPERTY(VisibleAnywhere, Category = "Game Progress")
	float RemainingTime;
};
