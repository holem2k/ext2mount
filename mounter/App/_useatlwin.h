#include <atlbase.h>
extern CComModule _Module;
#include <atlwin.h>
#include <commctrl.h>

class CATLWinStartup
{
public:
	CATLWinStartup()
	{
		INITCOMMONCONTROLSEX data;
		data.dwSize = sizeof(data);
		data.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&data);

		_Module.Init(NULL, ::GetModuleHandle(NULL));
	}
	~CATLWinStartup()
	{
		_Module.Term();
	}
};