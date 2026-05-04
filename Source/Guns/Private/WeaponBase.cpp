// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "GunsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Engine/DamageEvents.h"

DEFINE_LOG_CATEGORY(LogWeapon);

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
    MuzzlePoint->SetupAttachment(WeaponMesh);
    // 에디터에서 MuzzlePoint를 총구 끝으로 위치 조정할 것
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentAmmo = MagazineSize;
    OwnerCharacter = Cast<ACharacter>(GetOwner());

    UE_LOG(LogWeapon, Log,
        TEXT("[%s] 무기 스폰 완료 | 탄창:%d | 발사간격:%.2fs | 펠릿:%d | 분산:%.1f° | 반동(P:%.1f Y:%.1f)"),
        *GetName(), MagazineSize, FireRate, PelletCount, SpreadAngle,
        RecoilPitchMax, RecoilYawRandom);
}

void AWeaponBase::Fire()
{
    if (bIsReloading)
    {
        UE_LOG(LogWeapon, Verbose, TEXT("[%s] 발사 차단: 재장전 중"), *GetName());
        return;
    }
    if (!bCanFire)
    {
        UE_LOG(LogWeapon, Verbose, TEXT("[%s] 발사 차단: 쿨다운"), *GetName());
        return;
    }

    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogWeapon, Warning, TEXT("[%s] 탄창 비어있음 → 자동 재장전"), *GetName());

        if (EmptySound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EmptySound, GetActorLocation());
        }
        Reload();
        return;
    }

    PerformFire();
}

void AWeaponBase::PerformFire()
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogWeapon, Error, TEXT("[%s] PerformFire 실패: OwnerCharacter 없음"), *GetName());
        return;
    }

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC)
    {
        UE_LOG(LogWeapon, Error, TEXT("[%s] PerformFire 실패: PlayerController 없음"), *GetName());
        return;
    }

    // 탄약 감소
    CurrentAmmo--;

    UE_LOG(LogWeapon, Log,
        TEXT("[%s] 발사! 남은 탄약: %d/%d | 펠릿 %d발 발사"),
        *GetName(), CurrentAmmo, MagazineSize, PelletCount);

    // 발사 쿨다운
    bCanFire = false;
    GetWorldTimerManager().SetTimer(
        FireRateTimerHandle, this, &AWeaponBase::ResetFireCooldown, FireRate, false);

    FVector  CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

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

    UE_LOG(LogWeapon, Log,
        TEXT("[%s] 발사 결과: %d/%d 펠릿 명중 (총 데미지 %.1f)"),
        *GetName(), HitCount, PelletCount, HitCount * Damage);

    if (MuzzleFlashEffect && MuzzlePoint)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect, MuzzlePoint);
    }

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    if (CameraShakeClass)
    {
        PC->ClientStartCameraShake(CameraShakeClass, CameraShakeIntensity);
        UE_LOG(LogWeapon, Verbose, TEXT("[%s] 카메라 쉐이크 적용 (강도:%.1f)"),
            *GetName(), CameraShakeIntensity);
    }

    if (AGunsCharacter* SChar = Cast<AGunsCharacter>(OwnerCharacter))
    {
        SChar->AddRecoil(RecoilStrength, RecoilPitchMax, RecoilYawRandom);
    }

    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogWeapon, Log, TEXT("[%s] 마지막 탄 발사 → 다음 틱에 자동 재장전"), *GetName());
        GetWorldTimerManager().SetTimerForNextTick([this]() { Reload(); });
    }
}

void AWeaponBase::Reload()
{
    if (bIsReloading)
    {
        UE_LOG(LogWeapon, Verbose, TEXT("[%s] 재장전 차단: 이미 재장전 중"), *GetName());
        return;
    }
    if (CurrentAmmo == MagazineSize)
    {
        UE_LOG(LogWeapon, Verbose, TEXT("[%s] 재장전 불필요: 탄창 가득참"), *GetName());
        return;
    }

    bIsReloading = true;

    UE_LOG(LogWeapon, Log,
        TEXT("[%s] 재장전 시작 (%.1f초) | 현재:%d/%d"),
        *GetName(), ReloadTime, CurrentAmmo, MagazineSize);

    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
    }

    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle, this, &AWeaponBase::OnReloadComplete, ReloadTime, false);
}

void AWeaponBase::OnReloadComplete()
{
    CurrentAmmo  = MagazineSize;
    bIsReloading = false;

    UE_LOG(LogWeapon, Log,
        TEXT("[%s] 재장전 완료! 탄약 %d/%d"),
        *GetName(), CurrentAmmo, MagazineSize);
}

void AWeaponBase::ResetFireCooldown()
{
    bCanFire = true;
    UE_LOG(LogWeapon, Verbose, TEXT("[%s] 발사 쿨다운 해제"), *GetName());
}
