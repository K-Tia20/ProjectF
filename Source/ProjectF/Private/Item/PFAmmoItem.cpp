// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PFAmmoItem.h"
#include "Character/PFPlayerCharacter.h" 


void APFAmmoItem::OnPickup(APFPlayerCharacter* PlayerCharacter)
{
	//PlayerCharacter->GetCombatComponent()->AddAmmo(AmmoAmount);
	
	// UE_LOG(LogTemp, Log, TEXT("탄약 %d 발 획득!"), AmmoAmount);
	// Super::OnPickup(PlayerCharacter);
}
