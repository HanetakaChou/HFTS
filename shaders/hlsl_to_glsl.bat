:: %1 = Src HLSL file
:: %2 = Entry point
:: %3 = Shader type

del %2.glsl
copy /y "%1" "%2.hlsl"
CL.exe /P /EP /D %2 /D GL /D %3 "%2.hlsl"
Format_PreProcess_File.exe "%2.i" "%2.glsl"
copy /y "%2.glsl" "../../GL/GFSDK_ShadowApp_GL/Assets/Shaders/%2.glsl"
del %2.hlsl
del %2.i
del %2.glsl



