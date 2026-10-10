using UnrealBuildTool;
using System.Collections.Generic;

public class Knighthacks9jamEditorTarget : TargetRules
{
	public Knighthacks9jamEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Knighthacks9jam");
	}
}
