using UnrealBuildTool;

public class RepsiTarget : TargetRules
{
	public RepsiTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "RepsiCore" } );
	}
}
