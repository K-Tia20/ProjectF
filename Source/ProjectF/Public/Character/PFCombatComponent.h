#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PFCombatComponent.generated.h"

// 무기 종류를 정의하는 열거형 (Enum)
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
    
    // 장착된 무기 메시를 담을 변수
    UPROPERTY(VisibleAnywhere, Category = "Combat|Weapon")
    UStaticMeshComponent* CurrentWeaponMesh;

    //  던지기 전 모션 동안 손에 쥐고 있을 눈속임용 가짜 수류탄 메쉬
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Grenade")
    UStaticMeshComponent* HandGrenadeMesh;

    // 무기 스왑 함수
    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    void EquipWeapon(UStaticMesh* WeaponMesh);
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FireWeapon();

    // 무기 교체 함수 
    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    void SwitchWeapon(EWeaponType NewWeapon);
    
    // 수류탄 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grenade")
    class UAnimMontage* ThrowMontage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grenade")
    TSubclassOf<class APFGrenade> GrenadeClass;
    
    // [추가] 캐릭터 오른손에 등록한 수류탄용 소켓 이름 (에디터에서 수정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grenade")
    FName HandSocketName = TEXT("Hand_R_Socket");

    // 수류탄 애니메이션
    void PlayThrowAnimation();

    // 실제 수류탄이 스폰되어 날아가는 함수 (애니메이션 노티파이 바인딩용)
    UFUNCTION(BlueprintCallable, Category = "Combat|Grenade")
    void ExecuteThrow();

    // 현재 들고 있는 무기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    EWeaponType CurrentWeapon = EWeaponType::Rifle;

    // 현재 남은 총알
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    int32 CurrentAmmo = 30;

    // 최대탄창
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    int32 MaxAmmo = 30;
    
    //총알 늘려주는 함수 [추가] #88
    UFUNCTION(BlueprintCallable, Category = "Combat|Weapon")
    void AddAmmo(EWeaponType WeaponType, int32 Amount);

    // 수류탄 갯수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    int32 GrenadeCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.f;
};