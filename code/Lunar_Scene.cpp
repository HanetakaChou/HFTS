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

#pragma warning (disable: 4995)

#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "Strsafe.h"
#include <algorithm>
#include <limits>
#include "Lunar_Scene.h"


//--------------------------------------------------------------------------------------
// Preset views
//--------------------------------------------------------------------------------------
#define LUNAR_SCENE_NUM_PRESET_VIEWS ( 6 )
Scene::PresetView g_Lunar_Preset_Views[LUNAR_SCENE_NUM_PRESET_VIEWS];


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
Lunar_Scene::Lunar_Scene() 
{
	wcscpy_s( m_Name, L"Lunar" );

	m_fEyeNearPlane		= 0.1f;
	m_fEyeFarPlane		= 1000.0f;
	m_ShadowMapScale	= 4;
	m_ShadowMapRes		= 1024;
	m_RTMapScale		= 8;
	m_RTMapRes			= 256;
	m_FTMapScale		= 8;
	m_FTMapRes			= 256;
			
	// SM Desc
	m_Shadow.m_SMDesc.eViewType											= GFSDK_ShadowLib_ViewType_Single;
	m_Shadow.m_SMDesc.uResolutionWidth									= m_ShadowMapRes * m_ShadowMapScale;
	m_Shadow.m_SMDesc.uResolutionHeight									= m_ShadowMapRes * m_ShadowMapScale;
	m_Shadow.m_SMDesc.RayTraceMapDesc.bRequirePrimitiveMap				= true;
	m_Shadow.m_SMDesc.RayTraceMapDesc.uResolutionWidth					= m_RTMapRes * m_RTMapScale;
	m_Shadow.m_SMDesc.RayTraceMapDesc.uResolutionHeight					= m_RTMapRes * m_RTMapScale;
	m_Shadow.m_SMDesc.RayTraceMapDesc.uMaxNumberOfPrimitivesPerPixel	= 64;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.bRequireFrustumTraceMap		= true;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.uResolutionWidth				= m_FTMapRes * m_FTMapScale;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.uResolutionHeight				= m_FTMapRes * m_FTMapScale;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.uDynamicReprojectionCascades	= 2;
	// SM Render Params
	m_Shadow.m_SMRenderParams.LightDesc.eLightType						= GFSDK_ShadowLib_LightType_Spot;
	m_Shadow.m_SMRenderParams.LightDesc.fLightSize						= 1.73f;
	m_Shadow.m_SMRenderParams.ZBiasParams.iDepthBias					= 100;
	m_Shadow.m_SMRenderParams.ZBiasParams.fSlopeScaledDepthBias			= 10;
	m_Shadow.m_SMRenderParams.ZBiasParams.fDistanceBiasMin				= 0.000001f;
	m_Shadow.m_SMRenderParams.ZBiasParams.fDistanceBiasFactor			= 0.000001f;
	// SB Render Params
	m_Shadow.m_SMRenderParams.eTechniqueType							= GFSDK_ShadowLib_TechniqueType_HFTS;
	// Penumbra params
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold				= 247.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0]			= 1.8f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[1]			= 1.8f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[2]			= 1.8f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[3]			= 1.8f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent	= 3.0f;
	// FT params
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.eConservativeRasterType	= GFSDK_ShadowLib_ConservativeRasterType_HW;
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.eCullModeType				= GFSDK_ShadowLib_CullModeType_Back;
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.fHitEpsilon				= 0.009f;
	// RT params
	m_Shadow.m_SMRenderParams.RayTraceMapRenderParams.eConservativeRasterType		= GFSDK_ShadowLib_ConservativeRasterType_HW;
	m_Shadow.m_SMRenderParams.RayTraceMapRenderParams.eCullModeType					= GFSDK_ShadowLib_CullModeType_Back;
	m_Shadow.m_SMRenderParams.RayTraceMapRenderParams.fHitEpsilon					= 0.02f;
	
	m_BackgroundColor[0] = 0.0f;
	m_BackgroundColor[1] = 0.0f;
	m_BackgroundColor[2] = 0.0f;
	m_BackgroundColor[3] = 0.0f;

	m_bTextured = false;

	g_Lunar_Preset_Views[0].v3CameraPos		= D3DXVECTOR3( 15.605019, 15.881184, -22.889112 );
	g_Lunar_Preset_Views[0].v3CameraLookAt	= D3DXVECTOR3( 15.073380, 15.543786, -22.112246 );
	g_Lunar_Preset_Views[0].v3LightPos		= D3DXVECTOR3( 30.704916, 25.176689, -15.037009 );

	g_Lunar_Preset_Views[1].v3CameraPos		= D3DXVECTOR3( 4.3302798, 9.7973719, -10.274161 );
	g_Lunar_Preset_Views[1].v3CameraLookAt	= D3DXVECTOR3( 3.6713769, 9.4318237, -9.6167259 );
	g_Lunar_Preset_Views[1].v3LightPos		= D3DXVECTOR3( 45.639591, 36.273460, -18.525463 );
	
	g_Lunar_Preset_Views[2].v3CameraPos		= D3DXVECTOR3( 3.2168453, 7.9264979, -15.709046 );
	g_Lunar_Preset_Views[2].v3CameraLookAt	= D3DXVECTOR3( 2.8918090, 7.3437133, -14.964255 );
	g_Lunar_Preset_Views[2].v3LightPos		= D3DXVECTOR3( 42.403767, 40.583836, -25.070080 );

	g_Lunar_Preset_Views[3].v3CameraPos		= D3DXVECTOR3( 10.995111, 14.873145, -5.6792111 );
	g_Lunar_Preset_Views[3].v3CameraLookAt	= D3DXVECTOR3( 10.065209, 14.592802, -5.4411139 );
	g_Lunar_Preset_Views[3].v3LightPos		= D3DXVECTOR3( 30.929941, 38.952892, 14.559503  );

	g_Lunar_Preset_Views[4].v3CameraPos		= D3DXVECTOR3( -0.37388518, 26.506407, -27.975292 );
	g_Lunar_Preset_Views[4].v3CameraLookAt	= D3DXVECTOR3( -0.61374766, 25.873081, -27.239517 );
	g_Lunar_Preset_Views[4].v3LightPos		= D3DXVECTOR3( 30.962118, 27.587940, -14.498590 );

	g_Lunar_Preset_Views[5].v3CameraPos		= D3DXVECTOR3( 1.3544968, 13.624402, -8.0797052  );
	g_Lunar_Preset_Views[5].v3CameraLookAt	= D3DXVECTOR3( 0.84604335, 13.163376, -7.3524299 );
	g_Lunar_Preset_Views[5].v3LightPos		= D3DXVECTOR3( 29.639700, 26.205038, -17.038086 );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Lunar_Scene::CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext )
{
	m_Mesh.Create( pd3dDevice, L"Media\\Lunar_Scene\\lunar_pod.sdkmesh" );
	m_Mesh2.Create( pd3dDevice, L"Media\\Lunar_Scene\\lunar_scape.sdkmesh" );
		
	m_Shadow.m_SMDesc.RayTraceMapDesc.uMaxNumberOfPrimitives = GetMeshPrimCount(  &m_Mesh );
	
	Scene::CreateResources( pd3dDevice, pd3dImmediateContext );

	SetPresetView( 0 );
	
	m_Shadow.m_LightSizeMax = fabs( ( m_WorldSpaceBBox[1].x + m_WorldSpaceBBox[1].y + m_WorldSpaceBBox[1].z ) / 3.0f ) * 0.2f;
	m_Shadow.m_PenumbraMaxThresholdMin = m_WorldSpaceBBox[1].y;
	m_Shadow.m_PenumbraMaxThresholdRange = m_WorldSpaceBBox[1].y * 20.0f; 

	m_fBBMaxValue *= 4.0f;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Lunar_Scene::ReleaseResources()
{
	m_Mesh.Destroy();
	m_Mesh2.Destroy();
	Scene::ReleaseResources();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Lunar_Scene::ComputeSceneAABB()
{
	m_WorldSpaceBBox[0] = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_WorldSpaceBBox[1] = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	
	for( unsigned int i = 0; i < m_Mesh.GetNumMeshes(); i++ )
	{
		D3DXVECTOR3 bbCenter = m_Mesh.GetMeshBBoxCenter(i);
		D3DXVECTOR3 bbExtents;
		bbExtents.x = m_Mesh.GetMeshBBoxExtents(i).x;
		bbExtents.y = m_Mesh.GetMeshBBoxExtents(i).y;
		bbExtents.z = m_Mesh.GetMeshBBoxExtents(i).z;
			
		D3DXVECTOR3 bbMin = bbCenter - bbExtents;
		D3DXVECTOR3 bbMax = bbCenter + bbExtents;

		for (int k = 0; k < 3; ++k)
		{
			m_WorldSpaceBBox[0][k] = min(m_WorldSpaceBBox[0][k], bbMin[k]);
			m_WorldSpaceBBox[1][k] = max(m_WorldSpaceBBox[1][k], bbMax[k]);
		}
	}

	D3DXVECTOR3 Scale( 1.05f, 1.05f, 1.05f ); 
	m_WorldSpaceBBox[0].x *= Scale.x;
	m_WorldSpaceBBox[0].z *= Scale.z;
	m_WorldSpaceBBox[1].x *= Scale.x;
	m_WorldSpaceBBox[1].z *= Scale.z;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Lunar_Scene::RenderDepthOnlyScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer, 
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };

	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );

	// Setup VS
	pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSceneIA );
    pd3dImmediateContext->VSSetConstantBuffers( 0, 4, CB_Array );
    pd3dImmediateContext->VSSetShader( Shaders::Get()->pGBUfferSceneVS, NULL, 0 );

	// Setup PS
    pd3dImmediateContext->PSSetConstantBuffers(0, 4, CB_Array);
	pd3dImmediateContext->PSSetShader( Shaders::Get()->pGBUfferScenePS, NULL, 0 );
    
	// Draw the scene model
	m_Mesh.Render( pd3dImmediateContext );
	m_Mesh2.Render( pd3dImmediateContext );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Lunar_Scene::RenderMaps( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer, 
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };

	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 4, CB_Array );
	pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSceneIA );
	pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapVS, NULL, 0 );
	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
	
	if( !( m_Shadow.m_SMRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_FT || m_Shadow.m_SMRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_RT ) )
	{
		m_Shadow.InitializeMapRendering( GFSDK_ShadowLib_MapRenderType_Depth );

		for( unsigned int uView = 0; uView < (unsigned int)m_Shadow.m_SMDesc.eViewType; uView++ )
		{
			m_Shadow.BeginMapRendering( GFSDK_ShadowLib_MapRenderType_Depth, uView );
				
			D3DXMATRIX ViewProj, TranViewProj;
			D3DXMatrixMultiply( &ViewProj, (D3DXMATRIX*)&m_Shadow.m_LightViewMatrices[uView], (D3DXMATRIX*)&m_Shadow.m_LightProjectionMatrices[uView] );
			D3DXMatrixTranspose( &TranViewProj, &ViewProj );
			pd3dImmediateContext->UpdateSubresource( m_ShadowMapCB.pBuffer, 0, NULL, &TranViewProj, 0, 0 );
						
			pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );
			pd3dImmediateContext->PSSetShader( NULL, NULL, 0 );
	
			RenderMapMesh( &m_Mesh, GFSDK_ShadowLib_MapRenderType_Depth );

			m_Shadow.EndMapRendering( GFSDK_ShadowLib_MapRenderType_Depth, uView );
		}
	}

	if( m_Shadow.m_SMRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_HFTS || m_Shadow.m_SMRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_FT )
	{
		m_Shadow.InitializeMapRendering( GFSDK_ShadowLib_MapRenderType_FT );
				
		m_Shadow.BeginMapRendering( GFSDK_ShadowLib_MapRenderType_FT, 0 );
				
		D3DXMATRIX ViewProj, TranViewProj;
		D3DXMatrixMultiply( &ViewProj, (D3DXMATRIX*)&m_Shadow.m_LightViewMatrices[0], (D3DXMATRIX*)&m_Shadow.m_LightProjectionMatrices[0] );
		D3DXMatrixTranspose( &TranViewProj, &ViewProj );
		pd3dImmediateContext->UpdateSubresource( m_ShadowMapCB.pBuffer, 0, NULL, &TranViewProj, 0, 0 );
				
		RenderMapMesh( &m_Mesh, GFSDK_ShadowLib_MapRenderType_FT );

		m_Shadow.EndMapRendering( GFSDK_ShadowLib_MapRenderType_FT, 0 );
	}

	if( m_Shadow.m_SMRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_HRTS || m_Shadow.m_SMRenderParams.eTechniqueType == GFSDK_ShadowLib_TechniqueType_RT )
	{
		m_Shadow.InitializeMapRendering( GFSDK_ShadowLib_MapRenderType_RT );

		m_Shadow.BeginMapRendering( GFSDK_ShadowLib_MapRenderType_RT, 0 );
				
		D3DXMATRIX ViewProj, TranViewProj;
		D3DXMatrixMultiply( &ViewProj, (D3DXMATRIX*)&m_Shadow.m_LightViewMatrices[0], (D3DXMATRIX*)&m_Shadow.m_LightProjectionMatrices[0] );
		D3DXMatrixTranspose( &TranViewProj, &ViewProj );
		pd3dImmediateContext->UpdateSubresource( m_ShadowMapCB.pBuffer, 0, NULL, &TranViewProj, 0, 0 );
			
		RenderMapMesh( &m_Mesh, GFSDK_ShadowLib_MapRenderType_RT );
			
		m_Shadow.EndMapRendering( GFSDK_ShadowLib_MapRenderType_RT, 0 );
	}
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Lunar_Scene::RenderScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer, 
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };

	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	m_ObjectCB.CBData.HasTextures.x = 0.0f;
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );

	// Setup VS
	pd3dImmediateContext->IASetInputLayout(Shaders::Get()->pTexturedSceneIA);
    pd3dImmediateContext->VSSetConstantBuffers(0, 4, CB_Array);
    pd3dImmediateContext->VSSetShader(Shaders::Get()->pTexturedSceneVS, NULL, 0);

    // Setup PS
    pd3dImmediateContext->PSSetConstantBuffers(0, 4, CB_Array);
	if( m_Shadow.m_SBDesc.uSampleCount > 1 )
	{
		pd3dImmediateContext->PSSetShader(Shaders::Get()->pMSAATexturedScenePS, NULL, 0);
	}
	else
	{
		pd3dImmediateContext->PSSetShader(Shaders::Get()->pTexturedScenePS, NULL, 0);
	}
    
	// Draw the scene model
	m_MaterialCB.CBData.g_DiffuseColor[0] = 138.0f / 255.0f;
	m_MaterialCB.CBData.g_DiffuseColor[1] = 122.0f / 255.0f;
	m_MaterialCB.CBData.g_DiffuseColor[2] = 117.0f / 255.0f;
	pd3dImmediateContext->UpdateSubresource( m_MaterialCB.pBuffer, 0, NULL, &m_MaterialCB.CBData, 0, 0);
	m_Mesh.Render( pd3dImmediateContext, 1 );
	m_Mesh2.Render( pd3dImmediateContext, 1 );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Lunar_Scene::GetNumPresetViews()
{
	return LUNAR_SCENE_NUM_PRESET_VIEWS;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Lunar_Scene::SetPresetView( unsigned int uPresetView )
{
	m_uCurrentPresetView = uPresetView;

	m_SceneCamera.SetViewParams( &g_Lunar_Preset_Views[m_uCurrentPresetView].v3CameraPos, &g_Lunar_Preset_Views[m_uCurrentPresetView].v3CameraLookAt );
	m_ExtendedSceneCamera.SetViewParams( &g_Lunar_Preset_Views[m_uCurrentPresetView].v3CameraPos, &g_Lunar_Preset_Views[m_uCurrentPresetView].v3CameraLookAt );
	
	D3DXVECTOR3 LightPos = g_Lunar_Preset_Views[m_uCurrentPresetView].v3LightPos;
	D3DXVECTOR3 LightLookat = D3DXVECTOR3(0,0,0);
	m_LightControl.SetPosition( &LightPos );
	m_LightControl.SetLookAt( &LightLookat );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Lunar_Scene::GetPresetView()
{
	return m_uCurrentPresetView;
}


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------
