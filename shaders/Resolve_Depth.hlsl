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

#ifndef _RESOLVE_DEPTH_H
#define _RESOLVE_DEPTH_H

#include "Common.hlsl"


//--------------------------------------------------------------------------------------
// shader input/output structure
//--------------------------------------------------------------------------------------
struct VS_SCREEN_QUAD_INPUT
{
    float3 f3Position	SEMANTIC( POSITION )
    float2 f2TexCoord	SEMANTIC( TEXCOORD0 ) 
};

struct PS_SCREEN_QUAD_INPUT
{
    float4 f4Position	SEMANTIC( SV_Position )
    float2 f2TexCoord	SEMANTIC( TEXCOORD0 )
};


//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
TEXTURE_2DMS(float,tDepthBufferMSAA,t0)


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT Resolve_Depth( PS_SCREEN_QUAD_INPUT IN )
{
	PS_OUTPUT O;
	float RD = LOAD(tDepthBufferMSAA,int2( IN.f4Position.x, IN.f4Position.y ),int( 0 ),int2( 0, 0 )).x;
	O.f4Output = float4( RD, RD, RD, 1 );
	return O;
}

//--------------------------------------------------------------------------------------
// DX / GL stubs
//--------------------------------------------------------------------------------------

#ifdef GL

#ifdef VS
in float3 a_f3Position;
in float2 a_f2TexCoord;
out float2 v_f2TexCoord;
#endif

#ifdef PS
in float4 v_f4Position;
in float2 v_f2TexCoord;
out float4 o_f4Color;
#endif

#ifdef Resolve_Depth_PS
void main()
{
	PS_SCREEN_QUAD_INPUT IN;
	PS_OUTPUT OUT;
	IN.f4Position = gl_FragCoord;
	IN.f2TexCoord = v_f2TexCoord;
	OUT = Resolve_Depth( IN );
	o_f4Color = OUT.f4Output; 
}
#endif

#else

PS_OUTPUT Resolve_Depth_PS( PS_SCREEN_QUAD_INPUT IN ) : SV_TARGET
{
	return Resolve_Depth( IN );
}

#endif

#endif //_RESOLVE_DEPTH_H


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------
