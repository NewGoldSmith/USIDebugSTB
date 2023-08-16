//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "UserDefinition.h"
#include "resource.h"
#include "InputEdit.h"

#pragma once
using namespace std;
using namespace DUSI_COMMON;
namespace UsiDebugStb {
	class InputDialog
	{
		int fEnd;
		HWND hParentWnd;
		InputEdit SubClassEdit;
		const DLGPROC pDlgPrc;

		INT_PTR OnBnOK(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnCancel(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnSend(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnLBNone(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnLBCR(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnLBLF(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnLBCRLF(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkUnEscape(HWND hwnd, HWND hctrl, UINT code);

		BOOL OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
		INT_PTR OnWmClose(HWND hwnd);
		//INT_PTR OnWmWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos);
		INT_PTR OnWmCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		INT_PTR OnWmSysCommand(HWND hwnd, UINT cmd, int x, int y);
		INT_PTR OnWmKeyDownEnter(HWND hwnd);
		INT_PTR OnWmChar(HWND hwnd, TCHAR ch, int cRepeat);
		INT_PTR OnWmDoOffsetMove(HWND hwnd, LONG dx, LONG dy);
		INT_PTR OnWmExitSizeMove(HWND hwnd);

	public:
		HWND hDlg;
		tstring strCmd;
		InputDialog();
		InputDialog(InputDialog&) = delete;
		~InputDialog();
		void Close();
		void CreateDialogX
		(
			_In_opt_ HINSTANCE hInstance
			,_In_opt_ HWND hWndParent
		);
		void EnableSendButton(BOOL b);
		tstring GetLB();
	};
}