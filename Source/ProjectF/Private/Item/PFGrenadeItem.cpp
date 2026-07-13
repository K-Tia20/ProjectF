#include "Item/PFGrenadeItem.h"
#include "Character/PFPlayerCharacter.h"
#include "Character/PFCombatComponent.h"



void APFGrenadeItem::OnPickup(APFPlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		UPFCombatComponent* CombatComponent = PlayerCharacter->FindComponentByClass<UPFCombatComponent>();
		if (CombatComponent)
		{
			CombatComponent->AddGrenades(ItemAmount);
		}
	}
	Super::OnPickup(PlayerCharacter);
}