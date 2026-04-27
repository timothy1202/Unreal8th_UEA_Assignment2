// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GunsCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AWeaponBase;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AGunsCharacter : public ACharacter
{
	GENERATED_BODY()
public:
    AGunsCharacter();

    virtual void Tick(float DeltaTime) override;

    // WeaponBase에서 호출
    void AddRecoil(float Strength, float PitchMax, float YawRandom);

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* CurrentWeapon;

    // Getter — 외부 접근용
    USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
    UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCameraComponent; }

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    // ── 템플릿 컴포넌트 (이름 유지) ──────────────────────
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
    USkeletalMeshComponent* Mesh1P;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FirstPersonCameraComponent;

    // ── Enhanced Input (템플릿 자동생성 에셋 그대로 사용) ─
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    // ── 추가 Input Action ─────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* FireAction;       // 에디터에서 IA_Fire 할당

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ReloadAction;     // 에디터에서 IA_Reload 할당

    // ── 무기 스폰 클래스 ──────────────────────────────────
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<AWeaponBase> DefaultWeaponClass;

    // ── 이동 / 조준 ───────────────────────────────────────
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // ── 발사 / 재장전 ─────────────────────────────────────
    void StartFire();
    void StopFire();
    void StartReload();

private:
    void SpawnDefaultWeapon();
    void ProcessRecoilRecovery(float DeltaTime);

    // 반동 변수
    float TargetRecoilPitch  = 0.0f;
    float CurrentRecoilPitch = 0.0f;

    bool bIsFiring = false;
    FTimerHandle AutoFireHandle;

};

