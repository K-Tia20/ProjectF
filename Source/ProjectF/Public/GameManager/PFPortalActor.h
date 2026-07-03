// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PFPortalActor.generated.h"

UCLASS()
class PROJECTF_API APFPortalActor : public AActor
{
	GENERATED_BODY()

public:
	
	APFPortalActor();
	
	void SpawnMonster(TSubclassOf<APawn> MonsterClass);
	
	
	
protected:
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* PortalMesh;
};
