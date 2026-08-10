
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "../Monster/PFMonsterCharacter.h"

#include "PFWaveManager.generated.h"


USTRUCT(BlueprintType)
struct FMonsterSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Data")
	TSubclassOf<class APFMonsterCharacter> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Data")
	int32 SpawnCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBoss = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Data")
	FName TargetPortalTag;
};

USTRUCT(BlueprintType)
struct PROJECTF_API FPFWaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Data")
	int32 Level = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Data")
	int32 Wave = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Data")
	TArray<FMonsterSpawnInfo> MonsterSpawnList;
	
};



UCLASS()
class PROJECTF_API APFWaveManager : public AActor
{
	GENERATED_BODY()
    
public:	
	APFWaveManager();

	void CheckAndSpawnWave(int32 CurrentLevel, int32 CurrentWave, class APFSpawnManager* InSpawnManager);

	void SetWaveDataTable(UDataTable* NewDataTable) { PFWaveDataTable = NewDataTable; }
	
protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Wave Data")
	UDataTable* PFWaveDataTable;

	UPROPERTY()
	class APFSpawnManager* PFSpawnManager;
};