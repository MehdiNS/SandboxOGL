#include "stdafx.h"
#include "Options.h"

namespace OGL
{
	const std::string& translateDebugMode(DebugMode debugMode)
	{
		static std::string const Table[] =
		{
			"FinalFrame",
			"WorldNormal",
			"AO",
			"AOBlurred",
			"ShadowMap",
			"Diffuse",
			"Specular",
			"SSR",
			"WorldPosition",
			"Albedo",
			"CurrentFrame",
			"LastFrame",
			"SnowHeightmap"
		};
		return Table[debugMode];
	};
}