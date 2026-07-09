

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PFMonsterCharacter.generated.h"

UCLASS()
class PROJECTF_API APFMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APFMonsterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Damage / Attack / Death =====
	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void TakeDamageFromMonster(float DamageAmount, AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void Attack();

	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "Monster")
	bool IsMonsterDead() const;

protected:
	virtual void BeginPlay() override;

	// 데미지를 받은 뒤 자식 몬스터가 추가 반응을 할 수 있게 만든 함수
	// 예: 좀비는 피격 시 공격자를 즉시 추적
	virtual void OnDamaged(AActor* DamageCauser);


	// =====================================================
	// Status
	// =====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Status")
	float CurrentHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	float AttackCooldown = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Status")
	bool IsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Status")
	bool CanAttack = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Status")
	bool bIsBoss = false;//UI

	// =====================================================
	// Attack
	// =====================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	UBoxComponent* AttackBox = nullptr;

	FTimerHandle AttackTimerHandle;
	FTimerHandle AttackBoxTimerHandle;

	void ResetAttack();

	void EnableAttackBox();
	void DisableAttackBox();

	UFUNCTION()
	void OnAttackBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);


	// =====================================================
	// Death
	// =====================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Death")
	float DestroyDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Death")
	bool BDestroyOnDeath = true;
	
	// =====================================================
	// Evevt
	// =====================================================
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Monster|Event")
	void OnMonsterDamaged(float DamageAmount, AActor* DamageCauser);//UI
};
