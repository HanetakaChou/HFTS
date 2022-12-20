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
#include "Crypt_Scene.h"


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
Crypt_Scene::Crypt_Scene() 
{
	wcscpy_s( m_Name, L"Crypt" );

	m_fEyeNearPlane			= 0.01f;
	m_fEyeFarPlane			= 10.0f;
	m_ShadowMapScale		= 5;
	m_ShadowMapRes			= 1024;
	m_RTMapScale			= 8;
	m_RTMapRes				= 256;
	m_FTMapScale			= 8;
	m_FTMapRes				= 256;
				
	// SM Desc
	m_Shadow.m_SMDesc.eViewType											= GFSDK_ShadowLib_ViewType_Single;
	m_Shadow.m_SMDesc.uResolutionWidth									= m_ShadowMapRes * m_ShadowMapScale;
	m_Shadow.m_SMDesc.uResolutionHeight									= m_ShadowMapRes * m_ShadowMapScale;
	m_Shadow.m_SMDesc.RayTraceMapDesc.bRequirePrimitiveMap				= true;
	m_Shadow.m_SMDesc.RayTraceMapDesc.uResolutionWidth					= m_RTMapRes * m_RTMapScale;
	m_Shadow.m_SMDesc.RayTraceMapDesc.uResolutionHeight					= m_RTMapRes * m_RTMapScale;
	m_Shadow.m_SMDesc.RayTraceMapDesc.uMaxNumberOfPrimitivesPerPixel	= 32;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.bRequireFrustumTraceMap		= true;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.uResolutionWidth				= m_FTMapRes * m_FTMapScale;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.uResolutionHeight				= m_FTMapRes * m_FTMapScale;
	m_Shadow.m_SMDesc.FrustumTraceMapDesc.uDynamicReprojectionCascades	= 2;
	// SM Render Params
	m_Shadow.m_SMRenderParams.LightDesc.eLightType			= GFSDK_ShadowLib_LightType_Spot;
	m_Shadow.m_SMRenderParams.LightDesc.fLightSize			= 0.375f;
	m_Shadow.m_SMRenderParams.ZBiasParams.iDepthBias					= 100;
	m_Shadow.m_SMRenderParams.ZBiasParams.fSlopeScaledDepthBias			= 5;
	m_Shadow.m_SMRenderParams.eTechniqueType				= GFSDK_ShadowLib_TechniqueType_HRTS;
	// Penumbra params
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold				= 46.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0]			= 5.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[1]			= 5.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[2]			= 5.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[3]			= 5.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent	= 3.0f;
	// RT params
	m_Shadow.m_SMRenderParams.RayTraceMapRenderParams.fHitEpsilon	= 0.01f;
	// FT Params
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.eConservativeRasterType		= GFSDK_ShadowLib_ConservativeRasterType_HW;
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.eCullModeType					= GFSDK_ShadowLib_CullModeType_Back;
	
	m_BackgroundColor[0] = 0.0f;
	m_BackgroundColor[1] = 0.0f;
	m_BackgroundColor[2] = 0.0f;
	m_BackgroundColor[3] = 0.0f;

	m_bTextured = true;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Crypt_Scene::CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext )
{
	m_Mesh.Create( pd3dDevice, L"Media\\Crypt_Scene\\crypt.sdkmesh" );
	
	m_Shadow.m_SMDesc.RayTraceMapDesc.uMaxNumberOfPrimitives = GetMeshPrimCount(  &m_Mesh );
	
	Scene::CreateResources( pd3dDevice, pd3dImmediateContext );
		
	D3DXVECTOR3 ScenePos = D3DXVECTOR3( -8.4152727, 4.5252614, -8.7200823 );
	D3DXVECTOR3 SceneLookAt = D3DXVECTOR3( -7.7418432, 4.1358433, -8.0917149 );
		
	m_SceneCamera.SetViewParams( &ScenePos, &SceneLookAt );
	m_ExtendedSceneCamera.SetViewParams( &ScenePos, &SceneLookAt );
	
	D3DXVECTOR3 LightPos = D3DXVECTOR3(10.395399, 12.596478, 9.9019346);
	D3DXVECTOR3 LightLookat = D3DXVECTOR3(0,0,0);

	m_LightControl.SetPosition( &LightPos );
	m_LightControl.SetLookAt( &LightLookat );

	m_Shadow.m_LightSizeMax = 0.5f;
	m_Shadow.m_PenumbraMaxThresholdMin = 45.0f;
	m_Shadow.m_PenumbraMaxThresholdRange = m_fBBMaxValue * 8.0f - m_Shadow.m_PenumbraMaxThresholdMin; 

	m_fBBMaxValue *= 4.0f;
	m_fEyeFarPlane = m_fBBMaxValue;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Crypt_Scene::ReleaseResources()
{
	m_Mesh.Destroy();
	Scene::ReleaseResources();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Crypt_Scene::ComputeSceneAABB()
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

	D3DXVECTOR3 Scale( 0.7f, 0.7f, 0.7f ); 
	m_WorldSpaceBBox[0].x *= Scale.x;
	m_WorldSpaceBBox[0].z *= Scale.z;
	m_WorldSpaceBBox[1].x *= Scale.x;
	m_WorldSpaceBBox[1].z *= Scale.z;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Crypt_Scene::RenderDepthOnlyScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
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
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Crypt_Scene::RenderMaps( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
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
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Crypt_Scene::RenderScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer, 
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };

	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	m_ObjectCB.CBData.HasTextures.x = 1.0f;
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
	// Henge mesh
	m_MaterialCB.CBData.g_DiffuseColor[0] = 138.0f / 255.0f;
	m_MaterialCB.CBData.g_DiffuseColor[1] = 122.0f / 255.0f;
	m_MaterialCB.CBData.g_DiffuseColor[2] = 117.0f / 255.0f;
	pd3dImmediateContext->UpdateSubresource( m_MaterialCB.pBuffer, 0, NULL, &m_MaterialCB.CBData, 0, 0);
	m_Mesh.Render( pd3dImmediateContext, 1 );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Crypt_Scene::GetNumPresetViews()
{
	return 1;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Crypt_Scene::SetPresetView( unsigned int uPresetView )
{
	m_uCurrentPresetView = uPresetView;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Crypt_Scene::GetPresetView()
{
	return m_uCurrentPresetView;
}


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------
