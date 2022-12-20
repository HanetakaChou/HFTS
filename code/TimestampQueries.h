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
typedef enum
{
	GPU_TIME_DEPTH_PRE_PASS,
	GPU_TIME_MSAA_DEPTH_RESOLVE,
	GPU_TIME_MSAA_DETECT,
	GPU_TIME_SHADOW_MAPS,
	GPU_TIME_SHADOW_BUFFER,
	GPU_TIME_SCENE,
	GPU_TIME_FXAA,
	NUM_GPU_TIMES
} RenderTimeId;

typedef struct
{
    float GpuTimeMS[NUM_GPU_TIMES];
    float CpuTimeUS;
} RenderTimes;

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class TimestampQueries
{
public:
    void Create(ID3D11Device* pD3DDevice)
    {
        D3D11_QUERY_DESC queryDesc;
        queryDesc.MiscFlags = 0;

        queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        pD3DDevice->CreateQuery(&queryDesc, &m_pDisjointTimestampQuery);
        m_DisjointQueryInFlight = false;

        queryDesc.Query = D3D11_QUERY_TIMESTAMP;
        for (UINT i = 0; i < NUM_GPU_TIMES; ++i)
        {
            pD3DDevice->CreateQuery(&queryDesc, &m_pTimestampQueriesBegin[i]);
            pD3DDevice->CreateQuery(&queryDesc, &m_pTimestampQueriesEnd[i]);
            m_TimestampQueryInFlight[i] = false;
        }
    }

    void Release()
    {
        SAFE_RELEASE(m_pDisjointTimestampQuery);

        for (UINT i = 0; i < NUM_GPU_TIMES; ++i)
        {
            SAFE_RELEASE(m_pTimestampQueriesBegin[i]);
            SAFE_RELEASE(m_pTimestampQueriesEnd[i]);
        }
    }

    void Begin(ID3D11DeviceContext* pDeviceContext)
    {
        if (!m_DisjointQueryInFlight)
        {
            pDeviceContext->Begin(m_pDisjointTimestampQuery);
        }
    }

    void End(ID3D11DeviceContext* pDeviceContext, RenderTimes* pRenderTimes)
    {
        if (!m_DisjointQueryInFlight)
        {
            pDeviceContext->End(m_pDisjointTimestampQuery);
        }
        m_DisjointQueryInFlight = true;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointTimestampValue;
        if (pDeviceContext->GetData(m_pDisjointTimestampQuery, &disjointTimestampValue, sizeof(disjointTimestampValue), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK)
        {
            m_DisjointQueryInFlight = false;

            if (!disjointTimestampValue.Disjoint)
            {
                double InvFrequencyMS = 1000.0 / disjointTimestampValue.Frequency;
                for (UINT i = 0; i < NUM_GPU_TIMES; ++i)
                {
                    if (m_TimestampQueryInFlight[i])
                    {
                        UINT64 TimestampValueBegin;
                        UINT64 TimestampValueEnd;
                        if ((pDeviceContext->GetData(m_pTimestampQueriesBegin[i], &TimestampValueBegin, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) &&
                            (pDeviceContext->GetData(m_pTimestampQueriesEnd[i],   &TimestampValueEnd,   sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK))
                        {
                            m_TimestampQueryInFlight[i] = false;
                            pRenderTimes->GpuTimeMS[i] = float(double(TimestampValueEnd - TimestampValueBegin) * InvFrequencyMS);
                        }
                    }
                    else
                    {
                        pRenderTimes->GpuTimeMS[i] = 0.f;
                    }
                }
            }
        }
    }

    void StartTimer(ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
    {
        if (!m_TimestampQueryInFlight[Id])
        {
            pDeviceContext->End(m_pTimestampQueriesBegin[Id]);
        }
    }

    void StopTimer(ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
    {
        if (!m_TimestampQueryInFlight[Id])
        {
            pDeviceContext->End(m_pTimestampQueriesEnd[Id]);
        }
        m_TimestampQueryInFlight[Id] = true;
    }

protected:
    bool m_DisjointQueryInFlight;
    bool m_TimestampQueryInFlight[NUM_GPU_TIMES];
    ID3D11Query* m_pDisjointTimestampQuery;
    ID3D11Query* m_pTimestampQueriesBegin[NUM_GPU_TIMES];
    ID3D11Query* m_pTimestampQueriesEnd[NUM_GPU_TIMES];
};


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class GPUTimer
{
public:
    GPUTimer(TimestampQueries* pTimestampQueries, ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
        : m_pTimestampQueries(pTimestampQueries)
        , m_pDeviceContext(pDeviceContext)
        , m_NVAORenderTimeId(Id)
    {
        m_pTimestampQueries->StartTimer(m_pDeviceContext, m_NVAORenderTimeId);
    }
    ~GPUTimer()
    {
        m_pTimestampQueries->StopTimer(m_pDeviceContext, m_NVAORenderTimeId);
    }

private:
    TimestampQueries* m_pTimestampQueries;
    ID3D11DeviceContext* m_pDeviceContext;
    RenderTimeId m_NVAORenderTimeId;
};


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------

