// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// ApplyDamage 받으려면 반드시 true (기본값이 true)
	SetCanBeDamaged(true);
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
}

// ─────────────────────────────────────────────────────────
//  TakeDamage
//  ApplyPointDamage 호출 시 언리얼이 자동으로 이 함수 실행
// ─────────────────────────────────────────────────────────
float AEnemyCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	// 부모 TakeDamage 먼저 호출 (필수)
	float ActualDamage = Super::TakeDamage(
		DamageAmount, DamageEvent, EventInstigator, DamageCauser
	);

	if (ActualDamage <= 0.f) return 0.f;

	CurrentHP -= ActualDamage;

	UE_LOG(LogTemp, Warning,
		TEXT("[Enemy] HP: %.1f / %.1f  |  피해: %.1f  |  유발자: %s"),
		CurrentHP,
		MaxHP,
		ActualDamage,
		DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown")
	);

	if (CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void AEnemyCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("[Enemy] 사망 처리"));

	// 콜리전 제거
	SetActorEnableCollision(false);

	// 이동 정지
	GetCharacterMovement()->StopMovementImmediately();

	// 2초 후 액터 제거
	SetLifeSpan(2.0f);

	// 필요하면 여기에 사망 애니메이션 몽타주 추가
}

