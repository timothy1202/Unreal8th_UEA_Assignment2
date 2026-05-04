// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Shotgun.generated.h"

UCLASS()
class GUNS_API AShotgun : public AWeaponBase
{
    GENERATED_BODY()
public:
    AShotgun();

protected:
    // [템플릿 메서드 구현] 샷건 전용 멀티 펠릿 레이캐스트
    virtual int32 DoFireTrace(APlayerController* PC,
                              const FVector& CamLoc,
                              const FRotator& CamRot) override;

    // (선택) 샷건 발사 전 특수 처리가 필요하면 여기에 추가
    // virtual void OnBeforeFire() override;
};
