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

STATIC float g_fAmbient = 0.3f;


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
struct VS_DEPTH_ONLY_INPUT
{
    float3 Position     SEMANTIC( POSITION )
	float3 Normal		SEMANTIC( NORMAL )
};

struct VS_G_BUFFER_OUTPUT
{
    float4 Position			SEMANTIC( SV_POSITION )
	float3 EyeViewPosition  SEMANTIC( EYE_VIEW_POSITION )
	float  NdotLTerm		SEMANTIC( N_DOT_L_TERM )						
};

struct VS_TEXTURED_INPUT
{
    float3 Position     SEMANTIC( POSITION )
    float3 Normal       SEMANTIC( NORMAL )
	float2 TexCoord     SEMANTIC( TEXCOORD0 )
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

struct VS_TEXTURED_OUTPUT
{
    float4 Position     SEMANTIC( SV_POSITION )
    float3 WSPosition   SEMANTIC( TEXCOORD1 )
    float3 WSNormal     SEMANTIC( TEXCOORD2 )
	float2 TexCoord     SEMANTIC( TEXCOORD3 )
};

struct VS_MSAA_TEXTURED_OUTPUT
{
    float4 Position     SEMANTIC( SV_POSITION )
    float3 WSPosition   SEMANTIC( TEXCOORD1 )
    float3 WSNormal     SEMANTIC( TEXCOORD2 )
	float2 TexCoord     SEMANTIC( TEXCOORD3 )
	uint uSampleIndex	SEMANTIC( SV_SampleIndex )
};

struct SKINNED_INFO
{
    float4 Pos;
    float3 Norm;
};


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
VS_G_BUFFER_OUTPUT GBufferSceneGeom( VS_DEPTH_ONLY_INPUT IN )
{
    VS_G_BUFFER_OUTPUT Output;
    
	float4x4 WorldViewProjection = MUL( g_World, g_ViewProjection );
    Output.Position = MUL( float4( IN.Position, 1.0f ), WorldViewProjection );

	float4x4 WorldView = MUL( g_World, g_View );
	Output.EyeViewPosition = MUL( float4( IN.Position, 1.0f ), WorldView ).xyz;
	
	float3 N = normalize( mul( float4( IN.Normal.xyz, 0.0f ), WorldView ).xyz );
	float3 L = mul( float4( g_LightWorldPos.xyz, 1.0f ), g_View ).xyz;
	L = normalize( L - Output.EyeViewPosition );
	Output.NdotLTerm = ( dot( N, L ) < 0.0f ) ? ( 0.0f ) : ( 1.0f );
	
    return Output;   
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT GBufferScene_PS( VS_G_BUFFER_OUTPUT IN ) : SV_TARGET
{
	PS_OUTPUT O;

	O.f4Output = float4( IN.EyeViewPosition.z, IN.EyeViewPosition.z, IN.EyeViewPosition.z, 1.0f );

	return O;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT TexturedSceneGeom( VS_TEXTURED_INPUT IN )
{
    VS_TEXTURED_OUTPUT Output;

	float4x4 WorldViewProjection = MUL( g_World, g_ViewProjection );
	Output.Position = MUL( float4( IN.Position.xyz, 1.0f ), WorldViewProjection );
	Output.WSPosition.xyz = mul( float4( IN.Position.xyz, 1.0f ), g_World ).xyz;
    Output.WSNormal = normalize( IN.Normal );

	#ifdef GL
		Output.TexCoord = float2( IN.TexCoord.x, 1.0f - IN.TexCoord.y );
	#else
		Output.TexCoord = IN.TexCoord;
	#endif

    return Output;   
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
VS_G_BUFFER_OUTPUT GBufferSkinnedSceneGeom( VS_TEXTURED_SKINNED_INPUT IN )
{
    VS_G_BUFFER_OUTPUT Output;
    
	SKINNED_INFO Skinned = SkinVert( IN );

	float4x4 WorldViewProjection = MUL( g_World, g_ViewProjection );
    Output.Position = MUL( float4( Skinned.Pos.xyz, 1.0f ), WorldViewProjection );

	float4x4 WorldView = MUL( g_World, g_View );
	Output.EyeViewPosition = MUL( float4( Skinned.Pos.xyz, 1.0f ), WorldView ).xyz;
	
	float3 N = normalize( mul( float4( Skinned.Norm.xyz, 0.0f ), WorldView ).xyz );
	float3 L = mul( float4( g_LightWorldPos.xyz, 1.0f ), g_View ).xyz;
	L = normalize( L - Output.EyeViewPosition );
	Output.NdotLTerm = ( dot( N, L ) < 0.0f ) ? ( 0.0f ) : ( 1.0f );
	
    return Output;   
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT TexturedSkinnedSceneGeom( VS_TEXTURED_SKINNED_INPUT In )
{
    VS_TEXTURED_OUTPUT Output;
    	
    SKINNED_INFO Skinned = SkinVert( In );

	float4x4 WorldViewProjection = MUL( g_World, g_ViewProjection );
	Output.Position = mul( float4( Skinned.Pos.xyz, 1.0f ), WorldViewProjection );
	Output.WSPosition = MUL( float4( Skinned.Pos.xyz, 1.0f ), g_World ).xyz;
		
	Output.WSNormal = normalize( MUL( float4( Skinned.Norm, 0.0f ), g_World ).xyz );

	Output.TexCoord = In.TexCoord;
        
    return Output;   
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT TexturedScene( VS_TEXTURED_OUTPUT IN )
{
	float3 f3LightDir = normalize( g_LightWorldPos.xyz - IN.WSPosition.xyz );
	float fDot = dot( IN.WSNormal, f3LightDir );
	float fLit = ( fDot < 0 ) ? ( 0 ) : ( fDot );
	fLit = SATURATE( fLit );

	float4 vDiffuse = float4( 0.9f, 0.9f, 0.9f, 0.9f );
	if( g_HasTextures.x == 1.0f && g_Textured.x == 1.0f )
	{
		vDiffuse = SAMPLE( tDiffuse, LinearSampler, float2( IN.TexCoord.x, IN.TexCoord.y ) );
	}

	float3 f3Diffuse = fLit * vDiffuse.xyz;

	float2 uv = IN.Position.xy * g_ScreenRes.zw;
	float Shadow = SAMPLE_LEVEL( tShadowBuffer, PointSampler, uv, 0 ).r;
				
	float3 f3Ambient = vDiffuse.xyz * g_fAmbient;
		
	float3 f3LitColor = f3Ambient + f3Diffuse;
		
	float3 f3FinalColor = LERP( f3Ambient, f3LitColor, Shadow );
	
	PS_OUTPUT O;
	O.f4Output = float4( f3FinalColor, 1.0f );
	return O;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_OUTPUT MSAA_TexturedScene( VS_MSAA_TEXTURED_OUTPUT IN )
{
	float3 f3LightDir = normalize( g_LightWorldPos.xyz - IN.WSPosition.xyz );
	float fDot = dot( IN.WSNormal, f3LightDir );
	float fLit = ( fDot < 0 ) ? ( 0 ) : ( fDot );
	fLit = SATURATE( fLit );
	
	float4 vDiffuse = float4( 0.9f, 0.9f, 0.9f, 0.9f );
	if( g_HasTextures.x == 1.0f && g_Textured.x == 1.0f )
	{
		vDiffuse = SAMPLE( tDiffuse, LinearSampler, float2( IN.TexCoord.x, IN.TexCoord.y ) );
	}

	float3 f3Diffuse = fLit * vDiffuse.xyz;

	float Shadow = LOAD( tShadowBufferMSAA, int2( IN.Position.x, IN.Position.y ), int( IN.uSampleIndex ), int2( 0, 0 ) );
				
	float3 f3Ambient = vDiffuse.xyz * g_fAmbient;
		
	float3 f3LitColor = f3Ambient + f3Diffuse;
		
	float3 f3FinalColor = LERP( f3Ambient, f3LitColor, Shadow );
	
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
in float2 a_f2TexCoord;
out float3 v_f3WSPosition;
out float3 v_f3WSNormal;
out float2 v_f2TexCoord; 
#endif

#ifdef PS
in float3 v_f3WSPosition;
in float3 v_f3WSNormal;
in float2 v_f2TexCoord;
out float4 o_f4Color;
#endif

#ifdef DepthOnlySceneGeom_VS
void main() 
{
	VS_DEPTH_ONLY_INPUT IN;
	VS_DEPTH_ONLY_OUTPUT OUT;
	IN.Position = a_f3Position;
	OUT = DepthOnlySceneGeom( IN );
	gl_Position = OUT.Position; 
}
#endif

#ifdef TexturedSceneGeom_VS
void main() 
{
	VS_TEXTURED_INPUT IN;
	VS_TEXTURED_OUTPUT OUT;
	IN.Position = a_f3Position;
	IN.Normal = a_f3Normal;
	IN.TexCoord = a_f2TexCoord;
	OUT = TexturedSceneGeom( IN );
	gl_Position = OUT.Position;
	v_f3WSPosition = OUT.WSPosition;
	v_f3WSNormal = OUT.WSNormal;
	v_f2TexCoord = OUT.TexCoord;
}
#endif

#ifdef TexturedScene_PS
void main()
{
	VS_TEXTURED_OUTPUT IN;
	PS_OUTPUT OUT;
	IN.Position = gl_FragCoord;
	IN.WSPosition = v_f3WSPosition;
	IN.WSNormal = v_f3WSNormal;
	IN.TexCoord = v_f2TexCoord;
	OUT = TexturedScene( IN );
	o_f4Color = OUT.f4Output; 
}
#endif

#ifdef MSAA_TexturedScene_PS
void main() 
{
	VS_MSAA_TEXTURED_OUTPUT IN;
	PS_OUTPUT OUT;
	IN.Position = gl_FragCoord;
	IN.WSPosition = v_f3WSPosition;
	IN.WSNormal = v_f3WSNormal;
	IN.TexCoord = v_f2TexCoord;
	IN.uSampleIndex = gl_SampleID;
	OUT = MSAA_TexturedScene( IN );
	o_f4Color = OUT.f4Output;
}
#endif

#else

VS_G_BUFFER_OUTPUT GBufferSceneGeom_VS( VS_DEPTH_ONLY_INPUT IN )
{
	return GBufferSceneGeom( IN );
}

VS_TEXTURED_OUTPUT TexturedSceneGeom_VS( VS_TEXTURED_INPUT IN )
{
	return TexturedSceneGeom( IN );
}

VS_G_BUFFER_OUTPUT GBufferSkinnedSceneGeom_VS( VS_TEXTURED_SKINNED_INPUT IN )
{
	return GBufferSkinnedSceneGeom( IN );
}

VS_TEXTURED_OUTPUT TexturedSkinnedSceneGeom_VS( VS_TEXTURED_SKINNED_INPUT IN )
{
	return TexturedSkinnedSceneGeom( IN );
}

PS_OUTPUT TexturedScene_PS( VS_TEXTURED_OUTPUT IN ) : SV_TARGET
{
	return TexturedScene( IN );
}

PS_OUTPUT MSAA_TexturedScene_PS( VS_MSAA_TEXTURED_OUTPUT IN ) : SV_TARGET
{
	return MSAA_TexturedScene( IN );
}

#endif


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
