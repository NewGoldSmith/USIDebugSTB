/**
 * @file DefAncestorDialogPtr.h
 * @brief Pointter acquisition definition of Ancestor window.
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
// マクロは仮引数の名前に依存しています。

#ifdef  USING_GROBAL_THISPOINTER
#define GET_THIS_PTR reinterpret_cast<DEF_CLASS_PTR*>(pgMainDialog)
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast< DEF_CLASS_PTR* >\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);\
pgMainDialog=reinterpret_cast< DEF_CLASS_PTR*>(lParam);\
				return lParam;}())

#else // USING_GROBAL_THISPOINTER
#define GET_THIS_PTR \
		reinterpret_cast<DEF_CLASS_PTR*>(GetWindowLongPtr(hDlg, DWLP_USER))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<DEF_CLASS_PTR*>\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);return lParam;}())

#define HDL_DLG_MSG_CLS( msg, fn) \
    case(msg):return SetDlgMsgResult\
			(hDlg, msg, (HANDLE_##msg(hDlg, wParam, lParam,\
			 (msg==WM_INITDIALOG ?\
 GET_THIS_PTR_AT_INIT : GET_THIS_PTR)->fn)))
#endif
