// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PFBaseItem.h"
#include "PFHealingItem.generated.h"

UCLASS()
class PROJECTF_API APFHealingItem : public APFBaseItem
{
	GENERATED_BODY()
    
protected:
	virtual void OnPickup(class APFPlayerCharacter* PlayerCharacter) override;

	 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
	 float HealAmount = 20.f;
};
