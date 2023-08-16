//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "ConWnd.h"

using namespace DUSI_COMMON;
LRESULT UsiDebugStb::ConWnd::OnWmSysCommand(HWND hWnd, UINT cmd, int x, int y)
{
	return TRUE;
}
LRESULT UsiDebugStb::ConWnd::OnWmMoving(HWND hDlg, LPRECT pRect)
{
	return TRUE;
}
LRESULT UsiDebugStb::ConWnd::OnWmWindowPosChanging(HWND hWnd, LPWINDOWPOS lpwpos)
{
	if (fHide)
	{
		lpwpos->flags = lpwpos->flags
			& ~SWP_SHOWWINDOW
			| SWP_HIDEWINDOW;
	}
	else
	{
		lpwpos->flags = lpwpos->flags
			& ~SWP_HIDEWINDOW
			/*| SWP_SHOWWINDOW*/;
	}
	if ((lpwpos->flags & SWP_SHOWWINDOW) == SWP_SHOWWINDOW)
	{
		MyTRACE("SWP_SHOWWINDOW\r\n");
	}
	if ((lpwpos->flags & SWP_HIDEWINDOW) == SWP_HIDEWINDOW)
	{
		MyTRACE("SWP_HIDEWINDOW\r\n");
	}

	return LRESULT();
}

UsiDebugStb::ConWnd::ConWnd()
	:hWnd(NULL)
	,pPrevPrc(NULL)
	, fHide(FALSE)

	, pWndPrc{ [](
		  HWND hWnd,
		  UINT uMsg,
		  WPARAM wParam,
		  LPARAM lParam
	)->LRESULT{
#define GET_THIS_PTR \
		reinterpret_cast<ConWnd *>(GetWindowLongPtr(hWnd, DWLP_USER))

#define HDL_WND_MSG_CLS( msg, fn) \
			case(msg):{ \
				if(!(HANDLE_##msg(hWnd, wParam, lParam,\
			 GET_THIS_PTR->fn)))\
			{\
				return LRESULT(0);\
			;}}

#define HANDLE_WM_MOVING(hWnd, wParam,lParam,fn) \
((fn)((hWnd),(LPRECT)(lParam)))

			switch (uMsg)
			{
				//HDL_WND_MSG_CLS(WM_SYSCOMMAND, OnWmSysCommand);
				//HDL_WND_MSG_CLS(WM_WINDOWPOSCHANGING,
				//	OnWmWindowPosChanging);
				//HDL_WND_MSG_CLS(WM_MOVING, OnWmMoving);
			default:
				return CallWindowProc(
					GET_THIS_PTR->pPrevPrc
					, hWnd
					, uMsg
					, wParam
					, lParam);
			}
#undef HANDLE_WM_MOVING
#undef HDL_WND_MSG_CLS
#undef GET_THIS_PTR
} }

{
}

UsiDebugStb::ConWnd::~ConWnd()
{
}

void UsiDebugStb::ConWnd::Attach(HWND hWnd)
{
	this->hWnd = hWnd;
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR )this);
	pPrevPrc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pWndPrc);
	SetWindowPos(hWnd
		, HWND_TOP
		, 0
		, 0
		, 0
		, 0
		, SWP_NOOWNERZORDER|SWP_SHOWWINDOW| SWP_NOMOVE| SWP_NOSIZE);
}

void UsiDebugStb::ConWnd::Detach()
{
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pPrevPrc);
	pPrevPrc = NULL;
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
	SetWindowPos(hWnd
		, HWND_TOP
		, 0
		, 0
		, 0
		, 0
		, SWP_NOOWNERZORDER|SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	hWnd = NULL;
}
