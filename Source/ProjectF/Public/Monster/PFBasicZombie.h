
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float DetectRange = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float SightAngle = 90.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float RequiredDetectTime = 1.0f;
	
	float CurrentDetectTime = 0.f;
	
	// 플레이어를 놓친 뒤 기다리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Detect")
	float LostSightGraceTime = 2.f;

	// 현재 플레이어를 놓친 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Detect")
	float CurrentLostSightTime = 0.f;
	
	
	// ===== Move =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float WalkSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	float MoveAcceptanceRadius = 20.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie")
	AActor* TargetActor = nullptr;
	
	
	// ===== State =====
	UPROPERTY()
	FVector SpawnLocation;

	UPROPERTY()
	FVector LastKnownLocation;
	
	// 일반 좀비는 true, 보스는 false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	bool BReturnToSpawnLocation = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie")
	bool BHasTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie")
	bool BMovingToLastKnownLocation = false;
	
	
	// ===== Timer =====
	FTimerHandle DetectTimerHandle;
	FTimerHandle LastKnownLocationTimerHandle;
	
	
	// ===== AI Logic =====
	void DetectPlayer();
	void ChasePlayer();

	bool IsPlayerInSight(AActor* Player);
	bool HasLineOfSight(AActor* Player);

	virtual void OnDamaged(AActor* DamageCauser) override;

	// 플레이어를 놓쳤을 때 실행
	virtual void HandleLostTarget();

	// 마지막으로 본 위치로 이동
	virtual void MoveToLastKnownLocation();
	
	// 마지막 위치에 도착했는지 체크
	virtual void CheckLastKnownLocation();
	
	// 마지막 위치 도착 후 행동
	virtual void HandleAfterReachedLastKnownLocation();

	// 스폰 위치 복귀
	virtual void ReturnToSpawnLocation();
	
	
	
	// ===== Sound =====
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Sound")
	USoundBase* RoarSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Sound")
	USoundBase* DetectSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Sound")
	USoundBase* SearchSound = nullptr;

	virtual void PlayRoarSound();
	virtual void PlayDetectSound();
	virtual void PlaySearchSound();
	
	
	// ===== Debug =====
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool BDrawDebug = true;
	
	
public:
	virtual void Die() override;
};
