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

#ifndef _MSAA_DETECT_H
#define _MSAA_DETECT_H

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
// CB
//--------------------------------------------------------------------------------------
BEGIN_CONSTANT_BLOCK(MSAA_Detect_Constants,b0)
{
	float		g_MSAA_Detect_fSampleCount;
	float		g_MSAA_Detect_fDepthEpsilonPercent;
	float2		g_MSAA_Detect_f2Pad;
};


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_SCREEN_QUAD_INPUT Screen_Quad( VS_SCREEN_QUAD_INPUT IN )
{
    PS_SCREEN_QUAD_INPUT O;
    
    O.f4Position.x = IN.f3Position.x;
	O.f4Position.y = IN.f3Position.y;
    O.f4Position.z = IN.f3Position.z;
    O.f4Position.w = 1.0f;
    
    O.f2TexCoord = IN.f2TexCoord;
    
    return O;    
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT MSAA_Detect( PS_SCREEN_QUAD_INPUT IN )
{
	//float fDepth0 = tDepthBufferMSAA.Load( int2( IN.f4Position.x, IN.f4Position.y ), 0, int2( 0, 0 ) );
	float fDepth0 = LOAD( tDepthBufferMSAA, int2( IN.f4Position.x, IN.f4Position.y ), 0, int2( 0, 0 ) );
	float fEpsilon = fDepth0 * g_MSAA_Detect_fDepthEpsilonPercent;
	float fComplexSamples = 0.0f;

	for( int i = 1; i < g_MSAA_Detect_fSampleCount; i++ )
	{
		//float fDepthi = tDepthBufferMSAA.Load( int2( IN.f4Position.x, IN.f4Position.y ), i, int2( 0, 0 ) );
		float fDepthi = LOAD( tDepthBufferMSAA, int2( IN.f4Position.x, IN.f4Position.y ), i, int2( 0, 0 ) );

		if( abs( fDepth0 - fDepthi ) > fEpsilon )
		{
			fComplexSamples += 1.0f;
		}
	}

	if( fComplexSamples == 0.0f  )
	{
		discard;
	}

	PS_OUTPUT O;
	O.f4Output = float4( 0, 0, 0, 0 );
	return O;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT MSAA_Show_Complex( PS_SCREEN_QUAD_INPUT IN )
{
	PS_OUTPUT O;
	O.f4Output = float4( 1, 0, 0, 1 );
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

#ifdef Screen_Quad_VS
void main() 
{
	VS_SCREEN_QUAD_INPUT IN;
	PS_SCREEN_QUAD_INPUT OUT;
	IN.f3Position = a_f3Position;
	IN.f2TexCoord = a_f2TexCoord;
	OUT = Screen_Quad( IN );
	gl_Position = OUT.f4Position;
	v_f2TexCoord = OUT.f2TexCoord;
}
#endif

#ifdef MSAA_Detect_PS
void main()
{
	PS_SCREEN_QUAD_INPUT IN;
	PS_OUTPUT OUT;
	IN.f4Position = gl_FragCoord;
	IN.f2TexCoord = v_f2TexCoord;
	OUT = MSAA_Detect( IN );
	o_f4Color = OUT.f4Output; 
}
#endif

#ifdef MSAA_Show_Complex_PS
void main()
{
	PS_SCREEN_QUAD_INPUT IN;
	PS_OUTPUT OUT;
	IN.f4Position = gl_FragCoord;
	IN.f2TexCoord = v_f2TexCoord;
	OUT = MSAA_Show_Complex( IN );
	o_f4Color = OUT.f4Output; 
}
#endif

#else

PS_SCREEN_QUAD_INPUT Screen_Quad_VS( VS_SCREEN_QUAD_INPUT IN )
{
	return Screen_Quad( IN );
}

PS_OUTPUT MSAA_Detect_PS( PS_SCREEN_QUAD_INPUT IN ) : SV_TARGET
{
	return MSAA_Detect( IN );
}

PS_OUTPUT MSAA_Show_Complex_PS( PS_SCREEN_QUAD_INPUT IN ) : SV_TARGET
{
	return MSAA_Show_Complex( IN );
}

#endif


#endif //_MSAA_DETECT_H


//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------
