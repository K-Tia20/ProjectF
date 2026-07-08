// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PFBaseItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/PFPlayerCharacter.h"


APFBaseItem::APFBaseItem()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(50.f);
    
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APFBaseItem::BeginPlay()
{
	Super::BeginPlay();
    
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APFBaseItem::OnSphereOverlap);
}

void APFBaseItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		APFPlayerCharacter* PlayerCharacter = Cast<APFPlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			OnPickup(PlayerCharacter);
		}
	}
}

void APFBaseItem::OnPickup(APFPlayerCharacter* PlayerCharacter)
{
	Destroy();
}

