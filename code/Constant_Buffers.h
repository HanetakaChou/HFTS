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
#include "Shadow.h"


//--------------------------------------------------------------------------------------
// Global constant buffer
//--------------------------------------------------------------------------------------
class SceneConstantBuffer
{
public:
    ID3D11Buffer *pBuffer;

    SceneConstantBuffer() :
        pBuffer(NULL)
    {
    }

	struct
    {
		float		ScreenRes[4];
		D3DXMATRIX  View;
        D3DXMATRIX  ViewProj;
        D3DXVECTOR4 LightWorldPos;
		float		EyePos[4];
		D3DXVECTOR4 Textured;
    } CBData;

    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// Object constant buffer
//--------------------------------------------------------------------------------------
class ObjectConstantBuffer
{
public:
    ID3D11Buffer *pBuffer;

    ObjectConstantBuffer() :
        pBuffer(NULL)
    {
    }

	struct
    {
		D3DXMATRIX  World;
		D3DXVECTOR4 HasTextures;
    } CBData;

    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// material constant buffer
//--------------------------------------------------------------------------------------
class MaterialConstantBuffer
{
public:
    ID3D11Buffer *pBuffer;

    MaterialConstantBuffer() :
        pBuffer(NULL)
    {
    }

	struct	
	{
		float  g_DiffuseColor[4];
	}CBData;

    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// Shadow-map constant buffer
//--------------------------------------------------------------------------------------
class ShadowMapConstantBuffer
{
public:
    ID3D11Buffer *pBuffer;

    ShadowMapConstantBuffer() :
        pBuffer(NULL)
    {
    }

    struct
    {
        D3DXMATRIX  WorldToLightClip;
    } CBData;

    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
class MSAADetectConstantBuffer
{
public:
    ID3D11Buffer *pBuffer;

    MSAADetectConstantBuffer() :
        pBuffer(NULL)
    {
    }

    struct
    {
        float		fSampleCount;
		float		fDepthEpsilonPercent;
		float		fPad[2];
    } CBData;

    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
class BoneConstantBuffer
{
public:

	#define MAX_BONES ( 100 )

    ID3D11Buffer *pBuffer;

    BoneConstantBuffer() :
        pBuffer(NULL)
    {
    }

	struct
    {
		D3DXMATRIX	Bone[MAX_BONES];						
    }CBData;

    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
class FXAAConstantBuffer
{
public:

    ID3D11Buffer *pBuffer;

    FXAAConstantBuffer() :
        pBuffer(NULL)
    {
    }

	struct
	{
		D3DXVECTOR4 v4FXAA;
	}CBData;
	
    void CreateResources(ID3D11Device *pd3dDevice)
    {
        HRESULT hr;

        static D3D11_BUFFER_DESC desc = 
        {
             sizeof(CBData),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        V( pd3dDevice->CreateBuffer(&desc, NULL, &pBuffer) );
    }

    void ReleaseResources()
    {
        SAFE_RELEASE(pBuffer);
    }
};


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
