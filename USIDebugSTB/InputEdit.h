//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "UserDefinition.h"
#include "resource.h"
#pragma once
namespace UsiDebugStb {
	using namespace DUSI_COMMON;
	class InputEdit
	{
		HWND hWnd;
		int Index;
		/// <summary>
		/// ヒストリー個数の設定。
		/// </summary>
		const int numHistory = 20;
		vector<tstring> vsHistory;
		const WNDPROC  pfnEditSubPrc;
		WNDPROC pfnEditOriginPrc;
		LRESULT OnWmChar(HWND hwnd, TCHAR ch, int cRepeat);
		LRESULT OnWmKeydown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
		LRESULT OnWmDestroy(HWND hwnd);
	public:
		InputEdit();
		~InputEdit();
		void SubclassWindowX(HWND hwnd);
		void UnSubclasssWindowX();
	};
}
