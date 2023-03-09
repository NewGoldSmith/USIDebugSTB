//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "InputDialog.h"

using namespace std;
using namespace DUSI_COMMON;

BOOL UsiDebugStb::InputDialog::
OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	this->hDlg = hDlg;
	Button_Enable(GetDlgItem(hDlg, IDC_BUTTON_SEND), FALSE);
	SubClassEdit.SubclassWindowX(GetDlgItem(hDlg, IDC_EDIT_FORMAT));

	{
		//改行コードラジオボタン設定
		basic_string<TCHAR>tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("INPUT_DLG")
			, _T("RADIO_LB_CODE")
			, _T("3")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_LB_NONE)
			, tstrReturn == _T("0") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_LB_CR)
			, tstrReturn == _T("1") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_LB_LF)
			, tstrReturn == _T("2") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_LB_CRLF)
			, tstrReturn == _T("3") ? BST_CHECKED : BST_UNCHECKED);
	}
	return 0;
}

INT_PTR UsiDebugStb::InputDialog::OnWmClose(HWND hwnd)
{
	SubClassEdit.UnSubclasssWindowX();
	DestroyWindow(hDlg);
	hDlg = NULL;
	return 1;
}

INT_PTR UsiDebugStb::InputDialog::OnBnOK
(HWND hwnd, HWND ctrl, UINT code)
{
	PostMessage(hDlg, WM_CLOSE, 0, 0);
	return INT_PTR();
}

INT_PTR UsiDebugStb::InputDialog::OnBnCancel
(HWND hwnd, HWND, UINT code)
{
	PostMessage(hDlg, WM_CLOSE, 0, 0);
	return INT_PTR();
}

INT_PTR UsiDebugStb::InputDialog::OnBnSend
(HWND hwnd, HWND ctrl, UINT code)
{
	strCmd.resize(BUFFER_SIZE, _T('\0'));
	strCmd.resize(GetWindowText(
		GetDlgItem(hDlg, IDC_EDIT_FORMAT)
		, strCmd.data()
		, strCmd.size()));
	::PostMessage(hParentWnd, WM_INPUT_CONFIRMED, 0, 0);
	SetWindowText(GetDlgItem(hDlg, IDC_EDIT_FORMAT), _T(""));
	return INT_PTR();
}

INT_PTR UsiDebugStb::InputDialog::
OnBnLBNone(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("INPUT_DLG")
		, _T("RADIO_LB_CODE")
		, _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::InputDialog::
OnBnLBCR(HWND hwnd, HWND, UINT code)
{
	WritePrivateProfileString(
		_T("INPUT_DLG")
		, _T("RADIO_LB_CODE")
		, _T("1")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::InputDialog::
OnBnLBLF(HWND hwnd, HWND, UINT code)
{
	WritePrivateProfileString(
		_T("INPUT_DLG")
		, _T("RADIO_LB_CODE")
		, _T("2")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::InputDialog::
OnBnLBCRLF(HWND hwnd, HWND, UINT code)
{
	WritePrivateProfileString(
		_T("INPUT_DLG")
		, _T("RADIO_LB_CODE")
		, _T("3")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::InputDialog::OnWmCommand
(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	//マクロは仮引数の名前に依存しています。
#define HDL_CMD_CLS( ctlid, fn) \
		case ctlid: return this->fn(hwnd,hwndCtl,codeNotify)
	switch (id) {
		HDL_CMD_CLS(IDOK, OnBnOK);
		HDL_CMD_CLS(IDCANCEL, OnBnCancel);
		HDL_CMD_CLS(IDC_BUTTON_SEND, OnBnSend);
		HDL_CMD_CLS(IDC_RADIO_LB_NONE, OnBnLBNone);
		HDL_CMD_CLS(IDC_RADIO_LB_CR, OnBnLBCR);
		HDL_CMD_CLS(IDC_RADIO_LB_LF, OnBnLBLF);
		HDL_CMD_CLS(IDC_RADIO_LB_CRLF, OnBnLBCRLF);
	default:
		return 0;
	}
#undef HDL_CMD_CLS
}

INT_PTR UsiDebugStb::InputDialog::OnWmSysCommand
(HWND hwnd, UINT cmd, int x, int y)
{
	switch (cmd) {
	case SC_CLOSE:
		return 		PostMessage(hDlg
			, WM_COMMAND
			, MAKEWPARAM(IDCANCEL, BN_CLICKED)
			, (LPARAM)GetDlgItem(hwnd, IDCANCEL));
	default:
		return DefWindowProc(hwnd, WM_SYSCOMMAND, cmd, MAKELPARAM(x, y));
	}
}

INT_PTR UsiDebugStb::InputDialog::OnWmKeyDownEnter(HWND hwnd)
{
	//変数を使わない事が判っているのでNULLを代入。
	if (IsWindowEnabled(GetDlgItem(hDlg, IDC_BUTTON_SEND)))
	{
		OnBnSend(hwnd, NULL, NULL);
	}
	return TRUE;
}

INT_PTR UsiDebugStb::InputDialog::OnWmChar(HWND hwnd, TCHAR ch, int cRepeat)
{
	if (_T('\r') == ch || _T('\n') == ch)
	{
		SetActiveWindow(GetDlgItem(hwnd, IDC_BUTTON_SEND));
		::SendMessage(
			GetDlgItem(	hwnd, IDC_BUTTON_SEND)
			, BM_CLICK, 0UL, 0L);
		return INT_PTR();
	}
	
	return INT_PTR();
}


UsiDebugStb::InputDialog::InputDialog()
	:
	hDlg(NULL)
	, fEnd(FALSE)
	, hParentWnd(NULL)
	, pDlgPrc(
 [](
			HWND hDlg
			, UINT uMsg
			, WPARAM wParam
			, LPARAM lParam)->INT_PTR
		{
			//マクロは仮引数の名前に依存しています。
#define GET_THIS_PTR \
		reinterpret_cast<InputDialog *>(GetWindowLongPtr(hDlg, DWLP_USER))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<InputDialog*>\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);return lParam;}())

#define HDL_DLG_MSG_CLS( msg, fn) \
    case(msg):return SetDlgMsgResult\
			(hDlg, msg, (HANDLE_##msg(hDlg, wParam, lParam,\
			 (msg==WM_INITDIALOG ? GET_THIS_PTR_AT_INIT : GET_THIS_PTR)->fn)))
#define HANDLE_WM_KEYDOWN_ENTER(hwnd, wParam, lParam, fn) \
			((fn)((hwnd)), 0L)

			switch (uMsg)
			{
				HDL_DLG_MSG_CLS(WM_SYSCOMMAND, OnWmSysCommand);
				HDL_DLG_MSG_CLS(WM_CLOSE, OnWmClose);
				HDL_DLG_MSG_CLS(WM_COMMAND, OnWmCommand);
				HDL_DLG_MSG_CLS(WM_INITDIALOG, OnWmInitDialog);
				HDL_DLG_MSG_CLS(WM_KEYDOWN_ENTER, OnWmKeyDownEnter);
				HDL_DLG_MSG_CLS(WM_CHAR, OnWmChar);
			default:
				return FALSE;
			}

#undef HANDLE_WM_KEYDOWN_ENTER
#undef HDL_DLG_MSG_CLS
#undef GET_THIS_PTR_AT_INIT
#undef GET_THIS_PTR
		}
	)
{
}

UsiDebugStb::InputDialog::~InputDialog()
{
	if (hDlg)
	{
		DestroyWindow(hDlg);
	}
}

void UsiDebugStb::InputDialog::Close()
{
	DestroyWindow(hDlg);
}

void UsiDebugStb::InputDialog::CreateDialogX(
	_In_opt_ HINSTANCE hInstance
	, _In_opt_ HWND hWndParent
)
{
	hParentWnd = hWndParent;
	::CreateDialogParamW(
		hInstance
		, MAKEINTRESOURCE(IDD_INPUT_DIALOG)
		, hWndParent
		, pDlgPrc
		, (LPARAM)this
	);
}

void UsiDebugStb::InputDialog::EnableSendButton(BOOL b)
{
	Button_Enable(GetDlgItem(hDlg, IDC_BUTTON_SEND), b);
}

tstring UsiDebugStb::InputDialog::GetLB()
{
	return Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_LB_NONE))
		== BST_CHECKED ? tstring() :
		Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_LB_CR))
		== BST_CHECKED ? tstring(_T("\r")) :
		Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_LB_LF))
		== BST_CHECKED ? tstring(_T("\n")) :
		tstring(_T("\r\n"));
}

