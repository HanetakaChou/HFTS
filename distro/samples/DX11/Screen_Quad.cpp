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


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------

#include "DXUT.h"
#include "Shaders.h"
#include "Screen_Quad.h"


//--------------------------------------------------------------------------------------
// structures and globals used by the screen quad functions
//--------------------------------------------------------------------------------------
struct SQV
{
    D3DXVECTOR3 v3Pos;
	D3DXVECTOR2 v2TexCoord;
};

Screen_Quad::Screen_Quad()
{
    m_pSQ_VB = NULL;
}

Screen_Quad::~Screen_Quad()
{
    DestroyResources();
}

//--------------------------------------------------------------------------------------
// Creates resources for rendering a screen quad (VB, VS, IL) 
//--------------------------------------------------------------------------------------
HRESULT Screen_Quad::CreateResources( ID3D11Device* pD3D11Device )
{
    SAFE_RELEASE( m_pSQ_VB );
    
    // Fill out a unit quad
    SQV SQVs[6];
    SQVs[0].v3Pos = D3DXVECTOR3( -1.0f, -1.0f, 0.0f );
    SQVs[0].v2TexCoord = D3DXVECTOR2( 0.0f, 1.0f );
    SQVs[1].v3Pos = D3DXVECTOR3( -1.0f, 1.0f, 0.0f );
    SQVs[1].v2TexCoord = D3DXVECTOR2( 0.0f, 0.0f );
    SQVs[2].v3Pos = D3DXVECTOR3( 1.0f, -1.0f, 0.0f );
    SQVs[2].v2TexCoord = D3DXVECTOR2( 1.0f, 1.0f );
    SQVs[3].v3Pos = D3DXVECTOR3( -1.0f, 1.0f, 0.0f );
    SQVs[3].v2TexCoord = D3DXVECTOR2( 0.0f, 0.0f );
    SQVs[4].v3Pos = D3DXVECTOR3( 1.0f, 1.0f, 0.0f );
    SQVs[4].v2TexCoord = D3DXVECTOR2( 1.0f, 0.0f );
    SQVs[5].v3Pos = D3DXVECTOR3( 1.0f, -1.0f, 0.0f );
    SQVs[5].v2TexCoord = D3DXVECTOR2( 1.0f, 1.0f );


    // Create the vertex buffer
    D3D11_BUFFER_DESC BD;
    BD.Usage = D3D11_USAGE_DYNAMIC;
    BD.ByteWidth = sizeof( SQV ) * 6;
    BD.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BD.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = SQVs;
    HRESULT hr = pD3D11Device->CreateBuffer( &BD, &InitData, &m_pSQ_VB );
	return hr;
}


//--------------------------------------------------------------------------------------
// Destroys resources for rendering a screen quad (VB, VS, IL) 
//--------------------------------------------------------------------------------------
void Screen_Quad::DestroyResources()
{
    SAFE_RELEASE( m_pSQ_VB );
}


//--------------------------------------------------------------------------------------
// Render screen aligned quad, using specified Viewport, CB, PS, RTVs, and SRVs
//--------------------------------------------------------------------------------------
void Screen_Quad::Render(	ID3D11DeviceContext* pd3dImmediateContext,
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
							ID3D11DepthStencilView* pDSV )
{
    // Change the viewport based on input
    pd3dImmediateContext->RSSetViewports( 1, &VP );


    // Set the provided constant buffer
    pd3dImmediateContext->PSSetConstantBuffers( uCBSlot, uNumCBs, ppCBs );
	

    // Set the RTVs    
    pd3dImmediateContext->OMSetRenderTargets( uNumRTVs, (ID3D11RenderTargetView*const*)ppRTVs, pDSV );


    // Set the SRVs
    pd3dImmediateContext->PSSetShaderResources( uSRVStartSlot, uNumSRVs, ppSRVs );


    // Set common samplers
    pd3dImmediateContext->PSSetSamplers( 0, uNumSamplerStates, ppSamplerStates );


    // Set input layout and screen quad VS
    UINT Stride = sizeof( SQV );
    UINT Offset = 0;
	pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pScreenQuadIA );
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pSQ_VB, &Stride, &Offset );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pd3dImmediateContext->VSSetShader( Shaders::Get()->pScreenQuadVS, NULL, 0 );
    

    // Set input PS
    pd3dImmediateContext->PSSetShader( pPS, NULL, 0 );
    

	// Ensure other shader stages are disabled
	pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
	pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );


    // Do the draw
    pd3dImmediateContext->Draw( 6, 0 );
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------