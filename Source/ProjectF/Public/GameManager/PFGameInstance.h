// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PFGameInstance.generated.h"

class APFPlayerCharacter; // 전방 선언으로 캐릭터 클래스 인식

UCLASS()
class PROJECTF_API UPFGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPFGameInstance();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Data")
	float SavedPlayerHealth = -1.0f;

	void SavePlayerState(APFPlayerCharacter* PlayerCharacter);

	void LoadPlayerState(APFPlayerCharacter* PlayerCharacter);
};