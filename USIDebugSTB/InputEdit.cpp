//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "InputEdit.h"
using namespace UsiDebugStb;
using namespace DUSI_COMMON;

LRESULT UsiDebugStb::InputEdit::OnWmChar(HWND hwnd, TCHAR ch, int cRepeat)
{
//	MyTRACE("OnWmChar "+TtoA({ch}) + "\r\n");
	if (_T('\r') == ch || _T('\n') == ch)
	{
		::SendMessage(GetParent(hWnd), WM_KEYDOWN_ENTER, 0UL, 0L);
		return LRESULT();
	}
	FORWARD_WM_CHAR(hwnd, ch, cRepeat, pfnEditOriginPrc);
	return LRESULT();
}

InputEdit::InputEdit()
	:hWnd(NULL)
	, pfnEditSubPrc(
	[](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
	{
	//マクロは仮引数の名前に依存しています。
#define GET_THIS_PTR \
		reinterpret_cast<InputEdit *>(GetWindowLongPtr(hwnd, GWLP_USERDATA))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<InputEdit*>(\
[hwnd,lParam]()\
{\
	SetWindowLongPtr(\
		hwnd\
		, GWLP_USERDATA\
		, reinterpret_cast<LONG_PTR>\
			(reinterpret_cast<LPCREATESTRUCT>\
				(lParam)->lpCreateParams));\
	return reinterpret_cast<LPCREATESTRUCT>\
			(lParam)->lpCreateParams;\
}())

#define HDL_WND_MSG_CLS( msg, fn) \
    case msg:\
	{\
	 return (HANDLE_##msg(hwnd, wParam, lParam,\
			reinterpret_cast<InputEdit*>\
			(msg==WM_CREATE ?\
			GET_THIS_PTR_AT_INIT :\
			GET_THIS_PTR)\
			->fn));\
	}
		switch (uMsg)
		{
			HDL_WND_MSG_CLS(WM_CHAR, OnWmChar);
		default:
			return CallWindowProc(
				GET_THIS_PTR->pfnEditOriginPrc
				, hwnd
				, uMsg
				, wParam
				, lParam);
		}

#undef HDL_WND_MSG_CLS
#undef GET_THIS_PTR_AT_INIT
#undef GET_THIS_PTR

return LONG_PTR();
		}
	)


	, pfnEditOriginPrc(NULL)

{
}

InputEdit::~InputEdit()
{
	UnSubclasssWindowX();
}

void InputEdit::SubclassWindowX(HWND hwnd)
{
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	pfnEditOriginPrc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	hWnd = hwnd;
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnEditSubPrc);
	SetWindowPos(hwnd
		, HWND_TOP
		, 0
		, 0
		, 0
		, 0
		, SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
}

void InputEdit::UnSubclasssWindowX()
{
	if (pfnEditOriginPrc)
	{
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnEditOriginPrc);
		SetWindowPos(hWnd
			, HWND_TOP
			, 0
			, 0
			, 0
			, 0
			, SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		pfnEditOriginPrc = NULL;
		hWnd = NULL;
	}
}
