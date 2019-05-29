"./Dependencies/Premake/premake5.exe" --file=SolutionGenerator.lua vs2017

xcopy /y "%~dp0Dependencies\SDL2-2.0.9\lib\x64\SDL2.dll" "%~dp0Binaries\x64\Debug\"
xcopy /y "%~dp0Dependencies\SDL2-2.0.9\lib\x64\SDL2.dll" "%~dp0Binaries\x64\Release\"
      
pause