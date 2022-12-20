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
#include "Warrior_Scene.h"
#include "Constant_Buffers.h"


//--------------------------------------------------------------------------------------
// Preset views
//--------------------------------------------------------------------------------------
#define WARRIOR_SCENE_NUM_PRESET_VIEWS ( 4 )
Scene::PresetView g_Warrior_Preset_Views[WARRIOR_SCENE_NUM_PRESET_VIEWS];


//--------------------------------------------------------------------------------------
// Linked meshes
//--------------------------------------------------------------------------------------

struct MESHLINK
{
    WCHAR szMeshName[MAX_PATH];
    UINT iBone;
};

MESHLINK g_MeshLinkages[] =
{
    { L"Media\\Warrior_Scene\\Hammer.sdkmesh", 13 },
    { L"Media\\Warrior_Scene\\LeftForearm.sdkmesh", 54 },
    { L"Media\\Warrior_Scene\\RightForearm.sdkmesh", 66 },
    { L"Media\\Warrior_Scene\\RightShoulder.sdkmesh", 72 },
    { L"Media\\Warrior_Scene\\LeftShoulder.sdkmesh", 72 },
    { L"Media\\Warrior_Scene\\BackPlate.sdkmesh", 72 },
    { L"Media\\Warrior_Scene\\Helmet.sdkmesh", 51 },
    { L"Media\\Warrior_Scene\\Eyes.sdkmesh", 51 },
    { L"Media\\Warrior_Scene\\Belt.sdkmesh", 63 },
    { L"Media\\Warrior_Scene\\LeftThigh.sdkmesh", 58 },
    { L"Media\\Warrior_Scene\\RightThigh.sdkmesh", 70 },
    { L"Media\\Warrior_Scene\\LeftShin.sdkmesh", 56 },
    { L"Media\\Warrior_Scene\\RightShin.sdkmesh", 68 },
};

UINT g_NumLinkedMeshes = sizeof( g_MeshLinkages ) / sizeof( MESHLINK );


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
Warrior_Scene::Warrior_Scene() 
{
	wcscpy_s( m_Name, L"Warrior" );

	m_BackgroundColor[0]	= 0.2f;
	m_BackgroundColor[1]	= 0.2f;
	m_BackgroundColor[2]	= 0.6f;
	m_BackgroundColor[3]	= 0.0f;
	m_fEyeNearPlane			= 0.01f;
	m_fEyeFarPlane			= 20.0f;
	m_ShadowMapScale		= 3;
	m_ShadowMapRes			= 1024;
	m_RTMapScale			= 2;
	m_RTMapRes				= 512;
	m_FTMapScale			= 2;
	m_FTMapRes				= 512;
			
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
	m_Shadow.m_SMRenderParams.LightDesc.eLightType							= GFSDK_ShadowLib_LightType_Spot;
	m_Shadow.m_SMRenderParams.LightDesc.fLightSize							= 0.31f;
	m_Shadow.m_SMRenderParams.ZBiasParams.iDepthBias						= 10000;
	m_Shadow.m_SMRenderParams.ZBiasParams.fSlopeScaledDepthBias				= 5.8f;
	m_Shadow.m_SMRenderParams.eTechniqueType								= GFSDK_ShadowLib_TechniqueType_HRTS;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold				= 29.0f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0]			= 1.2f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[1]			= 1.2f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[2]			= 1.2f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[3]			= 1.2f;
	m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent	= 2.0f;
	// RT Params
	m_Shadow.m_SMRenderParams.RayTraceMapRenderParams.fHitEpsilon	= 0.002f;
	// FT Params
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.eConservativeRasterType		= GFSDK_ShadowLib_ConservativeRasterType_HW;
	m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.eCullModeType					= GFSDK_ShadowLib_CullModeType_Back;
	
	g_Warrior_Preset_Views[0].v3CameraPos		= D3DXVECTOR3( 0.30336335, 2.1672184, -2.3106112 );
	g_Warrior_Preset_Views[0].v3CameraLookAt	= D3DXVECTOR3( 0.094442233, 1.7257175, -1.4380124 );
	g_Warrior_Preset_Views[0].v3LightPos		= D3DXVECTOR3( 5.1311178, 4.7283025, -3.8235548 );

	g_Warrior_Preset_Views[1].v3CameraPos		= D3DXVECTOR3( 0.071449764, 2.0633519, -0.96845025 );
	g_Warrior_Preset_Views[1].v3CameraLookAt	= D3DXVECTOR3( -0.10641684, 1.5769727, -0.11299795 );
	g_Warrior_Preset_Views[1].v3LightPos		= D3DXVECTOR3( 5.1311178, 4.7283025, -3.8235548 );

	g_Warrior_Preset_Views[2].v3CameraPos		= D3DXVECTOR3( -0.40612826, 0.62921780, -0.95595235 );
	g_Warrior_Preset_Views[2].v3CameraLookAt	= D3DXVECTOR3( -0.61678964, 0.24692994, -0.056242585 );
	g_Warrior_Preset_Views[2].v3LightPos		= D3DXVECTOR3( 3.1808419, 4.7157569, -5.5523515 );

	g_Warrior_Preset_Views[3].v3CameraPos		= D3DXVECTOR3( -0.013201334, 2.0700881, -0.58578974 );
	g_Warrior_Preset_Views[3].v3CameraLookAt	= D3DXVECTOR3( -0.0066491123, 1.4710726, 0.21492070 );
	g_Warrior_Preset_Views[3].v3LightPos		= D3DXVECTOR3( 6.1790481, 8.8296528, -1.6547831 );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Warrior_Scene::CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext )
{
	m_Scene.Create( pd3dDevice, L"Media\\Warrior_Scene\\Scene.sdkmesh" );
	m_Warrior.Create( pd3dDevice, L"Media\\Warrior_Scene\\Warrior.sdkmesh" );
    m_Warrior.LoadAnimation( L"Media\\Warrior_Scene\\Warrior.sdkmesh_anim" );
		
    m_pArmor = new CDXUTSDKMesh[ g_NumLinkedMeshes ];
    for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
    {
        m_pArmor[iMesh].Create( pd3dDevice, g_MeshLinkages[iMesh].szMeshName );
    }
	
	m_Shadow.m_SMDesc.RayTraceMapDesc.uMaxNumberOfPrimitives = GetMeshPrimCount( &m_Warrior );
	for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
    {
		m_Shadow.m_SMDesc.RayTraceMapDesc.uMaxNumberOfPrimitives += GetMeshPrimCount( &m_pArmor[iMesh] );
	}
	
	Scene::CreateResources( pd3dDevice, pd3dImmediateContext );

	SetPresetView( 0 );
		
	m_Shadow.m_LightSizeMax = 0.5f;
	m_Shadow.m_PenumbraMaxThresholdMin = 0.0f;
	m_Shadow.m_PenumbraMaxThresholdRange = m_fBBMaxValue * 8.0f - m_Shadow.m_PenumbraMaxThresholdMin; 

	m_fBBMaxValue *= 4.0f;
	m_fEyeFarPlane = m_fBBMaxValue;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Warrior_Scene::ReleaseResources()
{
	m_Scene.Destroy();
	m_Warrior.Destroy();
	for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
    {
		m_pArmor[iMesh].Destroy();
	}
	SAFE_DELETE_ARRAY( m_pArmor );

	Scene::ReleaseResources();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Warrior_Scene::ComputeSceneAABB()
{
	m_WorldSpaceBBox[0] = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_WorldSpaceBBox[1] = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	
	for( unsigned int i = 0; i < m_Warrior.GetNumMeshes(); i++ )
	{
		D3DXVECTOR3 bbCenter = m_Warrior.GetMeshBBoxCenter(i);
		D3DXVECTOR3 bbExtents;
		bbExtents.x = m_Warrior.GetMeshBBoxExtents(i).x;
		bbExtents.y = m_Warrior.GetMeshBBoxExtents(i).y;
		bbExtents.z = m_Warrior.GetMeshBBoxExtents(i).z;
			
		D3DXVECTOR3 bbMin = bbCenter - bbExtents;
		D3DXVECTOR3 bbMax = bbCenter + bbExtents;

		for (int k = 0; k < 3; ++k)
		{
			m_WorldSpaceBBox[0][k] = min(m_WorldSpaceBBox[0][k], bbMin[k]);
			m_WorldSpaceBBox[1][k] = max(m_WorldSpaceBBox[1][k], bbMax[k]);
		}
	}

	float overall_max = -FLT_MAX;
	overall_max = max(fabs( m_WorldSpaceBBox[0][0] ), overall_max);
	overall_max = max(fabs( m_WorldSpaceBBox[0][1] ), overall_max);
	overall_max = max(fabs( m_WorldSpaceBBox[0][2] ), overall_max);
	overall_max = max(m_WorldSpaceBBox[1][0], overall_max);
	overall_max = max(m_WorldSpaceBBox[1][1], overall_max);
	overall_max = max(m_WorldSpaceBBox[1][2], overall_max);

	m_WorldSpaceBBox[0].x = m_WorldSpaceBBox[0].z = -overall_max;
	m_WorldSpaceBBox[0].y = 0.0f;
	m_WorldSpaceBBox[1].x = m_WorldSpaceBBox[1].y = m_WorldSpaceBBox[1].z = overall_max;
	
	D3DXVECTOR3 Scale( 1.0f, 0.7f, 1.0f ); 
	m_WorldSpaceBBox[0].x *= Scale.x;
	m_WorldSpaceBBox[0].y *= Scale.y;
	m_WorldSpaceBBox[0].z *= Scale.z;
	m_WorldSpaceBBox[1].x *= Scale.x;
	m_WorldSpaceBBox[1].y *= Scale.y;
	m_WorldSpaceBBox[1].z *= Scale.z;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Warrior_Scene::RenderDepthOnlyScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer,
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };
		
	// Setup VS
	pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSceneIA );
    pd3dImmediateContext->VSSetConstantBuffers( 0, 4, CB_Array );
    pd3dImmediateContext->VSSetShader( Shaders::Get()->pGBUfferSceneVS, NULL, 0 );

	// Setup PS
    pd3dImmediateContext->PSSetConstantBuffers(0, 4, CB_Array);
	pd3dImmediateContext->PSSetShader( Shaders::Get()->pGBUfferScenePS, NULL, 0 );
    
	// Scene
	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
	m_Scene.Render( pd3dImmediateContext, 1 );
			
	// Armor
	for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
    {
		RenderAnimationLinkedMesh( pd3dImmediateContext, &m_Warrior, &m_pArmor[iMesh], g_MeshLinkages[iMesh].iBone, fTime );
	}
	
	// Warrior
	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
	pd3dImmediateContext->IASetInputLayout(Shaders::Get()->pSkinnedSceneIA);
	pd3dImmediateContext->VSSetShader(Shaders::Get()->pGBUfferSkinnedSceneVS, NULL, 0);
	RenderSkinnedAnimatedMesh( pd3dImmediateContext, &m_Warrior, fTime );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Warrior_Scene::RenderMaps( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer, 
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };

	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 4, CB_Array );
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
	
			// Warrior
			pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSkinnedSceneIA );
			pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapSkinnedVS, NULL, 0 );
			RenderSkinnedAnimatedMapMesh( &m_Warrior, fTime, GFSDK_ShadowLib_MapRenderType_Depth );

			// Armor
			pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSceneIA );
			pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapVS, NULL, 0 );
			for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
			{	
				D3DXMATRIX mIdentity;
				D3DXMatrixIdentity( &mIdentity );
				m_Warrior.TransformMesh( &mIdentity, fTime );
				const D3DXMATRIX* pMat1 = m_Warrior.GetMeshInfluenceMatrix( 0, g_MeshLinkages[iMesh].iBone );
				D3DXMatrixTranspose( (D3DXMATRIX*)pMat1, (D3DXMATRIX*)pMat1 );
				D3DXMATRIX mWVP = TranViewProj * *pMat1;
				pd3dImmediateContext->UpdateSubresource( m_ShadowMapCB.pBuffer, 0, NULL, &mWVP, 0, 0 );
				RenderMapMesh( &m_pArmor[iMesh], GFSDK_ShadowLib_MapRenderType_Depth );
			}
						
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
	
		// Warrior
		pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSkinnedSceneIA );
		pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapSkinnedVS, NULL, 0 );
		RenderSkinnedAnimatedMapMesh( &m_Warrior, fTime, GFSDK_ShadowLib_MapRenderType_FT );

		// Armor
		pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSceneIA );
		pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapVS, NULL, 0 );
		for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
		{	
			D3DXMATRIX mIdentity;
			D3DXMatrixIdentity( &mIdentity );
			m_Warrior.TransformMesh( &mIdentity, fTime );
			const D3DXMATRIX* pMat = m_Warrior.GetMeshInfluenceMatrix( 0, g_MeshLinkages[iMesh].iBone );
			D3DXMatrixTranspose( &m_ObjectCB.CBData.World, (D3DXMATRIX*)pMat );
			pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
			RenderMapMesh( &m_pArmor[iMesh], GFSDK_ShadowLib_MapRenderType_FT );
		}
						
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
	
		// Warrior
		pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSkinnedSceneIA );
		pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapSkinnedVS, NULL, 0 );
		RenderSkinnedAnimatedMapMesh( &m_Warrior, fTime, GFSDK_ShadowLib_MapRenderType_RT );

		// Armor
		pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSceneIA );
		pd3dImmediateContext->VSSetShader( Shaders::Get()->pRenderShadowMapVS, NULL, 0 );
		for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
		{	
			D3DXMATRIX mIdentity;
			D3DXMatrixIdentity( &mIdentity );
			m_Warrior.TransformMesh( &mIdentity, fTime );
			const D3DXMATRIX* pMat = m_Warrior.GetMeshInfluenceMatrix( 0, g_MeshLinkages[iMesh].iBone );
			D3DXMatrixTranspose( &m_ObjectCB.CBData.World, (D3DXMATRIX*)pMat );
			pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
			RenderMapMesh( &m_pArmor[iMesh], GFSDK_ShadowLib_MapRenderType_RT );
		}
						
		m_Shadow.EndMapRendering( GFSDK_ShadowLib_MapRenderType_RT, 0 );
	}
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Warrior_Scene::RenderScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime )
{
	// Per object CB
	ID3D11Buffer* CB_Array[4] = {	m_GlobalCB.pBuffer, 
									m_ObjectCB.pBuffer,
									m_BoneCB.pBuffer, 
									m_ShadowMapCB.pBuffer };
	
	// Setup VS
	pd3dImmediateContext->IASetInputLayout(Shaders::Get()->pTexturedSceneIA);
    pd3dImmediateContext->VSSetConstantBuffers( 0, 4, CB_Array );
	pd3dImmediateContext->VSSetShader(Shaders::Get()->pTexturedSceneVS, NULL, 0);

    // Setup PS
	if( m_Shadow.m_SBDesc.uSampleCount > 1 )
	{
		pd3dImmediateContext->PSSetShader(Shaders::Get()->pMSAATexturedScenePS, NULL, 0);
	}
	else
	{
		pd3dImmediateContext->PSSetShader(Shaders::Get()->pTexturedScenePS, NULL, 0);
	}
	pd3dImmediateContext->PSSetConstantBuffers( 0, 4, CB_Array );

	// Scene
	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	m_ObjectCB.CBData.HasTextures.x = 0.0f;
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
	m_Scene.Render( pd3dImmediateContext, 1 );
			
	// Armor
	m_ObjectCB.CBData.HasTextures.x = 1.0f;
	for( UINT iMesh = 0; iMesh < g_NumLinkedMeshes; iMesh++ )
    {
		RenderAnimationLinkedMesh( pd3dImmediateContext, &m_Warrior, &m_pArmor[iMesh], g_MeshLinkages[iMesh].iBone, fTime );
	}
	
	// Warrior
	D3DXMatrixIdentity( &m_ObjectCB.CBData.World );
	m_ObjectCB.CBData.HasTextures.x = 1.0f;
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );
	pd3dImmediateContext->IASetInputLayout(Shaders::Get()->pSkinnedSceneIA);
	pd3dImmediateContext->VSSetShader(Shaders::Get()->pTexturedSkinnedSceneVS, NULL, 0);
	RenderSkinnedAnimatedMesh( pd3dImmediateContext, &m_Warrior, fTime );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Warrior_Scene::GetNumPresetViews()
{
	return WARRIOR_SCENE_NUM_PRESET_VIEWS;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Warrior_Scene::SetPresetView( unsigned int uPresetView )
{
	m_uCurrentPresetView = uPresetView;

	m_SceneCamera.SetViewParams( &g_Warrior_Preset_Views[m_uCurrentPresetView].v3CameraPos, &g_Warrior_Preset_Views[m_uCurrentPresetView].v3CameraLookAt );
	m_ExtendedSceneCamera.SetViewParams( &g_Warrior_Preset_Views[m_uCurrentPresetView].v3CameraPos, &g_Warrior_Preset_Views[m_uCurrentPresetView].v3CameraLookAt );
	
	D3DXVECTOR3 LightLookat = D3DXVECTOR3( 0, 0, 0 );
	
	m_LightControl.SetPosition( &g_Warrior_Preset_Views[m_uCurrentPresetView].v3LightPos );
	m_LightControl.SetLookAt( &LightLookat );
}

//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Warrior_Scene::GetPresetView()
{
	return m_uCurrentPresetView;
}


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------
