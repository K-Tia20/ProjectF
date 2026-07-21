


#pragma once

#include "CoreMinimal.h"
#include "Monster/PFBasicZombie.h"
#include "PFBossCharacter.generated.h"

class UAnimMontage;
class USoundBase;

UCLASS()
class PROJECTF_API APFBossCharacter : public APFBasicZombie
{
	GENERATED_BODY()

public:
	APFBossCharacter();

protected:
	virtual void BeginPlay() override;

	// 일반 좀비는 마지막 위치 도착 후 스폰 위치로 복귀하지만,
	// 보스는 마지막 위치 주변을 계속 수색한다.
	virtual void HandleAfterReachedLastKnownLocation() override;

	// 보스 피격 반응
	virtual void OnDamaged(AActor* DamageCauser) override;

	
public:
	// 보스는 방어도 때문에 데미지 계산이 다르다.
	virtual void TakeDamageFromMonster(float DamageAmount, AActor* DamageCauser = nullptr) override;
	
	virtual void Attack() override;
	// 보스 사망 처리
	virtual void Die() override;

	// =====================================================
	// Boss Armor
	// =====================================================

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Armor")
	float MaxArmor = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Armor")
	float CurrentArmor = 100.f;

	// 방어도가 남아있을 때 데미지 감소율
	// 0.7이면 데미지 70% 감소, 즉 30%만 받음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Armor")
	float ArmorDamageReductionRate = 0.8f;

public:
	// 자폭몬스터가 이 함수를 호출해서 보스 방어도를 깎는다.
	UFUNCTION(BlueprintCallable, Category = "Boss|Armor")
	void TakeArmorDamage(float ArmorDamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|Armor")
	bool IsArmorBroken() const;


	// =====================================================
	// Boss Search
	// =====================================================

protected:
	// LastKnownLocation 주변 수색 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Search")
	float SearchRadius = 800.f;

	// 몇 초마다 새로운 수색 지점으로 이동할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Search")
	float SearchInterval = 2.0f;

	// true면 계속 수색한다.
	// false면 MaxSearchCount만큼만 수색한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Search")
	bool BInfiniteSearch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Search")
	int32 MaxSearchCount = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Search")
	int32 CurrentSearchCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Search")
	bool BIsSearching = false;

	// 나중에 Behavior Tree / Blackboard로 옮길 때 사용할 수 있는 현재 수색 목적지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Search")
	FVector CurrentSearchLocation;

	FTimerHandle SearchTimerHandle;

	void StartSearchAroundLastKnownLocation();
	void MoveToRandomSearchLocation();
	void StopSearch();


	// =====================================================
	// Boss Summon Pattern
	// =====================================================
	
public:
	// 포효 몽타주의 소환 노티파이에서 호출
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Summon")
	void HandleRoarSummonNotify();
	
protected:
	// 포효 소환 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Summon")
	float RoarSummonCooldown = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Summon")
	int32 SummonCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Summon")
	float SummonRadius = 600.f;
	
	// 소환 패턴 사용 가능 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern|Summon")
	bool bCanRoarSummon = true;
	
	// 보스 포효 소환 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Summon")
	UAnimMontage* RoarSummonMontage = nullptr;

	// 소환 쿨타임 완료
	void ResetRoarSummonCooldown();

	// 소환할 부하 몬스터 클래스
	// BP_SlowZombie, BP_RunZombie, 자폭몬스터 BP 등을 BP에서 지정 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Summon")
	TSubclassOf<APFBasicZombie> SummonMonsterClass;
	
	// 이번 소환 패턴으로 생성된 몬스터들
	// 보스가 이 배열을 통해 각 몬스터에게 서로 다른 위치를 명령한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern|Summon")
	TArray<TObjectPtr<APFBasicZombie>> CurrentSummonedMinions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool BIsUsingPattern = false;
	
	// 플레이어를 처음 발견하면 현재 위치에서 소환 포효
	virtual void OnTargetAcquired(AActor* NewTarget) override;
	
	// 플레이어를 포위할 때 유지할 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Command")
	float MinionSurroundRadius = 400.f;

	// 이번에 소환한 몬스터들에게 포위 명령 전달
	void CommandSummonedMinionsToSurround();
	
	// 소환 쿨타임이 준비됐을 때 보스가 공격 가능해질 때까지 재확인
	FTimerHandle RoarSummonRetryTimerHandle;

	// 준비된 소환 패턴을 지금 사용할 수 있는지 확인
	void TryStartReadyRoarSummon();

	FTimerHandle RoarSummonTimerHandle;

	void StartRoarSummonPattern();
	void SummonMinions();
	void EndPattern();


	// =====================================================
	// Boss Phase
	// =====================================================

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Phase")
	int32 BossPhase = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Phase")
	float Phase2HPRatio = 0.35f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Phase")
	bool BIsPhase2 = false;

	void UpdateBossPhase();


	// =====================================================
	// Future Patterns
	// =====================================================

	
	
	// =====================================================
	// Boss Sound
	// =====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* ArmorHitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* ArmorBreakSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* Phase2Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* SummonSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* ChargeStartSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* ChargeImpactSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sound")
	USoundBase* RangedAttackSound = nullptr;
protected:
	// 나중에 구현 예정
	void StartChargePattern();

	// 나중에 구현 예정
	void StartRangedAttackPattern();
};