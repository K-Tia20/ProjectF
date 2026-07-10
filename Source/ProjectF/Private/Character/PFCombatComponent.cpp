#include "Character/PFCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "Monster/PFMonsterCharacter.h" 
#include "GameFramework/Pawn.h"

UPFCombatComponent::UPFCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPFCombatComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPFCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UPFCombatComponent::EquipWeapon(UStaticMesh* WeaponMesh)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // 기존에 무기가 있다면 제거
    if (CurrentWeaponMesh)
    {
        CurrentWeaponMesh->DestroyComponent();
    }

    //  새로운 무기 메시 컴포넌트 생성
    CurrentWeaponMesh = NewObject<UStaticMeshComponent>(Owner);
    CurrentWeaponMesh->RegisterComponent();
    CurrentWeaponMesh->SetStaticMesh(WeaponMesh);

    // 캐릭터 손 소켓에 붙이기 
    USkeletalMeshComponent* CharacterMesh = Cast<ACharacter>(Owner)->GetMesh();
    CurrentWeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
}


// 무기 교체 로직 무기마다 사거리 데미지가 다름 
void UPFCombatComponent::SwitchWeapon(EWeaponType NewWeapon)
{
    CurrentWeapon = NewWeapon;

    switch (CurrentWeapon)
    {
    case EWeaponType::Pistol:
        MaxAmmo = 15;
        AttackDamage = 15.f;
        AttackRange = 3000.f;
        break;
    case EWeaponType::Rifle:
        MaxAmmo = 30;
        AttackDamage = 25.f;
        AttackRange = 5000.f;
        break;
    case EWeaponType::Sniper:
        MaxAmmo = 5;
        AttackDamage = 100.f;
        AttackRange = 10000.f;
        break;
    }

    
    CurrentAmmo = MaxAmmo; 
    
    UE_LOG(LogTemp, Warning, TEXT("[Combat] 무기 교체됨! 현재 무기 탄약: %d, 데미지: %f"), CurrentAmmo, AttackDamage);
}

void UPFCombatComponent::FireWeapon()
{
    
    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] 총알이 부족하여 사격할 수 없습니다!"));
        return;
    }

    
    CurrentAmmo--;
    UE_LOG(LogTemp, Warning, TEXT("[Combat] 사격! 남은 총알: %d / %d"), CurrentAmmo, MaxAmmo);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* OwnerPawn = Cast<APawn>(Owner);
    if (!OwnerPawn || !OwnerPawn->GetController()) return;

    FVector StartLocation;
    FRotator StartRotation;
    OwnerPawn->GetController()->GetPlayerViewPoint(StartLocation, StartRotation);

    FVector EndLocation = StartLocation + (StartRotation.Vector() * AttackRange);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Owner); 

    bool bHit = GetWorld()->LineTraceSingleByChannel(
       HitResult,
       StartLocation,
       EndLocation,
       ECC_Visibility,
       CollisionParams
    );

    // 빨간선 디버그 (테스트 끝나면 지우세요)
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f, 0, 1.f);

    if (bHit)
    {
       AActor* HitActor = HitResult.GetActor();
       if (HitActor)
       {
          APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
          if (Monster)
          {
             Monster->TakeDamageFromMonster(AttackDamage, Owner);
          }
       }
    }
}