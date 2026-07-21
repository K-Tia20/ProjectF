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
#include "Monster/PFSuicideZombie.h"
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

bool UPFCombatComponent::SwitchWeapon(EWeaponType NewWeapon)
{
    /*
    int32 CurrentWave = 1; 
    if (GetWorld())
    {
        if (APFGameState* GameState = GetWorld()->GetGameState<APFGameState>())
        {   
            CurrentWave = GameState->GetCurrentLevel(); 
        }
    } 
    
    
    if (NewWeapon == EWeaponType::Rifle && CurrentWave < 2)
    {  
        UE_LOG(LogTemp, Warning, TEXT("라이플은 레벨 2부터 사용 가능 (현재 레벨: %d)"), CurrentWave);
        return false; 
    }

    if (NewWeapon == EWeaponType::Sword && CurrentWave < 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("대검은 레벨 3부터 사용 가능 (현재 레벨: %d)"), CurrentWave);
        return false;
    }
    */
    
    switch (CurrentWeapon)
    {
    case EWeaponType::Pistol:
        PistolCurrentAmmo = CurrentAmmo;
        break;
    case EWeaponType::Rifle:
        RifleCurrentAmmo = CurrentAmmo;
        break;
    case EWeaponType::Sword:
        SwordCurrentAmmo = CurrentAmmo;
        break;
    }
    
   
    CurrentWeapon = NewWeapon;
    
    AActor* Owner = GetOwner();
    if (Owner == nullptr) return false;
    
    
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
    APFPlayerController* PlayerController = OwnerPawn ? Cast<APFPlayerController>(OwnerPawn->GetController()) : nullptr;
    
    
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
        CurrentAmmo = PistolCurrentAmmo;
        AttackDamage = 15.f;
        AttackRange = 3000.f;
        FireRate = 0.25f;
        break;
    case EWeaponType::Rifle:
        MaxAmmo = 30;
        CurrentAmmo = RifleCurrentAmmo;
        AttackDamage = 25.f;
        AttackRange = 5000.f;
        FireRate = 0.1f;
        break;
    case EWeaponType::Sword:
        MaxAmmo = 5;
        CurrentAmmo = SwordCurrentAmmo;
        AttackDamage = 100.f;
        AttackRange = 150.f; 
        FireRate = 1.0f;
        break;
    }
    
    
    if (PlayerController != nullptr)
    {
        PlayerController->UpdateAmmoUI(CurrentAmmo, MaxAmmo); 
    }

    return true; 
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
    
    if (!bCanFire || CurrentAmmo <= 0) return;
    
    
    CurrentAmmo--;
    bCanFire = false;

    GetWorld()->GetTimerManager().SetTimer(
        FireTimerHandle,
        this,
        &UPFCombatComponent::ResetFire,
        FireRate, 
        false
    );

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

    
    if (CurrentWeapon == EWeaponType::Sword)
    {
        return; 
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
                FName HitBoneName = HitResult.BoneName;
                UPrimitiveComponent* HitComponent = HitResult.GetComponent();
                bool bWeakPointHit = false;

                
                if (APFSuicideZombie* SuicideZombie = Cast<APFSuicideZombie>(Monster))
                {
                    if (SuicideZombie->IsWeakPointComponent(HitComponent))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("자폭 좀비 약점 명중"));
                        SuicideZombie->TriggerWeakPointExplosion(); 
                        bWeakPointHit = true; 
                    }
                }

                // 약점이 아닌 경우 일반/헤드샷 데미지 처리
                if (!bWeakPointHit) 
                {
                    if (HitBoneName == TEXT("head"))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("🎯 헤드샷 크리티컬 히트!!"));
                        Monster->TakeDamageFromMonster(AttackDamage * 2.0f, Owner);
                    }
                    else
                    {
                        Monster->TakeDamageFromMonster(AttackDamage, Owner);
                    }
                }

                if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerPawn->GetController()))
                {
                    PlayerController->ShowHitMarker();
                }
            }
        }
    }
}


void UPFCombatComponent::ResetFire()
{
    bCanFire = true;
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
                    
                    UE_LOG(LogTemp, Warning, TEXT("광역 베기 적중: %s"), *HitActor->GetName());
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