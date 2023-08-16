//Copyright (c) 2022, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "BlinkingStatic.h"

BlinkingStatic::BlinkingStatic() noexcept
	: nIDEvent(0)
	, hwnd(NULL)
	, stShow(SW_SHOWDEFAULT)
	, TimerPrc{ [](
		HWND hWnd
		, UINT uiMessage
		, UINT_PTR lParam
		, DWORD dParam)
		{
			BlinkingStatic* pBS = (BlinkingStatic*)GetWindowLongPtrW(hWnd,GWLP_USERDATA);
			switch (uiMessage) {
			case WM_TIMER:
			{
				ShowWindow(hWnd, (pBS->stShow ? SW_HIDE : SW_SHOW));
				pBS->stShow = pBS->stShow ? SW_HIDE : SW_SHOW;
			}
			default: {

			}
			}
		}

	}
{
}

		BOOL BlinkingStatic::Attach(HWND hctrl, UINT_PTR   ID)
		{
			hwnd = hctrl;
			nIDEvent = ID;
			SetWindowLongPtrW(hctrl, GWLP_USERDATA, (LONG_PTR)this);
			if (!SetTimer(hwnd, nIDEvent, 1000, TimerPrc))
			{
				return FALSE;
			}
			return TRUE;
		}

		void BlinkingStatic::Detach()
		{
			KillTimer(hwnd, nIDEvent);
		}

