@echo off

rem Gather arguments
set config=%1

setlocal EnableDelayedExpansion

rem Switch to project root directory
pushd "%~dp0.."

rem Call vswhere.exe and get the path to the latest version installed
for /f "usebackq tokens=*" %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set vc_dir=%%i

rem Load version file if it exists and trim to make subdirectory
set vc_version_file="%vc_dir%\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt"
if exist %vc_version_file% (
  set /p vc_version=<%vc_version_file%
  set vc_version=!vc_version: =!
)

rem Check if path was found
if not "%vc_version%"=="" (
  set vcpath="%vc_dir%\VC\Tools\MSVC\%vc_version%"
  goto compiler_found
)

@echo Supported Visual C++ compiler not installed
exit /b

:compiler_found

rem Find the latest Windows 10 SDK
set sdk_include_root=C:\Program Files (x86)\Windows Kits\10\Include\
set sdk_lib_root=C:\Program Files (x86)\Windows Kits\10\Lib\
set sdk_test_file=\um\d3d12.h

set sdk_version=10.0.22621.0
if exist "%sdk_include_root%%sdk_version%%sdk_test_file%" goto winsdk_found

@echo Supported Windows 10 SDK not installed
exit /b

:winsdk_found

echo Visual C++ compiler version: %vc_version%
echo Windows 10 SDK version: %sdk_version%
set sdk_include=%sdk_include_root%%sdk_version%
set sdk_lib="%sdk_lib_root%%sdk_version%\um\x64"
set ucrt_lib="%sdk_lib_root%%sdk_version%\ucrt\x64"
set vcpath=%vcpath:"=%
set vc_compiler="%vcpath%\bin\HostX64\x64\cl.exe"
set vc_include="%vcpath%\include"
set vc_lib="%vcpath%\lib\x64"

rem Create build output and object directories
mkdir "build\win_%config%\obj" 2>nul

set optimization=/O2 /GL /D NDEBUG
if %config%==debug set optimization=/Od

rem Invoke compiler
%vc_compiler% ^
%optimization% ^
/Zi ^
/W3 /wd4007 ^
/Oi /GS- /DYNAMICBASE:NO /fp:fast /Gv /diagnostics:caret ^
/nologo ^
/D _CRT_SECURE_NO_WARNINGS ^
/Fd"build\win_%config%\zpress.pdb" ^
/Fo"build\win_%config%\obj\\" ^
/Fe"build\win_%config%\zpress.exe" ^
/Fm"build\win_%config%\zpress.map" ^
/I %vc_include% /I "%sdk_include%\um" /I "%sdk_include%\ucrt" /I "%sdk_include%\shared" ^
"src\kpress_unity.c" /link /OPT:ref /subsystem:windows /incremental:no libucrt.lib Comdlg32.lib User32.lib /LIBPATH:%vc_lib% /LIBPATH:%sdk_lib% /LIBPATH:%ucrt_lib%

rem Switch back to calling directory
popd
endlocal