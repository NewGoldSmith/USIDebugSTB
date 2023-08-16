/**
 * @file MinimumUIDlg.cpp
 * @brief ���C���_�C�A���O�N���X�̐錾
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "pch.h"
#include "MinimumUIDlg.h"
using namespace std;
using namespace DUSI_COMMON;

UsiDebugStb::MinimumUIDlg::MinimumUIDlg()
	:hParent(NULL)
	, hDlg(NULL)
//	, hCurrentFont(NULL)
	, vhrcDlgItemsBasePos()
{
}

UsiDebugStb::MinimumUIDlg::~MinimumUIDlg()
{
}

HWND UsiDebugStb::MinimumUIDlg::
CreateDialogX(_In_opt_ HINSTANCE hInstance, _In_opt_ HWND hWndParent)
{
	hParent = hWndParent;
	hDlg = ::CreateDialogParamW(
		hInstance
		, MAKEINTRESOURCE(IDD_SIMPLEUI)
		, hWndParent
		, [](
			HWND hDlg
			, UINT uMsg
			, WPARAM wParam
			, LPARAM lParam)->INT_PTR
		{
			//�}�N���͉������̖��O�Ɉˑ����Ă��܂��B
#define GET_THIS_PTR \
		reinterpret_cast<MinimumUIDlg *>(GetWindowLongPtr(hDlg, DWLP_USER))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<MinimumUIDlg*>\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);return lParam;}())

#define HDL_DLG_MSG_CLS( msg, fn) \
    case(msg):return SetDlgMsgResult\
			(hDlg, msg, (HANDLE_##msg(hDlg, wParam, lParam,\
			 (msg==WM_INITDIALOG ? GET_THIS_PTR_AT_INIT : GET_THIS_PTR)->fn)))
#define HANDLE_WM_USER_SETFONT(hwnd,wParam,lParam, fn) \
	((fn)((hwnd),(HFONT)(wParam)))

			switch (uMsg)
			{
				HDL_DLG_MSG_CLS(WM_SETFONT, OnWmSetFont);
				HDL_DLG_MSG_CLS(WM_USER_SETFONT, OnWmUserSetFont);
				HDL_DLG_MSG_CLS(WM_GETFONT, OnWmGetFont);
				HDL_DLG_MSG_CLS(WM_CLOSE, OnWmClose);
				HDL_DLG_MSG_CLS(WM_COMMAND, OnWmCommand);
				HDL_DLG_MSG_CLS(WM_INITDIALOG, OnWmInitDialog);
				HDL_DLG_MSG_CLS(WM_DESTROY, OnWmDestroy);
			default:
				return FALSE;
			}

#undef HANDLE_WM_USER_SETFONT
#undef HDL_DLG_MSG_CLS
#undef GET_THIS_PTR_AT_INIT
#undef GET_THIS_PTR
		}
		, (LPARAM)this
			);
	return hDlg;
}

BOOL UsiDebugStb::MinimumUIDlg::SetInitialPlacement()
{
	// �f�U�C�����̃t�H���g�擾
	LOGFONT lfBase;
	if (!GetPrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, &lfBase
		, sizeof(lfBase)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	HFONT hfont;
	if (!(hfont = CreateFontIndirect(&lfBase)))
	{
		_D("font error.");
		return FALSE;
	}

	// �e�_�C�A���O�A�C�e����WM_SETFONT���M
	for (const auto& pair : vhrcDlgItemsBasePos)
	{
		::SendMessage(pair.first, WM_SETFONT, (WPARAM)hfont, 0L);
	}
	DeleteFont(hfont);

	//***********�x�[�X�T�C�Y�ɖ߂��B***************
		// �_�C�A���O���x�[�X�T�C�Y�ɖ߂��B
	{
		RECT rect{};
		if (!GetPrivateProfileStruct(
			_T("MINIMUM_UI_DLG")
			, _T("DLG_WINDOW_BASE_RECT")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		SetWindowPos(
			hDlg
			, 0
			, 0
			, 0
			, rect.right - rect.left
			, rect.bottom - rect.top
			, SWP_NOMOVE
			| SWP_NOZORDER
			| SWP_NOOWNERZORDER
		);
	}
	// �_�C�A���O�A�C�e�����x�[�X�|�W�V�����ɖ߂��B
	for (const auto& dlgItem : vhrcDlgItemsBasePos)
	{
		SetWindowPos(
			dlgItem.first
			, 0
			, dlgItem.second.left
			, dlgItem.second.top
			, dlgItem.second.right - dlgItem.second.left
			, dlgItem.second.bottom - dlgItem.second.top
			, SWP_NOZORDER);
	}
	return TRUE;
}

BOOL UsiDebugStb::MinimumUIDlg::OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	// �x�[�X�_�C�A���ORECT�ۑ��B
	{
		RECT rect{};
		GetWindowRect(hDlg, &rect);
		OffsetRect(&rect, -rect.left, -rect.top);
		if (!WritePrivateProfileStruct(
			_T("MINIMUM_UI_DLG")
			, _T("DLG_WINDOW_BASE_RECT")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
	}

	//�_�C�A���O���T�C�Y�ݒ�B
	{
		// �_�C�A���O�A�C�e���̃n���h���ƈʒu�ۑ�
		EnumChildWindows(hDlg, SaveChildWindowPosition, (LPARAM)&vhrcDlgItemsBasePos);
		RECT rect{};

		LOGFONT lgCur{};
		if (GetPrivateProfileStruct(
			_T("MINIMUM_UI_DLG")
			, _T("DLG_LOG_FONT_CURRENT")
			, &lgCur
			, sizeof(lgCur)
			, MakeIniPath().c_str())) {
			// ���[�U�[�ݒ�̃t�H���g�ɕύX
			HFONT hfont(NULL);
			if (!(hfont = CreateFontIndirect(&lgCur)))
			{
				ErrMes("Create Font Err");
				return FALSE;
			}
			SendMessage(hDlg, WM_USER_SETFONT, (WPARAM)hfont, (LPARAM)0);
		}
	}
	return 0;
}

INT_PTR UsiDebugStb::MinimumUIDlg::OnWmCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (!this)
		return 0;
	FORWARD_WM_COMMAND(hParent, id, hwndCtl, codeNotify, SNDMSG);
	return INT_PTR();
}

INT_PTR UsiDebugStb::MinimumUIDlg::OnWmSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
{
	// �e�_�C�A���O�A�C�e����WM_SETFONT���M
	auto lparam = make_tuple(hfont, fRedraw);
	EnumChildWindows(hwndCtl, SetFontForChildWindow, (LPARAM)&lparam);
	HFONT hfontnew(NULL);
	if (!GetPrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_INIT_FONT_HANDLE")
		, &hfontnew
		, sizeof(hfontnew)
		, MakeIniPath().c_str()))
	{
		if (!WritePrivateProfileStruct(
			_T("MINIMUM_UI_DLG")
			, _T("DLG_INIT_FONT_HANDLE")
			, &hfont
			, sizeof(HFONT)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		LOGFONT lf{};
		if (!GetObject(hfont, sizeof(lf), &lf))
		{
			ErrMes("Create logfont failed.");
			return TRUE;
		}
		if (!WritePrivateProfileStruct(
			_T("MINIMUM_UI_DLG")
			, _T("DLG_LOG_FONT_BASE")
			, &lf
			, sizeof(lf)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		return TRUE;
	}
	if (!WritePrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_CURRENT")
		, &hfont
		, sizeof(HFONT)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	if (!GetAncestor(hwndCtl, GA_PARENT))
	{
		DeleteFont(hfont);
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MinimumUIDlg::OnWmUserSetFont(HWND hwndCtl, HFONT hfont)
{
	// �f�U�C�����̏����z�u�ɖ߂��B
	SetInitialPlacement();

	// �e�_�C�A���O�A�C�e����WM_SETFONT���M
	auto lparam = make_tuple(hfont, TRUE);
	EnumChildWindows(hDlg, SetFontForChildWindow, (LPARAM)&lparam);

	// �_�C�A���O�f�U�C�����̃��W�t�H���g���擾�B
	LOGFONT lgBase{};
	tstring tstr(BUFFER_SIZE, _T('\0'));
	if (!GetPrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, &lgBase
		, sizeof(lgBase)
		, MakeIniPath().c_str()))
	{
		_D("Err");
		return FALSE;
	}

	// �V�����Z�b�g���ꂽhfont�̃��W�t�H���g�̎擾�B
	LOGFONT lgNew{};
	if (!GetObject(hfont, sizeof(LOGFONT), &lgNew))
	{
		ErrMes("Err. GetObject");
		return FALSE;
	}

	// �_�C�A���O�̃T�C�Y�ύX
	{
		// �x�[�XRECT�擾
		RECT rect{};
		if (!GetPrivateProfileStruct(
			_T("MINIMUM_UI_DLG")
			, _T("DLG_WINDOW_BASE_RECT")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		if (!SetWindowPos(hDlg, NULL
			, 0
			, 0
			, (rect.right - rect.left) * (DOUBLE)lgNew.lfHeight / lgBase.lfHeight
			, (rect.bottom - rect.top) * (DOUBLE)lgNew.lfHeight / lgBase.lfHeight
			, SWP_ASYNCWINDOWPOS
			| SWP_NOMOVE
			| SWP_NOZORDER))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
	}

	// �_�C�A���O�A�C�e���̃T�C�Y�ƈʒu�ύX�B
	auto data = make_tuple(hDlg, (DOUBLE)lgNew.lfHeight / lgBase.lfHeight);
	EnumChildWindows(hDlg, ResizeChildWindow, (LPARAM)&data);

	// �t�H���g�폜
	DeleteFont(hfont);

	// �Z�b�g���ꂽ�t�H���g��ۑ��B
	if (!WritePrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_CURRENT")
		, &lgNew
		, sizeof(lgNew)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	if (!GetAncestor(hwndCtl, GA_PARENT))
	{
		DeleteFont(hfont);
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MinimumUIDlg::OnWmGetFont(HWND hwnd)
{
	LOGFONT lgCur{};
	HFONT hfont{};
	if (GetPrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_CURRENT")
		, &lgCur
		, sizeof(lgCur)
		, MakeIniPath().c_str())) {
		// ���[�U�[�ݒ�̃t�H���g�ɕύX
		if (!(hfont = CreateFontIndirect(&lgCur)))
		{
			ErrMes("Create Font Err");
			return FALSE;
		}
	}
	else if (GetPrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, &lgCur
		, sizeof(lgCur)
		, MakeIniPath().c_str())) {
		// ���[�U�[�ݒ�̃t�H���g�ɕύX
		if (!(hfont = CreateFontIndirect(&lgCur)))
		{
			ErrMes("Create Font Err");
		}
	}
	SetWindowLongPtr(hwnd, DWLP_MSGRESULT, reinterpret_cast<LONG_PTR>(hfont));
	return (INT_PTR)hfont;
}

INT_PTR UsiDebugStb::MinimumUIDlg::OnWmDestroy(HWND hwnd)
{
	// ���ڂ��폜
	if (!WritePrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, NULL
		, NULL
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	// ���ڂ��폜
	if (!WritePrivateProfileStruct(
		_T("MINIMUM_UI_DLG")
		, _T("DLG_INIT_FONT_HANDLE")
		, NULL
		, NULL
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	return INT_PTR();
}

INT_PTR UsiDebugStb::MinimumUIDlg::OnWmClose(HWND hwnd)
{
	return INT_PTR();
}
