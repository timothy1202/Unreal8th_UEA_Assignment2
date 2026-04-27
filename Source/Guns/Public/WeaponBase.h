// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class GUNS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
public:
    AWeaponBase();
    virtual void BeginPlay() override;

    // 외부에서 호출
    virtual void Fire();
    virtual void Reload();

    // ==============================================
    //  반동 설정 — BP Child마다 다르게 커스터마이징
    // ==============================================
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
    float RecoilStrength = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
    float RecoilRecoverySpeed = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
    float RecoilPitchMax = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
    float RecoilYawRandom = 1.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
    float CameraShakeIntensity = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    // ==============================================
    //  발사 설정
    // ==============================================
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
    int32 PelletCount = 1;          // 탄환 수 (샷건은 8)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
    float SpreadAngle = 2.0f;       // 분산 각도 (도)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
    float MaxRange = 3000.0f;       // 사거리

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
    float Damage = 20.0f;           // 탄환 1발당 피해

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
    float FireRate = 0.5f;          // 발사 간격 (초)

    // ==============================================
    //  탄창 설정
    // ==============================================
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
    int32 MagazineSize = 8;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
    float ReloadTime = 2.5f;

    // ==============================================
    //  이펙트
    // ==============================================
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    UParticleSystem* MuzzleFlashEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    UParticleSystem* ImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    USoundBase* FireSound;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    USoundBase* ReloadSound;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    USoundBase* EmptySound;

    // ==============================================
    //  런타임 상태
    // ==============================================
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
    int32 CurrentAmmo;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
    bool bIsReloading = false;

    // ==============================================
    //  컴포넌트
    // ==============================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* MuzzlePoint;   // 총구 위치 (에디터에서 조정)

    // 소유 캐릭터 참조
    UPROPERTY()
    ACharacter* OwnerCharacter;

protected:
    virtual void PerformFire();     // 실제 발사 처리
    void OnReloadComplete();
    void ResetFireCooldown();

    bool bCanFire = true;

    FTimerHandle ReloadTimerHandle;
    FTimerHandle FireRateTimerHandle;
};
