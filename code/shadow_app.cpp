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

#pragma warning (disable: 4995)
#define NOMINMAX

#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "Strsafe.h"
#include <algorithm>
#include <limits>
#include "Scene.h"
#include "Powerplant_Scene.h"
#include "Crypt_Scene.h"
#include "Warrior_Scene.h"
#include "Lunar_Scene.h"


//--------------------------------------------------------------------------------------
// Scenes
//--------------------------------------------------------------------------------------
Powerplant_Scene	g_PowerplantScene;
Crypt_Scene			g_CryptScene;
Warrior_Scene		g_WarriorScene;
Lunar_Scene			g_LunarScene;

int MAX_SCENE = 0;

Scene* g_pScenes[4]; 
Scene* g_pCurrentScene = NULL;


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager  g_DialogResourceManager;    // manager for shared resources of dialogs

CD3DSettingsDlg             g_D3DSettingsDlg;           // Device settings dialog
CDXUTDialog                 g_HUD;                      // dialog for standard controls
CDXUTTextHelper*            g_pTxtHelper = NULL;

ID3D11Texture2D*			g_pDepthStencilTexture = NULL;
ID3D11ShaderResourceView*	g_pDepthStencilSRV = NULL;
ID3D11DepthStencilView*		g_pDepthStencilView = NULL;
ID3D11DepthStencilView*		g_pDepthStencilViewRO = NULL;

ID3D11Texture2D*			g_pEyeViewPositionTexture = NULL;
ID3D11ShaderResourceView*	g_pEyeViewPositionSRV = NULL;
ID3D11RenderTargetView*		g_pEyeViewPositionRTV = NULL;

bool g_ShowGUI = true;
bool g_ShowText = true;
bool g_ShowHelp = false;
bool g_bPause = true;
double g_fTime = 0.0f;
double g_fPauseTime = 0.0f;


//--------------------------------------------------------------------------------------
// Cmd line params structure
//--------------------------------------------------------------------------------------
struct CmdLineParams
{
	unsigned int uWidth;
	unsigned int uHeight;
	unsigned int uSamples;
	WCHAR AppTitle[256];
	
	CmdLineParams()
	{
		uWidth = ::GetSystemMetrics( SM_CXSCREEN );
		uHeight = ::GetSystemMetrics( SM_CYSCREEN );
		uSamples = 1;
		wcscpy_s( AppTitle, L"GFSDK_ShadowApp_DX11" );
	}
};
CmdLineParams g_CmdLineParams;


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                         void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                      DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, double fTime,
                                 float fElapsedTime, void* pUserContext );
void InitUI();

HRESULT CreateSurface( ID3D11Texture2D** ppTexture, ID3D11ShaderResourceView** ppTextureSRV,
					   ID3D11RenderTargetView** ppTextureRTV, ID3D11UnorderedAccessView** ppTextureUAV, 
					   DXGI_FORMAT Format, unsigned int uWidth, unsigned int uHeight, unsigned int uSampleCount );

HRESULT CreateDepthStencilSurface( ID3D11Texture2D** ppDepthStencilTexture, ID3D11ShaderResourceView** ppDepthStencilSRV,
                                   ID3D11DepthStencilView** ppDepthStencilView, ID3D11DepthStencilView** ppDepthStencilViewRO, 
								   DXGI_FORMAT DSFormat, DXGI_FORMAT SRVFormat, unsigned int uWidth, unsigned int uHeight, unsigned int uSampleCount );
void UpdateSettingsFromGamePad( DXUT_GAMEPAD* pGamePadState );
void UpdateGUIFromSceneParams();
void ForceLightType();
unsigned int Log2( unsigned int Value );
void CaptureFrame();
bool IsNextArg( WCHAR*& strCmdLine, WCHAR* strArg );
bool GetCmdParam( WCHAR*& strCmdLine, WCHAR* strFlag );
void ParseCommandLine( CmdLineParams* pCmdLineParams );


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum 
{
    IDC_TOGGLEFULLSCREEN = 1,
    IDC_TOGGLEREF,
    IDC_CHANGEDEVICE,
    
	IDC_STATIC_SCENE,
	IDC_COMBOBOX_SCENE,

	IDC_CHECKBOX_TEXTURED,
			
	IDC_STATIC_SHADOW_TECHNIQUE_TYPE,
    IDC_COMBOBOX_SHADOW_TECHNIQUE_TYPE,
		
	IDC_CHECKBOX_FT_DYNAMIC_REPROJECTION,
	IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_STATIC,
    IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_SLIDER,
		
	IDC_STATIC_LIGHT_TYPE,
    IDC_COMBOBOX_LIGHT_TYPE,
	IDC_LIGHT_SIZE_STATIC,
    IDC_LIGHT_SIZE_SLIDER,

	IDC_PENUMBRA_MIN_SIZE_PERCENT_STATIC,
    IDC_PENUMBRA_MIN_SIZE_PERCENT_SLIDER,
	IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_STATIC,
	IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_SLIDER,
	IDC_PENUMBRA_MAX_THRESHOLD_STATIC,
	IDC_PENUMBRA_MAX_THRESHOLD_SLIDER,
			    
	IDC_CHECKBOX_VIEW_SHADOW_BUFFER,
	IDC_CHECKBOX_VIEW_SHADOW_MAPS,
	IDC_CHECKBOX_VIEW_CASCADES,
	IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES,
	IDC_CHECKBOX_VIEW_FRUSTUMS,
};


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if(bKeyDown)
    {
        switch(nChar)
        {
        case VK_F1:
            g_ShowHelp = !g_ShowHelp;
            break;
        case 'G':
            g_ShowGUI = !g_ShowGUI;
            break;
		case 'T':
            g_ShowText = !g_ShowText;
            break;
		case 'F':
			g_pCurrentScene->m_bShowWireFrame = !g_pCurrentScene->m_bShowWireFrame;
            break;
		case 'C':
			CaptureFrame();
			break;
		case 'P':
			g_bPause = !g_bPause; 
			if( g_bPause )
			{
				g_fPauseTime = g_fTime;
			}
			break;
        case VK_ESCAPE:
           PostQuitMessage( 0 );
           break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Disable gamma correction on this sample
    DXUTSetIsInGammaCorrectMode( false );

    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // IMPORTANT: set SDK media search path to include source directory of this sample, when started from .\Bin
    HRESULT hr;
    V_RETURN(DXUTSetMediaSearchPath(L"..\\samples\\DX11\\"));
	
	ParseCommandLine( &g_CmdLineParams );
	
	MAX_SCENE = 4;
	g_pScenes[0] = &g_LunarScene;
	g_pScenes[1] = &g_WarriorScene;
	g_pScenes[2] = &g_PowerplantScene; 
	g_pScenes[3] = &g_CryptScene;
	g_pCurrentScene = &g_LunarScene;
		
	InitUI();

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( g_CmdLineParams.AppTitle );
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, g_CmdLineParams.uWidth, g_CmdLineParams.uHeight );
	DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the GUI
//--------------------------------------------------------------------------------------
void InitUI()
{
	#define ITEM_HEIGHT	(20)	
	#define ITEM_DELTA	(ITEM_HEIGHT + 2)
	#define GROUP_DELTA	(ITEM_DELTA * 2)
	#define ITEM_WIDTH	(170)
	#define ITEM_LEFT	(15)
    
	WCHAR str[100];
    int iY = 0;
	CDXUTComboBox* pCombo;
	
	g_D3DSettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
	g_HUD.SetBackgroundColors( 0x88000000 );
	g_HUD.SetCallback( OnGUIEvent );
	g_HUD.EnableNonUserEvents( false );
		
	// Main HUD
	iY = 20;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", ITEM_LEFT, iY, ITEM_WIDTH, ITEM_HEIGHT );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, VK_F3 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, VK_F2 );
	iY += GROUP_DELTA;
    

	// Scene
	g_HUD.AddStatic( IDC_STATIC_SCENE, L"Scene:", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
	g_HUD.AddComboBox( IDC_COMBOBOX_SCENE, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, true, &pCombo );
    if( pCombo != NULL )
    {
        pCombo->SetDropHeight( 70 );
		for( int i = 0; i < MAX_SCENE; i++ )
		{
			pCombo->AddItem( g_pScenes[i]->m_Name, NULL );
		}
		pCombo->SetSelectedByIndex( 0 );
    }
	g_HUD.AddCheckBox( IDC_CHECKBOX_TEXTURED, L"Textured", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, g_pCurrentScene->m_bTextured, 'V' );
	iY += GROUP_DELTA;

	// Shadows
	g_HUD.AddStatic( IDC_STATIC_SHADOW_TECHNIQUE_TYPE, L"Shadow Technique Type:", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
    g_HUD.AddComboBox( IDC_COMBOBOX_SHADOW_TECHNIQUE_TYPE, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, true, &pCombo );
    if( pCombo != NULL )
    {
		pCombo->SetDropHeight( 120 );
        pCombo->AddItem( L"Hard", NULL );
		pCombo->AddItem( L"PCF", NULL );	
		pCombo->AddItem( L"PCSS+", NULL );	
		pCombo->AddItem( L"RT", NULL );	
		pCombo->AddItem( L"HRTS", NULL );	
		pCombo->AddItem( L"FT", NULL );
		pCombo->AddItem( L"HFTS", NULL );
		pCombo->SetSelectedByIndex( g_pCurrentScene->m_Shadow.m_SMRenderParams.eTechniqueType );
    }
	g_HUD.AddCheckBox( IDC_CHECKBOX_FT_DYNAMIC_REPROJECTION, L"FT Dynamic Reprojection", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, g_pCurrentScene->m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.bUseDynamicReprojection, 'V' );
	StringCchPrintf( str, 100, L"FT List Length Threshold : %d", g_pCurrentScene->m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.uListLengthTolerance );
    g_HUD.AddStatic( IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_STATIC, str, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
	g_HUD.AddSlider( IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_SLIDER, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, 100, (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.uListLengthTolerance / 2048.0f * 100 ) );

	iY += GROUP_DELTA;	
	// Lights
	g_HUD.AddStatic( IDC_STATIC_LIGHT_TYPE, L"Light Type:", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
    g_HUD.AddComboBox( IDC_COMBOBOX_LIGHT_TYPE, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, true, &pCombo );
    if( pCombo != NULL )
    {
        pCombo->SetDropHeight( 30 );
    }
	StringCchPrintf( str, 100, L"Light Size: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.fLightSize );
    g_HUD.AddStatic( IDC_LIGHT_SIZE_STATIC, str, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
	g_HUD.AddSlider( IDC_LIGHT_SIZE_SLIDER, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, 100, (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.fLightSize / g_pCurrentScene->m_Shadow.m_LightSizeMax * 100.0f ) );
	ForceLightType();
	

	// Penumbra
	StringCchPrintf( str, 100, L"Penumbra Min Size %%: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0] );
    g_HUD.AddStatic( IDC_PENUMBRA_MIN_SIZE_PERCENT_STATIC, str, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
	g_HUD.AddSlider( IDC_PENUMBRA_MIN_SIZE_PERCENT_SLIDER, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, 100, (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0] * 20.0f ) );
	StringCchPrintf( str, 100, L"Penumbra Min Threshold %%: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent );
    g_HUD.AddStatic( IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_STATIC, str, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
	g_HUD.AddSlider( IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_SLIDER, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 1, 100, (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent ) );
	StringCchPrintf( str, 100, L"Penumbra Max Threshold: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold );
    g_HUD.AddStatic( IDC_PENUMBRA_MAX_THRESHOLD_STATIC, str, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT );
	g_HUD.AddSlider( IDC_PENUMBRA_MAX_THRESHOLD_SLIDER, ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, 0, 100, (int)( ( ( g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold - g_pCurrentScene->m_Shadow.m_PenumbraMaxThresholdMin ) / ( g_pCurrentScene->m_Shadow.m_PenumbraMaxThresholdRange ) ) * 100.0f ) );
	iY += GROUP_DELTA;

	// Dev
	g_HUD.AddCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER, L"View Shadow Buffer", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, false, 'V' );
	g_HUD.AddCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS, L"View Shadow Maps", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, false, 'V' );
	g_HUD.AddCheckBox( IDC_CHECKBOX_VIEW_CASCADES, L"View Cascades", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, false, 'V' );
	g_HUD.AddCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES, L"View FT List Nodes", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, false, 'V' );
	g_HUD.AddCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS, L"View Light Frustum", ITEM_LEFT, iY += ITEM_DELTA, ITEM_WIDTH, ITEM_HEIGHT, false, 'V' );
	iY += GROUP_DELTA;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( pDeviceSettings->ver == DXUT_D3D11_DEVICE );

	// Disable vsync
	pDeviceSettings->d3d11.SyncInterval = 0;
	
    // For the first device created if it is a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if (s_bFirstTime)
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D9_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF ) ||
           ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
            pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }
        // Also, prefer 4xAA, first time
        if (DXUT_D3D11_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d11.DriverType != D3D_DRIVER_TYPE_REFERENCE )
        {
			pDeviceSettings->d3d11.sd.SampleDesc.Count = g_CmdLineParams.uSamples;
            pDeviceSettings->d3d11.sd.SampleDesc.Quality = 0;
        }
    }

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_pCurrentScene->m_ExtendedSceneCamera.FrameMove(fElapsedTime);
	g_pCurrentScene->m_SceneCamera.FrameMove(fElapsedTime);
	g_pCurrentScene->SetMainCamera();
}


//--------------------------------------------------------------------------------------
// Render text for the UI using DXUT
//--------------------------------------------------------------------------------------
void RenderText_DXUT()
{
	UINT nTextHeight = 15;
	UINT nTextTab = 15;
	float fontBrightScale = 1.0f;

    g_pTxtHelper->Begin();
	g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 0.0f / 255.0f * fontBrightScale, 255.0f / 255.0f * fontBrightScale, 0.0f / 255.0f * fontBrightScale, 1.0f ) );
	
	g_pTxtHelper->SetInsertionPos( nTextTab, nTextHeight );
	g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
	g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
	g_pCurrentScene->RenderTimeStampInfo( g_pTxtHelper );
	
	if( g_ShowHelp )
    {
		UINT nBackBufferHeight = DXUTGetDXGIBackBufferSurfaceDesc()->Height;
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 9 );
		g_pTxtHelper->DrawTextLine( L"--Controls--" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 8 );
		g_pTxtHelper->DrawTextLine( L"Light Control: Right Mouse & Mousewheel" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 7 );
		g_pTxtHelper->DrawTextLine( L"Scene Camera: Left Mouse & Keyboard ( W A S D )" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 6 );
		g_pTxtHelper->DrawTextLine( L"Toggle Wireframe: F" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 5 );
		g_pTxtHelper->DrawTextLine( L"Toggle GUI: G" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 4 );
		g_pTxtHelper->DrawTextLine( L"Toggle Text: T" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 3 );
		g_pTxtHelper->DrawTextLine( L"Toggle Help: F1" );
		g_pTxtHelper->SetInsertionPos( nTextTab, nBackBufferHeight - nTextHeight * 2 );
		g_pTxtHelper->DrawTextLine( L"Quit: Esc" );
    }
    else
    {
		g_pTxtHelper->DrawTextLine( L"Press F1 for help" );
	}
		
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                         void* pUserContext )
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
	
    g_pCurrentScene->m_ExtendedSceneCamera.HandleMessages( hWnd, uMsg, wParam, lParam );
	g_pCurrentScene->m_SceneCamera.HandleMessages( hWnd, uMsg, wParam, lParam );
	g_pCurrentScene->m_LightControl.MsgProc( hWnd, uMsg, wParam, lParam );
	
    return 0;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void UpdateGUIFromSceneParams()
{
	WCHAR str[100];

	g_HUD.GetCheckBox( IDC_CHECKBOX_TEXTURED )->SetChecked( g_pCurrentScene->m_bTextured );

	g_HUD.GetComboBox( IDC_COMBOBOX_SHADOW_TECHNIQUE_TYPE )->SetSelectedByIndex( g_pCurrentScene->m_Shadow.m_SMRenderParams.eTechniqueType );

	g_HUD.GetComboBox( IDC_COMBOBOX_LIGHT_TYPE )->SetSelectedByIndex( g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.eLightType );

	g_HUD.GetSlider( IDC_LIGHT_SIZE_SLIDER )->SetValue( (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.fLightSize / g_pCurrentScene->m_Shadow.m_LightSizeMax * 100.0f ) );
	StringCchPrintf( str, 100, L"Light Size: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.fLightSize );
	g_HUD.GetStatic( IDC_LIGHT_SIZE_STATIC )->SetText( str );
	
	g_HUD.GetSlider( IDC_PENUMBRA_MIN_SIZE_PERCENT_SLIDER )->SetValue( (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0] * 20.0f ) );
	StringCchPrintf( str, 100, L"Penumbra Min Size %%: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0] );
	g_HUD.GetStatic( IDC_PENUMBRA_MIN_SIZE_PERCENT_STATIC )->SetText( str );
	
	g_HUD.GetSlider( IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_SLIDER )->SetValue( (int)( g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent ) );
	StringCchPrintf( str, 100, L"Penumbra Min Threshold %%: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent );
	g_HUD.GetStatic( IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_STATIC )->SetText( str );

	g_HUD.GetSlider( IDC_PENUMBRA_MAX_THRESHOLD_SLIDER )->SetValue( (int)( ( ( g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold - g_pCurrentScene->m_Shadow.m_PenumbraMaxThresholdMin ) / ( g_pCurrentScene->m_Shadow.m_PenumbraMaxThresholdRange ) ) * 100.0f ) );
	StringCchPrintf( str, 100, L"Penumbra Max Threshold: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold );
	g_HUD.GetStatic( IDC_PENUMBRA_MAX_THRESHOLD_STATIC )->SetText( str );

	// Dev
	g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType == GFSDK_ShadowLib_DebugViewType_Cascades );
	g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType == GFSDK_ShadowLib_DebugViewType_FrustumTraceNodeList );
	g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetEnabled( g_pCurrentScene->m_Shadow.m_SMDesc.FrustumTraceMapDesc.bRequireFrustumTraceMap );

	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->GetChecked() )
	{
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
	}
	
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->GetChecked() )
	{
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
	}
	
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->GetChecked() )
	{
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
	}
	
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->GetChecked() )
	{
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
	}
	
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->GetChecked() )
	{
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    WCHAR str[100];

    switch( nControlID )
    {
		// D3D settings
		case IDC_TOGGLEFULLSCREEN:
			DXUTToggleFullScreen();
			break;
		case IDC_TOGGLEREF:
			DXUTToggleREF();
			break;
		case IDC_CHANGEDEVICE:
			g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() );
			break;

		// Scene settings
		case IDC_COMBOBOX_SCENE:
			g_pCurrentScene = g_pScenes[g_HUD.GetComboBox( IDC_COMBOBOX_SCENE )->GetSelectedIndex()];
			ForceLightType();
			UpdateGUIFromSceneParams();
			g_pCurrentScene->m_Shadow.ChangeShadowSettings( DXUTGetD3D11Device(), DXUTGetDXGIBackBufferSurfaceDesc()->Width, DXUTGetDXGIBackBufferSurfaceDesc()->Height, DXUTGetDXGIBackBufferSurfaceDesc()->SampleDesc.Count, g_pDepthStencilViewRO );
			break;

		// Scene settings
		case IDC_CHECKBOX_TEXTURED:
			g_pCurrentScene->m_bTextured = g_HUD.GetCheckBox( IDC_CHECKBOX_TEXTURED )->GetChecked();
			break;

		case IDC_CHECKBOX_FT_DYNAMIC_REPROJECTION:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.bUseDynamicReprojection = g_HUD.GetCheckBox( IDC_CHECKBOX_FT_DYNAMIC_REPROJECTION )->GetChecked();
			break;

		case IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_SLIDER:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.uListLengthTolerance = (unsigned int)(g_HUD.GetSlider( IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_SLIDER )->GetValue() / 100.0f * 2048);
			StringCchPrintf( str, 100, L"FT List Length Threshold : %d", g_pCurrentScene->m_Shadow.m_SMRenderParams.FrustumTraceMapRenderParams.uListLengthTolerance );
			g_HUD.GetStatic( IDC_FT_DYNAMIC_REPROJECTION_THRESHOLD_STATIC )->SetText( str );
			break;
					
		// Shadow map / buffer settings
		case IDC_COMBOBOX_SHADOW_TECHNIQUE_TYPE:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.eTechniqueType = (GFSDK_ShadowLib_TechniqueType)g_HUD.GetComboBox( IDC_COMBOBOX_SHADOW_TECHNIQUE_TYPE )->GetSelectedIndex();
			ForceLightType();
			break;
		
		// Light settings
		case IDC_COMBOBOX_LIGHT_TYPE:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.eLightType = (GFSDK_ShadowLib_LightType)g_HUD.GetComboBox( IDC_COMBOBOX_LIGHT_TYPE )->GetSelectedIndex();
			break;
		case IDC_LIGHT_SIZE_SLIDER:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.fLightSize = (float)g_HUD.GetSlider( IDC_LIGHT_SIZE_SLIDER )->GetValue() / 100.0f * g_pCurrentScene->m_Shadow.m_LightSizeMax;
			StringCchPrintf( str, 100, L"Light Size: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.fLightSize );
			g_HUD.GetStatic( IDC_LIGHT_SIZE_STATIC )->SetText( str );
			break;
		
		case IDC_PENUMBRA_MIN_SIZE_PERCENT_SLIDER:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0] = (float)g_HUD.GetSlider( IDC_PENUMBRA_MIN_SIZE_PERCENT_SLIDER )->GetValue() / 20.0f;
			g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[1] = g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0];
			g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[2] = g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0];
			g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[3] = g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0];
			StringCchPrintf( str, 100, L"Penumbra Min Size %%: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinSizePercent[0] );
			g_HUD.GetStatic( IDC_PENUMBRA_MIN_SIZE_PERCENT_STATIC )->SetText( str );
			break;
		case IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_SLIDER:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent = (float)g_HUD.GetSlider( IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_SLIDER )->GetValue();
			StringCchPrintf( str, 100, L"Penumbra Min Threshold %%: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMinWeightThresholdPercent );
			g_HUD.GetStatic( IDC_PENUMBRA_MIN_WEIGHT_THRESHOLD_PERCENT_STATIC )->SetText( str );
			break;

		case IDC_PENUMBRA_MAX_THRESHOLD_SLIDER:
			g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold = (float)g_HUD.GetSlider( IDC_PENUMBRA_MAX_THRESHOLD_SLIDER )->GetValue() / 100.0f * g_pCurrentScene->m_Shadow.m_PenumbraMaxThresholdRange + g_pCurrentScene->m_Shadow.m_PenumbraMaxThresholdMin;
			StringCchPrintf( str, 100, L"Penumbra Max Threshold: %.2f", g_pCurrentScene->m_Shadow.m_SMRenderParams.PCSSPenumbraParams.fMaxThreshold );
			g_HUD.GetStatic( IDC_PENUMBRA_MAX_THRESHOLD_STATIC )->SetText( str );
			break;
					

		// Dev
		case IDC_CHECKBOX_VIEW_CASCADES:
			g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType = ( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->GetChecked() ) ? ( GFSDK_ShadowLib_DebugViewType_Cascades ) : ( GFSDK_ShadowLib_DebugViewType_None );
			if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->GetChecked() )
			{
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
			}
			break;
		case IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES:
			g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType = ( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->GetChecked() ) ? ( GFSDK_ShadowLib_DebugViewType_FrustumTraceNodeList ) : ( GFSDK_ShadowLib_DebugViewType_None );
			if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->GetChecked() )
			{
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
			}
			break;
		case IDC_CHECKBOX_VIEW_SHADOW_BUFFER:
			g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType = GFSDK_ShadowLib_DebugViewType_None;
			if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->GetChecked() )
			{
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
			}
			break;
		case IDC_CHECKBOX_VIEW_SHADOW_MAPS:
			g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType = GFSDK_ShadowLib_DebugViewType_None;
			if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->GetChecked() )
			{
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
			}
			break;
		case IDC_CHECKBOX_VIEW_FRUSTUMS:
			g_pCurrentScene->m_Shadow.m_SBRenderParams.eDebugViewType = GFSDK_ShadowLib_DebugViewType_None;
			if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->GetChecked() )
			{
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->SetChecked( false );
				g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->SetChecked( false );
			}
			break;
    }
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                      DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 

// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnD3D11DestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;

    ID3D11DeviceContext* pd3dContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dContext, &g_DialogResourceManager, 15 );

	for( int i = 0; i < MAX_SCENE; i++ )
	{
		g_pScenes[i]->CreateResources( pd3dDevice, pd3dContext );
	}

	States::Claim();
	States::Get()->CreateResources( pd3dDevice );

	Shaders::Claim();
	Shaders::Get()->CreateResources( pd3dDevice );

	UpdateGUIFromSceneParams();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Called whenever the swap chain is resized.  
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	CreateDepthStencilSurface( &g_pDepthStencilTexture, &g_pDepthStencilSRV,
                               &g_pDepthStencilView, &g_pDepthStencilViewRO, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 
							   pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, pBackBufferSurfaceDesc->SampleDesc.Count );


	CreateSurface( &g_pEyeViewPositionTexture, &g_pEyeViewPositionSRV, &g_pEyeViewPositionRTV, NULL, 
					   DXGI_FORMAT_R32_FLOAT, pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, pBackBufferSurfaceDesc->SampleDesc.Count );
	
	for( int i = 0; i < MAX_SCENE; i ++ )
	{
		g_pScenes[i]->SetScreenResolution( pd3dDevice, ( D3DX_PI / 4.0f ), pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, pBackBufferSurfaceDesc->SampleDesc.Count, g_pDepthStencilViewRO );
	}
	
    UINT MainHudWidth  = 250;
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - MainHudWidth, 0 );
    g_HUD.SetSize( MainHudWidth, pBackBufferSurfaceDesc->Height );
	
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Callback function that renders the frame.  This function sets up the rendering 
// matrices and renders the scene and UI.
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext )
{
	g_fTime = fTime;
		
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if (g_D3DSettingsDlg.IsActive())
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }
		
	// Store off main render target and depth/stencil
	ID3D11RenderTargetView* pMainRTV = NULL;
	ID3D11DepthStencilView* pMainDSV = NULL;
	pd3dImmediateContext->OMGetRenderTargets(1, &pMainRTV, &pMainDSV);
	
	// Render the whole scene (including shadows)
	g_pCurrentScene->Render(	pd3dImmediateContext, 
								pMainRTV, 
								g_pDepthStencilView, 
								g_pDepthStencilViewRO, 
								g_pDepthStencilSRV,
								g_pEyeViewPositionRTV,
								g_pEyeViewPositionSRV,
								g_bPause ? g_fPauseTime : fTime );
	
	// Optionally, show the shadow maps
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_MAPS )->GetChecked() )
	{
		g_pCurrentScene->m_Shadow.DisplayShadowMaps(	pd3dImmediateContext, 
														pMainRTV, 
														DXUTGetDXGIBackBufferSurfaceDesc()->Width, 
														DXUTGetDXGIBackBufferSurfaceDesc()->Height );
	}

	// Display shadow map frustums
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUMS )->GetChecked() )
	{
		g_pCurrentScene->m_Shadow.DisplayMapFrustums( pMainRTV, g_pDepthStencilView );
	}
		
	// Optionally, show the shadow buffer
	if( g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_CASCADES )->GetChecked() ||
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_SHADOW_BUFFER )->GetChecked() ||
		g_HUD.GetCheckBox( IDC_CHECKBOX_VIEW_FRUSTUM_TRACE_LIST_NODES )->GetChecked() )
	{
		g_pCurrentScene->m_Shadow.DisplayShadowBuffer( pMainRTV );
	}
	
	// Restore initial render target and depth/stencil
	pd3dImmediateContext->OMSetRenderTargets(1, &pMainRTV, g_pDepthStencilView);
	SAFE_RELEASE(pMainRTV);
	SAFE_RELEASE(pMainDSV);
	
	// Render GUI and text
	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	
	if( g_ShowGUI )
	{
		g_HUD.OnRender(fElapsedTime);
	}
	if( g_ShowText )
	{
		RenderText_DXUT();
	}
		
	DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnD3D11CreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

	SAFE_RELEASE( g_pDepthStencilTexture );
	SAFE_RELEASE( g_pDepthStencilSRV );
	SAFE_RELEASE( g_pDepthStencilView );
	SAFE_RELEASE( g_pDepthStencilViewRO );

	SAFE_RELEASE( g_pEyeViewPositionTexture );
	SAFE_RELEASE( g_pEyeViewPositionSRV ); 
	SAFE_RELEASE( g_pEyeViewPositionRTV );

	for( int i = 0; i < MAX_SCENE; i ++ )
	{
		g_pScenes[i]->ReleaseResources();
	}

	States::Get()->ReleaseResources();
	States::Destroy();

	Shaders::Get()->ReleaseResources();
	Shaders::Destroy();
}


//--------------------------------------------------------------------------------------
// Release the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();

	SAFE_RELEASE( g_pDepthStencilTexture );
	SAFE_RELEASE( g_pDepthStencilSRV );
	SAFE_RELEASE( g_pDepthStencilView );
	SAFE_RELEASE( g_pDepthStencilViewRO );
	
	SAFE_RELEASE( g_pEyeViewPositionTexture );
	SAFE_RELEASE( g_pEyeViewPositionSRV );
	SAFE_RELEASE( g_pEyeViewPositionRTV );
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
HRESULT CreateSurface( ID3D11Texture2D** ppTexture, ID3D11ShaderResourceView** ppTextureSRV,
					   ID3D11RenderTargetView** ppTextureRTV, ID3D11UnorderedAccessView** ppTextureUAV, 
					   DXGI_FORMAT Format, unsigned int uWidth, unsigned int uHeight, unsigned int uSampleCount )
{
	HRESULT hr = D3D_OK;
		
	if( ppTexture )
	{
        D3D11_TEXTURE2D_DESC Desc;
        ZeroMemory( &Desc, sizeof( Desc ) );

		if( NULL == *ppTexture )
        {
		    Desc.Width = uWidth;
		    Desc.Height = uHeight;
		    Desc.MipLevels = 1;
		    Desc.ArraySize = 1;
		    Desc.Format = Format;
		    Desc.SampleDesc.Count = uSampleCount;
            Desc.SampleDesc.Quality = 0;
		    Desc.Usage = D3D11_USAGE_DEFAULT;
		    Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		    if( ppTextureUAV )
		    {
			    Desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		    }
		    hr = DXUTGetD3D11Device()->CreateTexture2D( &Desc, NULL, ppTexture );
		    assert( D3D_OK == hr );
        }

		if( ppTextureSRV )
		{
			SAFE_RELEASE( *ppTextureSRV );

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
			ZeroMemory( &SRVDesc, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
			SRVDesc.Format = Format;
            SRVDesc.ViewDimension = ( uSampleCount > 1 ) ? ( D3D11_SRV_DIMENSION_TEXTURE2DMS ) : ( D3D11_SRV_DIMENSION_TEXTURE2D );
			SRVDesc.Texture2D.MostDetailedMip = 0;
			SRVDesc.Texture2D.MipLevels = 1;
			hr = DXUTGetD3D11Device()->CreateShaderResourceView( *ppTexture, &SRVDesc, ppTextureSRV );
			assert( D3D_OK == hr );
		}

		if( ppTextureRTV )
		{
			SAFE_RELEASE( *ppTextureRTV );

			D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
			ZeroMemory( &RTVDesc, sizeof( D3D11_RENDER_TARGET_VIEW_DESC ) );
			RTVDesc.Format = Format;
            RTVDesc.ViewDimension = ( uSampleCount > 1 ) ? ( D3D11_RTV_DIMENSION_TEXTURE2DMS ) : ( D3D11_RTV_DIMENSION_TEXTURE2D );
            RTVDesc.Texture2D.MipSlice = 0;
			hr = DXUTGetD3D11Device()->CreateRenderTargetView( *ppTexture, &RTVDesc, ppTextureRTV );
			assert( D3D_OK == hr );
		}

		if( ppTextureUAV )
		{
			SAFE_RELEASE( *ppTextureUAV );

			D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
			ZeroMemory( &UAVDesc, sizeof( D3D11_UNORDERED_ACCESS_VIEW_DESC ) );
			UAVDesc.Format = Format;
			UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			UAVDesc.Buffer.FirstElement = 0;
			UAVDesc.Buffer.NumElements = Desc.Width * Desc.Height;
			hr = DXUTGetD3D11Device()->CreateUnorderedAccessView( *ppTexture, &UAVDesc, ppTextureUAV );
			assert( D3D_OK == hr );
		}
	}

	return hr;
}


//--------------------------------------------------------------------------------------
// Creates a depth stencil surface and optionally creates the following objects:
// ID3D11ShaderResourceView
// ID3D11DepthStencilView
//--------------------------------------------------------------------------------------
HRESULT CreateDepthStencilSurface( ID3D11Texture2D** ppDepthStencilTexture, ID3D11ShaderResourceView** ppDepthStencilSRV,
                                   ID3D11DepthStencilView** ppDepthStencilView, ID3D11DepthStencilView** ppDepthStencilViewRO, 
								   DXGI_FORMAT DSFormat, DXGI_FORMAT SRVFormat, unsigned int uWidth, unsigned int uHeight, unsigned int uSampleCount )
{
    HRESULT hr = D3D_OK;
    DXGI_FORMAT TextureFormat;
    
    if( ppDepthStencilTexture )
	{
        switch( DSFormat )
        {
            case DXGI_FORMAT_D32_FLOAT:
                TextureFormat = DXGI_FORMAT_R32_TYPELESS;
                break;

            case DXGI_FORMAT_D24_UNORM_S8_UINT:
                TextureFormat = DXGI_FORMAT_R24G8_TYPELESS;
                break;

            case DXGI_FORMAT_D16_UNORM:
                TextureFormat = DXGI_FORMAT_R16_TYPELESS;
                break;

            default:
                TextureFormat = DXGI_FORMAT_UNKNOWN;
                break;
        }
        assert( TextureFormat != DXGI_FORMAT_UNKNOWN );
        
        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth;
        descDepth.Width = uWidth;
        descDepth.Height = uHeight;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = TextureFormat;
        descDepth.SampleDesc.Count = uSampleCount;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        if( NULL != ppDepthStencilSRV )
        {
            if( ( descDepth.SampleDesc.Count == 1 ) || ( DXUTGetD3D11DeviceFeatureLevel() == D3D_FEATURE_LEVEL_10_1 ) || ( DXUTGetD3D11DeviceFeatureLevel() == D3D_FEATURE_LEVEL_11_0 ) )
            {
                descDepth.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
        }

        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = DXUTGetD3D11Device()->CreateTexture2D( &descDepth, NULL, ppDepthStencilTexture );
        assert( D3D_OK == hr );

        if( NULL != ppDepthStencilView )
        {
            // Create the depth stencil view
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			descDSV.Flags = 0;
            descDSV.Format = DSFormat;
            if( descDepth.SampleDesc.Count > 1 )
            {
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            }
            descDSV.Texture2D.MipSlice = 0;
            
            hr = DXUTGetD3D11Device()->CreateDepthStencilView( (ID3D11Resource*)*ppDepthStencilTexture, &descDSV, ppDepthStencilView );
            assert( D3D_OK == hr );
        }

		if( NULL != ppDepthStencilViewRO )
        {
            // Create the depth stencil view (read only version)
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			descDSV.Flags = D3D11_DSV_READ_ONLY_DEPTH;
            descDSV.Format = DSFormat;
            if( descDepth.SampleDesc.Count > 1 )
            {
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            }
            descDSV.Texture2D.MipSlice = 0;
            
            hr = DXUTGetD3D11Device()->CreateDepthStencilView( (ID3D11Resource*)*ppDepthStencilTexture, &descDSV, ppDepthStencilViewRO );
            assert( D3D_OK == hr );
        }
		
        if( NULL != ppDepthStencilSRV )
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC SRDesc;
   	        SRDesc.Format = SRVFormat;
            SRDesc.ViewDimension = ( uSampleCount > 1 ) ? ( D3D11_SRV_DIMENSION_TEXTURE2DMS ) : ( D3D11_SRV_DIMENSION_TEXTURE2D );
	        SRDesc.Texture2D.MostDetailedMip = 0;
	        SRDesc.Texture2D.MipLevels = 1;
	        
            hr = DXUTGetD3D11Device()->CreateShaderResourceView( (ID3D11Resource*)*ppDepthStencilTexture, &SRDesc, ppDepthStencilSRV );
	        assert( D3D_OK == hr );
        }
    }

    return hr;
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void ForceLightType()
{
#ifndef EVANG_GUI
	CDXUTComboBox* pCombo = g_HUD.GetComboBox( IDC_COMBOBOX_LIGHT_TYPE );
	pCombo->RemoveAllItems();
	if( g_pCurrentScene->m_Shadow.m_SMDesc.eViewType > GFSDK_ShadowLib_ViewType_Single )
	{

		pCombo->AddItem( L"Directional", NULL );
		pCombo->SetSelectedByIndex( 0 );

		g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.eLightType = GFSDK_ShadowLib_LightType_Directional;
	}
	else
	{

		pCombo->AddItem( L"Directional", NULL );
		pCombo->AddItem( L"Spot", NULL );

		pCombo->SetSelectedByIndex( g_pCurrentScene->m_Shadow.m_SMRenderParams.LightDesc.eLightType );
	}
#endif
}


//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
unsigned int Log2( unsigned int Value )
{
	unsigned int L = 0;
	
	while( Value > 1 )
	{
		Value /= 2;
		L++;
	}

	return L;
}


//--------------------------------------------------------------------------------------
// Captures a frame and dumps it to disk 
//--------------------------------------------------------------------------------------
void CaptureFrame()
{
    static unsigned int uCaptureNumber = 0;
	WCHAR szCaptureFileName[256];
	swprintf_s( szCaptureFileName, L"GFSDK_ShadowLib_Shot%d.bmp", uCaptureNumber );
	
	// Retrieve RT resource
	ID3D11Resource *pRTResource;
	DXUTGetD3D11RenderTargetView()->GetResource( &pRTResource );

	// Retrieve a Texture2D interface from resource & and get desc
	ID3D11Texture2D* RTTexture;
	pRTResource->QueryInterface( __uuidof( ID3D11Texture2D ), ( LPVOID* )&RTTexture );
	D3D11_TEXTURE2D_DESC TexDesc;
	RTTexture->GetDesc( &TexDesc );

	// We need a screen-sized STAGING resource for frame capturing
	ID3D11Texture2D* pCaptureTexture = NULL;
    D3D11_TEXTURE2D_DESC StagingTexDesc;
    DXGI_SAMPLE_DESC SingleSample = { 1, 0 };
    StagingTexDesc.Width = TexDesc.Width;
    StagingTexDesc.Height = TexDesc.Height;
    StagingTexDesc.Format = TexDesc.Format;
    StagingTexDesc.SampleDesc = SingleSample;
    StagingTexDesc.MipLevels = 1;
    StagingTexDesc.Usage = D3D11_USAGE_STAGING;
    StagingTexDesc.MiscFlags = 0;
    StagingTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    StagingTexDesc.BindFlags = 0;
    StagingTexDesc.ArraySize = 1;
    DXUTGetD3D11Device()->CreateTexture2D( &StagingTexDesc, NULL, &pCaptureTexture );
   
	// Check if RT is multisampled or not
	if( TexDesc.SampleDesc.Count > 1 )
	{
		// RT is multisampled, need resolving before dumping to disk
		// Create single-sample RT of the same type and dimensions
		TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		TexDesc.MipLevels = 1;
		TexDesc.Usage = D3D11_USAGE_DEFAULT;
		TexDesc.CPUAccessFlags = 0;
		TexDesc.BindFlags = 0;
		TexDesc.SampleDesc = SingleSample;

		ID3D11Texture2D *pSingleSampleTexture;
		DXUTGetD3D11Device()->CreateTexture2D( &TexDesc, NULL, &pSingleSampleTexture );

		DXUTGetD3D11DeviceContext()->ResolveSubresource( pSingleSampleTexture, 0, RTTexture, 0, TexDesc.Format );

		// Copy RT into STAGING texture
		DXUTGetD3D11DeviceContext()->CopyResource( pCaptureTexture, pSingleSampleTexture );

		D3DX11SaveTextureToFile( DXUTGetD3D11DeviceContext(), pCaptureTexture, D3DX11_IFF_BMP, szCaptureFileName );

		SAFE_RELEASE( pSingleSampleTexture );
	}
	else
	{
		// Single sample case

		// Copy RT into STAGING texture
		DXUTGetD3D11DeviceContext()->CopyResource( pCaptureTexture, pRTResource );

		D3DX11SaveTextureToFile( DXUTGetD3D11DeviceContext(), pCaptureTexture, D3DX11_IFF_BMP, szCaptureFileName );
	}

	SAFE_RELEASE( RTTexture );

	SAFE_RELEASE( pRTResource );

	SAFE_RELEASE( pCaptureTexture );

	uCaptureNumber++;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
bool IsNextArg( WCHAR*& strCmdLine, WCHAR* strArg )
{
   int nArgLen = (int) wcslen(strArg);
   int nCmdLen = (int) wcslen(strCmdLine);

   if( nCmdLen >= nArgLen && 
      _wcsnicmp( strCmdLine, strArg, nArgLen ) == 0 && 
      (strCmdLine[nArgLen] == 0 || strCmdLine[nArgLen] == L':' || strCmdLine[nArgLen] == L'=' ) )
   {
      strCmdLine += nArgLen;
      return true;
   }

   return false;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
bool GetCmdParam( WCHAR*& strCmdLine, WCHAR* strFlag )
{
   if( *strCmdLine == L':' || *strCmdLine == L'=' )
   {       
      strCmdLine++; // Skip ':'

      // Place NULL terminator in strFlag after current token
      wcscpy_s( strFlag, 256, strCmdLine );
      WCHAR* strSpace = strFlag;
      while (*strSpace && (*strSpace > L' '))
         strSpace++;
      *strSpace = 0;

      // Update strCmdLine
      strCmdLine += wcslen(strFlag);
      return true;
   }
   else
   {
      strFlag[0] = 0;
      return false;
   }
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
void ParseCommandLine( CmdLineParams* pCmdLineParams )
{
    assert( NULL != pCmdLineParams );
	
	// Perform application-dependant command line processing
    WCHAR* strCmdLine = GetCommandLine();
    WCHAR strFlag[MAX_PATH];
    int nNumArgs;
    WCHAR** pstrArgList = CommandLineToArgvW( strCmdLine, &nNumArgs );
    for( int iArg=1; iArg<nNumArgs; iArg++ )
    {
		strCmdLine = pstrArgList[iArg];

		// Handle flag args
		if( *strCmdLine == L'/' || *strCmdLine == L'-' )
		{
			strCmdLine++;

			if( IsNextArg( strCmdLine, L"width" ) )
			{
				if( GetCmdParam( strCmdLine, strFlag ) )
				{
				   pCmdLineParams->uWidth = _wtoi(strFlag);
				}
				continue;
			}

			if( IsNextArg( strCmdLine, L"height" ) )
			{
				if( GetCmdParam( strCmdLine, strFlag ) )
				{
				   pCmdLineParams->uHeight = _wtoi(strFlag);
				}
				continue;
			}

			if( IsNextArg( strCmdLine, L"apptitle" ) )
			{
				if( GetCmdParam( strCmdLine, strFlag ) )
				{
					wcscpy_s( pCmdLineParams->AppTitle, strFlag );
				}
				continue;
			}
		}
	}
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------

