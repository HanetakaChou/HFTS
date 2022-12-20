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
//
//--------------------------------------------------------------------------------------
STATIC float4 g_f4LightAmbient = float4( 0.3f, 0.3f, 0.3f, 1.0f );
STATIC float4 g_f4LightDiffuse = float4( 1.0f, 0.4f, 0.09f, 1.0f );
STATIC float4 g_MaterialDiffuseColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
STATIC float4 g_MaterialSpecularColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
STATIC float  g_fSpecularPower = 32.0f;


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Position     SEMANTIC( POSITION )
    float3 Normal       SEMANTIC( NORMAL )
};

struct VS_OUTPUT
{
    float4 Position     SEMANTIC( SV_POSITION )
    float3 WSPosition   SEMANTIC( TEXCOORD1 )
    float3 WSNormal     SEMANTIC( TEXCOORD2 ) 
};

struct PS_OUTPUT
{
	float4 f4Output;
};


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
BEGIN_CONSTANT_BLOCK(Light_Control_Constants,b0)
{
    float4x4    g_SceneViewProj;
	float4		g_EyePos;
    float4      g_LightWorldPos;
	float4		g_LightLookAt;
	float4		g_LightSize;
};


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
VS_OUTPUT Light_Control_Geom( VS_INPUT IN )
{
    VS_OUTPUT Output;
    
	Output.WSPosition = ( IN.Position.xyz * g_LightSize.xxx ) + g_LightWorldPos.xyz;
    Output.WSNormal = normalize( IN.Normal ); 

	Output.Position = MUL( float4( Output.WSPosition, 1 ), g_SceneViewProj );
        
    return Output;   
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT Light_Control( VS_OUTPUT IN )
{
	float3 f3CamDir = normalize( g_EyePos.xyz - IN.WSPosition.xyz ); 
    float3 f3LightDir = normalize( g_LightLookAt.xyz - g_LightWorldPos.xyz );
	float3 f3HalfAngle = normalize( f3CamDir - f3LightDir );

	float2 f2Lit;
	float fNdotL = dot( IN.WSNormal, f3LightDir );
	float fNdotH = dot( IN.WSNormal, f3HalfAngle );
	f2Lit.x = ( fNdotL < 0 ) ? ( 0 ) : ( fNdotL );
	f2Lit.y = ( ( fNdotL < 0 ) || ( fNdotH < 0 ) ) ? ( 0 ) : ( pow( fNdotH, g_fSpecularPower ) );
    
    float3 f3Ambient = g_MaterialDiffuseColor.xyz * 0.3f;
    float3 f3Diffuse = f2Lit.xxx * g_f4LightDiffuse.xyz * g_MaterialDiffuseColor.xyz;
	float3 f3Specular = f2Lit.yyy * g_MaterialSpecularColor.xyz * g_f4LightDiffuse.xyz;
	    
    float3 f3FinalColor = f3Ambient + f3Diffuse + f3Specular;
	  
	PS_OUTPUT O;
	O.f4Output = float4( f3FinalColor, 1.0f );
	return O;
}


//--------------------------------------------------------------------------------------
// DX / GL stubs
//--------------------------------------------------------------------------------------

#ifdef GL

#ifdef VS
in float3 a_f3Position;
in float3 a_f3Normal;
out float3 v_f3WSPosition;
out float3 v_f3WSNormal;
#endif

#ifdef PS
in float4 v_f4Position;
in float3 v_f3WSPosition;
in float3 v_f3WSNormal;
out float4 o_f4Color;
#endif

#ifdef Light_Control_VS
void main() 
{
	VS_INPUT IN;
	VS_OUTPUT OUT;
	IN.Position = a_f3Position;
	IN.Normal = a_f3Normal;
	OUT = Light_Control_Geom( IN );
	gl_Position = OUT.Position;
	v_f3WSPosition = OUT.WSPosition;
	v_f3WSNormal = OUT.WSNormal;
}
#endif

#ifdef Light_Control_PS
void main()
{
	VS_OUTPUT IN;
	PS_OUTPUT OUT;
	IN.Position = gl_FragCoord;
	IN.WSPosition = v_f3WSPosition;
	IN.WSNormal = v_f3WSNormal;
	OUT = Light_Control( IN );
	o_f4Color = OUT.f4Output; 
}
#endif

#else

VS_OUTPUT Light_Control_VS( VS_INPUT IN )
{
	return Light_Control_Geom( IN );
}

PS_OUTPUT Light_Control_PS( VS_OUTPUT IN ) : SV_TARGET
{
	return Light_Control( IN );
}

#endif


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------