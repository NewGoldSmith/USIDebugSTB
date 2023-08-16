/**
 * @file DefUserMessageCracker.h
 * @brief User message cracker definition.
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once

// User definition.
#define HDL_DLG_MSG_CLS( msg, fn) \
    case(msg):return SetDlgMsgResult\
			(hDlg, msg, (HANDLE_##msg(hDlg, wParam, lParam,\
			 (msg==WM_INITDIALOG ?\
 GET_THIS_PTR_AT_INIT : GET_THIS_PTR)->fn)))

#define HANDLE_WM_NOTIFYICON(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam),(LPARAM)(lParam)))
#define HANDLE_WM_END(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd)))
#define HANDLE_WM_USER_SETFONT(hwnd,wParam,lParam, fn) \
	((fn)((hwnd),(HFONT)(wParam)))
#define HANDLE_WM_INPUT_CONFIRMED(hwnd, wParam,lParam,fn) \
((fn)((hwnd),(WPARAM)(wParam),(LPARAM)(lParam)))
#define HANDLE_WM_DO_OFFSET_MOVE(hwnd,wParim,lParam, fn) \
((fn)((hwnd),(LONG)wParam,(LONG)lParam))
#define HANDLE_WM_KEYDOWN_ENTER(hwnd, wParam, lParam, fn) \
			((fn)((hwnd)), 0L)
