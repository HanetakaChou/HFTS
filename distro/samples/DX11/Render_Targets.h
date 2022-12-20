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
//
//--------------------------------------------------------------------------------------
class SimpleRT
{
public:
    ID3D11Texture2D* pTexture;
    ID3D11RenderTargetView* pRTV;
    ID3D11ShaderResourceView* pSRV;

    SimpleRT(ID3D11Device* pd3dDevice, UINT Width, UINT Height, DXGI_FORMAT Format) :
        pTexture(NULL), pRTV(NULL), pSRV(NULL)
    {
        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = Width;
        desc.Height             = Height;
        desc.Format             = Format;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);
        pd3dDevice->CreateShaderResourceView(pTexture, NULL, &pSRV);
        pd3dDevice->CreateRenderTargetView(pTexture, NULL, &pRTV);
    }

    ~SimpleRT()
    {
        SAFE_RELEASE(pTexture);
        SAFE_RELEASE(pRTV);
        SAFE_RELEASE(pSRV);
    }
};


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------