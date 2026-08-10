// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager/PFGameInstance.h"
#include "Character/PFPlayerCharacter.h"
#include "GameManager/PFPlayerController.h"

UPFGameInstance::UPFGameInstance()
{
}

void UPFGameInstance::SavePlayerState(APFPlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter == nullptr) return;

	SavedPlayerHealth = PlayerCharacter->Health;
	
	UE_LOG(LogTemp, Warning, TEXT("GameInstance: 플레이어 체력 백업 완료! (저장된 체력: %f)"), SavedPlayerHealth);
}

void UPFGameInstance::LoadPlayerState(APFPlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter == nullptr) return;

	if (SavedPlayerHealth < 0.0f) return;

	PlayerCharacter->Health = SavedPlayerHealth;

	if (APFPlayerController* PlayerController = Cast<APFPlayerController>(PlayerCharacter->GetController()))
	{
		PlayerController->UpdatePlayerHP(PlayerCharacter->Health, PlayerCharacter->MaxHealth);
	}

	UE_LOG(LogTemp, Warning, TEXT("GameInstance: 새로운 레벨에 플레이어 체력 이식 및 UI 동기화 완료!"));
}





