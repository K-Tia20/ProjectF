// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PFHealingItem.h"
#include "Character/PFPlayerCharacter.h" 


void APFHealingItem::OnPickup(APFPlayerCharacter* PlayerCharacter)
{
	 if (PlayerCharacter)
	 {
	 	PlayerCharacter->Heal(HealAmount);
	 }
	
	 UE_LOG(LogTemp, Log, TEXT("체력 %f 회복 완료!"), HealAmount);
	 Super::OnPickup(PlayerCharacter);
}




