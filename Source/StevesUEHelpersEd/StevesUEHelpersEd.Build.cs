using UnrealBuildTool;

public class StevesUEHelpersEd : ModuleRules
{
    public StevesUEHelpersEd(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                
                "UnrealEd",
                "PropertyEditor",
                "DataTableEditor"
            }
        );
    }
}