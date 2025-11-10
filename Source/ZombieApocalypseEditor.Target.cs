// Copyright University of Inland Norway

using UnrealBuildTool;
using System.Collections.Generic;

public class ZombieApocalypseEditorTarget : TargetRules
{
	public ZombieApocalypseEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "ZombieApocalypse" } );
	}
}
