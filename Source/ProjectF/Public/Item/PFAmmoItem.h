// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/PFCombatComponent.h"
#include "Item/PFBaseItem.h"
#include "PFAmmoItem.generated.h"

UCLASS()
class PROJECTF_API APFAmmoItem : public APFBaseItem
{
	GENERATED_BODY()
    
public:
	APFAmmoItem();

protected:
	virtual void OnPickup(class APFPlayerCharacter* PlayerCharacter) override;

	//BP에서 작업
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Settings")
	EWeaponType TargetWeaponType = EWeaponType::Pistol;

	//BP에서 작업
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo Settings")
	int32 AmmoAmount = 15;
};