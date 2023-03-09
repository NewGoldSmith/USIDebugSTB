//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#pragma once
#include "pch.h"
#include "InputDialog.h"
#include "resource.h"

using namespace std;
using namespace DUSI_COMMON;
namespace UsiDebugStb {
#define USING_GROBAL_THISPOINTER
#define WM_NOTIFYICON	(WM_APP + 2)
#define WM_END			(WM_APP + 3)
	//メインダイアロググローバルポインタ
	class MainDialog
	{
		HWND hDlg;
		BOOL fHide;
		TP_CALLBACK_ENVIRON* pcbe;
		TP_CLEANUP_GROUP* ptpcg;
		HANDLE hIn;
		HANDLE hOut;
		HANDLE hPrevIn;
		HANDLE hPrevOut;
		HANDLE hDownRd;
		HANDLE hDownWt;
		HANDLE hUpRd;
		HANDLE hUpWt;
		HWND hCon;
		HWND hPrevConParent;
		HGLOBAL hgCopy;
		uintptr_t hEndThread;
		POINT ptPrevDlgPos;
		atomic<int> fEnd;
		NOTIFYICONDATA nid;
		PROCESS_INFORMATION PI;
		ofstream ofLog;
		atomic<u_int> LineNum;
		UINT Lv_Status;
		int Lv_iPrevItem;
		string ParentReadBuf;
		string ParentRemBuf;
		string ChildReadBuf;
		string ChildRemBuf;
		BlinkingStatic BS;
		const tstring EscCode;
		const map < TCHAR, tstring> mapChToStrT;
		const unique_ptr<InputDialog
			,void(*)(InputDialog *)> pInputDlg;
		const unique_ptr< FILETIME
			, void (*)(FILETIME*)> pFT1sec;
		const unique_ptr<remove_pointer_t<HANDLE>
			, decltype(CloseHandle)*>	hSemParentRead;
		const unique_ptr<remove_pointer_t<HANDLE>
			, decltype(CloseHandle)*>	hSemParentWrite;
		const unique_ptr<remove_pointer_t<HANDLE>
			, decltype(CloseHandle)*>	hSemChildWrite;
		const unique_ptr<remove_pointer_t<HANDLE>
			, decltype(CloseHandle)*>	hSemChildRead;
		const unique_ptr<remove_pointer_t<HANDLE>
			, decltype(CloseHandle)*>	hSemLv;
		typedef tuple<MainDialog*, string> pDlgString_type;
		vector<basic_string<TCHAR>> const vstrHeader
		{ _T("Line"),_T("Device") ,_T("Dir"),_T("Text"),_T("Undef") };
		enum enumHeader :const unsigned 
		{ Line, Device, Dir, Text, Undef };
		const unique_ptr<pDlgString_type, void(*)(pDlgString_type[])>
			pDlgStringArrayHeap;
		RingBuf<pDlgString_type> pDlgStringPool;
		basic_string<TCHAR> MakeDispLogFileName();
		void SaveProfile();
		void LoadProfile();
		BOOL LoadInputDlgPosition(HWND hDlg, RECT& rect);
		BOOL SaveInputDlgPosition(HWND hDlg);
		BOOL ParentRead();
		BOOL ChildRead();
		BOOL DoExecute();
		void CleanupExecute();

		INT_PTR OnNM_List1(HWND hDlg, WPARAM wParam, LPNMHDR lParam);

		INT_PTR OnBnOK(HWND hwnd, HWND, UINT code);
		INT_PTR OnBnCancel(HWND hwnd, HWND, UINT code);
		INT_PTR OnBnSelectLogFile(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnLogPathClear(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnDeleteAll(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnCopy(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnExecute(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnSelectFile(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnTerminateProcess(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkDispTaskTray(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkHideWindowAtStart(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkHideChild(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkMakeLogFile(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkAddTimeStamp(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkStartChildAtStartup(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkEndAtChild(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkShowPrompt(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkShowInputDlg(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoLogFileApp(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoLogFileTrunc(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoRejectIORedirect(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoAcceptIORedirect(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnMnTasktrayClose(HWND hwnd, HWND hctrl, UINT code);

		BOOL OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
		INT_PTR OnWmNotify(HWND hDlg, WPARAM wParam, LPNMHDR lParam);
		INT_PTR OnWmSysCommand(HWND hwnd, UINT cmd, int x, int y);
		INT_PTR OnWmCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		INT_PTR OnWmEnd(HWND hwnd);
		INT_PTR OnWmWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos);
		INT_PTR OnWmSize(HWND hwnd, UINT state, int cx, int cy);
		INT_PTR OnWmDestroyClipBoard(HWND hwnd);
		INT_PTR OnWmNotifyIcon(HWND hDlg, WPARAM wParam, LPARAM lParam);
		INT_PTR OnWmInputConfirmed(HWND hDlg, WPARAM wParam, LPARAM lParam);
		INT_PTR OnWmMoving(HWND hDlg, LPRECT pRect);
		const _beginthreadex_proc_type pfnEndProc;
		void(* const pfnDSDeleter)(pDlgString_type*);

		const PTP_TIMER_CALLBACK pfnDelayEndProcesing;
		const PTP_WAIT_CALLBACK pfnDetectEnd;
		const PTP_WAIT_CALLBACK pfnParentReadRepeat;
		const PTP_WAIT_CALLBACK pfnChildReadRepeat;
		const PTP_WAIT_CALLBACK pfnSerializedWriteToParent;
		const PTP_WAIT_CALLBACK pfnSerializedWriteToChild;
		const PTP_WAIT_CALLBACK pfnLvSerializedWriteFromParentToLog;
		const PTP_WAIT_CALLBACK pfnLvSerializedWriteFromChildToLog;

	public:
		MainDialog();
		~MainDialog();
		INT_PTR DialogBoxParamW
		(_In_opt_ HINSTANCE hInstance
			, _In_opt_ HWND      hWndParent
			, PTP_CALLBACK_ENVIRON pcbeIn
		) noexcept;
	};
#ifdef USING_GROBAL_THISPOINTER
	extern MainDialog* pgMainDialog;
#endif // USING_GROBAL_THISPOINTER

}