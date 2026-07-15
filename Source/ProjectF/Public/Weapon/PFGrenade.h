#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PFGrenade.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTF_API APFGrenade : public AActor
{
	GENERATED_BODY()
    
public:    

	APFGrenade();

protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;
	
	// 폭발 이펙트
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	class UNiagaraComponent* ExplosionEffect;
	
	// 폭발 사운드
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* ExplosionSound;
	
	// 폭발 범위
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* ExplosionSphereComponent;
	
	// 폭발 데미지
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ExplosionDamage = 100.0f;
	
	// 폭발 딜레이 (3초)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FuseTime = 3.0f;

private:

	FTimerHandle ExplodeTimerHandle;

	// 폭발 함수
	void Explode();
};