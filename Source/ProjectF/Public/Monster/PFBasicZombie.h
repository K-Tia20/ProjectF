
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Detect")
	float DetectInterval = 0.1f;
	
	
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
	
	virtual void OnGetUpFinished() override;
	
	// 플레이어를 처음 타겟으로 등록했을 때 자식 몬스터가 반응
	virtual void OnTargetAcquired(AActor* NewTarget);
	
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
public:
	
	// 보스가 지정한 위치로 우선 이동하라는 명령
	UFUNCTION(BlueprintCallable, Category = "Zombie|BossCommand")
	void ReceiveBossMoveCommand(
		const FVector& InCommandLocation,
		AActor* InCommandTarget
	);
	
	// 보스가 플레이어에게 즉시 돌격하라고 명령
	UFUNCTION(BlueprintCallable, Category = "Zombie|BossCommand")
	void ReceiveBossRushCommand(AActor* InCommandTarget);
	
protected:
	
	// 보스의 이동 명령을 수행 중인지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|BossCommand")
	bool bFollowingBossCommand = false;

	// 보스가 지정한 포위 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|BossCommand")
	FVector BossCommandLocation = FVector::ZeroVector;

	// 포위 후 공격할 플레이어
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|BossCommand")
	AActor* BossCommandTarget = nullptr;

	// 명령 위치 도착 판정 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|BossCommand")
	float BossCommandAcceptanceRadius = 100.f;
	
	// 포위 지점까지 이동할 수 있는 제한시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|BossCommand")
	float BossCommandTimeout = 2.f;

	// 포위 명령 시간제한 타이머
	FTimerHandle BossCommandTimeoutTimerHandle;

	// 제한시간 안에 도착하지 못했을 때 실행
	void HandleBossCommandTimeout();

	void UpdateBossCommand();
	void FinishBossCommand();
	
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
