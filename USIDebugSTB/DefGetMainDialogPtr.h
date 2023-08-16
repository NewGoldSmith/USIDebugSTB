#pragma once
#ifdef  USING_GROBAL_THISPOINTER
#define GET_THIS_PTR reinterpret_cast<MainDialog*>(pgMainDialog)
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<MainDialog*>\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);\
pgMainDialog=reinterpret_cast<MainDialog*>(lParam);\
				return lParam;}())
#define HDL_DLG_MSG_CLS( msg, fn) \
    case(msg):{return SetDlgMsgResult\
			(hDlg, msg, (HANDLE_##msg(hDlg, wParam, lParam,\
			 (msg==WM_INITDIALOG ?\
				GET_THIS_PTR_AT_INIT : GET_THIS_PTR)->fn)));}

#else
#define GET_THIS_PTR \
		reinterpret_cast<MainDialog *>(GetWindowLongPtr(hDlg, DWLP_USER))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<MainDialog*>\
([hDlg,lParam]()\
	{SetWindowLongPtr(hDlg, DWLP_USER, lParam);return lParam;}())

#define HDL_DLG_MSG_CLS( msg, fn) \
    case(msg):return SetDlgMsgResult\
			(hDlg, msg, (HANDLE_##msg(hDlg, wParam, lParam,\
			 (msg==WM_INITDIALOG ?\
 GET_THIS_PTR_AT_INIT : GET_THIS_PTR)->fn)))
#endif
