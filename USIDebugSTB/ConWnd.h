//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#pragma once
#include "pch.h"
namespace UsiDebugStb {
	class ConWnd
	{
		BOOL fHide;
		WNDPROC pPrevPrc;
		WNDPROC pWndPrc;
		LRESULT OnWmSysCommand(HWND hWnd, UINT cmd, int x, int y);
		LRESULT OnWmMoving(HWND hWnd, LPRECT pRect);
		LRESULT OnWmWindowPosChanging(HWND hWnd, LPWINDOWPOS lpwpos);

	public:
		ConWnd();
		~ConWnd();
		void Attach(HWND hWnd);
		void Detach();
		HWND hWnd;
	};
}
