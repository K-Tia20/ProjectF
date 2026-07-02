// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PFBaseItem.h"
#include "PFGrenadeItem.generated.h"

UCLASS()
class PROJECTF_API APFGrenadeItem : public APFBaseItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APFGrenadeItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
