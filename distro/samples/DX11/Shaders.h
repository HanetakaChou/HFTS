//--------------------------------------------------------------------------------------
// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright © 2012, NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"


//--------------------------------------------------------------------------------------
// Input layouts, vertex shaders and pixel shaders
//--------------------------------------------------------------------------------------
class Shaders
{
public:

	static void Claim();
	static void Destroy();
	static Shaders* Get();

	// All other scenes
	ID3D11VertexShader*	pGBUfferSceneVS;
	ID3D11VertexShader*	pGBUfferSkinnedSceneVS;
	ID3D11PixelShader*	pGBUfferScenePS;
	ID3D11InputLayout*	pSceneIA;
	ID3D11VertexShader*	pTexturedSceneVS;
	ID3D11VertexShader*	pTexturedSkinnedSceneVS;
	ID3D11PixelShader*	pTexturedScenePS;
	ID3D11PixelShader*	pMSAATexturedScenePS;
	ID3D11InputLayout*	pTexturedSceneIA;
	
	// Light Control
	ID3D11VertexShader*	pLightControlVS;
	ID3D11PixelShader*	pLightControlPS;
	ID3D11InputLayout*	pLightControlIA;
	// Shadow map VS
    ID3D11VertexShader* pRenderShadowMapVS;
	ID3D11VertexShader* pRenderShadowMapSkinnedVS;
	ID3D11InputLayout*	pSkinnedSceneIA;

	// MSAA Detect
	ID3D11VertexShader*	pScreenQuadVS;
	ID3D11PixelShader*	pMSAADetectPS;
	ID3D11PixelShader*	pMSAAShowComplexPS;
	ID3D11InputLayout*	pScreenQuadIA;
	
	// Depth Resolve
	ID3D11PixelShader*	pResolveDepthPS;

	// FXAA
	ID3D11PixelShader*	pFXAAPS;
	
    Shaders();
	~Shaders();
	
    void CreateResources( ID3D11Device *pd3dDevice );
    void ReleaseResources();
    HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
};


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------