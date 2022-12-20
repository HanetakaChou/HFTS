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


#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "Shaders.h"

static Shaders* s_pShaders = NULL;


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Shaders::Claim()
{
	if( NULL == s_pShaders ) 
	{
		s_pShaders = new Shaders;
	}
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Shaders::Destroy()
{
	SAFE_DELETE( s_pShaders );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
Shaders* Shaders::Get()
{
	return s_pShaders;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
Shaders::Shaders()
{
	pGBUfferSceneVS = NULL;
	pGBUfferSkinnedSceneVS = NULL;
	pGBUfferScenePS = NULL;
	pSceneIA = NULL;
	pTexturedSceneVS = NULL;
	pTexturedSkinnedSceneVS = NULL;
	pTexturedScenePS = NULL;
	pMSAATexturedScenePS = NULL;
	pTexturedSceneIA = NULL;
	pLightControlVS = NULL;
	pLightControlPS = NULL;
	pLightControlIA = NULL;
	pRenderShadowMapVS = NULL;
	pRenderShadowMapSkinnedVS = NULL;
	pSkinnedSceneIA = NULL;
	pScreenQuadVS = NULL;
	pMSAADetectPS = NULL;
	pMSAAShowComplexPS = NULL;
	pScreenQuadIA = NULL;
	pResolveDepthPS = NULL;
	pFXAAPS = NULL;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
Shaders::~Shaders()
{
	ReleaseResources();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Shaders::CreateResources(ID3D11Device *pd3dDevice)
{
    HRESULT hr;

	const D3D11_INPUT_ELEMENT_DESC SceneLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT SceneNumElements = sizeof(SceneLayoutDesc)/sizeof(SceneLayoutDesc[0]);

	const D3D11_INPUT_ELEMENT_DESC TexturedSceneLayoutDesc[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT TexturedSceneNumElements = sizeof(TexturedSceneLayoutDesc)/sizeof(TexturedSceneLayoutDesc[0]);

	const D3D11_INPUT_ELEMENT_DESC LightControlLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT LightControlNumElements = sizeof(LightControlLayoutDesc)/sizeof(LightControlLayoutDesc[0]);
	
	D3D11_INPUT_ELEMENT_DESC ScreenQuadLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT ScreenQuadNumElements = sizeof(ScreenQuadLayoutDesc)/sizeof(ScreenQuadLayoutDesc[0]);

	D3D11_INPUT_ELEMENT_DESC SkinnedSceneLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT SkinnedNumElements = sizeof(SkinnedSceneLayoutDesc)/sizeof(SkinnedSceneLayoutDesc[0]);


    //--------------------------------------------------------------------------------------
    // Load the shaders for the forward rendering
    //--------------------------------------------------------------------------------------

    WCHAR ShaderPath[MAX_PATH];
    V( DXUTFindDXSDKMediaFileCch( ShaderPath, MAX_PATH, L"Shaders\\ForwardRendering.hlsl" ) );

	ID3DBlob *pBlob;
    V( CompileShaderFromFile(ShaderPath, "GBufferSceneGeom_VS", "vs_5_0", &pBlob) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pGBUfferSceneVS) );
    V( pd3dDevice->CreateInputLayout(SceneLayoutDesc, SceneNumElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pSceneIA) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "GBufferScene_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pGBUfferScenePS) );
    pBlob->Release();
	
	V( CompileShaderFromFile(ShaderPath, "TexturedSceneGeom_VS", "vs_5_0", &pBlob) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pTexturedSceneVS) );
    V( pd3dDevice->CreateInputLayout(TexturedSceneLayoutDesc, TexturedSceneNumElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pTexturedSceneIA) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "GBufferSkinnedSceneGeom_VS", "vs_5_0", &pBlob) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pGBUfferSkinnedSceneVS) );
    V( pd3dDevice->CreateInputLayout(SkinnedSceneLayoutDesc, SkinnedNumElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pSkinnedSceneIA) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "TexturedSkinnedSceneGeom_VS", "vs_5_0", &pBlob) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pTexturedSkinnedSceneVS) );
    pBlob->Release();
	
	V( CompileShaderFromFile(ShaderPath, "TexturedScene_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pTexturedScenePS) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "MSAA_TexturedScene_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pMSAATexturedScenePS) );
    pBlob->Release();
	
	//--------------------------------------------------------------------------------------
    // Load the shaders for the light control
    //--------------------------------------------------------------------------------------

	V( DXUTFindDXSDKMediaFileCch( ShaderPath, MAX_PATH, L"Shaders\\Light_Control.hlsl" ) );

	V( CompileShaderFromFile(ShaderPath, "Light_Control_VS", "vs_5_0", &pBlob) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pLightControlVS) );
    V( pd3dDevice->CreateInputLayout(LightControlLayoutDesc, LightControlNumElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pLightControlIA) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "Light_Control_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pLightControlPS) );
    pBlob->Release();

    //--------------------------------------------------------------------------------------
    // Load the shaders for the shadow-map generation
    //--------------------------------------------------------------------------------------
                
    V( DXUTFindDXSDKMediaFileCch( ShaderPath, MAX_PATH, L"Shaders\\ShadowMapRendering.hlsl" ) );

	V( CompileShaderFromFile(ShaderPath, "ShadowMapGeom_VS", "vs_5_0", &pBlob ) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pRenderShadowMapVS) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "ShadowMapGeomSkinned_VS", "vs_5_0", &pBlob ) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pRenderShadowMapSkinnedVS) );
	pBlob->Release();
		
	//--------------------------------------------------------------------------------------
    // Load the shaders for MSAA detection of simple/complex pixels
    //--------------------------------------------------------------------------------------

	V( DXUTFindDXSDKMediaFileCch( ShaderPath, MAX_PATH, L"Shaders\\MSAA_Detect.hlsl" ) );

	V( CompileShaderFromFile(ShaderPath, "Screen_Quad_VS", "vs_5_0", &pBlob ) );
    V( pd3dDevice->CreateVertexShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pScreenQuadVS) );
	V( pd3dDevice->CreateInputLayout(ScreenQuadLayoutDesc, ScreenQuadNumElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pScreenQuadIA) );
	pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "MSAA_Detect_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pMSAADetectPS) );
    pBlob->Release();

	V( CompileShaderFromFile(ShaderPath, "MSAA_Show_Complex_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pMSAAShowComplexPS) );
    pBlob->Release();

	//--------------------------------------------------------------------------------------
    // Load the shaders for MSAA depth resolve
    //--------------------------------------------------------------------------------------

	V( DXUTFindDXSDKMediaFileCch( ShaderPath, MAX_PATH, L"Shaders\\Resolve_Depth.hlsl" ) );

	V( CompileShaderFromFile(ShaderPath, "Resolve_Depth_PS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pResolveDepthPS) );
    pBlob->Release();

	//--------------------------------------------------------------------------------------
    // FXAA ps
    //--------------------------------------------------------------------------------------

	V( DXUTFindDXSDKMediaFileCch( ShaderPath, MAX_PATH, L"Shaders\\FXAA.hlsl" ) );

	V( CompileShaderFromFile(ShaderPath, "FxaaPS", "ps_5_0", &pBlob) );
    V( pd3dDevice->CreatePixelShader((DWORD*)pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pFXAAPS) );
    pBlob->Release();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Shaders::ReleaseResources()
{
	SAFE_RELEASE(pGBUfferSceneVS);
	SAFE_RELEASE(pGBUfferSkinnedSceneVS);
	SAFE_RELEASE(pGBUfferScenePS);
	SAFE_RELEASE(pSceneIA);

	SAFE_RELEASE( pTexturedSceneVS );
	SAFE_RELEASE( pTexturedSkinnedSceneVS );
	SAFE_RELEASE( pTexturedScenePS );
	SAFE_RELEASE( pMSAATexturedScenePS );
	SAFE_RELEASE( pTexturedSceneIA );

	SAFE_RELEASE(pLightControlVS);
	SAFE_RELEASE(pLightControlPS);
	SAFE_RELEASE(pLightControlIA);

	SAFE_RELEASE(pRenderShadowMapVS);
	SAFE_RELEASE(pRenderShadowMapSkinnedVS);
	SAFE_RELEASE(pSkinnedSceneIA);

	SAFE_RELEASE( pScreenQuadVS );
	SAFE_RELEASE( pMSAADetectPS );
	SAFE_RELEASE( pMSAAShowComplexPS );
	SAFE_RELEASE( pScreenQuadIA );
	
	SAFE_RELEASE( pResolveDepthPS );

	SAFE_RELEASE( pFXAAPS );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
HRESULT Shaders::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    ID3D10Blob *pErrorMessages;
    HRESULT hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel, 0, 0, NULL, ppBlobOut, &pErrorMessages, NULL);
	if( hr != S_OK )
	{
		char* pErr = (char*)(pErrorMessages->GetBufferPointer());
		assert( false );
		printf( pErr );
	}
	SAFE_RELEASE(pErrorMessages);
    return hr;
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------