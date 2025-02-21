using UnrealBuildTool;

public class Module_Character : ModuleRules
{
	public Module_Character(ReadOnlyTargetRules Target) : base(Target)  // Make visible to Unreal build system
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
