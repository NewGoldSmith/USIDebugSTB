/**
 * @file DefChildDialogPtr.h
 * @brief Acquisition definition of child window pointer.
 * @author Gold Smith
 * @date 2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
			//マクロは仮引数の名前に依存しています。
#define GET_THIS_PTR \
		reinterpret_cast<DEF_CLASS_PTR *>(GetWindowLongPtr(hDlg, DWLP_USER))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<DEF_CLASS_PTR *>\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);return lParam;}())

