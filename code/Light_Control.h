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

#include "States.h"
#include "Shaders.h"
#include "Constant_Buffers.h"

#define GAME_PAD_LIGHT_ROTATION_SPEED		( 20.0f )

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
class Light_Control
{
public:

	enum BUTTON_ID
	{
		LEFT_MOUSE,
		MIDDLE_MOUSE,
		RIGHT_MOUSE,
	};

	Light_Control() 
	{
		m_bActivateButtonDown = false;
		m_iLastMouseX = 0;
		m_iLastMouseY = 0;

		m_v3Position = D3DXVECTOR3( 1.0f, 1.0f, 1.0f );
		m_v3LookAt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		m_pCB = NULL;
		m_fSize = 1.0f;
		m_eActivateButton = RIGHT_MOUSE;
		m_fRotationScaler = 1.0f;
		m_fVerticalScaler = 1.0f;
		m_fZoomScaler = 1.0f;
	}

	~Light_Control() 
	{
		ReleaseResources();
	}

	HRESULT CreateResources( ID3D11Device *pd3dDevice )
	{
		HRESULT hr;
		
		V_RETURN( m_Mesh.Create( pd3dDevice, L"Media\\Light_Control\\Light_Control.sdkmesh" ) );

		m_v3Position = m_Mesh.GetMeshBBoxExtents( 0 );
		m_v3LookAt = m_Mesh.GetMeshBBoxCenter( 0 );

		D3D11_BUFFER_DESC desc = 
        {
             sizeof( CBData ),				//ByteWidth
             D3D11_USAGE_DEFAULT,			//Usage
             D3D11_BIND_CONSTANT_BUFFER,	//BindFlags
             0,								//CPUAccessFlags
             0,								//MiscFlags
             0,								//StructureByteStride (new in D3D11)
        };
        
		V_RETURN( pd3dDevice->CreateBuffer( &desc, NULL, &m_pCB ) );

		return hr;
	}

	void ReleaseResources()
	{
		m_Mesh.Destroy();
		SAFE_RELEASE( m_pCB );
	}

	void Render(	ID3D11DeviceContext* pd3dImmediateContext, 
					
					D3DXMATRIX* pSceneViewProj,
					D3DXVECTOR3* pEye,
					ID3D11RenderTargetView* pOutputRTV,
					ID3D11DepthStencilView* pOutputDSV )
	{
		// RT
		pd3dImmediateContext->OMSetRenderTargets( 1, &pOutputRTV, pOutputDSV );
		
		// States
		float BlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		pd3dImmediateContext->OMSetBlendState( States::Get()->pNoBlend_BS, BlendFactor, 0xffffffff );
		pd3dImmediateContext->OMSetDepthStencilState( States::Get()->pDepthNoStencil_DS, 0 );
		pd3dImmediateContext->RSSetState(States::Get()->pBackfaceCull_RS);

		// Update CB
		D3DXMatrixTranspose( &CBData.SceneViewProj, pSceneViewProj );
		CBData.Eye[0] = pEye->x; 
		CBData.Eye[1] = pEye->y; 
		CBData.Eye[2] = pEye->z; 
		CBData.WorldPos[0] = m_v3Position.x; 
		CBData.WorldPos[1] = m_v3Position.y; 
		CBData.WorldPos[2] = m_v3Position.z; 
		CBData.LookAt[0] = m_v3LookAt.x; 
		CBData.LookAt[1] = m_v3LookAt.y; 
		CBData.LookAt[2] = m_v3LookAt.z; 
		CBData.Size[0] = m_fSize; 
		CBData.Size[1] = m_fSize; 
		CBData.Size[2] = m_fSize; 
		pd3dImmediateContext->UpdateSubresource( m_pCB, 0, NULL, &CBData, 0, 0 );

		// Setup VS
		pd3dImmediateContext->IASetInputLayout( Shaders::Get()->pLightControlIA );
		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCB );
		pd3dImmediateContext->VSSetShader(Shaders::Get()->pLightControlVS, NULL, 0);

		// Setup PS
		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCB );
		pd3dImmediateContext->PSSetShader(Shaders::Get()->pLightControlPS, NULL, 0);
		
		// Render the light model
		m_Mesh.Render( pd3dImmediateContext );
	}

	void SetPosition( D3DXVECTOR3* pv3Position )
	{
		m_v3Position = *pv3Position;
	}

	D3DXVECTOR3* GetPosition()
	{
		return &m_v3Position;
	}

	void SetLookAt( D3DXVECTOR3* pv3LookAt )
	{
		m_v3LookAt = *pv3LookAt;
	}

	D3DXVECTOR3* GetLookAt()
	{
		return &m_v3LookAt;
	}

	void SetSize( float fSize )
	{
		m_fSize = fSize;
		m_fSize = ( m_fSize < ( m_fRange * 0.01f ) ) ? ( m_fRange * 0.01f ) : ( m_fSize );
	}

	float GetSize()
	{
		return m_fSize;
	}

	void SetActivateButtonID( BUTTON_ID eActivateButton )
	{
		m_eActivateButton = eActivateButton;
	}

	void SetAxisScale( float fX, float fY, float fZ )
	{
		m_fXAxis = fX;
		m_fYAxis = fY;
		m_fZAxis = fZ;
		m_fRange = ( m_fXAxis + m_fYAxis + m_fZAxis ) / 3.0f;

		m_fRotationScaler = 0.1f;
		m_fVerticalScaler = m_fYAxis * 0.01f;
		m_fZoomScaler = m_fZAxis * 0.02f;
	}
		
	void GamePadProc( DXUT_GAMEPAD* pGamePadState )
	{
		for( int i = 0; i < DXUT_MAX_CONTROLLERS; i++ )
		{
			if( pGamePadState[i].bConnected )
			{
				if( pGamePadState[i].wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER )
				{
					int iDeltaMouseX = (int)(pGamePadState[i].fThumbLX * GAME_PAD_LIGHT_ROTATION_SPEED);
					int iDeltaMouseY = (int)(pGamePadState[i].fThumbLY * GAME_PAD_LIGHT_ROTATION_SPEED);
				
					Rotate( iDeltaMouseX, iDeltaMouseY );
				}
			}
		}
	}

	void MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		int iMouseX;
		int iMouseY; 
				
		switch( uMsg )
		{
			case WM_LBUTTONDOWN:
				if( m_eActivateButton == LEFT_MOUSE )
					m_bActivateButtonDown = true;
				break;

			case WM_LBUTTONUP:
				if( m_eActivateButton == LEFT_MOUSE )
					m_bActivateButtonDown = false;
				break;

			case WM_MBUTTONDOWN:
				if( m_eActivateButton == MIDDLE_MOUSE )
					m_bActivateButtonDown = true;
				break;

			case WM_MBUTTONUP:
				if( m_eActivateButton == MIDDLE_MOUSE )
					m_bActivateButtonDown = false;
				break;

			case WM_RBUTTONDOWN:
				if( m_eActivateButton == RIGHT_MOUSE )
					m_bActivateButtonDown = true;
				break;

			case WM_RBUTTONUP:
				if( m_eActivateButton == RIGHT_MOUSE )
					m_bActivateButtonDown = false;
				break;

			case WM_MOUSEMOVE:
				iMouseX = (short)LOWORD(lParam);
				iMouseY = (short)HIWORD(lParam);
				if( m_bActivateButtonDown )
				{
					int iDeltaMouseX = iMouseX - m_iLastMouseX;
					int iDeltaMouseY = iMouseY - m_iLastMouseY;
										
					Rotate( iDeltaMouseX, iDeltaMouseY );
				}
				m_iLastMouseX = iMouseX;
				m_iLastMouseY = iMouseY;
				break;

			case WM_MOUSEWHEEL:
				if( m_bActivateButtonDown )
				{
					short iWheel = ( short )HIWORD( wParam );
					short iDeltaWheel = ( iWheel < 0 ) ? ( -1 ) : ( 1 );
					float fZoom = iDeltaWheel * m_fZoomScaler;
					D3DXMATRIX Trans;
					D3DXVECTOR3 TempDir = m_v3Position - m_v3LookAt;
					float Length = sqrtf( TempDir.x * TempDir.x + TempDir.y * TempDir.y + TempDir.z * TempDir.z );
					D3DXVec3Normalize( &TempDir, &TempDir );
					D3DXMatrixTranslation( &Trans, TempDir.x * fZoom, TempDir.y * fZoom, TempDir.z * fZoom );   		
					D3DXVECTOR3 TempPos;
					D3DXVec3TransformCoord( &TempPos, &m_v3Position, &Trans );
					if( iWheel < 0 )
					{
						m_v3Position = ( Length <= m_fRange * 2.0f ) ? ( m_v3Position ) : ( TempPos );
					}
					else
					{
						m_v3Position = TempPos;
					}
				}
				break;
		}
	}

private:

	void Rotate( int iDeltaMouseX, int iDeltaMouseY )
	{
		// Rotate around Y axis
		D3DXMATRIX RotY;
		float fAngle = D3DX_PI / ( 180.0f * ( 1.0f / m_fRotationScaler ) );
		D3DXMatrixRotationY( &RotY, fAngle * (float)iDeltaMouseX );
		D3DXVec3TransformCoord( &m_v3Position, &m_v3Position, &RotY );
					
		// Translate vertically
		D3DXMATRIX TransY;
		D3DXMatrixTranslation( &TransY, 0.0f, -iDeltaMouseY * m_fVerticalScaler , 0.0f );
		// But limit how close to the ground and how high in the sky it can get					
		D3DXVECTOR3 TempPos, TempDir;
		D3DXVec3TransformCoord( &TempPos, &m_v3Position, &TransY );
		TempDir = TempPos - m_v3LookAt;
		float O = TempPos.y - m_v3LookAt.y;
		float H = sqrtf( TempDir.x * TempDir.x + TempDir.y * TempDir.y + TempDir.z * TempDir.z );
		float Theta = sinh( O / H ); 
		TempPos = ( Theta < D3DX_PI / 10 ) ? ( m_v3Position ) : ( TempPos );
		m_v3Position = ( Theta > D3DX_PI / 3 ) ? ( m_v3Position ) : ( TempPos );
	}

	struct
    {
        D3DXMATRIX  SceneViewProj;
		float		Eye[4];
        float       WorldPos[4];
		float		LookAt[4];
		float		Size[4];		
	}CBData;

	bool			m_bActivateButtonDown;
	int				m_iLastMouseX;
	int				m_iLastMouseY;

	CDXUTSDKMesh	m_Mesh; 
	D3DXVECTOR3		m_v3Position;
	D3DXVECTOR3		m_v3LookAt;
	float			m_fSize;
	BUTTON_ID		m_eActivateButton;
	ID3D11Buffer*	m_pCB;
	float			m_fRotationScaler;
	float			m_fVerticalScaler;
	float			m_fZoomScaler;
	float			m_fXAxis;
	float			m_fYAxis;
	float			m_fZAxis;
	float			m_fRange;
};

//--------------------------------------------------------------------------------------
// EOF.
//--------------------------------------------------------------------------------------
