// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class GUNS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MaxHP = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	float CurrentHP;

protected:
	virtual void BeginPlay() override;

	// ★ ApplyPointDamage가 내부적으로 이 함수를 자동 호출 ★
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

private:
	void Die();
};
