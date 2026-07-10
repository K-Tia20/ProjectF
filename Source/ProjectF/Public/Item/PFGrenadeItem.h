// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PFBaseItem.h"
#include "PFGrenadeItem.generated.h"

UCLASS()
class PROJECTF_API APFGrenadeItem : public APFBaseItem
{
	GENERATED_BODY()
    
protected:
	//virtual void OnPickup(class APFPlayerCharacter* PlayerCharacter) override;

	// 보급해 줄 수류탄 개수
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
	// int32 GrenadeAmount = 1;
};
