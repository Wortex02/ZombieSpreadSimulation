// Copyright University of Inland Norways

using UnrealBuildTool;
using System.Collections.Generic;

public class ZombieApocalypseTarget : TargetRules
{
	public ZombieApocalypseTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "ZombieApocalypse" } );
	}
}
