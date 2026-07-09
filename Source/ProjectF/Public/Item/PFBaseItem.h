// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PFBaseItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class PROJECTF_API APFBaseItem : public AActor
{
	GENERATED_BODY()
    
public:    
	APFBaseItem();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Components")
	UStaticMeshComponent* ItemMesh;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 4. 자식 클래스(탄약, 힐팩 등)들이 각자 다르게 구현할 핵심 가상 함수
	virtual void OnPickup(class APFPlayerCharacter* PlayerCharacter);
};