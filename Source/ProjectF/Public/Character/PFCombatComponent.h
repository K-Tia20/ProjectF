#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"       
#include "PFCombatComponent.generated.h"  

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Pistol UMETA(DisplayName = "Pistol"),
    Rifle UMETA(DisplayName = "Rifle"),
    Sword UMETA(DisplayName = "Sword") 
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTF_API UPFCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public: 
    UPFCombatComponent();

    
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    bool bIsGrenadeOnCooldown = false;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    float GrenadeCooldown = 5.0f;
 
    FTimerHandle GrenadeCooldownTimerHandle;

    void ResetGrenadeCooldown() { bIsGrenadeOnCooldown = false; };

public: 
    UPROPERTY(VisibleAnywhere, Category = "Combat|Weapon")
    UStaticMeshComponent* CurrentWeaponMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
    TSubclassOf<UCameraShakeBase> FireCameraShakeClass;
   
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Grenade")
    UStaticMeshComponent* HandGrenadeMesh;
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteMeleeHitCheck();

    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    void EquipWeapon(UStaticMesh* WeaponMesh);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
    class UAnimMontage* PistolFireMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
    class UAnimMontage* RifleFireMontage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
    class UAnimMontage* SwordMontage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    FName DefaultPistolBoneName = TEXT("weapon_r");
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FireWeapon();

    // [추가] 연사 시작 / 종료 함수 선언
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartFire(); // [추가]

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopFire();  // [추가]

    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    bool SwitchWeapon(EWeaponType NewWeapon);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grenade")
    class UAnimMontage* ThrowMontage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grenade")
    TSubclassOf<class APFGrenade> GrenadeClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grenade")
    FName HandSocketName = TEXT("Hand_R_Socket");

    void PlayThrowAnimation();

    UFUNCTION(BlueprintCallable, Category = "Combat|Grenade")
    void ExecuteThrow();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    EWeaponType CurrentWeapon = EWeaponType::Pistol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    int32 CurrentAmmo = 15;

    // 최대탄창
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    int32 MaxAmmo = 30;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PistolCurrentAmmo = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 RifleCurrentAmmo = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 SwordCurrentAmmo = 5;
    
    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    void AddAmmo(EWeaponType WeaponType, int32 Amount);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FireRate = 0.15f;
    
    bool bCanFire = true;
    
    FTimerHandle FireTimerHandle;
    
    void ResetFire();

    
    UFUNCTION(BlueprintCallable, Category = "Combat|Grenade")
    void AddGrenades(int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Combat|Grenade")
    int32 GetGrenadeCount() const { return GrenadeCount; }
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    int32 GrenadeCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.f;

private:
    
    FTimerHandle AutoFireTimerHandle; 
    bool bIsAutomatic = false;       
};