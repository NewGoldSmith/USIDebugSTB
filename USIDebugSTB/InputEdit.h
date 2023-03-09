//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "resource.h"
#pragma once
namespace UsiDebugStb {
#define WM_KEYDOWN_ENTER	(WM_APP + 5)

	class InputEdit
	{
		HWND hWnd;
		const WNDPROC  pfnEditSubPrc;
		LRESULT OnWmChar(HWND hwnd, TCHAR ch, int cRepeat);

	public:
		WNDPROC pfnEditOriginPrc;
		InputEdit();
		~InputEdit();
		void SubclassWindowX(HWND hwnd);
		void UnSubclasssWindowX();
	};
}
