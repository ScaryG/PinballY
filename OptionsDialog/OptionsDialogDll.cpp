// This file is part of PinballY
// Copyright 2018 Michael J Roberts | GPL v3 or later | NO WARRANTY
//
// Main entrypoint for options DLL.
//

#include "stdafx.h"
#include "OptionsDialogDll.h"
#include "OptionsDialogExports.h"
#include "OptionsDialog.h"
#include "../Utilities/InstanceHandle.h"
#include "../Utilities/InputManagerWithConfig.h"
#include "../Utilities/Config.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(COptionsDialogApp, CWinApp)
END_MESSAGE_MAP()

extern "C" DWORD WINAPI GetOptionsDialogVersion()
{
	return PINBALLY_OPTIONS_DIALOG_IFC_VSN;
}

extern "C" void WINAPI ShowOptionsDialog(
	ConfigSaveCallback configSaveCallback,
	InitializeDialogPositionCallback initPosCallback,
	bool isAdminHostRunning,
	SetUpAdminAutoRunCallback setUpAdminAutoRunCallback,
	RECT *finalDialogRect)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// initialize subsystems we use
	ConfigManager::Init();
	InputManagerWithConfig::Init();

	// load the configuration
	ConfigManager::GetInstance()->Load(MainConfigFileDesc);

	// set up for save notifications
	class Receiver : ConfigManager::Subscriber
	{
	public:
		Receiver(ConfigSaveCallback configSaveCallback) :
			configSaveCallback(configSaveCallback)
		{
			// subscribe for config notifications
			ConfigManager::GetInstance()->Subscribe(this);
		}
		
		ConfigSaveCallback configSaveCallback;
		virtual void OnConfigPostSave(bool succeeded) override 
		{ 
			configSaveCallback(succeeded);
		}

	} receiver(configSaveCallback);

	// show the dialog
	MainOptionsDialog dlg(initPosCallback, isAdminHostRunning, setUpAdminAutoRunCallback, finalDialogRect);
	dlg.DoModal();

	// save any pending changes to the in-memory configuration
	ConfigManager::GetInstance()->SaveIfDirty();

	// shut down the subsystems we use
	ConfigManager::Shutdown();
	InputManager::Shutdown();
}


extern "C" void WINAPI Cleanup()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

// application object singleton
COptionsDialogApp theApp;

COptionsDialogApp::COptionsDialogApp()
{
}

BOOL COptionsDialogApp::InitInstance()
{
	// do superclass initialization first
	BOOL result = __super::InitInstance();

	// save the instance handle in our global
	G_hInstance = m_hInstance;

	// return the base class result
	return result;
}

