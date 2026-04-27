// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunCameraShake.h"

UShotgunCameraShake::UShotgunCameraShake()
{
	OscillationDuration       = 0.3f;
	OscillationBlendInTime    = 0.05f;
	OscillationBlendOutTime   = 0.15f;

	// 위아래 (반동 느낌)
	RotOscillation.Pitch.Amplitude     = 4.0f;
	RotOscillation.Pitch.Frequency     = 10.0f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;

	// 좌우
	RotOscillation.Yaw.Amplitude     = 2.0f;
	RotOscillation.Yaw.Frequency     = 8.0f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	// 앞뒤 (킥백)
	LocOscillation.Y.Amplitude     = 3.0f;
	LocOscillation.Y.Frequency     = 12.0f;
	LocOscillation.Y.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;
}