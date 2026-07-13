// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/PFBaseItem.h"
#include "PFGrenadeItem.generated.h"

UCLASS()
class PROJECTF_API APFGrenadeItem : public APFBaseItem
{
	GENERATED_BODY()


protected:
	
	virtual void OnPickup(class APFPlayerCharacter* PlayerCharacter) override;
};
