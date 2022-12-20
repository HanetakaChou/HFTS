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


#include "Scene.h"


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
class Warrior_Scene : public Scene
{
public:

    Warrior_Scene();

    void CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext );
    
    void ReleaseResources();

	void ComputeSceneAABB();

	void RenderDepthOnlyScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime );
	void RenderMaps( ID3D11DeviceContext* pd3dImmediateContext, double fTime );
	void RenderScene( ID3D11DeviceContext* pd3dImmediateContext, double fTime );

	unsigned int GetNumPresetViews();
	void SetPresetView( unsigned int uPresetView );
	unsigned int GetPresetView();
	
private:
		    
	CDXUTSDKMesh			m_Scene;
	CDXUTSDKMesh			m_Warrior;
	CDXUTSDKMesh*			m_pArmor;
};


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------





	