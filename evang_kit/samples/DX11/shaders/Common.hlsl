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

#include "Cross_Platform_Defines.hlsl"


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

#define MAX_LIGHTS 4
#define MAX_BONE_MATRICES 100

BEGIN_CONSTANT_BLOCK(globalconstants,b0)
{
	float4		g_ScreenRes;
	float4x4	g_View;
	float4x4    g_ViewProjection;
    float4      g_LightWorldPos;
	float4		g_EyePos;
	float4		g_Textured;
};

BEGIN_CONSTANT_BLOCK(objectconstants,b1)
{
	float4x4    g_World;
	float4		g_HasTextures;
};

BEGIN_CONSTANT_BLOCK(boneconstants,b2)
{
	float4x4	g_Bone[MAX_BONE_MATRICES];
};

BEGIN_CONSTANT_BLOCK(shadowmapconstants,b3)
{
    float4x4	g_LightViewProj;
};


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
	float4 f4Output;
};


//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------

TEXTURE_2D(float,tShadowBuffer,t0)
TEXTURE_2DMS(float,tShadowBufferMSAA,t0)
TEXTURE_2D(float4,tDiffuse,t1)
SAMPLER(PointSampler,s0)
SAMPLER(LinearSampler,s1)


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------

