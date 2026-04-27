// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Guns : ModuleRules
{
	public Guns(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"GameplayCameras"   // ULegacyCameraShake (Shakes/LegacyCameraShake.h)
		});

		// Guns 루트 폴더(GunsCharacter.h 등)와 Public 폴더 모두 검색 경로에 추가
		PublicIncludePaths.AddRange(new string[]
		{
			"Guns",
			"Guns/Public"
		});
	}
}