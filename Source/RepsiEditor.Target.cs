using UnrealBuildTool;

public class RepsiEditorTarget : TargetRules
{
	public RepsiEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "RepsiCore" } );
	}
}
