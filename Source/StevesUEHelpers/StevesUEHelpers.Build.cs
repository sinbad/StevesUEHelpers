using UnrealBuildTool;
using System.IO;

public class StevesUEHelpers : ModuleRules
{
	public StevesUEHelpers(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
            }
            );

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"EnhancedInput",
				"Slate",
				"SlateCore",
				"UMG",
				"Paper2D"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"RenderCore",
				"PhysicsCore",
				"Chaos",
				"NavigationSystem"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
