// mounter.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "MountGUI.h"
#include "MountMan.h"
#include "herlpers.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	CRunOnce once(_T("Mounter.Runonce.Mutex"));
	if (once.IsRunning())
		once.Show(NULL);
	else
	{
		CMountMan manager;
		CMountGUI gui(&manager);
		gui.Run();
	}
	return 0;
}

