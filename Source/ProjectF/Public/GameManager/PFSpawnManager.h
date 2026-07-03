// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PFSpawnManager.generated.h"


class APFPortalActor;

UCLASS()
class PROJECTF_API APFSpawnManager : public AActor
{
	GENERATED_BODY()

public:

	APFSpawnManager();

	void SpawnWaveMonsters(TSubclassOf<APawn> MonsterClass, int32 TotalAmount);


protected:
	
	virtual void BeginPlay() override;

public:
	
	UPROPERTY()
	TArray<APFPortalActor*> PFPortalActors;
};
