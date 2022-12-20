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


#ifndef _CROSS_PLATFORM_DEFINES_H
#define _CROSS_PLATFORM_DEFINES_H

#ifdef GL

#define BEGIN_CONSTANT_BLOCK(name,slot)	uniform name
#define SEMANTIC(x) ;

#define int2 ivec2
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4x4

#define ARRAY_BEGIN(name,type,size) type name[size] = type[size](
#define ARRAY_ITEM_V1(a) float(a)
#define ARRAY_ITEM_V2(a,b) float2(a,b)
#define ARRAY_ITEM_V3(a,b,c) float3(a,b,c)
#define ARRAY_ITEM_V4(a,b,c,d) float4(a,b,c,d)
#define ARRAY_END );

#define SHADOW_2D(type,name,slot) uniform sampler2DShadow name;
#define SHADOW_2DARRAY(type,name,slot) uniform sampler2DArrayShadow name;
#define TEXTURE_2D(type,name,slot) uniform sampler2D name;
#define TEXTURE_2DMS(type,name,slot) uniform sampler2DMS name;
#define TEXTURE_2DARRAY(type,name,slot) uniform sampler2DArray name;
#define SAMPLER(name,slot)
#define SAMPLER_COMPARISON_STATE(name,slot)

#define SAMPLE_LEVEL(name,sampler,coord,level) textureLod(name, coord, level)
#define SAMPLE(name,sampler,coord) texture(name, coord)
#define LOAD(name,coord,index,offset) texelFetch(name, coord, index)
#define SAMPLE_CMP_LEVEL_ZERO(name,sampler,coord,value) texture(name, float3( coord.x, coord.y, value ) )
#define SAMPLE_CMP_LEVEL_ZERO_ARRAY(name,sampler,coord,value,slice) texture(name, float4( coord.x, coord.y, slice, value ) )

#define LERP(a,b,c) mix(a,b,c)
#define MUL(a,b) (a * b)
#define SATURATE(a) clamp(a, 0.0f, 1.0f)

//#pragma optionNV(strict on)
//#pragma optionNV(fastprecision on)
//#pragma optionNV(fastmath on)
#define UNROLL #pragma optionNV(unroll all)
#define BRANCH #pragma optionNV(ifcvt all)
#define STATIC

#else // DX11

#define DEBUG_VIEW
#define CASCADES
#define SINGLE
#define MSAA

#define ARRAY_BEGIN(name,type,size) type name[] = {
#define ARRAY_ITEM_V1(a) {a}
#define ARRAY_ITEM_V2(a,b) {a,b}
#define ARRAY_ITEM_V3(a,b,c) {a,b,c}
#define ARRAY_ITEM_V4(a,b,c,d) {a,b,c,d}
#define ARRAY_END };

#define BEGIN_CONSTANT_BLOCK(name,slot)	cbuffer name : register(slot)
#define SEMANTIC(x) : x;

#define TEXTURE_2D(type,name,slot) Texture2D<type> name : register(slot);
#define TEXTURE_2DMS(type,name,slot) Texture2DMS<type> name : register(slot);
#define TEXTURE_2DARRAY(type,name,slot) Texture2DArray<type> name : register(slot);
#define SAMPLER(name,slot) sampler name : register(slot);
#define SAMPLER_COMPARISON_STATE(name,slot) SamplerComparisonState name : register(slot);

#define SAMPLE_LEVEL(name,_sampler,coord,level) name.SampleLevel(_sampler, coord, level)
#define SAMPLE(name,_sampler,coord) name.Sample(_sampler, coord)
#define LOAD(name,coord,index,offset) name.Load(coord, index, offset)
#define SAMPLE_CMP_LEVEL_ZERO(name,_sampler,coord,value) name.SampleCmpLevelZero(_sampler, coord, value)
#define SAMPLE_CMP_LEVEL_ZERO_ARRAY(name,_sampler,coord,value,slice) name.SampleCmpLevelZero(_sampler, float3( coord.x, coord.y, slice), value)

#define LERP(x,y,z) lerp(x,y,z)
#define MUL(a,b) mul(a,b)
#define SATURATE(a) saturate(a)

#define BRANCH [branch]
#define UNROLL [unroll]
#define STATIC static

#endif

#endif // _CROSS_PLATFORM_DEFINES_H

//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
