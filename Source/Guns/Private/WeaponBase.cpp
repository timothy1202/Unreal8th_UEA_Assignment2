// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "GunsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Engine/DamageEvents.h"

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
}

// ─────────────────────────────────────────────────────────
//  Fire — 발사 진입점 (캐릭터에서 호출)
// ─────────────────────────────────────────────────────────
void AWeaponBase::Fire()
{
    // 발사 불가 조건 체크
    if (bIsReloading || !bCanFire)
    {
        return;
    }

    if (CurrentAmmo <= 0)
    {
        // 빈 탄창 사운드
        if (EmptySound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this, EmptySound, GetActorLocation()
            );
        }
        Reload();
        return;
    }

    PerformFire();
}

// ─────────────────────────────────────────────────────────
//  PerformFire — LineTrace + ApplyPointDamage 핵심 로직
// ─────────────────────────────────────────────────────────
void AWeaponBase::PerformFire()
{
    if (!OwnerCharacter) return;

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC) return;

    // 탄약 감소
    CurrentAmmo--;

    // 발사 쿨다운 시작
    bCanFire = false;
    GetWorldTimerManager().SetTimer(
        FireRateTimerHandle,
        this,
        &AWeaponBase::ResetFireCooldown,
        FireRate,
        false
    );

    // ── 카메라 기준 시작 위치/방향 가져오기 ──────────────
    FVector  CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    // ── PelletCount만큼 LineTrace 반복 (샷건 핵심) ────────
    for (int32 i = 0; i < PelletCount; i++)
    {
        // ── 분산 각도 랜덤 적용 ───────────────────────────
        FRotator SpreadRot = CamRot;
        SpreadRot.Pitch += FMath::RandRange(-SpreadAngle, SpreadAngle);
        SpreadRot.Yaw   += FMath::RandRange(-SpreadAngle, SpreadAngle);

        FVector Direction = SpreadRot.Vector();
        FVector TraceStart = CamLoc;
        FVector TraceEnd   = CamLoc + Direction * MaxRange;

        // ── LineTrace 설정 ────────────────────────────────
        FHitResult Hit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);              // 총 자신 무시
        QueryParams.AddIgnoredActor(OwnerCharacter);    // 플레이어 무시
        QueryParams.bTraceComplex = true;               // 정밀 메시 충돌
        QueryParams.bReturnPhysicalMaterial = true;     // 재질 정보 반환

        // ★ LineTraceSingleByChannel ★
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,              // 충돌 결과 저장
            TraceStart,       // 시작점 (카메라 위치)
            TraceEnd,         // 끝점 (사거리 끝)
            ECC_Visibility,   // 충돌 채널
            QueryParams
        );

        // 개발용 디버그 라인 (완성 후 이 블록 삭제)
        DrawDebugLine(
            GetWorld(),
            TraceStart,
            bHit ? Hit.ImpactPoint : TraceEnd,
            FColor::Orange,
            false,    // 영구 표시 여부
            0.5f,     // 표시 시간 (초)
            0,
            1.5f      // 선 두께
        );

        // ── 충돌 처리 ──────────────────────────────────────
        if (bHit && Hit.GetActor())
        {
            // ★ ApplyPointDamage ★
            // → 맞은 액터의 TakeDamage 자동 호출
            UGameplayStatics::ApplyPointDamage(
                Hit.GetActor(),                  // 피해 받을 대상
                Damage,                           // 피해량
                Direction,                        // 피해 방향 (넉백 방향)
                Hit,                              // HitResult (맞은 위치, 본 등)
                PC,                               // 피해 유발자 Controller
                this,                             // 피해 유발자 Actor (총)
                UDamageType::StaticClass()        // 피해 타입
            );

            // 임팩트 이펙트 스폰
            if (ImpactEffect)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                    ImpactEffect,
                    Hit.ImpactPoint,
                    Hit.ImpactNormal.Rotation()
                );
            }
        }
    }

    // ── 머즐 플래시 ───────────────────────────────────────
    if (MuzzleFlashEffect && MuzzlePoint)
    {
        UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlashEffect,
            MuzzlePoint
        );
    }

    // ── 발사 사운드 ───────────────────────────────────────
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, FireSound, GetActorLocation()
        );
    }

    // ── 카메라 쉐이크 ─────────────────────────────────────
    if (CameraShakeClass)
    {
        PC->ClientStartCameraShake(
            CameraShakeClass, CameraShakeIntensity
        );
    }

    // ── 반동 전달 (캐릭터에서 처리) ───────────────────────
    if (AGunsCharacter* SChar =
        Cast<AGunsCharacter>(OwnerCharacter))
    {
        SChar->AddRecoil(RecoilStrength, RecoilPitchMax, RecoilYawRandom);
    }

    // ── 탄창 비면 자동 재장전 ─────────────────────────────
    if (CurrentAmmo <= 0)
    {
        GetWorldTimerManager().SetTimerForNextTick([this]()
        {
            Reload();
        });
    }
}

// ─────────────────────────────────────────────────────────
//  Reload
// ─────────────────────────────────────────────────────────
void AWeaponBase::Reload()
{
    if (bIsReloading || CurrentAmmo == MagazineSize) return;

    bIsReloading = true;

    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, ReloadSound, GetActorLocation()
        );
    }

    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &AWeaponBase::OnReloadComplete,
        ReloadTime,
        false
    );
}

void AWeaponBase::OnReloadComplete()
{
    CurrentAmmo  = MagazineSize;
    bIsReloading = false;
}

void AWeaponBase::ResetFireCooldown()
{
    bCanFire = true;
}
