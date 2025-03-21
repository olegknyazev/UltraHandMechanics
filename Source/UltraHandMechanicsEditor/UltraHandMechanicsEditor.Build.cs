using UnrealBuildTool;

public class UltraHandMechanicsEditor : ModuleRules
{
	public UltraHandMechanicsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
		[
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"ComponentVisualizers",
			"UltraHandMechanics"
		]);
	}
}
