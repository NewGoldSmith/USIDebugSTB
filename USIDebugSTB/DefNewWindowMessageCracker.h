/**
 * @file DefNewWindowMessageCracker.h
 * @brief New window message cracker.
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once

#define HANDLE_WM_SIZING(hwnd,wParam,lParam,fn) \
	((fn)((hwnd),(UINT)(wParam),reinterpret_cast<RECT*>(lParam)))
#define HANDLE_WM_MOVING(hwnd, wParam,lParam,fn) \
((fn)((hwnd),reinterpret_cast<LPRECT>(lParam)))
#define HANDLE_WM_EXITSIZEMOVE(hwnd,wParam,lParam,fn)\
((fn)(hwnd))
