project "Premake"
	kind "Utility"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{wks.location}/**premake5.lua"
	}
	
	postbuildmessage "Regenerating project files with Premake5!"
	filter "platforms:Windows"
		postbuildcommands {
			"./bin/Windows/premake5 %{_ACTION} --file=\"%{wks.location}/premake5.lua\""
		}
	filter "platforms:Linux"
		postbuildcommands {
			"./bin/Linux/premake5 %{_ACTION} --file=\"%{wks.location}/premake5.lua\""
		}