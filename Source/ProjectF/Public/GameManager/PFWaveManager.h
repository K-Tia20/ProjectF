// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "PFWaveManager.generated.h"

USTRUCT(BlueprintType)
struct FPFWaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 Level;        
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 Wave;          
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<APawn> MonsterClass; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 TotalSpawnCount; 
};



UCLASS()
class PROJECTF_API APFWaveManager : public AActor
{
	GENERATED_BODY()
    
public:	
	APFWaveManager();

	void CheckAndSpawnWave(int32 CurrentLevel, int32 CurrentWave);

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Wave Data")
	UDataTable* PFWaveDataTable;

	UPROPERTY()
	class APFSpawnManager* PFSpawnManager;
};