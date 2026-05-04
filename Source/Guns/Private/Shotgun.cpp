// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

AShotgun::AShotgun()
{
	// ── 샷건 기본값 설정 ──────────────────────────────────
	// EditDefaultsOnly라서 BP Child에서 자유롭게 덮어쓰기 가능

	// 발사
	PelletCount  = 8;        // 탄환 8발 동시 발사
	SpreadAngle  = 6.0f;     // 분산 각도
	Damage       = 15.0f;    // 탄환 1발당 피해 (8발 = 최대 120)
	MaxRange     = 2500.0f;
	FireRate     = 0.9f;     // 발사 간격

	// 탄창
	MagazineSize = 6;
	ReloadTime   = 3.0f;

	// 반동
	RecoilStrength       = 6.0f;
	RecoilRecoverySpeed  = 4.0f;
	RecoilPitchMax       = 8.0f;
	RecoilYawRandom      = 2.5f;
	CameraShakeIntensity = 3.0f;

    UE_LOG(LogTemp, Log,
        TEXT("[Shotgun] 생성자 호출 — 샷건 파라미터 적용 (펠릿 %d, 분산 %.1f°)"),
        PelletCount, SpreadAngle);
}