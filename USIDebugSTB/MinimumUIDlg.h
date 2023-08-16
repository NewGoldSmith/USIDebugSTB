/**
 * @file MinimumUIDlg.h
 * @brief メインダイアログクラスの宣言
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once

#include "pch.h"
#include "USIDebugSTB.h"
#include "resource.h"
using namespace std;
using namespace DUSI_COMMON;
namespace UsiDebugStb {
	class MinimumUIDlg
	{
		HWND hParent;
		const vector<std::pair<const HWND, const RECT>> vhrcDlgItemsBasePos;

		BOOL OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
		INT_PTR OnWmCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		INT_PTR OnWmSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
		INT_PTR OnWmUserSetFont(HWND hwndCtl, HFONT hfont);
		INT_PTR OnWmGetFont(HWND hwnd);
		INT_PTR OnWmDestroy(HWND hwnd);
		INT_PTR OnWmClose(HWND hwnd);

	public:
		HWND hDlg;
		MinimumUIDlg();
		~MinimumUIDlg();
		HWND CreateDialogX(_In_opt_ HINSTANCE hInstance, _In_opt_ HWND hWndParent);
		/// <summary>
		/// ダイアログ・ダイアログの位置・サイズをデザイン時の状態に戻す。
		/// </summary>
		/// <returns>TRUE成功、FALSE失敗</returns>
		BOOL SetInitialPlacement();
	};
}
