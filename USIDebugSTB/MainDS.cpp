// Copyright (c) 2022 2023, Gold Smith
// Released under the MIT license
// https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "MainDS.h"


using namespace UsiDebugStb;
using namespace std;
int APIENTRY wWinMain
(_In_ HINSTANCE hInstance
	, _In_opt_ HINSTANCE hPrevInstance
	, _In_ LPWSTR    lpCmdLine
	, _In_ int       nCmdShow
)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	int result(0);
	{
		INITCOMMONCONTROLSEX cc;
		cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
		cc.dwICC =
			ICC_LISTVIEW_CLASSES
			| ICC_NATIVEFNTCTL_CLASS
			| ICC_INTERNET_CLASSES
			| ICC_UPDOWN_CLASS
			| ICC_STANDARD_CLASSES
			;
		InitCommonControlsEx(&cc);

		{
			MainDialog MainDlg;
#ifdef USING_GROBAL_THISPOINTER
			UsiDebugStb::pgMainDialog = &MainDlg;
#endif // USING_GROBAL_THISPOINTER
			result = MainDlg.DialogBoxParamW(hInstance, NULL);
		}
	}
	_CrtDumpMemoryLeaks();
	return result;
}
