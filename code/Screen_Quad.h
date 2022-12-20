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

class Screen_Quad
{
public:
	
    Screen_Quad();
    ~Screen_Quad();

    HRESULT CreateResources( ID3D11Device* pD3D11Device );

    void DestroyResources();

    void Render(    ID3D11DeviceContext* pd3dImmediateContext,
					D3D11_VIEWPORT VP, 
                    ID3D11PixelShader* pPS, 
                    ID3D11Buffer** ppCBs,
					unsigned int uNumCBs, 
                    unsigned int uCBSlot,
                    ID3D11RenderTargetView** ppRTVs, 
                    unsigned int uNumRTVs, 
                    ID3D11ShaderResourceView** ppSRVs, 
                    unsigned int uNumSRVs,
                    unsigned int uSRVStartSlot,
                    ID3D11SamplerState** ppSamplerStates,
                    unsigned int uNumSamplerStates,
					ID3D11DepthStencilView* pDSV );
                        
private:

    ID3D11Buffer*			m_pSQ_VB;
};


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------