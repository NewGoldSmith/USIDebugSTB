/**
 * @file UserDefinition.h
 * @brief User definition.
 * @author Gold Smith
 * @date 2022-2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#define NAMED_PIPE_PREFIX APPNAME
#define WM_NOTIFYICON	(WM_APP + 2)
#define WM_END			(WM_APP + 3)
#define WM_INPUT_CONFIRMED			(WM_APP + 4)
/// <summary>
/// WM_USER_SETFONT WPARAM hfont,LPARAM 0L
/// </summary>
#define WM_USER_SETFONT (WM_APP+5)
/// <summary>
/// hWnd, WM_DO_OFFSET_MOVE, WPARAM cx, LPARAM cy
/// </summary>
#define WM_DO_OFFSET_MOVE (WM_APP+6)
/// <summary>
/// hWnd, WM_KEYDOWN_ENTER, 0L, 0L
/// </summary>
#define WM_KEYDOWN_ENTER	(WM_APP + 7)


