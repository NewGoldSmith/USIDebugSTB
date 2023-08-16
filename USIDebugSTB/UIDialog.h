/**
 * @file UIDialog.h
 * @brief UI�_�C�A���O�N���X�̐錾
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#include "pch.h"
#include "UserDefinition.h"
#include "resource.h"
using namespace std;
using namespace DUSI_COMMON;
namespace UsiDebugStb {
	class UIDialog
	{
		HWND hParent;
		tstring strKey;
		const vector<std::pair<const HWND, const RECT>> vhrcDlgItemsBasePos;
		BOOL OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
		INT_PTR OnWmCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		INT_PTR OnWmSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
		INT_PTR OnWmUserSetFont(HWND hwndCtl, HFONT hfont);
		//INT_PTR OnWmGetFont(HWND hwnd);
		INT_PTR OnWmDestroy(HWND hwnd);
		INT_PTR OnWmClose(HWND hwnd);
		INT_PTR OnWmNotify(HWND hDlg, WPARAM wParam, LPNMHDR lParam);

	public:
		RECT rcWindow{};
		HWND hDlg;
		UIDialog();
			~UIDialog();
		HWND CreateDialogX(
			_In_opt_ HINSTANCE hInstance
			, _In_opt_ HWND hWndParent
			,int resourceIDDlg
		,const tstring keyname);
		/// <summary>
		/// �_�C�A���O�E�_�C�A���O�̈ʒu�E�T�C�Y���f�U�C�����̏�Ԃɖ߂��B
		/// </summary>
		/// <returns>TRUE�����AFALSE���s</returns>
		BOOL SetInitialPlacement();

	};
}

