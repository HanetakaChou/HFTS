@set vsvars_path="%VS100COMNTOOLS%\vsvars32.bat"
@set vcupgrade_path="%VS100COMNTOOLS%\vcupgrade.exe"
@if exist %vsvars_path% goto do_vsvars
@echo ERROR: vsvars32.bat not found in
@echo %vsvars_path%
@echo Please edit this batch file to set correct path to vsvars32.bat
:do_vsvars
@call %vsvars_path%

:: Forward Rendering
@call HLSL_to_GLSL.bat ForwardRendering.HLSL DepthOnlySceneGeom_VS VS
@call HLSL_to_GLSL.bat ForwardRendering.HLSL TexturedSceneGeom_VS VS
@call HLSL_to_GLSL.bat ForwardRendering.HLSL TexturedScene_PS PS
@call HLSL_to_GLSL.bat ForwardRendering.HLSL MSAA_TexturedScene_PS PS

:: Light Control
@call HLSL_to_GLSL.bat Light_Control.hlsl Light_Control_VS VS
@call HLSL_to_GLSL.bat Light_Control.hlsl Light_Control_PS PS

:: Screen Quad
@call HLSL_to_GLSL.bat MSAA_Detect.hlsl Screen_Quad_VS VS

:: Depth Resolve
@call HLSL_to_GLSL.bat Resolve_Depth.hlsl Resolve_Depth_PS PS

:: MSAA Detect
@call HLSL_to_GLSL.bat MSAA_Detect.hlsl MSAA_Detect_PS PS
@call HLSL_to_GLSL.bat MSAA_Detect.hlsl MSAA_Show_Complex_PS PS

:: Shadow Map Rendering
@call HLSL_to_GLSL.bat ShadowMapRendering.hlsl ShadowMapGeom_VS VS

exit

