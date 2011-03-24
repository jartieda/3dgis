@set BUILDDIR=%CD%
@set PROJECT=%1%

@IF '%PROJECT%' == '' (set PROJECT="libMini")
@IF '%PROJECT%' == 'mini' (set PROJECT="libMini")
@IF '%PROJECT%' == 'libmini' (set PROJECT="libMini")
@IF '%PROJECT%' == 'viewer' (set PROJECT="libMini Viewer")

@IF EXIST "%ProgramFiles(x86)%" (set "PFDIR=%ProgramFiles(x86)%") ELSE ( set "PFDIR=%ProgramFiles%")
@call "%PFDIR%\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"

@echo building %PROJECT% with DEBUG configuration
@echo ...
@devenv %BUILDDIR%\viewer-vc8.sln /Build debug /project %PROJECT%

@echo building %PROJECT% with RELEASE configuration
@echo ...
@devenv %BUILDDIR%\viewer-vc8.sln /Build release /project %PROJECT%
