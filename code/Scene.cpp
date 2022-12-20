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
#include "Scene.h"


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext )
{
	m_Shadow.CreateResources( pd3dDevice, pd3dImmediateContext );
	m_LightControl.CreateResources( pd3dDevice );
	m_LightControl.SetActivateButtonID( Light_Control::RIGHT_MOUSE );
	m_GlobalCB.CreateResources(pd3dDevice);
	m_ObjectCB.CreateResources(pd3dDevice);
	m_MaterialCB.CreateResources(pd3dDevice);
    m_ShadowMapCB.CreateResources(pd3dDevice);
	m_MSAADetectCB.CreateResources(pd3dDevice);
	m_BoneCB.CreateResources(pd3dDevice);
	m_FXAACB.CreateResources(pd3dDevice);
    m_TimestampQueries.Create( pd3dDevice );
	m_SceneCamera.SetRotateButtons( true, false, false, false );
    m_SceneCamera.SetEnablePositionMovement( true );
	m_ExtendedSceneCamera.SetRotateButtons( true, false, false, false );
    m_ExtendedSceneCamera.SetEnablePositionMovement( true );
	ComputeSceneAABB();
	CameraSetupFromAABB();
	m_ScreenQuad.CreateResources( pd3dDevice );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::ReleaseResources()
{
	m_Shadow.ReleaseResources();
	m_LightControl.ReleaseResources();
    m_GlobalCB.ReleaseResources();
	m_ObjectCB.ReleaseResources();
	m_MaterialCB.ReleaseResources();
    m_ShadowMapCB.ReleaseResources();
	m_MSAADetectCB.ReleaseResources();
	m_FXAACB.ReleaseResources();
	m_BoneCB.ReleaseResources();
    m_TimestampQueries.Release();
	SAFE_DELETE( m_ColorSceneRT );
	SAFE_DELETE( m_ResolvedDepth );
	m_ScreenQuad.DestroyResources();
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::Render(	ID3D11DeviceContext* pd3dImmediateContext,
					ID3D11RenderTargetView* pOutputRTV,
					ID3D11DepthStencilView* pOutputDSV,
					ID3D11DepthStencilView* pOutputDSV_RO,
					ID3D11ShaderResourceView* pDepthStencilSRV,
					ID3D11RenderTargetView* pEyeViewPositionRTV,
					ID3D11ShaderResourceView* pEyeViewPositionSRV,
					double fTime )
{
	// Timestamp queries need to be wraped inside a disjoint query begin/end
	m_TimestampQueries.Begin( pd3dImmediateContext );
		    
    // This SDK sample does not use any hull, domain or geometry shaders
    pd3dImmediateContext->HSSetShader(NULL, NULL, 0);
    pd3dImmediateContext->DSSetShader(NULL, NULL, 0);
    pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	
	// Update the global CB
	m_GlobalCB.CBData.Textured.x = m_bTextured ? 1.0f : 0.0f; 
	pd3dImmediateContext->UpdateSubresource(m_GlobalCB.pBuffer, 0, NULL, &m_GlobalCB.CBData, 0, 0);


	//--------------------------------------------------------------------------------------
    // STEP 1: Depth Pre-Pass
    //--------------------------------------------------------------------------------------
	{
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Depth Pre-Pass");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_DEPTH_PRE_PASS );

		// Clear depth
		pd3dImmediateContext->ClearDepthStencilView( pOutputDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0x00 );
		
		// Set State
		pd3dImmediateContext->OMSetDepthStencilState(States::Get()->pDepthNoStencil_DS, 0);
		float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		pd3dImmediateContext->OMSetBlendState(States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff);
		pd3dImmediateContext->RSSetViewports(1, &m_Viewport);
		pd3dImmediateContext->RSSetState(States::Get()->pNoCull_RS);
			
		// Set Depth buffer
		pd3dImmediateContext->OMSetRenderTargets( 1, &pEyeViewPositionRTV, pOutputDSV );
		
		// Render the scene
		RenderDepthOnlyScene( pd3dImmediateContext, fTime );
		
		DXUT_EndPerfEvent();
	}


	//--------------------------------------------------------------------------------------
    // STEP 2: Resolve depth (take sub-sample 0)
    //--------------------------------------------------------------------------------------
	{
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"MSAA Depth Resolve");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_MSAA_DEPTH_RESOLVE );

		if( m_Shadow.m_SBDesc.uSampleCount > 1 )
		{
			// Set State
			pd3dImmediateContext->OMSetDepthStencilState(States::Get()->pNoDepthNoStencil_DS, 0x00);
			float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			pd3dImmediateContext->OMSetBlendState(States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff);
			pd3dImmediateContext->RSSetViewports(1, &m_Viewport);
			pd3dImmediateContext->RSSetState(States::Get()->pNoCull_RS);
			
			// Resources for this render
			ID3D11ShaderResourceView*	SRV_Array[1] = { pDepthStencilSRV };
			ID3D11RenderTargetView*		RTV_Array[1] = { m_ResolvedDepth->pRTV };
			ID3D11SamplerState*			Sampler_Array[1] = { NULL };
			ID3D11Buffer*				CB_Array[1] = { NULL };

			// Render full screen quad to detect simple/complex pixels in the MSAA depth buffer
			m_ScreenQuad.Render(	pd3dImmediateContext,
									m_Viewport, 
									Shaders::Get()->pResolveDepthPS, 
									CB_Array,
									1, 
									0,
									RTV_Array, 
									1, 
									SRV_Array, 
									1,
									0,
									Sampler_Array,
									1,
									NULL );

			SRV_Array[0] = NULL;
			pd3dImmediateContext->PSSetShaderResources( 0, 1, SRV_Array );
		}

		DXUT_EndPerfEvent();
	}


	//--------------------------------------------------------------------------------------
    // STEP 3: Detect complex pixels for MSAA mode
    //--------------------------------------------------------------------------------------
	{
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"MSAA Detect");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_MSAA_DETECT );

		if( m_Shadow.m_SBDesc.uSampleCount > 1 )
		{
			// Set State
			pd3dImmediateContext->OMSetDepthStencilState(States::Get()->pNoDepthStencil_DS, 0x01);
			float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			pd3dImmediateContext->OMSetBlendState(States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff);
			pd3dImmediateContext->RSSetViewports(1, &m_Viewport);
			pd3dImmediateContext->RSSetState(States::Get()->pNoCull_RS);
			
			// Update the CB
			m_MSAADetectCB.CBData.fSampleCount = (float)m_Shadow.m_SBDesc.uSampleCount;
			m_MSAADetectCB.CBData.fDepthEpsilonPercent = 0.0001f;
			pd3dImmediateContext->UpdateSubresource( m_MSAADetectCB.pBuffer, 0, NULL, &m_MSAADetectCB.CBData, 0, 0 );

			// Resources for this render
			ID3D11ShaderResourceView*	SRV_Array[1] = { pDepthStencilSRV };
			ID3D11RenderTargetView*		RTV_Array[1] = { NULL };
			ID3D11SamplerState*			Sampler_Array[1] = { NULL };
			ID3D11Buffer*				CB_Array[1] = { m_MSAADetectCB.pBuffer };

			// Render full screen quad to detect simple/complex pixels in the MSAA depth buffer
			m_ScreenQuad.Render(	pd3dImmediateContext,
									m_Viewport, 
									Shaders::Get()->pMSAADetectPS, 
									CB_Array,
									1, 
									0,
									RTV_Array, 
									1, 
									SRV_Array, 
									1,
									0,
									Sampler_Array,
									1,
									pOutputDSV_RO );

			SRV_Array[0] = NULL;
			pd3dImmediateContext->PSSetShaderResources( 0, 1, SRV_Array );
		}

		DXUT_EndPerfEvent();
	}
	

	//--------------------------------------------------------------------------------------
    // STEP 4: Render shadow maps
    //--------------------------------------------------------------------------------------
	{
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Shadow Maps");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_SHADOW_MAPS );
		
		m_Shadow.m_SMRenderParams.DepthBufferDesc.eDepthType = GFSDK_ShadowLib_DepthType_DepthBuffer;
		//m_Shadow.m_SMRenderParams.DepthBufferDesc.eDepthType = GFSDK_ShadowLib_DepthType_EyeViewZ;
		
		if( m_Shadow.m_SMRenderParams.DepthBufferDesc.eDepthType == GFSDK_ShadowLib_DepthType_DepthBuffer )
		{
			m_Shadow.m_SMRenderParams.DepthBufferDesc.DepthSRV.pSRV = pDepthStencilSRV;
			m_Shadow.m_SMRenderParams.DepthBufferDesc.ResolvedDepthSRV.pSRV = NULL;
		}
		else
		{
			m_Shadow.m_SMRenderParams.DepthBufferDesc.DepthSRV.pSRV = pEyeViewPositionSRV;
			m_Shadow.m_SMRenderParams.DepthBufferDesc.ResolvedDepthSRV.pSRV = NULL;
		}
		m_Shadow.m_SMRenderParams.DepthBufferDesc.ReadOnlyDSV.pDSV = pOutputDSV_RO;
		
		
		if( m_Shadow.m_SBDesc.uSampleCount > 1 )
		{
			if( m_Shadow.m_SMRenderParams.DepthBufferDesc.eDepthType == GFSDK_ShadowLib_DepthType_DepthBuffer )
			{
				m_Shadow.m_SMRenderParams.DepthBufferDesc.ResolvedDepthSRV.pSRV = m_ResolvedDepth->pSRV;
			}
			else
			{
				// TODO: Add a single sample version of eye view Z
				m_Shadow.m_SMRenderParams.DepthBufferDesc.ResolvedDepthSRV.pSRV = m_ResolvedDepth->pSRV;
			}
			m_Shadow.m_SMRenderParams.DepthBufferDesc.uComplexRefValue = 0x01;
			m_Shadow.m_SMRenderParams.DepthBufferDesc.uSimpleRefValue = 0x00;
		}
						
		m_Shadow.SetMapRenderParams();
		
		m_Shadow.UpdateMapBounds();
		
		RenderMaps( pd3dImmediateContext, fTime );
		
		pd3dImmediateContext->RSSetViewports(1, &m_Viewport);
	
		DXUT_EndPerfEvent();
	}
        

	//--------------------------------------------------------------------------------------
	// STEP 5: Render shadow buffer
	//--------------------------------------------------------------------------------------
	{
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Shadow Buffer");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_SHADOW_BUFFER );

		if( m_Shadow.m_SBDesc.uSampleCount > 1 )
		{
			m_Shadow.m_SBRenderParams.eMSAARenderMode = GFSDK_ShadowLib_MSAARenderMode_ComplexPixelMask;
		}

		m_Shadow.RenderShadowBuffer( pd3dImmediateContext, pDepthStencilSRV, m_ResolvedDepth->pSRV );
			
		DXUT_EndPerfEvent();
	}
		    
		
	//--------------------------------------------------------------------------------------
    // STEP 6: Forward render scene using shadow buffer 
	//--------------------------------------------------------------------------------------
	{
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Scene");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_SCENE );
		
		// Clear buffers
		pd3dImmediateContext->ClearRenderTargetView( m_ColorSceneRT->pRTV, m_BackgroundColor );
		pd3dImmediateContext->ClearRenderTargetView( pOutputRTV, m_BackgroundColor );
		pd3dImmediateContext->ClearDepthStencilView( pOutputDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

		// Set States
		float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		pd3dImmediateContext->OMSetBlendState( States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff );
		pd3dImmediateContext->OMSetDepthStencilState( States::Get()->pDepthNoStencil_DS, 0 );
		pd3dImmediateContext->RSSetViewports( 1, &m_Viewport );
		if( m_bShowWireFrame )
		{
			pd3dImmediateContext->RSSetState( ( m_Shadow.m_SBDesc.uSampleCount > 1 ) ? States::Get()->pBackfaceCull_WireFrameMSAA_RS : States::Get()->pBackfaceCull_WireFrame_RS );
		}
		else
		{
			pd3dImmediateContext->RSSetState( ( m_Shadow.m_SBDesc.uSampleCount > 1 ) ? States::Get()->pBackfaceCullMSAA_RS : States::Get()->pBackfaceCull_RS );
		}
						    
		// Setup RTV and DS	
		pd3dImmediateContext->OMSetRenderTargets( 1, &pOutputRTV, pOutputDSV );
		
		// Set the shadow buffer as a SRV
		ID3D11RenderTargetView* RTV_Array[1] = { NULL };
		ID3D11ShaderResourceView* SRV_Array[1] = { m_Shadow.m_ShadowBufferSRV.pSRV };
		pd3dImmediateContext->PSSetShaderResources( 0, 1, SRV_Array );
		ID3D11SamplerState* pSamplerStates[2] = { States::Get()->pPointClampSampler, States::Get()->pLinearSampler };
		pd3dImmediateContext->PSSetSamplers(0, 2, pSamplerStates);
			
		RenderScene( pd3dImmediateContext, fTime );
	
		pd3dImmediateContext->OMSetRenderTargets( 1, RTV_Array, NULL );

		pd3dImmediateContext->RSSetState( States::Get()->pBackfaceCull_RS );

		DXUT_EndPerfEvent();
	}   
	

	//--------------------------------------------------------------------------------------
    // STEP 7: Optionally show complex pixels
    //--------------------------------------------------------------------------------------
	{
		static bool s_bShowComplexPixels = false;
		if( m_Shadow.m_SBDesc.uSampleCount > 1 && s_bShowComplexPixels )
		{
			// Set State
			pd3dImmediateContext->OMSetDepthStencilState(States::Get()->pNoDepthStencilComplex_DS, 0x01);
			float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			pd3dImmediateContext->OMSetBlendState(States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff);
			pd3dImmediateContext->RSSetViewports(1, &m_Viewport);
			pd3dImmediateContext->RSSetState(States::Get()->pNoCull_RS);
			
			// Resources for this render
			ID3D11ShaderResourceView*	SRV_Array[1] = { NULL };
			ID3D11RenderTargetView*		RTV_Array[1] = { pOutputRTV };
			ID3D11SamplerState*			Sampler_Array[1] = { NULL };
			ID3D11Buffer*				CB_Array[1] = { NULL };

			// Render full screen quad to detect simple/complex pixels in the MSAA depth buffer
			m_ScreenQuad.Render(	pd3dImmediateContext,
									m_Viewport, 
									Shaders::Get()->pMSAAShowComplexPS, 
									CB_Array,
									1, 
									0,
									RTV_Array, 
									1, 
									SRV_Array, 
									1,
									0,
									Sampler_Array,
									1,
									pOutputDSV_RO );
		}
	}


	//--------------------------------------------------------------------------------------
    // STEP 8: Render the light control
    //--------------------------------------------------------------------------------------
	
	D3DXMATRIX ViewProj = *m_ExtendedSceneCamera.GetViewMatrix() * *m_ExtendedSceneCamera.GetProjMatrix();
	
	m_LightControl.SetSize( m_Shadow.m_SMRenderParams.LightDesc.fLightSize );
	m_LightControl.Render( pd3dImmediateContext, &ViewProj, (D3DXVECTOR3*)m_ExtendedSceneCamera.GetEyePt(), pOutputRTV, pOutputDSV );
	
	//--------------------------------------------------------------------------------------
	// STEP 9: Resolve color
	//--------------------------------------------------------------------------------------
	{
		if( m_Shadow.m_SBDesc.uSampleCount >= 1 )
		{
			ID3D11Resource* pSrc;
			pOutputRTV->GetResource( &pSrc );
			pd3dImmediateContext->ResolveSubresource( (ID3D11Resource*)m_ColorSceneRT->pTexture, 0, pSrc, 0, DXGI_FORMAT_R8G8B8A8_UNORM );
			pSrc->Release();
		}
		else
		{
			ID3D11Resource* pSrc;
			pOutputRTV->GetResource( &pSrc );
			pd3dImmediateContext->CopyResource( (ID3D11Resource*)m_ColorSceneRT->pTexture, pSrc );
			pSrc->Release();
		}
	}


	//--------------------------------------------------------------------------------------
	// STEP 10: Apply FXAA
	//--------------------------------------------------------------------------------------
	{
		
		DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"FXAA");
		GPUTimer timer( &m_TimestampQueries, pd3dImmediateContext, GPU_TIME_FXAA );
				
		// Set State
		pd3dImmediateContext->OMSetDepthStencilState(States::Get()->pNoDepthNoStencil_DS, 0x01);
		float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		pd3dImmediateContext->OMSetBlendState(States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff);
		pd3dImmediateContext->RSSetViewports(1, &m_Viewport);
		pd3dImmediateContext->RSSetState(States::Get()->pNoCull_RS);
		
		pd3dImmediateContext->UpdateSubresource(m_FXAACB.pBuffer, 0, NULL, &m_FXAACB.CBData, 0, 0);

		// Resources for this render
		ID3D11ShaderResourceView*	SRV_Array[1] = { m_ColorSceneRT->pSRV };
		ID3D11RenderTargetView*		RTV_Array[1] = { pOutputRTV };
		ID3D11SamplerState*			Sampler_Array[4] = { NULL, NULL, NULL, States::Get()->pAnisoSampler };
		ID3D11Buffer*				CB_Array[1] = { m_FXAACB.pBuffer };
		
		// Render full screen quad to apply FXAA
		m_ScreenQuad.Render(	pd3dImmediateContext,
								m_Viewport, 
								Shaders::Get()->pFXAAPS, 
								CB_Array,
								1, 
								0,
								RTV_Array, 
								1, 
								SRV_Array, 
								1,
								0,
								Sampler_Array,
								4,
								NULL );
		
		DXUT_EndPerfEvent();

	}
			
	m_TimestampQueries.End( pd3dImmediateContext, &m_RenderTimes );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::RenderTimeStampInfo( CDXUTTextHelper* pTxtHelper )
{
	WCHAR str[100];

	float fTotal =	m_RenderTimes.GpuTimeMS[GPU_TIME_DEPTH_PRE_PASS] +
					m_RenderTimes.GpuTimeMS[GPU_TIME_MSAA_DEPTH_RESOLVE] +
					m_RenderTimes.GpuTimeMS[GPU_TIME_MSAA_DETECT] +
					m_RenderTimes.GpuTimeMS[GPU_TIME_SCENE] + 
					m_RenderTimes.GpuTimeMS[GPU_TIME_FXAA];

	pTxtHelper->DrawTextLine( L"" );
	pTxtHelper->DrawTextLine( L"--App Render Times--" );
	StringCchPrintf(str, 100, L"Total: %.1f ms", fTotal );
    pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Depth Pre-Pass: %.1f ms", m_RenderTimes.GpuTimeMS[GPU_TIME_DEPTH_PRE_PASS] );
    pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"MSAA Depth Resolve: %.1f ms", m_RenderTimes.GpuTimeMS[GPU_TIME_MSAA_DEPTH_RESOLVE] );
    pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"MSAA Detect Complex: %.1f ms", m_RenderTimes.GpuTimeMS[GPU_TIME_MSAA_DETECT] );
    pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Eye Scene: %.1f ms", m_RenderTimes.GpuTimeMS[GPU_TIME_SCENE] );
    pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"FXAA: %.1f ms", m_RenderTimes.GpuTimeMS[GPU_TIME_FXAA] );
    pTxtHelper->DrawTextLine( str );

	float LibTimes[GFSDK_ShadowLib_RenderTimesType_Max];
	
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_SDSM_Bounds, &LibTimes[GFSDK_ShadowLib_RenderTimesType_SDSM_Bounds] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_DynamicReprojection, &LibTimes[GFSDK_ShadowLib_RenderTimesType_DynamicReprojection] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_Frustum_Trace_List, &LibTimes[GFSDK_ShadowLib_RenderTimesType_Frustum_Trace_List] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_Clear_DepthMap, &LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_DepthMap] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_Clear_FrustumTraceMap, &LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_FrustumTraceMap] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_Clear_RayTraceMap, &LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_RayTraceMap] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_DepthMap_0, &LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_0] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_DepthMap_1, &LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_1] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_DepthMap_2, &LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_2] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_DepthMap_3, &LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_3] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_FrustumTraceMap, &LibTimes[GFSDK_ShadowLib_RenderTimesType_FrustumTraceMap] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_RayTraceMap, &LibTimes[GFSDK_ShadowLib_RenderTimesType_RayTraceMap] );
	m_Shadow.m_pShadowLibCtx->DevModeGetTimeStampInfo( GFSDK_ShadowLib_RenderTimesType_Shadow_Buffer, &LibTimes[GFSDK_ShadowLib_RenderTimesType_Shadow_Buffer] );
		
	//float MapTime = m_RenderTimes.GpuTimeMS[GPU_TIME_SHADOW_MAPS] - LibTimes[GFSDK_ShadowLib_RenderTimesType_SDSM_Bounds] - LibTimes[GFSDK_ShadowLib_RenderTimesType_Frustum_Trace_List];

	fTotal =	LibTimes[GFSDK_ShadowLib_RenderTimesType_SDSM_Bounds] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_DynamicReprojection] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_Frustum_Trace_List] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_DepthMap] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_FrustumTraceMap] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_RayTraceMap] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_0] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_1] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_2] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_3] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_FrustumTraceMap] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_RayTraceMap] +
				LibTimes[GFSDK_ShadowLib_RenderTimesType_Shadow_Buffer];
				
	pTxtHelper->DrawTextLine( L"" );
	pTxtHelper->DrawTextLine( L"--GFSDK_ShadowLib Render Times--" );
	StringCchPrintf(str, 100, L"Total: %.1f ms", fTotal );
    pTxtHelper->DrawTextLine( str );

	StringCchPrintf(str, 100, L"SDSM Bounds: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_SDSM_Bounds] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Dynamic Reprojection: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_DynamicReprojection] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Frustum Trace List: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_Frustum_Trace_List] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Clear Depth Map: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_DepthMap] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Clear Frustum Trace Map: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_FrustumTraceMap] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Clear Ray Trace Map: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_Clear_RayTraceMap] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"DepthMap 0: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_0] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"DepthMap 1: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_1] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"DepthMap 2: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_2] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"DepthMap 3: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_DepthMap_3] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Frustum Trace Map: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_FrustumTraceMap] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Ray Trace Map: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_RayTraceMap] );
	pTxtHelper->DrawTextLine( str );
	StringCchPrintf(str, 100, L"Shadow Buffer: %.1f ms", LibTimes[GFSDK_ShadowLib_RenderTimesType_Shadow_Buffer] );
	pTxtHelper->DrawTextLine( str );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::SetScreenResolution( ID3D11Device *pd3dDevice, float fFovyRad, UINT Width, UINT Height, UINT uSampleCount, ID3D11DepthStencilView* pReadOnlyDSV )
{
    m_Viewport.Width  = (FLOAT)Width;
    m_Viewport.Height = (FLOAT)Height;
    m_Viewport.MinDepth = 0;
    m_Viewport.MaxDepth = 1;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;

	m_GlobalCB.CBData.ScreenRes[0] = (float)Width;
	m_GlobalCB.CBData.ScreenRes[1] = (float)Height;
	m_GlobalCB.CBData.ScreenRes[2] = 1.0f / Width;
	m_GlobalCB.CBData.ScreenRes[3] = 1.0f / Height;

	float fAspectRatio = Width / ( FLOAT )Height;

	m_ExtendedSceneCamera.SetProjParams( fFovyRad, fAspectRatio, m_fEyeNearPlane, m_fEyeFarPlane * 10.0f );
	m_SceneCamera.SetProjParams( fFovyRad, fAspectRatio, m_fEyeNearPlane, m_fEyeFarPlane );
	
    SAFE_DELETE(m_ColorSceneRT);
    m_ColorSceneRT = new SimpleRT(pd3dDevice, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);
	
	SAFE_DELETE(m_ResolvedDepth);
    m_ResolvedDepth = new SimpleRT(pd3dDevice, Width, Height, DXGI_FORMAT_R32_FLOAT);
	
	m_Shadow.SetScreenResolution( pd3dDevice, fFovyRad, Width, Height, uSampleCount, pReadOnlyDSV );

	m_FXAACB.CBData.v4FXAA = D3DXVECTOR4( 1.0f / Width, 1.0f / Height, 0, 0 );
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::SetMainCamera()
{
    D3DXMatrixMultiply( &m_GlobalCB.CBData.ViewProj, m_SceneCamera.GetViewMatrix(), m_SceneCamera.GetProjMatrix() );
	D3DXMatrixTranspose( &m_GlobalCB.CBData.ViewProj, &m_GlobalCB.CBData.ViewProj );
	D3DXMatrixTranspose( &m_GlobalCB.CBData.View, m_SceneCamera.GetViewMatrix() );

	memcpy(&m_GlobalCB.CBData.EyePos, m_SceneCamera.GetEyePt(), 3*sizeof(float));
	
	m_GlobalCB.CBData.LightWorldPos.x = m_LightControl.GetPosition()->x;
	m_GlobalCB.CBData.LightWorldPos.y = m_LightControl.GetPosition()->y;
	m_GlobalCB.CBData.LightWorldPos.z = m_LightControl.GetPosition()->z;

	memcpy( &m_Shadow.m_SMRenderParams.m4x4EyeViewMatrix, m_SceneCamera.GetViewMatrix(), sizeof( gfsdk_float4x4 ) );
	memcpy( &m_Shadow.m_SMRenderParams.m4x4EyeProjectionMatrix, m_SceneCamera.GetProjMatrix(), sizeof( gfsdk_float4x4 ) );
	memcpy( m_Shadow.m_SMRenderParams.v3WorldSpaceBBox, m_WorldSpaceBBox, sizeof( D3DXVECTOR3 ) * 2 ); 
	m_fEyeNearPlane = m_SceneCamera.GetNearClip();
	m_fEyeFarPlane = m_SceneCamera.GetFarClip();

	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightPos[0], m_LightControl.GetPosition(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightLookAt[0], m_LightControl.GetLookAt(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightPos[1], m_LightControl.GetPosition(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightLookAt[1], m_LightControl.GetLookAt(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightPos[2], m_LightControl.GetPosition(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightLookAt[2], m_LightControl.GetLookAt(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightPos[3], m_LightControl.GetPosition(), sizeof( gfsdk_float3 ) );
	memcpy( &m_Shadow.m_SMRenderParams.LightDesc.v3LightLookAt[3], m_LightControl.GetLookAt(), sizeof( gfsdk_float3 ) );
}


//--------------------------------------------------------------------------------------
// Selects a sensible eye and look at point for a camera, based upon a meshes total AABB
//--------------------------------------------------------------------------------------
void Scene::CameraSetupFromAABB()
{
    D3DXVECTOR3 v3Min = m_WorldSpaceBBox[0];
    D3DXVECTOR3 v3Max = m_WorldSpaceBBox[1];
    
    D3DXVECTOR3 v3Range = v3Max - v3Min;
    
    float fMax = v3Range.x;
    fMax = ( v3Range.y > fMax ) ? ( v3Range.y ) : ( fMax );
    fMax = ( v3Range.z > fMax ) ? ( v3Range.z ) : ( fMax );
    D3DXVECTOR3 LookAt = v3Min + ( v3Max - v3Min ) * 0.5f;
    D3DXVECTOR3 Eye = LookAt;
    Eye.z -= fMax;
	
	m_fBBMaxValue = fMax;
	m_fEyeFarPlane = ( m_fEyeFarPlane > m_fBBMaxValue ) ? ( m_fEyeFarPlane ) : ( m_fBBMaxValue );
				        
    float fScaler =  ( ( v3Range.x + v3Range.y + v3Range.z ) / 3.0f );
	m_SceneCamera.SetScalers( 0.005f, fScaler );
    m_SceneCamera.SetViewParams( &Eye, &LookAt );
    m_ExtendedSceneCamera.SetScalers( 0.005f, fScaler );
    m_ExtendedSceneCamera.SetViewParams( &Eye, &LookAt );

	fScaler *= 2.0f;
	D3DXVECTOR3 LightPos( fScaler, fScaler, fScaler );

	m_LightControl.SetPosition( &LightPos );
	m_LightControl.SetLookAt( &LookAt );
	m_LightControl.SetAxisScale( v3Range.x, v3Range.y, v3Range.z );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void Scene::RenderMapMesh(	
	CDXUTSDKMesh*					pDXUTMesh,
	GFSDK_ShadowLib_MapRenderType	eMapRenderType )
{
    #define MAX_D3D11_VERTEX_STREAMS D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT

    assert( NULL != pDXUTMesh );

    if( 0 < pDXUTMesh->GetOutstandingBufferResources() )
    {
        return;
    }
	
	for( unsigned int uMesh = 0; uMesh < pDXUTMesh->GetNumMeshes(); uMesh++ )
	{
		SDKMESH_MESH* pMesh = pDXUTMesh->GetMesh( uMesh );

		UINT Strides[MAX_D3D11_VERTEX_STREAMS];
		UINT Offsets[MAX_D3D11_VERTEX_STREAMS];
		ID3D11Buffer* pVB[MAX_D3D11_VERTEX_STREAMS];

		if( pMesh->NumVertexBuffers > MAX_D3D11_VERTEX_STREAMS )
		{
			return;
		}

		for( UINT64 i = 0; i < pMesh->NumVertexBuffers; i++ )
		{
			pVB[i] = pDXUTMesh->GetVB11( uMesh, (UINT)i );
			Strides[i] = pDXUTMesh->GetVertexStride( uMesh, (UINT)i );
			Offsets[i] = 0;
		}

		ID3D11Buffer* pIB;
		pIB = pDXUTMesh->GetIB11( uMesh );
		DXGI_FORMAT ibFormat = pDXUTMesh->GetIBFormat11( uMesh );
    
		DXUTGetD3D11DeviceContext()->IASetVertexBuffers( 0, pMesh->NumVertexBuffers, pVB, Strides, Offsets );
		DXUTGetD3D11DeviceContext()->IASetIndexBuffer( pIB, ibFormat, 0 );

		SDKMESH_SUBSET* pSubset = NULL;

		for( UINT uSubset = 0; uSubset < pMesh->NumSubsets; uSubset++ )
		{
			pSubset = pDXUTMesh->GetSubset( uMesh, uSubset );
			        
			DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology( pDXUTMesh->GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType ) );
			
			UINT IndexCount = ( UINT )pSubset->IndexCount;
			UINT IndexStart = ( UINT )pSubset->IndexStart;
			UINT VertexStart = ( UINT )pSubset->VertexStart;
        
			DXUTGetD3D11DeviceContext()->DrawIndexed( IndexCount, IndexStart, VertexStart );

			m_Shadow.IncrementMapPrimitiveCounter( eMapRenderType, IndexCount / 3 );
		}
	}
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
unsigned int Scene::GetMeshPrimCount( CDXUTSDKMesh* pDXUTMesh )
{
	unsigned int PrimCount = 0;

	for( unsigned int uMesh = 0; uMesh < pDXUTMesh->GetNumMeshes(); uMesh++ )
	{
		SDKMESH_MESH* pMesh = pDXUTMesh->GetMesh( uMesh );

		for( UINT uSubset = 0; uSubset < pMesh->NumSubsets; uSubset++ )
		{
			SDKMESH_SUBSET* pSubset = pDXUTMesh->GetSubset( uMesh, uSubset );

			PrimCount += (UINT)(pSubset->IndexCount / 3);
		}
	}

	return PrimCount;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::RenderSkinnedAnimatedMapMesh(	CDXUTSDKMesh*					pAnimMesh,
											double							fTime,
											GFSDK_ShadowLib_MapRenderType	eMapRenderType )
{
	ID3D11Buffer* pBuffers[1];
    UINT stride[1];
    UINT offset[1] = { 0 };
	    
	DXUTGetD3D11DeviceContext()->IASetInputLayout( Shaders::Get()->pSkinnedSceneIA );
	
    for( UINT m = 0; m < pAnimMesh->GetNumMeshes(); m++ )
    {
        pBuffers[0] = pAnimMesh->GetVB11( m, 0 );
        stride[0] = ( UINT )pAnimMesh->GetVertexStride( m, 0 );
        offset[0] = 0;

        DXUTGetD3D11DeviceContext()->IASetVertexBuffers( 0, 1, pBuffers, stride, offset );
        DXUTGetD3D11DeviceContext()->IASetIndexBuffer( pAnimMesh->GetIB11( m ), pAnimMesh->GetIBFormat11( m ), 0 );

        SDKMESH_SUBSET* pSubset = NULL;
        D3D11_PRIMITIVE_TOPOLOGY PrimType;
			
	    D3DXMATRIX mIdentity;
        D3DXMatrixIdentity( &mIdentity );
        pAnimMesh->TransformMesh( &mIdentity, fTime );

        for( UINT i = 0; i < pAnimMesh->GetNumInfluences( m ); i++ )
        {
            const D3DXMATRIX* pMat = pAnimMesh->GetMeshInfluenceMatrix( m, i );

			memcpy( &m_BoneCB.CBData.Bone[i], pMat, sizeof( D3DXMATRIX ) );

			D3DXMatrixTranspose( &m_BoneCB.CBData.Bone[i], &m_BoneCB.CBData.Bone[i] );
	    }
		DXUTGetD3D11DeviceContext()->UpdateSubresource( m_BoneCB.pBuffer, 0, NULL, &m_BoneCB.CBData, 0, 0 );
		                
        for( UINT subset = 0; subset < pAnimMesh->GetNumSubsets( m ); subset++ )
        {
            pSubset = pAnimMesh->GetSubset( m, subset );

            PrimType = pAnimMesh->GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
            DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology( PrimType );
			
            DXUTGetD3D11DeviceContext()->DrawIndexed( ( UINT )pSubset->IndexCount, ( UINT )pSubset->IndexStart, ( UINT )pSubset->VertexStart );

			m_Shadow.IncrementMapPrimitiveCounter( eMapRenderType, ( UINT )pSubset->IndexCount / 3 );
        }
    }
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::RenderSkinnedAnimatedMesh( ID3D11DeviceContext* pd3dImmediateContext, CDXUTSDKMesh* pAnimMesh, double fTime )
{
    ID3D11Buffer* pBuffers[1];
    UINT stride[1];
    UINT offset[1] = { 0 };
	    
	pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pSkinnedSceneIA );
	
    for( UINT m = 0; m < pAnimMesh->GetNumMeshes(); m++ )
    {
        pBuffers[0] = pAnimMesh->GetVB11( m, 0 );
        stride[0] = ( UINT )pAnimMesh->GetVertexStride( m, 0 );
        offset[0] = 0;

        pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, stride, offset );
        pd3dImmediateContext->IASetIndexBuffer( pAnimMesh->GetIB11( m ), pAnimMesh->GetIBFormat11( m ), 0 );

        SDKMESH_SUBSET* pSubset = NULL;
        SDKMESH_MATERIAL* pMat = NULL;
        D3D11_PRIMITIVE_TOPOLOGY PrimType;
			
	    D3DXMATRIX mIdentity;
        D3DXMatrixIdentity( &mIdentity );
        pAnimMesh->TransformMesh( &mIdentity, fTime );

        for( UINT i = 0; i < pAnimMesh->GetNumInfluences( m ); i++ )
        {
            const D3DXMATRIX* pMat = pAnimMesh->GetMeshInfluenceMatrix( m, i );

			memcpy( &m_BoneCB.CBData.Bone[i], pMat, sizeof( D3DXMATRIX ) );

			D3DXMatrixTranspose( &m_BoneCB.CBData.Bone[i], &m_BoneCB.CBData.Bone[i] );
	    }
		pd3dImmediateContext->UpdateSubresource( m_BoneCB.pBuffer, 0, NULL, &m_BoneCB.CBData, 0, 0 );
		                
        for( UINT subset = 0; subset < pAnimMesh->GetNumSubsets( m ); subset++ )
        {
            pSubset = pAnimMesh->GetSubset( m, subset );

            PrimType = pAnimMesh->GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
            pd3dImmediateContext->IASetPrimitiveTopology( PrimType );

            pMat = pAnimMesh->GetMaterial( pSubset->MaterialID );
            if( pMat )
            {
				ID3D11ShaderResourceView* SRV_Array[1] = { pMat->pDiffuseRV11 };
				pd3dImmediateContext->PSSetShaderResources( 1, 1, SRV_Array );
            }

            pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, ( UINT )pSubset->IndexStart, ( UINT )pSubset->VertexStart );
        }
    }
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void Scene::RenderAnimationLinkedMesh(	ID3D11DeviceContext* pd3dImmediateContext, 
										CDXUTSDKMesh* pAnimMesh,
										CDXUTSDKMesh* pLinkedMesh,
										unsigned int uBone,
										double fTime )
{
	D3DXMATRIX mIdentity;
    D3DXMatrixIdentity( &mIdentity );
    pAnimMesh->TransformMesh( &mIdentity, fTime );

	const D3DXMATRIX* pMat = pAnimMesh->GetMeshInfluenceMatrix( 0, uBone );
	D3DXMatrixTranspose( (D3DXMATRIX*)pMat, (D3DXMATRIX*)pMat );
		
	memcpy( &m_ObjectCB.CBData.World, pMat, sizeof( D3DXMATRIX ) );
	pd3dImmediateContext->UpdateSubresource( m_ObjectCB.pBuffer, 0, NULL, &m_ObjectCB.CBData, 0, 0 );

	pLinkedMesh->Render( pd3dImmediateContext, 1 );
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------