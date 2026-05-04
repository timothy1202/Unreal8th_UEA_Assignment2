// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All); 
    
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
    // ================================================
    //  [템플릿 메서드] 발사 알고리즘의 골격
    //  → 자식 클래스에서 오버라이드 금지 (final)
    // ================================================
    void PerformFire();

    // ------------------------------------------------
    //  [훅 메서드] 자식 클래스에서 선택적으로 오버라이드
    // ------------------------------------------------

    /** 발사 직전 호출 — 자식이 선조건/애니메이션 등을 추가할 수 있음 */
    virtual void OnBeforeFire() {}

    /** 
     * [순수 가상] 실제 탄환/레이캐스트 처리.
     * 자식 클래스가 반드시 구현해야 함.
     * @param PC        발사하는 플레이어 컨트롤러
     * @param CamLoc    카메라 위치
     * @param CamRot    카메라 회전
     * @return          명중한 펠릿 수
     */
    virtual int32 DoFireTrace(APlayerController* PC,
                              const FVector& CamLoc,
                              const FRotator& CamRot) PURE_VIRTUAL(AWeaponBase::DoFireTrace, return 0;);

    /** 레이캐스트 완료 후 호출 — HitCount를 받아 후처리 가능 */
    virtual void OnAfterFireTrace(int32 HitCount) {}

    /** 총구 화염·사운드 재생 — 자식이 오버라이드해 다른 이펙트 추가 가능 */
    virtual void PlayFireEffects(APlayerController* PC);

    /** 반동 적용 — 자식이 오버라이드해 다른 반동 공식 적용 가능 */
    virtual void ApplyRecoil();

    // ------------------------------------------------
    //  내부 유틸
    // ------------------------------------------------
    void OnReloadComplete();
    void ResetFireCooldown();

    bool bCanFire = true;

    FTimerHandle ReloadTimerHandle;
    FTimerHandle FireRateTimerHandle;
};
