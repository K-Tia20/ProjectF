#include "Character/PFCombatComponent.h"
#include "DrawDebugHelpers.h"
#include "GameManager/PFPlayerController.h"
#include "Monster/PFMonsterCharacter.h" 
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "weapon/PFGrenade.h"
#include "TimerManager.h" 
#include "Engine/OverlapResult.h"
//#include "GameManager/PFGameState.h"  

UPFCombatComponent::UPFCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    HandGrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandGrenadeMesh"));
}

void UPFCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    SwitchWeapon(EWeaponType::Pistol);

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
       
        FName TargetSocketName = TEXT("WeaponSocket"); 

        if (CurrentWeapon == EWeaponType::Sword)
        {
            TargetSocketName = TEXT("SwordSocket"); 
        }

        CurrentWeaponMesh->AttachToComponent(
            Character->GetMesh(), 
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
            TargetSocketName
        );
    }
}

void UPFCombatComponent::SwitchWeapon(EWeaponType NewWeapon)
{
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
            
            if (CurrentWeaponMesh)
            {
                FName TargetSocketName = TEXT("WeaponSocket");
                
                if (CurrentWeapon == EWeaponType::Sword)
                {
                    TargetSocketName = TEXT("SwordSocket");
                }
                
                CurrentWeaponMesh->AttachToComponent(
                    Character->GetMesh(), 
                    FAttachmentTransformRules::SnapToTargetIncludingScale, 
                    TargetSocketName
                );
            }
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
        AttackRange = 150.f; // 검 사거리는 이제 C++ SphereTrace에서 사용할 거라 그대로 둬도 됨
        break;
    }
    
    CurrentAmmo = MaxAmmo; 
    
    if (PlayerController != nullptr)
    {
        PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo); 
    }
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
    
    if (!Character || !GrenadeClass || GrenadeCount <= 0 || bIsGrenadeOnCooldown) return;

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

    bIsGrenadeOnCooldown = true;

    APFPlayerController* PlayerController = Cast<APFPlayerController>(Character->GetController());
    if (PlayerController)
    {
        PlayerController->UpdateGrenadeCooldownUI(GrenadeCooldown); 
    }

    GetWorld()->GetTimerManager().SetTimer(
        GrenadeCooldownTimerHandle, 
        this, 
        &UPFCombatComponent::ResetGrenadeCooldown, 
        GrenadeCooldown, 
        false
    );
}


void UPFCombatComponent::FireWeapon()
{
    if (CurrentAmmo <= 0) return;
    
    CurrentAmmo--;
    
    AActor* Owner = GetOwner();

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

    // 현재 무기가 '검'일 경우, 즉시 나가는 라인트레이스(총기용)를 무시합니다
    if (CurrentWeapon == EWeaponType::Sword)
    {
        // 검 판정은 애니메이션 노티파이가 별도의 함수를 호출하여 처리합니다.
        return; 
    }

    // ==========================================
    //  총기류 전용 라인트레이스
    // ==========================================

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
             FName HitBoneName = HitResult.BoneName;

             if (HitBoneName == TEXT("head"))
             {
                 UE_LOG(LogTemp, Warning, TEXT("🎯 헤드샷 크리티컬 히트!!"));
                 Monster->TakeDamageFromMonster(AttackDamage * 2.0f, Owner);
             }
             else
             {
                 Monster->TakeDamageFromMonster(AttackDamage, Owner);
             }

             if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
             {
                 PlayerController->ShowHitMarker();
             }
          }
       }
    }
}


void UPFCombatComponent::ExecuteMeleeHitCheck()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    
    FVector StartLocation = Owner->GetActorLocation() + (Owner->GetActorForwardVector() * 100.f); 

    
    float AttackRadius = 150.f; 

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(Owner); 

    
    bool bHit = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        StartLocation,
        FQuat::Identity,
        ECC_Pawn, 
        FCollisionShape::MakeSphere(AttackRadius),
        CollisionParams
    );

    //디버깅용 (테스트끝나면지우기)
    DrawDebugSphere(GetWorld(), StartLocation, AttackRadius, 12, FColor::Green, false, 2.f);

    if (bHit)
    {
        
        TArray<AActor*> HitActors;

        for (const FOverlapResult& Overlap : OverlapResults)
        {
            AActor* HitActor = Overlap.GetActor();
            
            
            if (HitActor && !HitActors.Contains(HitActor))
            {
                APFMonsterCharacter* Monster = Cast<APFMonsterCharacter>(HitActor);
                if (Monster)
                {
                    
                    Monster->TakeDamageFromMonster(AttackDamage, Owner);
                    
                    
                    HitActors.Add(HitActor);
                    
                    UE_LOG(LogTemp, Warning, TEXT("⚔️ 광역 베기 적중: %s"), *HitActor->GetName());
                }
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