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

#include "Common.hlsl"


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Position		SEMANTIC( POSITION )
	float3 Normal		SEMANTIC( NORMAL )
};

struct VS_TEXTURED_SKINNED_INPUT
{
    float4 Position     SEMANTIC( POSITION )
    float3 Normal       SEMANTIC( NORMAL )
	float2 TexCoord     SEMANTIC( TEXCOORD0 )
	float3 Tangent      SEMANTIC( TANGENT )
	uint4 Bones			SEMANTIC( BONES )
    float4 Weights		SEMANTIC( WEIGHTS )
};

struct VS_OUTPUT
{
	float4 Position		SEMANTIC( SV_Position )
};

struct SKINNED_INFO
{
    float4 Pos;
    float3 Norm;
};


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
VS_OUTPUT ShadowMapGeom( VS_INPUT IN )
{
	VS_OUTPUT O;

	float4x4 WorldViewProjection = MUL( g_World, g_LightViewProj );
	O.Position = mul( float4( IN.Position.xyz, 1.0f ), WorldViewProjection );

	return O;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
SKINNED_INFO SkinVert( VS_TEXTURED_SKINNED_INPUT In )
{
    SKINNED_INFO Output = (SKINNED_INFO)0;
    
	float4 pos = float4(In.Position.xyz,1);
	float3 norm = In.Normal;
    
    uint iBone = In.Bones.x;
    float fWeight = In.Weights.x;
    
    matrix m = g_Bone[ iBone ];
    Output.Pos += fWeight * mul( pos, m );
    Output.Norm += fWeight * mul( float4( norm, 0.0f), m ).xyz;
    
    iBone = In.Bones.y;
    fWeight = In.Weights.y;
    m = g_Bone[ iBone ];
    Output.Pos += fWeight * mul( pos, m );
    Output.Norm += fWeight * mul( float4( norm, 0.0f), m ).xyz;

    iBone = In.Bones.z;
    fWeight = In.Weights.z;
    m = g_Bone[ iBone ];
    Output.Pos += fWeight * mul( pos, m );
    Output.Norm += fWeight * mul( float4( norm, 0.0f), m ).xyz;
    
    iBone = In.Bones.w;
    fWeight = In.Weights.w;
    m = g_Bone[ iBone ];
    Output.Pos += fWeight * mul( pos, m );
    Output.Norm += fWeight * mul( float4( norm, 0.0f), m ).xyz;
    
    return Output;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
VS_OUTPUT ShadowMapGeomSkinned( VS_TEXTURED_SKINNED_INPUT IN )
{
	VS_OUTPUT O;

	SKINNED_INFO Skinned = SkinVert( IN );
	
	float4x4 WorldViewProjection = MUL( g_World, g_LightViewProj );
	O.Position = mul( float4( Skinned.Pos.xyz, 1.0f ), WorldViewProjection );

	return O;
}


//--------------------------------------------------------------------------------------
// DX / GL stubs
//--------------------------------------------------------------------------------------

#ifdef GL

#ifdef VS
in float3 a_f3Position;
#endif

#ifdef ShadowMapGeom_VS
void main() 
{
	VS_INPUT IN;
	VS_OUTPUT OUT;
	IN.WorldPos = a_f3Position;
	OUT = ShadowMapGeom( IN );
	gl_Position = OUT.Position;
}
#endif

#else

VS_OUTPUT ShadowMapGeom_VS( VS_INPUT IN )
{
	return ShadowMapGeom( IN );
}

VS_OUTPUT ShadowMapGeomSkinned_VS( VS_TEXTURED_SKINNED_INPUT IN )
{
	return ShadowMapGeomSkinned( IN );
}

#endif

//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------