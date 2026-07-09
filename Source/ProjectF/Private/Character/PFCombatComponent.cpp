#include "Character/PFCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Monster/PFMonsterCharacter.h" 
#include "GameFramework/Pawn.h"

UPFCombatComponent::UPFCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPFCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPFCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPFCombatComponent::FireWeapon()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn || !OwnerPawn->GetController()) return;

	FVector StartLocation;
	FRotator StartRotation;
	OwnerPawn->GetController()->GetPlayerViewPoint(StartLocation, StartRotation);

	FVector EndLocation = StartLocation + (StartRotation.Vector() * AttackRange);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Owner); 

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		CollisionParams
	);

	// 빨간선 디버그 (테스트 끝나면 지우세요)
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f, 0, 1.f);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
			if (Monster)
			{
				Monster->TakeDamageFromMonster(AttackDamage, Owner);
			}
		}
	}
}