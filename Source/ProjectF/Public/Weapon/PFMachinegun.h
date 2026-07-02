// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PFWeaponBase.h"
#include "PFMachinegun.generated.h"

UCLASS()
class PROJECTF_API APFMachinegun : public APFWeaponBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APFMachinegun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
