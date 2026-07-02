// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PFWeaponBase.h"
#include "PFPistol.generated.h"

UCLASS()
class PROJECTF_API APFPistol : public APFWeaponBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APFPistol();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
