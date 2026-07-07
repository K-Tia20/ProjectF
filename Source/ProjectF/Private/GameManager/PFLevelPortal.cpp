// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFLevelPortal.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameManager/PFGameMode.h"
#include "Character/PFPlayerCharacter.h"
#include "GameManager/PFGameInstance.h"


APFLevelPortal::APFLevelPortal()
	: InteractionSphere(nullptr), PortalMesh(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	RootComponent = InteractionSphere;
	InteractionSphere->SetSphereRadius(200.0f); // 상호작용 가능한 범위 반지름 (2미터)

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComponent);
}

void APFLevelPortal::BeginPlay()
{
	Super::BeginPlay();
	/*
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APFLevelPortal::OnOverlapBegin);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APFLevelPortal::OnOverlapEnd);
	*/
}

/*
void APFLevelPortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APFPlayerCharacter* PlayerCharacter = Cast<APFPlayerCharacter>(OtherActor))
	{
		PlayerCharacter->SetTargetPortal(this);

	}
}
*/

/*
 void APFLevelPortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APFPlayerCharacter* PlayerCharacter = Cast<APFPlayerCharacter>(OtherActor))
	{
		if (PlayerCharacter->GetTargetPortal() == this)
		{
			PlayerCharacter->SetTargetPortal(nullptr);
		}
	}
}
*/

void APFLevelPortal::Interact()
{
	if (APFPlayerCharacter* PlayerCharacter = Cast<APFPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))
	{
		if (UPFGameInstance* GameInstance = Cast<UPFGameInstance>(GetGameInstance()))
		{
			GameInstance->SavePlayerState(PlayerCharacter);
		}
	}
	if (APFGameMode* GameMode = Cast<APFGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->ProceedToNextLevel();
	}
}

