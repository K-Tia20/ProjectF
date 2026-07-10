#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PFCombatComponent.generated.h"

// [추가] 무기 종류를 정의하는 열거형 (Enum)
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol UMETA(DisplayName = "Pistol"),
	Rifle UMETA(DisplayName = "Rifle"),
	Sniper UMETA(DisplayName = "Sniper")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTF_API UPFCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public: 
	UPFCombatComponent();

protected:
	virtual void BeginPlay() override;

public: 
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
	// [추가] 장착된 무기 메시를 담을 변수
	UPROPERTY(VisibleAnywhere, Category = "Combat|Weapon")
	UStaticMeshComponent* CurrentWeaponMesh;

	// [추가] 무기 스왑 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void EquipWeapon(UStaticMesh* WeaponMesh);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FireWeapon();

	// 무기 교체 함수 
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
	void SwitchWeapon(EWeaponType NewWeapon);

	

	// 현재 들고 있는 무기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	EWeaponType CurrentWeapon = EWeaponType::Rifle;

	// 현재 남은 총알
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	int32 CurrentAmmo = 30;

	//최대탄창
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	int32 MaxAmmo = 30;

	//수류탄 갯수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	int32 GrenadeCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 25.f;
};