#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WorldCollision.h"   // FTraceHandle, FTraceDatum
#include "EnemyCharacter.generated.h"

UCLASS()
class GUNS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	// ── HP ─────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHP = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHP = 100.0f;

	// ── 시야 / 탐지 설정 (커스터마이징 가능) ───────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float SightRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float SightAngle = 60.0f;        // 시야각 (도, 좌우 절반)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
	float TraceInterval = 0.3f;      // 비동기 트레이스 발사 간격(초)

	UPROPERTY(BlueprintReadOnly, Category = "AI|Sight")
	bool bPlayerInSight = false;

protected:
	virtual void BeginPlay() override;

	void Die();

private:
	// ── 비동기 트레이스 핵심 ───────────────────────────
	void RequestAsyncSightTrace();

	/** AsyncTrace 결과를 받는 콜백 — UFUNCTION 아님 (delegate 타입) */
	void OnSightTraceCompleted(const FTraceHandle& Handle, FTraceDatum& Data);

	/** 매 트레이스 요청 시 갱신되는 핸들 */
	FTraceHandle  CurrentTraceHandle;

	/** 콜백 델리게이트 (한 번만 만들어 재사용) */
	FTraceDelegate TraceDelegate;

	FTimerHandle  SightTimerHandle;

	/** 마지막으로 발견한 플레이어 (옵션) */
	UPROPERTY()
	AActor* LastSeenPlayer = nullptr;
};