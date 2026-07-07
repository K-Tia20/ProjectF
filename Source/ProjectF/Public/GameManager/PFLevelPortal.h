// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PFLevelPortal.generated.h"

UCLASS()
class PROJECTF_API APFLevelPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APFLevelPortal();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PortalMesh;

/*
	UFUNCTION()
	void OnOverlapBegin(UCLASS(meta=(AllowedClasses="PrimitiveComponent")) UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION()
	void OnOverlapEnd(UCLASS(meta=(AllowedClasses="PrimitiveComponent")) UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
*/
		
	void Interact();
};
