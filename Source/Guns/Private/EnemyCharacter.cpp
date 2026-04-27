#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    SetCanBeDamaged(true);
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;

    // ── 비동기 트레이스 콜백 1회 바인딩 ─────────────────
    TraceDelegate.BindUObject(this, &AEnemyCharacter::OnSightTraceCompleted);

    // ── 일정 주기로 트레이스 요청 ───────────────────────
    GetWorldTimerManager().SetTimer(
        SightTimerHandle,
        this,
        &AEnemyCharacter::RequestAsyncSightTrace,
        TraceInterval,
        true,    // 반복
        0.5f     // 첫 실행 지연 (스폰 직후 안정화)
    );

    UE_LOG(LogTemp, Log,
        TEXT("[Enemy] %s 시야 트레이스 시작 | 사거리:%.0f | 시야각:%.0f° | 주기:%.2fs"),
        *GetName(), SightRange, SightAngle, TraceInterval);
}

// ─────────────────────────────────────────────────────────
//  매 주기마다 호출 — 비동기 트레이스 "요청"
// ─────────────────────────────────────────────────────────
void AEnemyCharacter::RequestAsyncSightTrace()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 플레이어 위치 가져오기
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    FVector EyeLoc       = GetActorLocation() + FVector(0, 0, 60);  // 눈 높이
    FVector PlayerLoc    = PlayerPawn->GetActorLocation();
    FVector ToPlayer     = PlayerLoc - EyeLoc;
    float   Distance     = ToPlayer.Size();

    // ── 1차 필터: 거리 ─────────────────────────────────
    if (Distance > SightRange)
    {
        if (bPlayerInSight)
        {
            bPlayerInSight = false;
            UE_LOG(LogTemp, Verbose, TEXT("[Enemy] %s 사거리 밖 — 시야 잃음"), *GetName());
        }
        return;
    }

    // ── 2차 필터: 시야각 ───────────────────────────────
    FVector Forward    = GetActorForwardVector();
    FVector ToPlayerN  = ToPlayer.GetSafeNormal();
    float   DotResult  = FVector::DotProduct(Forward, ToPlayerN);
    float   AngleDeg   = FMath::RadiansToDegrees(FMath::Acos(DotResult));

    if (AngleDeg > SightAngle)
    {
        if (bPlayerInSight)
        {
            bPlayerInSight = false;
            UE_LOG(LogTemp, Verbose, TEXT("[Enemy] %s 시야각 밖 (%.1f°)"), *GetName(), AngleDeg);
        }
        return;
    }

    // ── 3차: 비동기 LineTrace로 장애물 차단 검사 ──────
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.bTraceComplex = true;

    // ★★ 핵심: AsyncLineTraceByChannel ★★
    CurrentTraceHandle = World->AsyncLineTraceByChannel(
        EAsyncTraceType::Single,   // Single / Multi / Test
        EyeLoc,                    // Start
        PlayerLoc,                 // End
        ECC_Visibility,            // 충돌 채널
        Params,
        FCollisionResponseParams::DefaultResponseParam,
        &TraceDelegate,            // 결과 콜백
        0                          // UserData
    );

    // 디버그 라인 (노란 = 트레이스 요청 중)
    DrawDebugLine(World, EyeLoc, PlayerLoc, FColor::Yellow, false, TraceInterval, 0, 1.0f);

    UE_LOG(LogTemp, Verbose,
        TEXT("[Enemy] %s 비동기 트레이스 요청 (거리:%.0f, 각도:%.1f°)"),
        *GetName(), Distance, AngleDeg);
}

// ─────────────────────────────────────────────────────────
//  비동기 트레이스 결과 콜백 (다음 프레임에 도착)
// ─────────────────────────────────────────────────────────
void AEnemyCharacter::OnSightTraceCompleted(
    const FTraceHandle& Handle, FTraceDatum& Data)
{
    // 다른 트레이스 결과인지 확인 (안전장치)
    if (Handle != CurrentTraceHandle) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;

    bool bSawPlayer = false;

    // OutHits 배열에 결과가 들어있음
    if (Data.OutHits.Num() > 0)
    {
        const FHitResult& Hit = Data.OutHits[0];

        if (Hit.GetActor() == PlayerPawn)
        {
            bSawPlayer = true;
        }
        else
        {
            // 장애물에 막힘
            UE_LOG(LogTemp, Verbose,
                TEXT("[Enemy] %s 시야 차단됨 — 장애물: %s"),
                *GetName(),
                Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"));

            DrawDebugLine(World, Data.Start, Hit.ImpactPoint,
                FColor::Red, false, TraceInterval, 0, 1.5f);
        }
    }
    else
    {
        // 트레이스가 막히지 않고 끝까지 갔다면 = 가시선 확보
        bSawPlayer = true;
    }

    // ── 상태 변경 시점만 로그 ──────────────────────────
    if (bSawPlayer && !bPlayerInSight)
    {
        bPlayerInSight = true;
        LastSeenPlayer = PlayerPawn;

        UE_LOG(LogTemp, Warning,
            TEXT("[Enemy] %s ★ 플레이어 발견! ★ → %s"),
            *GetName(), *PlayerPawn->GetName());

        DrawDebugLine(World, Data.Start, PlayerPawn->GetActorLocation(),
            FColor::Green, false, TraceInterval, 0, 2.5f);
    }
    else if (!bSawPlayer && bPlayerInSight)
    {
        bPlayerInSight = false;
        UE_LOG(LogTemp, Log, TEXT("[Enemy] %s 플레이어 시야 잃음"), *GetName());
    }
    else if (bSawPlayer)
    {
        // 계속 보이는 중 — 초록 라인 갱신
        DrawDebugLine(World, Data.Start, PlayerPawn->GetActorLocation(),
            FColor::Green, false, TraceInterval, 0, 2.0f);
    }
}

// ─────────────────────────────────────────────────────────
//  TakeDamage / Die (기존)
// ─────────────────────────────────────────────────────────
float AEnemyCharacter::TakeDamage(
    float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(
        DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.f) return 0.f;

    CurrentHP -= ActualDamage;

    UE_LOG(LogTemp, Warning,
        TEXT("[Enemy] HP: %.1f / %.1f | 피해: %.1f | 유발자: %s"),
        CurrentHP, MaxHP, ActualDamage,
        DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"));

    if (CurrentHP <= 0.f) Die();

    return ActualDamage;
}

void AEnemyCharacter::Die()
{
    UE_LOG(LogTemp, Warning, TEXT("[Enemy] %s 사망 처리"), *GetName());

    // 시야 트레이스 중단
    GetWorldTimerManager().ClearTimer(SightTimerHandle);

    SetActorEnableCollision(false);
    GetCharacterMovement()->StopMovementImmediately();
    SetLifeSpan(2.0f);
}