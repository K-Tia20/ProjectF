

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PFMonsterCharacter.generated.h"

UCLASS()
class PROJECTF_API APFMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	APFMonsterCharacter();

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster")
	float MaxHP = 100.f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Monster")
	float CurrentHP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster")
	float AttackDamage = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster")
	float AttackRange = 150.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster")
	float AttackCooldown = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster")
	bool IsDead = false;
	
	
	
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable)
	void TakeDamageFromMonster(float DamageAmount);
	
	UFUNCTION(BlueprintCallable)
	void Attack();
	
	UFUNCTION(BlueprintCallable)
	void Die();
	
	
};
