#include "Character/PFDashComponent.h" 
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h" 
#include "GameManager/PFPlayerController.h"

UPFDashComponent::UPFDashComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPFDashComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPFDashComponent::ExecuteDash()
{
    if (!bCanDash) return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    
    FVector DashDirection = OwnerCharacter->GetLastMovementInputVector();
    if (DashDirection.IsNearlyZero())
    {
       DashDirection = OwnerCharacter->GetActorForwardVector();
    }
    DashDirection.Normalize();

   
    OwnerCharacter->LaunchCharacter(DashDirection * DashImpulse, true, true);

    
    if (DashNiagaraVFX && OwnerCharacter->GetMesh())
    {
       UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
          DashNiagaraVFX,
          OwnerCharacter->GetMesh(),
          NAME_None,
          FVector::ZeroVector,
          FRotator::ZeroRotator,
          EAttachLocation::SnapToTargetIncludingScale,
          true,  
          true   
       );

       if (NiagaraComp)
       {
          
          TWeakObjectPtr<UNiagaraComponent> WeakNiagara = NiagaraComp;

          FTimerHandle VFXTimerHandle;
          GetWorld()->GetTimerManager().SetTimer(
             VFXTimerHandle,
             [WeakNiagara]()
             {
                
                if (WeakNiagara.IsValid())
                {
                   WeakNiagara->Deactivate();
                }
             },
             0.6f, 
             false
          );
       }
    }

    
   bCanDash = false;
   
   if (APFPlayerController* PlayerController = Cast<APFPlayerController>(OwnerCharacter->GetController()))
   {
      PlayerController->UpdateDashCooldownUI(DashCooldown);
   }

   GetWorld()->GetTimerManager().SetTimer(
       DashTimerHandle,
       this,
       &UPFDashComponent::ResetDash,
       DashCooldown,
       false
   );
}

void UPFDashComponent::ResetDash()
{
   bCanDash = true;
}