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
#include "States.h"
#include "Shaders.h"
#include "Render_Targets.h"
#include "Light_Control.h"
#include "TimestampQueries.h"
#include "Shadow.h"
#include "Screen_Quad.h"


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class Scene
{
public:

	#define MAX_BONES			( 100 )

	struct PresetView
	{
		D3DXVECTOR3	v3CameraPos;
		D3DXVECTOR3	v3CameraLookAt;
		D3DXVECTOR3	v3LightPos;
	};

	unsigned int				m_uCurrentPresetView;
	WCHAR						m_Name[256];
	CFirstPersonCamera          m_ExtendedSceneCamera;
	CFirstPersonCamera          m_SceneCamera;
	Light_Control				m_LightControl;
	D3DXVECTOR3					m_WorldSpaceBBox[2];
	Shadow						m_Shadow;
	Screen_Quad					m_ScreenQuad;
	TimestampQueries            m_TimestampQueries;
	RenderTimes					m_RenderTimes;
	SimpleRT*					m_ColorSceneRT;
	SimpleRT*					m_ResolvedDepth;
    SceneConstantBuffer         m_GlobalCB;
	ObjectConstantBuffer		m_ObjectCB;
	MaterialConstantBuffer		m_MaterialCB;
    ShadowMapConstantBuffer     m_ShadowMapCB;
	MSAADetectConstantBuffer	m_MSAADetectCB;
	FXAAConstantBuffer			m_FXAACB;
	BoneConstantBuffer			m_BoneCB;
	D3D11_VIEWPORT              m_Viewport;
    D3DXMATRIX                  m_WorldToEyeViewI;
	float						m_BackgroundColor[4];
	float						m_fBBMaxValue;
	bool						m_bShowWireFrame;
	bool						m_bTextured;
	unsigned int				m_ShadowMapScale;
	unsigned int				m_ShadowMapRes;
	unsigned int				m_RTMapScale;
	unsigned int				m_RTMapRes;
	unsigned int				m_FTMapScale;
	unsigned int				m_FTMapRes;
	float						m_fEyeNearPlane;
	float						m_fEyeFarPlane;

	Scene() 
	{
		m_uCurrentPresetView = 0;

		m_ShadowMapScale = 1;
		m_RTMapScale = 1;
		m_FTMapScale = 1;
		m_bShowWireFrame = false;
		m_bTextured = true;

		m_ColorSceneRT = NULL;
		m_ResolvedDepth = NULL;
	}

	virtual ~Scene()
	{
		//ReleaseResources();
	}
	
    virtual void CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext );
	    
    virtual void ReleaseResources();
	    
	void Render(	ID3D11DeviceContext* pd3dImmediateContext,
					ID3D11RenderTargetView* pOutputRTV,
					ID3D11DepthStencilView* pOutputDSV,
					ID3D11DepthStencilView* pOutputDSV_RO,
					ID3D11ShaderResourceView* pDepthStencilSRV,
					ID3D11RenderTargetView* pEyeViewPositionRTV,
					ID3D11ShaderResourceView* pEyeViewPositionSRV,
					double fTime );

	virtual void ComputeSceneAABB() = 0;

	virtual void RenderDepthOnlyScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime ) = 0;
	virtual void RenderMaps( ID3D11DeviceContext* pd3dImmediateContext, double fTime ) = 0;
	virtual void RenderScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime ) = 0;
		
	void RenderTimeStampInfo( CDXUTTextHelper* pTxtHelper );
	    
    void SetScreenResolution( ID3D11Device* pd3dDevice, float FovyRad, UINT Width, UINT Height, UINT uSampleCount, ID3D11DepthStencilView* pReadOnlyDSV );
    
    void SetMainCamera();

	void RenderMapMesh(	CDXUTSDKMesh*					pDXUTMesh,
						GFSDK_ShadowLib_MapRenderType	eMapRenderType );

	void RenderSkinnedAnimatedMapMesh(	CDXUTSDKMesh*					pDXUTMesh,
										double							fTime,
										GFSDK_ShadowLib_MapRenderType	eMapRenderType );

	/*
	void RenderMeshOverrideTopology(	CDXUTSDKMesh* pDXUTMesh,
										UINT uDrawCallID,
										D3D11_PRIMITIVE_TOPOLOGY PrimType, 
										UINT uDiffuseSlot,
										UINT uNormalSlot,
										UINT uSpecularSlot );
										*/

	void RenderSkinnedAnimatedMesh( ID3D11DeviceContext* pd3dImmediateContext, CDXUTSDKMesh* pAnimMesh, double fTime );

	//void RenderSkinnedAnimatedMeshDrawCall( ID3D11DeviceContext* pd3dImmediateContext, CDXUTSDKMesh* pAnimMesh, double fTime, unsigned int uDrawCall );

	void RenderAnimationLinkedMesh(	ID3D11DeviceContext* pd3dImmediateContext, 
									CDXUTSDKMesh* pAnimMesh,
									CDXUTSDKMesh* pLinkedMesh,
									unsigned int uBone,
									double fTime );

	virtual unsigned int GetNumPresetViews() = 0;
	virtual void SetPresetView( unsigned int uPresetView ) = 0;
	virtual unsigned int GetPresetView() = 0;
	
protected:

	void CameraSetupFromAABB();

	unsigned int GetMeshPrimCount( CDXUTSDKMesh* pDXUTMesh );

	//void GetMeshPrimsPerDrawCall( CDXUTSDKMesh* pDXUTMesh, unsigned int uDrawCallStart, unsigned int* pPrimsPerDraw );
	//void GetMeshNumDrawCalls( CDXUTSDKMesh* pDXUTMesh, unsigned int* pNumDrawCalls );
};


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------