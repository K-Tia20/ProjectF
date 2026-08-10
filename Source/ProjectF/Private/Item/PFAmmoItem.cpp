// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PFAmmoItem.h"
#include "Character/PFPlayerCharacter.h"
#include "Character/PFCombatComponent.h"

APFAmmoItem::APFAmmoItem()
{
}

void APFAmmoItem::OnPickup(APFPlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	UPFCombatComponent* CombatComponent = PlayerCharacter->FindComponentByClass<UPFCombatComponent>();
	if (CombatComponent)
	{
		// 3. 에디터(블루프린트)에 설정된 무기 타입과 총알 개수만큼 충전
		CombatComponent->AddAmmo(TargetWeaponType, AmmoAmount);
        
		UE_LOG(LogTemp, Log, TEXT("[Item] %s 아이템이 플레이어에게 획득되었습니다."), *GetName());
	
	
	}
	Super::OnPickup(PlayerCharacter);
}