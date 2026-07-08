
#pragma once

#include "CoreMinimal.h"
#include "PFMonsterCharacter.h"
#include "PFBasicZombie.generated.h"

UCLASS()
class PROJECTF_API APFBasicZombie : public APFMonsterCharacter
{
	GENERATED_BODY()

public:
	APFBasicZombie();

protected:
	virtual void BeginPlay() override;
	
	
	// ===== Detect =====
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Zombie")
	float DetectRange = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
	float SightAngle = 90.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
	float RequiredDetectTime = 1.0f;
	
	float CurrentDetectTime = 0.f;
	
	
	// ===== Move =====
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Zombie")
	float WalkSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float MoveAcceptanceRadius = 20.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Zombie")
	AActor* TargetActor;
	
		
	// ================
	FTimerHandle DetectTimerHandle;
	
	void DetectPlayer();
	
	void ChasePlayer();

	bool IsPlayerInSight(AActor* Player);
	
	bool HasLineOfSight(AActor* Player);

	virtual void OnDamaged(AActor* DamageCauser) override;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	bool BDrawDebug = true;
	
	//좀비 위치 저장(스폰구역외 배치된 녀석들을 위한 위치저장용)
	UPROPERTY()
	FVector SpawnLocation;

	UPROPERTY()
	FVector LastKnownLocation;
	//복귀할지 말지 정하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie")
	bool BReturnToSpawnLocation = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Zombie")
	bool BHasTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Zombie")
	bool BMovingToLastKnownLocation = false;
	
	void MoveToLastKnownLocation();
	
	void CheckLastKnownLocation();
	
	void ReturnToSpawnLocation();
	
	FTimerHandle LastKnownLocationTimerHandle;
	
public:
	
	virtual void Die() override;

	
};
