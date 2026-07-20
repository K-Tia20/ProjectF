// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFLevelPortal.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameManager/PFGameMode.h"
#include "Character/PFPlayerCharacter.h"
#include "GameManager/PFGameInstance.h"
#include "NiagaraComponent.h"
#include "GameManager/PFPlayerController.h"
#include "Kismet/GameplayStatics.h"


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
	
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APFLevelPortal::OnOverlapBegin);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APFLevelPortal::OnOverlapEnd);
	
	SetPortalActive(false);

	if (APFGameMode* GameMode = Cast<APFGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->RegisterLevelPortal(this);
	}
}


void APFLevelPortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsPortalActive) return;
	
	if (OtherActor)
	{
		APFPlayerCharacter* PlayerCharacter = Cast<APFPlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			PlayerCharacter->SetTargetPortal(this);
			
			if (APFPlayerController* PlayerController = Cast<APFPlayerController>(PlayerCharacter->GetController()))
				{
				PlayerController->ShowInteractionPrompt(TEXT("Portal"));
				}
			
			UE_LOG(LogTemp, Log, TEXT("포탈 반경 진입! F키를 누르세요."));
		}
	}
}



void APFLevelPortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bIsPortalActive) return;
	
	if (OtherActor)
	{
		APFPlayerCharacter* PlayerCharacter = Cast<APFPlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			if (PlayerCharacter->GetTargetPortal() == this)
			{
				PlayerCharacter->SetTargetPortal(nullptr);
				if (APFPlayerController* PlayerController = Cast<APFPlayerController>(PlayerCharacter->GetController()))
				{
					PlayerController->HideInteractionPrompt();
				}
				UE_LOG(LogTemp, Log, TEXT("포탈 반경을 벗어남."));
			}
		}
	}
}


void APFLevelPortal::Interact(class APFPlayerCharacter* PlayerCharacter)
{
	if (!bIsPortalActive) return;
	
	if (PlayerCharacter)
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



void APFLevelPortal::SetPortalActive(bool bActive)
{
	bIsPortalActive = bActive;

	// 액터 활성화 / 비활성화
	SetActorHiddenInGame(!bActive);

	// 콜리전 활성화 / 비활성화
	SetActorEnableCollision(bActive);

	// BP에 추가된 나이아가라 컴포넌트 활성화 / 비활성화
	if (UNiagaraComponent* PortalEffect = FindComponentByClass<UNiagaraComponent>())
	{
		if (bActive)
		{
			PortalEffect->Activate(true);
		}
		else
		{
			PortalEffect->Deactivate();
		}
	}
}
