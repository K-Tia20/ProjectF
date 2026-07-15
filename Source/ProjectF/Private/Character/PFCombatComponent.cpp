#include "Character/PFCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameManager/PFPlayerController.h"
#include "Monster/PFMonsterCharacter.h" 
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "weapon/PFGrenade.h"
#include "GameManager/PFGameState.h" 

UPFCombatComponent::UPFCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    HandGrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandGrenadeMesh"));
}

void UPFCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentWeapon = EWeaponType::Pistol;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->GetMesh() && HandGrenadeMesh)
    {
        HandGrenadeMesh->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            HandSocketName
        );
        HandGrenadeMesh->SetVisibility(false);
        HandGrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void UPFCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPFCombatComponent::EquipWeapon(UStaticMesh* WeaponMesh)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    
    if (CurrentWeaponMesh)
    {
        CurrentWeaponMesh->DestroyComponent();
        CurrentWeaponMesh = nullptr; 
    }

    
    if (!WeaponMesh)
    {
        return; 
    }
    

    
    CurrentWeaponMesh = NewObject<UStaticMeshComponent>(Owner);
    CurrentWeaponMesh->RegisterComponent();
    CurrentWeaponMesh->SetStaticMesh(WeaponMesh);

    
    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh())
    {
        CurrentWeaponMesh->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
    }
}



void UPFCombatComponent::SwitchWeapon(EWeaponType NewWeapon)
{
    int32 CurrentLevel = 1; 

    if (GetWorld())
    {
        if (APFGameState* GameState = GetWorld()->GetGameState<APFGameState>())
        {   
            CurrentLevel = GameState->GetCurrentLevel(); 
        }
    } 
    
    if (NewWeapon == EWeaponType::Rifle && CurrentLevel < 2)
    {  
        UE_LOG(LogTemp, Warning, TEXT("라이플은 레벨 2부터 사용 가능 (현재 레벨: %d)"), CurrentLevel);
        return; 
    }

    if (NewWeapon == EWeaponType::Sword && CurrentLevel < 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("대검은 레벨 3부터 사용 가능 (현재 레벨: %d)"), CurrentLevel);
        return;
    }
    
    CurrentWeapon = NewWeapon;
    
    AActor* Owner = GetOwner();
    if (Owner == nullptr) return;

    
    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh())
    {
        if (CurrentWeapon == EWeaponType::Pistol)
        {
            if (CurrentWeaponMesh)
            {
                CurrentWeaponMesh->DestroyComponent();
                CurrentWeaponMesh = nullptr;
            }
            Character->GetMesh()->UnHideBoneByName(DefaultPistolBoneName);
        }
        else 
        {
            Character->GetMesh()->HideBoneByName(DefaultPistolBoneName, EPhysBodyOp::PBO_None);
        }
    }

    APawn* OwnerPawn = Cast<APawn>(Owner);
    if (OwnerPawn == nullptr) return;

    APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController());
    
    if (PlayerController != nullptr)
    {
        FString NameToShow = TEXT("기본 무기");
        if (CurrentWeapon == EWeaponType::Rifle)
        {
            NameToShow = TEXT("Assault Rifle");
        }
        else if (CurrentWeapon == EWeaponType::Pistol)
        {
            NameToShow = TEXT("Pistol");
        }
        else if (CurrentWeapon == EWeaponType::Sword)
        {
            NameToShow = TEXT("Sword");
        }

        PlayerController->UpdateWeaponNameUI(NameToShow);
    }

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
    case EWeaponType::Sword:
        MaxAmmo = 5;
        AttackDamage = 100.f;
        AttackRange = 10000.f;
        break;
    }
    
    CurrentAmmo = MaxAmmo; 
}

void UPFCombatComponent::AddAmmo(EWeaponType WeaponType, int32 Amount)
{
    if (CurrentWeapon == WeaponType)
    {
        CurrentAmmo = FMath::Clamp(CurrentAmmo + Amount, 0, MaxAmmo);
        AActor* Owner = GetOwner();
        if (Owner)
        {
            if (APawn* OwnerPawn = Cast<APawn>(Owner))
            {
                if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
                {
                    PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo);
                }
            }
        }
    }
}

void UPFCombatComponent::PlayThrowAnimation()
{
    if (GrenadeCount <= 0) return;

    AActor* Owner = GetOwner();
    ACharacter* Character = Cast<ACharacter>(Owner);
    
    if (Character && ThrowMontage)
    {
        if (HandGrenadeMesh)
        {
            HandGrenadeMesh->SetVisibility(true);
        }
        Character->PlayAnimMontage(ThrowMontage);
    }
}

void UPFCombatComponent::ExecuteThrow()
{
    AActor* Owner = GetOwner();
    ACharacter* Character = Cast<ACharacter>(Owner);

    if (!Character || !GrenadeClass || GrenadeCount <= 0) return;

    if (HandGrenadeMesh)
    {
        HandGrenadeMesh->SetVisibility(false);
    }

    GrenadeCount--;
    
    FVector SpawnLocation = Character->GetMesh()->GetSocketLocation(HandSocketName); 
    FRotator SpawnRotation = Character->GetControlRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Character;

    GetWorld()->SpawnActor<APFGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, SpawnParams);
}

void UPFCombatComponent::FireWeapon()
{
    if (CurrentAmmo <= 0) return;
    
    CurrentAmmo--;
    
    AActor* Owner = GetOwner();

    // ---------------------------------------------------------
    // [3번 추가] 무기 타입에 따라 다른 애니메이션 몽타주 재생
    // ---------------------------------------------------------
    ACharacter* Character = Cast<ACharacter>(Owner);
    if (Character && Character->GetMesh())
    {
        UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            if (CurrentWeapon == EWeaponType::Pistol && PistolFireMontage)
            {
                AnimInstance->Montage_Play(PistolFireMontage, 1.0f);
            }
            else if (CurrentWeapon == EWeaponType::Rifle && RifleFireMontage)
            {
                AnimInstance->Montage_Play(RifleFireMontage, 1.0f);
            }
            else if (CurrentWeapon == EWeaponType::Sword && SwordMontage)
            {
                AnimInstance->Montage_Play(SwordMontage, 1.0f);
            }
        }
    }

    if (Owner)
    {
        if (APawn* OwnerPawn = Cast<APawn>(Owner))
        {
            if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
            {
                PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo);
            }
        }
    }

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

void UPFCombatComponent::AddGrenades(int32 Amount)
{
    if (Amount <= 0) return;
    GrenadeCount += Amount;
    GrenadeCount = FMath::Clamp(GrenadeCount, 0, 3);
}