// Copyright Epic Games, Inc. All Rights Reserved.

#include "GunsCharacter.h"
#include "GunsProjectile.h"
#include "WeaponBase.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AGunsCharacter

AGunsCharacter::AGunsCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캡슐 크기
    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    // ── 1인칭 카메라 ─────────────────────────────────────
    FirstPersonCameraComponent =
        CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    // ── 1인칭 팔 메시 ─────────────────────────────────────
    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetOnlyOwnerSee(true);           // 본인만 보임
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->CastShadow         = false;
    Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

    // 전신 메시는 본인한테 안 보이게
    GetMesh()->SetOwnerNoSee(true);
}

void AGunsCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input 매핑 등록
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    SpawnDefaultWeapon();
}

// ─────────────────────────────────────────────────────────
//  무기 스폰 & 팔 메시 GripPoint 소켓에 부착
// ─────────────────────────────────────────────────────────
void AGunsCharacter::SpawnDefaultWeapon()
{
    if (!DefaultWeaponClass)
    {
        UE_LOG(LogTemplateCharacter, Error,
            TEXT("[Character] DefaultWeaponClass 미설정 — 무기 스폰 실패"));
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner      = this;
    Params.Instigator = GetInstigator();

    CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, Params);

    if (CurrentWeapon)
    {
        CurrentWeapon->AttachToComponent(
            Mesh1P,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("GripPoint"));

        CurrentWeapon->WeaponMesh->SetOnlyOwnerSee(true);
        CurrentWeapon->WeaponMesh->bCastDynamicShadow = false;

        UE_LOG(LogTemplateCharacter, Log,
            TEXT("[Character] 무기 장착 완료: %s"),
            *CurrentWeapon->GetName());
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error,
            TEXT("[Character] 무기 스폰 실패"));
    }
}

// ─────────────────────────────────────────────────────────
//  반동 추가
// ─────────────────────────────────────────────────────────
void AGunsCharacter::AddRecoil(
    float Strength, float PitchMax, float YawRandom)
{
    float OldPitch = TargetRecoilPitch;

    TargetRecoilPitch = FMath::Clamp(
        TargetRecoilPitch + Strength, 0.0f, PitchMax);

    float Yaw = FMath::RandRange(-YawRandom, YawRandom);
    AddControllerYawInput(Yaw);

    UE_LOG(LogTemplateCharacter, Verbose,
        TEXT("[Recoil] Pitch: %.2f → %.2f (Max:%.1f) | Yaw랜덤: %.2f"),
        OldPitch, TargetRecoilPitch, PitchMax, Yaw);
}
// ─────────────────────────────────────────────────────────
//  Tick — 반동 회복 처리
// ─────────────────────────────────────────────────────────
void AGunsCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ProcessRecoilRecovery(DeltaTime);
}

void AGunsCharacter::ProcessRecoilRecovery(float DeltaTime)
{
    if (!CurrentWeapon) return;

    // TargetRecoilPitch를 0으로 부드럽게 회복
    TargetRecoilPitch = FMath::FInterpTo(
        TargetRecoilPitch,
        0.0f,
        DeltaTime,
        CurrentWeapon->RecoilRecoverySpeed
    );

    // 이전 프레임 대비 변화량만큼 카메라에 적용
    float Delta = TargetRecoilPitch - CurrentRecoilPitch;
    CurrentRecoilPitch = TargetRecoilPitch;

    if (FMath::Abs(Delta) > 0.001f)
    {
        // 음수 = 위쪽으로 카메라 이동 (반동)
        AddControllerPitchInput(-Delta);
    }
}

// ─────────────────────────────────────────────────────────
//  입력 바인딩
// ─────────────────────────────────────────────────────────
void AGunsCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    UEnhancedInputComponent* EI =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EI) return;

    EI->BindAction(MoveAction,   ETriggerEvent::Triggered, this, &AGunsCharacter::Move);
    EI->BindAction(LookAction,   ETriggerEvent::Triggered, this, &AGunsCharacter::Look);
    EI->BindAction(JumpAction,   ETriggerEvent::Started,   this, &ACharacter::Jump);
    EI->BindAction(JumpAction,   ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    EI->BindAction(FireAction,   ETriggerEvent::Started,   this, &AGunsCharacter::StartFire);
    EI->BindAction(FireAction,   ETriggerEvent::Completed, this, &AGunsCharacter::StopFire);
    EI->BindAction(ReloadAction, ETriggerEvent::Started,   this, &AGunsCharacter::StartReload);
}

void AGunsCharacter::Move(const FInputActionValue& Value)
{
    FVector2D Axis = Value.Get<FVector2D>();
    if (Controller && !Axis.IsZero())
    {
        AddMovementInput(GetActorForwardVector(), Axis.Y);
        AddMovementInput(GetActorRightVector(),   Axis.X);
    }
}

void AGunsCharacter::Look(const FInputActionValue& Value)
{
    FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void AGunsCharacter::StartFire()
{
    if (!CurrentWeapon)
    {
        UE_LOG(LogTemplateCharacter, Warning,
            TEXT("[Character] StartFire 호출되었으나 무기 없음"));
        return;
    }

    UE_LOG(LogTemplateCharacter, Log, TEXT("[Input] StartFire"));

    bIsFiring = true;
    CurrentWeapon->Fire();

    FTimerDelegate FireDelegate = FTimerDelegate::CreateLambda([this]()
    {
        if (bIsFiring && CurrentWeapon)
            CurrentWeapon->Fire();
    });

    GetWorldTimerManager().SetTimer(
        AutoFireHandle, FireDelegate, CurrentWeapon->FireRate, true);
}

void AGunsCharacter::StopFire()
{
    if (bIsFiring)
    {
        UE_LOG(LogTemplateCharacter, Log, TEXT("[Input] StopFire"));
    }
    bIsFiring = false;
    GetWorldTimerManager().ClearTimer(AutoFireHandle);
}

void AGunsCharacter::StartReload()
{
    UE_LOG(LogTemplateCharacter, Log, TEXT("[Input] StartReload (R키)"));
    if (CurrentWeapon)
        CurrentWeapon->Reload();
}