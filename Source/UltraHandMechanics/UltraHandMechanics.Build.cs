// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UltraHandMechanics : ModuleRules
{
	public UltraHandMechanics(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
