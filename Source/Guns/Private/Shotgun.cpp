// Fill out your copyright notice in the Description page of Project Settings.

#include "Shotgun.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"    
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"

AShotgun::AShotgun()
{
    // ── 샷건 기본값 설정 ──────────────────────────────────
    PelletCount  = 8;
    SpreadAngle  = 6.0f;
    Damage       = 15.0f;
    MaxRange     = 2500.0f;
    FireRate     = 0.9f;

    MagazineSize = 6;
    ReloadTime   = 3.0f;

    RecoilStrength       = 6.0f;
    RecoilRecoverySpeed  = 4.0f;
    RecoilPitchMax       = 8.0f;
    RecoilYawRandom      = 2.5f;
    CameraShakeIntensity = 3.0f;

    UE_LOG(LogTemp, Log,
        TEXT("[Shotgun] 생성자 호출 — 샷건 파라미터 적용 (펠릿 %d, 분산 %.1f°)"),
        PelletCount, SpreadAngle);
}

int32 AShotgun::DoFireTrace(APlayerController* PC,
                             const FVector& CamLoc,
                             const FRotator& CamRot)
{
    int32 HitCount = 0;

    for (int32 i = 0; i < PelletCount; i++)
    {
        FRotator SpreadRot = CamRot;
        SpreadRot.Pitch += FMath::RandRange(-SpreadAngle, SpreadAngle);
        SpreadRot.Yaw   += FMath::RandRange(-SpreadAngle, SpreadAngle);

        FVector Direction  = SpreadRot.Vector();
        FVector TraceStart = CamLoc;
        FVector TraceEnd   = CamLoc + Direction * MaxRange;

        FHitResult Hit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(OwnerCharacter);
        QueryParams.bTraceComplex = false;
        QueryParams.bReturnPhysicalMaterial = true;

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams);

        DrawDebugLine(GetWorld(), TraceStart,
            bHit ? Hit.ImpactPoint : TraceEnd,
            FColor::Orange, false, 0.5f, 0, 1.5f);

        if (bHit && Hit.GetActor())
        {
            HitCount++;

            UE_LOG(LogWeapon, Warning,
                TEXT("  └ 펠릿 #%d 명중: %s (Class:%s) | 거리:%.0f | 데미지:%.1f"),
                i + 1, *Hit.GetActor()->GetName(),
                *Hit.GetActor()->GetClass()->GetName(),
                Hit.Distance, Damage);

            float Applied = UGameplayStatics::ApplyPointDamage(
                Hit.GetActor(), Damage, Direction, Hit, PC, this,
                UDamageType::StaticClass());

            UE_LOG(LogWeapon, Warning,
                TEXT("  └ ApplyPointDamage 반환값: %.1f"), Applied);

            if (ImpactEffect)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(), ImpactEffect,
                    Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
            }
        }
    }

    return HitCount;
}