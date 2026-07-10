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
	// 딱 생성자 하나만 깔끔하게 남겨둡니다.
	APFGrenade();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;
};