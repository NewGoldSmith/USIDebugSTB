// Copyright (c) 2022 2023, Gold Smith
// MainDlgDS.cpp
// Released under the MIT license
// https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "MainDlgDS.h"
using namespace std;
using namespace DUSI_COMMON;

#ifdef USING_GROBAL_THISPOINTER
	// グローバルメインダイアログポインタ
UsiDebugStb::MainDialog* UsiDebugStb::pgMainDialog(NULL);
#endif // USING_GROBAL_THISPOINTER

UsiDebugStb::MainDialog::
MainDialog()
	:hDlgMain(NULL)
	, ptppUI
	{
		[]() {
		PTP_POOL ptpp = CreateThreadpool(NULL);
		if (!ptpp)
		{
			throw runtime_error(EOut);
		}
			SetThreadpoolThreadMaximum(ptpp, 2);
			return ptpp;
		}()
	,
		[](PTP_POOL ptpp) {
		CloseThreadpool(ptpp);
	}}

	, pcbeUI
	{
		[this]() {
			const auto pcbeUI = new TP_CALLBACK_ENVIRON;
			InitializeThreadpoolEnvironment(pcbeUI);
			SetThreadpoolCallbackPool(pcbeUI, ptppUI.get());
			return pcbeUI;
		}()
		,
		[](PTP_CALLBACK_ENVIRON pcbeUI) {
			DestroyThreadpoolEnvironment(pcbeUI);
			delete pcbeUI;
		}
	}

	, ptpcgUI
	{
		[this]() {
			const auto ptpcgUI = CreateThreadpoolCleanupGroup();
			SetThreadpoolCallbackCleanupGroup(pcbeUI.get(), ptpcgUI, NULL);
			return ptpcgUI;
		}()
		,
		[](PTP_CLEANUP_GROUP ptpcgUI) {
			CloseThreadpoolCleanupGroupMembers(ptpcgUI,TRUE,NULL);
			CloseThreadpoolCleanupGroup(ptpcgUI);
		}
	}

	, ptpp
	{
		[]() {
		PTP_POOL ptpp = CreateThreadpool(NULL);
		if (!ptpp)
		{
			throw runtime_error(EOut);
		}
			SetThreadpoolThreadMaximum(ptpp, 5);
			return ptpp;
		}()
	,
		[](PTP_POOL ptpp) {
		CloseThreadpool(ptpp);
	}}
	
	, pcbe
	{
		[this]() {
			const auto pcbe = new TP_CALLBACK_ENVIRON;
			InitializeThreadpoolEnvironment(pcbe);
			SetThreadpoolCallbackPool(pcbe, ptpp.get());
			return pcbe;
		}()
		,
		[](PTP_CALLBACK_ENVIRON pcbe) {
			DestroyThreadpoolEnvironment(pcbe);
			delete pcbe;
		}
	}

	, ptpcg
	{
		[this]() {
			const PTP_CLEANUP_GROUP ptpcg = CreateThreadpoolCleanupGroup();
			SetThreadpoolCallbackCleanupGroup(pcbe.get(), ptpcg, pfnCGCC);
			return ptpcg;
		}()
		,
		[](PTP_CLEANUP_GROUP ptpcg) {
			CloseThreadpoolCleanupGroup(ptpcg);
		}
	}
	
	, fHide(FALSE)
	, hIn(NULL)
	, hOut(NULL)
	, hPrevIn(NULL)
	, hPrevOut(NULL)
	, hDownRd(NULL)
	, hDownWt(NULL)
	, hUpRd(NULL)
	, hUpWt(NULL)
	, hwndCon(NULL)
	, hgCopy(NULL)
	, hCurrentFont(NULL)
	, hEndThread(NULL)
	, rcPrevDlgPos{MAXLONG,MAXLONG}
	, fEnd(0)
	, nid{}
	, PI{}
	, olConConnectedComp{}
	, hLog(NULL)
	, olLog{}
	, LineNum(0)
	, Lv_Status(0)
	, Lv_iPrevItem(-1)
	, pTP_IOLog(NULL)
	, pNormalUIDlg{
		new UIDialog()
		, [](UIDialog*p)
		{
			delete p;
		}
	}

	, pMinimumUIDlg{
		new UIDialog()
		, [](UIDialog* p)
		{
			delete p;
		}
	}

	, pInputDlg{
		new InputDialog
		,[](InputDialog* p)
		{
			delete p;
		}
	}

	// ファイルタイム1秒
	, pFT1sec
	{
		[]()
		{
			const auto pFiletime = new FILETIME;
			ULARGE_INTEGER ulDueTime;
			ulDueTime.QuadPart = (ULONGLONG)-(1 * 10 * 1000 * 1000 * 1);
			pFiletime->dwHighDateTime = ulDueTime.HighPart;
			pFiletime->dwLowDateTime = ulDueTime.LowPart;
			return pFiletime;
		}()
		,
		[](_Inout_ FILETIME* pFTNumsec)
		{
			delete pFTNumsec;
		}
	}
	
	, EscCode{}

	, mapChToStrT
	{
		{_T('\0'),_T("NUL")}
		,{_T('\x03'),_T("ETX")}
		,{_T('\a'),_T("BEL")}
		,{_T('\b'),_T("BS")}
		,{_T('\t'),_T("HT")}
		,{_T('\n'),_T("LF")}
		,{_T('\f'),_T("FF")}
		,{_T('\r'),_T("CR")}
		,{_T('\x1b'),_T("ESC")}
		,{_T('\x7F'),_T("DEL")}
	}

	, pDlgStringArrayHeap
	{
		new pDlgString_type[DLG_STRING_ARRAY_SIZE*4]
		,
		[](pDlgString_type p[])
		{
			delete[]p;
		}
	}

	,pOLLogArrayHeap
	{
		new OL_LOG[DLG_STRING_ARRAY_SIZE]
		,
		[](OL_LOG p[])
		{
			delete[]p;
		}
	}

	, vhrcDlgItemsBasePos()

	, hEvOL
	{
		[]() {
			HANDLE h = CreateEvent(NULL,TRUE,FALSE,NULL);
			if (!h) {
				throw runtime_error(ErrOut(GetLastError()));
			}
			return h;
		}()
		,CloseHandle
	}

	, hSemParentRead{[]()
	{
		HANDLE hSem = CreateSemaphore(NULL, 1, 1, NULL);
		if (!hSem) {
			throw runtime_error(ErrOut(GetLastError()));
		}
		return hSem;
	}()
	, CloseHandle
	}

	, hSemParentWrite
	{
		[]()
		{
			HANDLE hSem = CreateSemaphore(NULL, 1, 1, NULL);
			if (!hSem) {
				throw runtime_error(ErrOut(GetLastError()));
			}
			return hSem;
		}()
		, CloseHandle
	}

	, hSemChildWrite{[]()
	{
		HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
		if (!hSem) {
			throw runtime_error(ErrOut(GetLastError()));
		}
		return hSem;
		}()
		, CloseHandle
	}

	, hSemChildRead{[]()
	{
		HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
		if (!hSem) {
			throw runtime_error(ErrOut(GetLastError()));
		}
		return hSem;
		}()
		,CloseHandle
	}

	,hSemLv{ []()
	{
		HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
		if (!hSem) {
			throw runtime_error(ErrOut(GetLastError()));
		}
		return hSem;
		}()
		,CloseHandle
	}

	, mrDlgStr(pDlgStringArrayHeap.get(), DLG_STRING_ARRAY_SIZE*4)
	, mrOLLog(pOLLogArrayHeap.get(), DLG_STRING_ARRAY_SIZE)

	, pfnEndProc{[](void* __stdcall pvoid)->unsigned 
	{
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(pvoid);
		pDlg->fEnd.store(1);
		if (pDlg->hgCopy)
		{
			EmptyClipboard();
		}
		//pDlg->SaveProfile();
		if (pDlg->PI.hProcess)
		{
			::SendMessage(
				pDlg->hDlgMain
				, WM_COMMAND
				, MAKEWPARAM( IDC_BUTTON_TERMINATE_PROCESS,BN_CLICKED)
				, (LPARAM)GetDlgItem(
					pDlg->hDlgMain
					, IDC_BUTTON_TERMINATE_PROCESS));
		}
		CancelIoEx(pDlg->hIn, NULL);
		CancelIoEx(pDlg->hUpRd, NULL);

		pDlg->pTP_IOLog = NULL;
		if (pDlg->hLog)
		{
			CloseHandle(pDlg->hLog);
		}
		Shell_NotifyIcon(NIM_DELETE, &pDlg->nid);
		DestroyIcon(pDlg->nid.hIcon);
		pDlg->pInputDlg->hDlg;
		RECT rect{};
		TCHAR buf[MAX_PATH];
		::GetWindowRect(pDlg->pInputDlg->hDlg, &rect);
		_itot_s(rect.left, buf, MAX_PATH, 10);
		if (!WritePrivateProfileStruct(_T("INPUT_DLG")
			, _T("WINDOW_POS")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		CloseThreadpoolCleanupGroupMembers(&*pDlg->ptpcg, TRUE, NULL);
		::PostMessage(pDlg->hDlgMain, WM_END, NULL, NULL);
		return S_OK;
	} }

	, pfnFontSelectProc{ [](void* __stdcall pvoid)->unsigned
	{
		if (!pvoid)
		{
			ErrMes("pvoid is NULL.");
			return FALSE;
		}
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(pvoid);
		// フォント選択ダイアログボックスの表示
		CHOOSEFONT cf = { sizeof(CHOOSEFONT) };
		LOGFONT lf{};
		// 現在のフォント設定があるか確認。
		if (!GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_LOG_FONT_CURRENT")
			, &lf
			, sizeof(lf)
			, MakeIniPath().c_str()))
		{
			// なければ、ベースフォントをlfに入れる。
			{
				if (!GetPrivateProfileStruct(
					_T("MAIN_DLG")
					, _T("DLG_LOG_FONT_BASE")
					, &lf
					, sizeof(lf)
					, MakeIniPath().c_str()))
				{
					ErrOut(GetLastError());
					return FALSE;
				}
			}
		}

		cf.Flags =
			CF_SCREENFONTS
			| CF_FORCEFONTEXIST
			//| CF_NOSTYLESEL
			//| CF_NOVECTORFONTS
			| CF_NOVERTFONTS
			//| CF_FIXEDPITCHONLY
			| CF_INITTOLOGFONTSTRUCT
			| CF_APPLY
			| CF_ENABLEHOOK
			;
		cf.hwndOwner = pDlg->hDlgMain;
		cf.lpLogFont = &lf;
		cf.lCustData = (LPARAM)pDlg;
		cf.lpfnHook = [](
			HWND hWnd,
			UINT msg,
			WPARAM wParam,
			LPARAM lParam
			)->UINT_PTR {
				UINT ui(0);
				WORD comm(0);
				LOGFONT lf{};
				static CHOOSEFONT* pcf(NULL);
				static MainDialog* pDlg(NULL);
				switch (msg)
				{
				case WM_COMMAND: {
					switch (LOWORD(wParam))
					{
					case IDOK: {
						LOGFONT lf{};
						SendMessage(hWnd, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)pcf->lpLogFont);
						return FALSE;
					}
					case IDCANCEL: {
						return FALSE;
					}
					case 1026: {
						LOGFONT lf{};
						SendMessage(hWnd, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)&lf);
						HFONT hFont(NULL);
						if (!(hFont = CreateFontIndirect(&lf)))
						{
							ErrMes("Create Font Err");
							return FALSE;
						}
						SendMessage(pDlg->hDlgMain
							, WM_USER_SETFONT
							, (WPARAM)hFont
							, (LPARAM)0);
						return TRUE;
					}
					default:;
					}
					return FALSE;
				}
				case WM_INITDIALOG: {
					//_D("ChooseFont WM_INITDIALOG");
					pcf = (CHOOSEFONT*)lParam;
					pDlg = (MainDialog*)pcf->lCustData;
					return TRUE;
				}
				default:
					break;
				}
				return FALSE;
		};

		if (ChooseFont(&cf))
		{
			HFONT hFont(NULL);
			if (!(hFont = CreateFontIndirect(cf.lpLogFont)))
			{
				ErrMes("Create Font Err");
				return FALSE;
			}
			SendMessage(pDlg->hDlgMain, WM_USER_SETFONT, (WPARAM)hFont, (LPARAM)0);
		}
		return S_OK;
	 } }

	, pfnDSDeleter{[](pDlgString_type *pDS) {
		reinterpret_cast<MainDialog *>(get<0>(*pDS))
			->mrDlgStr.Return(pDS);
	}}

	, pfnConsoleHandlerRoutine{[]
	(
	 _In_ DWORD CtrlType
	)->BOOL
	{
		if (CtrlType == CTRL_C_EVENT) {
			// 名前付きパイプに接続する
		  // プロセスIDを取得する
			DWORD pid;
			if (!(pid = GetCurrentProcessId()))
			{
				ErrOut(GetLastError());
				return FALSE;
			}
			// プロセスIDを名前に加える。
			string pipename(string("\\\\.\\pipe\\")\
				+ _STRINGIZE(NAMED_PIPE_PREFIX)\
				+ to_string((int)pid));
			HANDLE hPipe = CreateFile(
				AtoT(pipename).c_str() // パイプ名
				, GENERIC_WRITE | GENERIC_READ // 書きアクセス
				, 1 // 共有モード（0は排他的アクセス）
				, NULL // セキュリティ属性（NULLはデフォルト）
				, OPEN_EXISTING // 既存のパイプに接続する
				, 0 // ファイル属性（0はデフォルト）
				, NULL // テンプレートファイル（NULLは指定しない）
			);
			if (hPipe == INVALID_HANDLE_VALUE) {
				ErrOut(GetLastError());
				return TRUE;
			}
			tstring strCtrlC({ _T('\x03') });
			int size = strCtrlC.size();
			WriteFile(hPipe, strCtrlC.c_str(), strCtrlC.size(), NULL, NULL);

			// 名前付きパイプを閉じる
			CloseHandle(hPipe);

			return TRUE;
		}
		return FALSE;
	}}

	, pfnDelyedChildProcessTermination{[]
	(
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_TIMER             Timer
	){
		CloseThreadpoolTimer(Timer);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg(reinterpret_cast<MainDialog*>(Context));
		
		pDlg->fEnd.store(1);
		// コンソール解放
		if (pDlg->hwndCon)
		{
			RECT rect{};
			GetWindowRect(pDlg->hwndCon, &rect);
			if (!WritePrivateProfileStruct(
				_T("CONSOLE")
				, _T("WINDOW_POS")
				, &rect
				, sizeof(rect)
				, MakeIniPath().c_str()))
			{
				ErrOut(GetLastError());
			}
			FreeConsole();
			::SendMessage(pDlg->hwndCon, WM_CLOSE, 0L, 0L);
			pDlg->hwndCon = NULL;
		}
		// キャンセルIO
		CancelIoEx(pDlg->hIn, NULL);
		CancelIoEx(pDlg->hUpRd,NULL);
		CloseHandle(pDlg->hUpRd);
		pDlg->hUpRd = NULL;
		CloseHandle(pDlg->hDownWt);
		pDlg->hDownWt = NULL;
		pDlg->hDownRd = NULL;
		pDlg->hUpWt = NULL;

		// ログを閉じる為のスレッドプールタスクを投入
		TP_WAIT* pTPWaitLog(NULL);
		if (!(pTPWaitLog = CreateThreadpoolWait(
			pDlg->pfnLvEndLog, pDlg, &*pDlg->pcbe)))
		{
			EOut;
		}
		// リストビュー連動になる為そのセマフォで制御
		SetThreadpoolWait(pTPWaitLog, pDlg->hSemLv.get(), NULL);

		// コンソールイベントハンドラからの接続待ちの名前付きパイプを終了。
		if (pDlg->olConConnectedComp.hPipe) 
		{
			pDlg->olConConnectedComp.bEnd = TRUE;
			DisconnectNamedPipe(pDlg->olConConnectedComp.hPipe);
			// 接続待ちを取り消す
			if (pDlg->olConConnectedComp.pTPWait)
			{
				CancelIoEx(pDlg->olConConnectedComp.hPipe, NULL);
				pDlg->olConConnectedComp.pTPWait = NULL;
			}
			CloseHandle(pDlg->olConConnectedComp.hPipe);
			pDlg->olConConnectedComp.hPipe = NULL;
		}
		WaitForSingleObject(pDlg->PI.hThread, INFINITE);
		CloseHandle(pDlg->PI.hThread);
		pDlg->PI.hThread = NULL;
		WaitForSingleObject(pDlg->PI.hProcess, INFINITE);
		CloseHandle(pDlg->PI.hProcess);
		pDlg->PI.hProcess = NULL;

		// タスクトレイのティップを元に戻す。
		//pDlg->nid.hIcon = (HICON)LoadIcon(NULL, IDI_APPLICATION);
		_tcscpy_s(pDlg->nid.szTip, 128, _T(_CRT_STRINGIZE(APPNAME)));
		int ret = (int)Shell_NotifyIcon(NIM_MODIFY, &pDlg->nid);
		// ダイアログタイトルを変更
		SetWindowText(pDlg->hDlgMain, _T(_CRT_STRINGIZE(APPNAME)));
		// ブリンキングスタティックをデタッチ。
		pDlg->BS.Detach();
		// Executeボタン有効
		Button_Enable(GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_BUTTON_EXECUTE), TRUE);
		// Terminateボタン無効。
		Button_Enable(GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_BUTTON_TERMINATE_PROCESS), FALSE);
		// リダイレクション選択ラジオボタン有効化
		Button_Enable(GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT), TRUE);
		Button_Enable(GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT), TRUE);
		// Inputダイアログの送信ボタン無効
		pDlg->pInputDlg->EnableSendButton(FALSE);


		// IOリダイレクトを受け入れないをチェックの場合
		// ハンドルをCloseする。(Close必要か要確認）
		if (Button_GetCheck(
			GetDlgItem(
				pDlg->pNormalUIDlg->hDlg
				, IDC_RADIO_IO_REDIRECT_MODE_REJECT)) == BST_CHECKED)
		{
			CloseHandle(pDlg->hIn);
			pDlg->hIn = NULL;
			CloseHandle(pDlg->hOut);
			pDlg->hOut = NULL;
		}
		// リダイレクトの場合ハンドルはCloseしない。
		else {
			pDlg->hIn = NULL;
			pDlg->hOut = NULL;
		}

		// 子プログラム終了時にこのプログラムも終了するか確認。
		if (Button_GetCheck(::GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_CHECK_END_AT_CHILD)) == BST_CHECKED)
		{
			PostMessage(pDlg->pNormalUIDlg->hDlg
				, WM_COMMAND
				, MAKEWPARAM(IDCANCEL,BN_CLICKED)
				, (LPARAM)GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDCANCEL));
		}

		// 標準ハンドルを最初の状態にする。
		if (!SetStdHandle(STD_INPUT_HANDLE, pDlg->hPrevIn))
		{
			ErrOut(GetLastError());
			return;
		}
		if (!SetStdHandle(STD_OUTPUT_HANDLE, pDlg->hPrevOut))
		{
			ErrOut(GetLastError());
			return;
		}

		// ブリンキングスタティックを非表示
		ShowWindow(
			GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_STATIC_CHILD_IS_RUNNING)
			, SW_HIDE);

	}}

	, pfnDetectEnd{ []
	(
		_Inout_     PTP_CALLBACK_INSTANCE Instance,
		_Inout_opt_ PVOID                 Context,
		_Inout_     PTP_WAIT              Wait,
		_In_        TP_WAIT_RESULT        WaitResult
	){
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(Context);
		PTP_TIMER pTPTimer(NULL);
		if (!(pTPTimer = CreateThreadpoolTimer(
			pDlg->pfnDelyedChildProcessTermination
			, Context
			, &*pDlg->pcbe)))
		{
			ErrOut(GetLastError());
			return;
		}
		SetThreadpoolTimer(pTPTimer, pDlg->pFT1sec.get(), 0,0);
	}}

	, pfnParentReadRepeat{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(Context);
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemParentRead.get(), 1);
		CallbackMayRunLong(Instance);

		// CancelIoされた場合
		if (pDlg->fEnd)
		{
			CloseThreadpoolWait(Wait);
			return;
		}
		pDlg->ParentReadBuf.resize(BUFFER_SIZE,'\0');
		DWORD cbRead(0);



		if (!ReadFile(pDlg->hIn, pDlg->ParentReadBuf.data(), pDlg->ParentReadBuf.size(), &cbRead, NULL))
		{
			ErrOut(GetLastError());
			CloseThreadpoolWait(Wait);
			return;
		}
		// CancelIoされた場合
		if (pDlg->fEnd)
		{
			CloseThreadpoolWait(Wait);
			ErrMes("CancelIo.");
			return;
		}
		pDlg->ParentReadBuf.resize(cbRead);
		pDlg->ParentRemBuf += pDlg->ParentReadBuf;

		vector<string> vstr = DUSI_COMMON::vSplitLineBreakWithLBCodeA(pDlg->ParentRemBuf);
		for (string& str : vstr)
		{
			string strOrg = str;
			//Childへ転送
			{
				pDlgString_type* pDS = &((*pDlg->mrDlgStr.Lend())
					= { pDlg, str });

				TP_WAIT* pTPWait(NULL);
				if (!(pTPWait = CreateThreadpoolWait(
					pDlg->pfnSerializedWriteToChild, pDS, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError());
					pDlg->mrDlgStr.Return(pDS);
					CloseThreadpoolWait(Wait);
					return;
				}
				else {
					SetThreadpoolWait(pTPWait, pDlg->hSemChildWrite.get(), NULL);
				}
			}

			// 読み込んだものをログへ書き込む
			{
				pDlgString_type* pDS = &((*pDlg->mrDlgStr.Lend())
					= { pDlg, str });
				TP_WAIT* pTPWaitLog(NULL);
				if (!(pTPWaitLog = CreateThreadpoolWait(
					pDlg->pfnLvSerializedWriteFromParentToLog
					, pDS
					, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError());
					pDlg->mrDlgStr.Return(pDS);
					CloseThreadpoolWait(Wait);
					return;
				}
				else {
					SetThreadpoolWait(pTPWaitLog, pDlg->hSemLv.get(), NULL);
				}
			}
		}
		SetThreadpoolWait(Wait, pDlg->hSemParentRead.get(), NULL);
	} }

	, pfnChildReadRepeat{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(Context);
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemChildRead.get(), 1);
		CallbackMayRunLong(Instance);

		if (pDlg->fEnd)
		{
			CloseThreadpoolWait(Wait);
			return;
		}
		pDlg->ChildReadBuf.resize(BUFFER_SIZE,'\0');
		DWORD cbRead(0);
		if (!ReadFile(pDlg->hUpRd, pDlg->ChildReadBuf.data(), pDlg->ChildReadBuf.size(), &cbRead, NULL))
		{
			ErrOut(GetLastError());
			CloseThreadpoolWait(Wait);
			return;
		}
		if (pDlg->fEnd)
		{
			CloseThreadpoolWait(Wait);
			return;
		}
		pDlg->ChildReadBuf.resize(cbRead);
		pDlg->ChildRemBuf += pDlg->ChildReadBuf;

		vector<string> vstr = DUSI_COMMON::vSplitLineBreakWithLBCodeA(pDlg->ChildRemBuf);
		for (string& str : vstr)
		{
			{
				// 親へ書き込む
				pDlgString_type* pDSWr
					= &((*pDlg->mrDlgStr.Lend())
						= { pDlg,str });
				TP_WAIT* pTPWait(NULL);
				if (!(pTPWait = CreateThreadpoolWait(
					pDlg->pfnSerializedWriteToParent, pDSWr, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError());
					CloseThreadpoolWait(Wait);
					return;
				}
				// 排他制御で親へ書き込む。
				SetThreadpoolWait(pTPWait, pDlg->hSemParentWrite.get(), NULL);
			}
			{
				// 読み込んだものをログへ書き込む
				pDlgString_type* pDSWr
					= &((*pDlg->mrDlgStr.Lend())
						= { pDlg,str });
				TP_WAIT* pTPWaitLog(NULL);
				if (!(pTPWaitLog = CreateThreadpoolWait(
					pDlg->pfnLvSerializedWriteFromChildToLog, pDSWr, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError());
					CloseThreadpoolWait(Wait);
					return;
				}
				// リストビュー操作になる為そのセマフォで制御
				SetThreadpoolWait(pTPWaitLog, pDlg->hSemLv.get(), NULL);
			}
		}
		SetThreadpoolWait(Wait, pDlg->hSemChildRead.get(), NULL);
	} }
	
	, pfnSerializedWriteToParent{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg = get<0>(*reinterpret_cast<pDlgString_type*>(Context));
		string str = move(get<1>(*reinterpret_cast<pDlgString_type*>(Context)));
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemParentWrite.get(), 1);
		DWORD Writen(0);
		if (!WriteFile(pDlg->hOut, str.data(), str.size(), &Writen, NULL))
		{
			ErrOut(GetLastError());
		}
		pDlg->mrDlgStr.Return((pDlgString_type* )Context);
		return;

	} }

	, pfnSerializedWriteToChild{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		unique_ptr<remove_pointer_t<pDlgString_type>
			, decltype(MainDialog::pfnDSDeleter)>puDS
			= { reinterpret_cast<pDlgString_type*>(Context)
			,get<0>(*reinterpret_cast<pDlgString_type*>(Context))->pfnDSDeleter	};
		MainDialog* pDlg = get<0>(*puDS.get());
		string strsrc = move(get<1>(*puDS.get()));
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemChildWrite.get(), 1);

		DWORD Writen(0);
		if (!WriteFile(pDlg->hDownWt, strsrc.data(), strsrc.size(), &Writen, NULL))
		{
			ErrOut(GetLastError());
			return;
		}
	} }

	, pfnLvSerializedWriteFromParentToLog{
	[](
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		unique_ptr<remove_pointer_t<pDlgString_type>
			, decltype(MainDialog::pfnDSDeleter)>puDS
			= {reinterpret_cast<pDlgString_type *>(Context)
			, get<0>(*reinterpret_cast<pDlgString_type *>(Context))->pfnDSDeleter};
		_Post_ _Notnull_ MainDialog *pDlg = get<0>(*puDS.get());
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemLv.get(), 1);

		// ListViewに書き込み。
		HWND hLv = GetDlgItem(pDlg->hDlgMain, IDC_LIST1);
		vector<string> vs = move(vSplitLineBreakWithLBCodeA(get<1>(*puDS.get())));
		for (string& str : vs)
		{
			TCHAR strNumLine[16]{};
			_itot_s(pDlg->LineNum, strNumLine, (std::size)(strNumLine), 10);
			//アイテム設定
			LVITEM m =
			{ /*UINT   mask       */LVIF_TEXT | LVIF_PARAM
			, /*int    Lv_iPrevItem      */MAXINT
			, /*int    iSubItem   */0
			, /*UINT   state      */0
			, /*UINT   stateMask  */0
			, /*LPTSTR pszText    */strNumLine
			, /*int    cchTextMax */0
			, /*int    iImage     */0
			, /*LPARAM lParam     */(LPARAM)pDlg->LineNum++
			};
			INT Index = ListView_InsertItem(hLv, &m);
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Device
				, const_cast<LPTSTR>(_T("Parent")));
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Dir
				, const_cast<LPTSTR>(_T(">")));
			tstring tstr = AtoT(str);
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Text
				, const_cast<LPTSTR>(tstr.c_str()));

			// ログファイルに書き込み。
			if (pDlg->hLog)
			{
				stringstream ss;
				ss << right << setw(6) << TtoA(strNumLine)
					<< noshowpos << " Parent > "
					<< ConvertLineEndingsWithCustomStringsA(str, "\\r", "\\n")
					<< "\r\n";

				OL_LOG* pOL = &(*pDlg->mrOLLog.Lend() = {});
				pOL->str = ss.str().c_str();
				StartThreadpoolIo(pDlg->pTP_IOLog);
				if (!WriteFile(
					pDlg->hLog
					, pOL->str.data()
					, pOL->str.size()
					, NULL
					, pOL)) {
					if (DWORD dw = GetLastError() != ERROR_IO_PENDING) {
						ErrOut(dw);
						pDlg->mrOLLog.Return(pOL);
						return;
					}
				}
			}
		}

		// 文字が残っていたらそれも書き込む
		string str;
		if ((str=move(get<1>(*puDS.get()))).size())
		{
			TCHAR strNumLine[16]{};
			_itot_s(pDlg->LineNum, strNumLine, (std::size)(strNumLine), 10);
			// アイテム設定
			LVITEM m =
			{ /*UINT   mask       */LVIF_TEXT | LVIF_PARAM
			, /*int    Lv_iPrevItem      */MAXINT
			, /*int    iSubItem   */0
			, /*UINT   state      */0
			, /*UINT   stateMask  */0
			, /*LPTSTR pszText    */strNumLine
			, /*int    cchTextMax */0
			, /*int    iImage     */0
			, /*LPARAM lParam     */(LPARAM)pDlg->LineNum++
			};
			INT Index = ListView_InsertItem(hLv, &m);
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Device
				, const_cast<LPTSTR>(_T("Parent")));
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Dir
				, const_cast<LPTSTR>(_T(">")));
			tstring tstr = AtoT(str);
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Text
				, const_cast<LPTSTR>(tstr.c_str()));
			if (pDlg->hLog)
			{
				stringstream ss;
				// ログファイルに書き込み。
				ss << right << setw(6) << TtoA(strNumLine)
					<< noshowpos << " Parent > "
					<< ConvertLineEndingsWithCustomStringsA(str, "\\r", "\\n")
					<< "\r\n";

				OL_LOG* pOL = &(*pDlg->mrOLLog.Lend() = {});
				pOL->str = ss.str().c_str();
				StartThreadpoolIo(pDlg->pTP_IOLog);
				if (!WriteFile(
					pDlg->hLog
					, pOL->str.data()
					, pOL->str.size()
					, NULL
					, pOL)) {
					if (DWORD dw = GetLastError() != ERROR_IO_PENDING) {
						ErrOut(dw);
						pDlg->mrOLLog.Return(pOL);
						return;
					}
				}
				SYSTEMTIME lt;
				GetLocalTime(&lt);
				//_D("pfnLvSerializedWriteFromParentToLog" + to_string(lt.wSecond)+"."+to_string(lt.wMilliseconds));
			}
		}

		// 最大表示行数まで削除する。
		int maxraw = ::SendMessage(
			GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_SPIN_MAX_RAW)
			, UDM_GETPOS32
			, 0
			, NULL);
		int c = ListView_GetItemCount(hLv);
		int over = c - maxraw;
		for (; 0 < over; --over)
		{
			ListView_DeleteItem(hLv, 0);
		}
		// リストビューをスクロールさせるか判定
		if (ListView_GetNextItem(hLv, -1, LVNI_SELECTED) == -1)
		{
			ListView_EnsureVisible(hLv, ListView_GetItemCount(hLv) - 1, FALSE);
		}
		return;
	} }

	, pfnLvSerializedWriteFromChildToLog{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		unique_ptr<remove_pointer_t<pDlgString_type>
			, decltype(MainDialog::pfnDSDeleter)>puDS
			= { reinterpret_cast<pDlgString_type*>(Context)
			,get<0>(*reinterpret_cast<pDlgString_type*>(Context))->pfnDSDeleter };
		_Post_ _Notnull_ MainDialog* pDlg = get<0>(*puDS.get());
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemLv.get(), 1);
		if (WaitResult == WAIT_TIMEOUT)
		{
			return;
		}

		vector<string> vs = move(vSplitLineBreakWithLBCodeA(get<1>(*puDS.get())));
		// ListViewに書き込み。
		HWND hLv = GetDlgItem(pDlg->hDlgMain, IDC_LIST1);
		for (string& str : vs)
		{
			TCHAR strNumLine[16]{};
			_itot_s(pDlg->LineNum, strNumLine, (std::size)(strNumLine), 10);
			// アイテム設定
			LVITEM m =
			{ /*UINT   mask       */LVIF_TEXT | LVIF_PARAM
			, /*int    Lv_iPrevItem      */MAXINT
			, /*int    iSubItem   */0
			, /*UINT   state      */0
			, /*UINT   stateMask  */0
			, /*LPTSTR pszText    */strNumLine
			, /*int    cchTextMax */0
			, /*int    iImage     */0
			, /*LPARAM lParam     */(LPARAM)pDlg->LineNum++
			};
			INT Index = ListView_InsertItem(hLv, &m);
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Device
				, const_cast<LPTSTR>(_T("Child")));
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Dir
				, const_cast<LPTSTR>(_T("<")));
			tstring tstr = move(AtoT(str));
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Text
				, const_cast<LPTSTR>(tstr.c_str()));

			// ログファイルに書き込み。
			if(pDlg->hLog)
			{
				stringstream ss;
				ss << right << setw(6) << TtoA(strNumLine)
					<< noshowpos << "  Child < "
					<< ConvertLineEndingsWithCustomStringsA(str, "\\r", "\\n")
					<< "\r\n";
				OL_LOG* pOL = &(*pDlg->mrOLLog.Lend() = {});
				pOL->str = move(ss.str().c_str());
				if (!pDlg->pTP_IOLog)
				{
					return;
				}
				StartThreadpoolIo(pDlg->pTP_IOLog);
				if (!WriteFile(
					pDlg->hLog
					, pOL->str.data()
					, pOL->str.size()
					, NULL
					, pOL)) {
					if (DWORD dw = GetLastError() != ERROR_IO_PENDING) {
						ErrOut(dw);
						pDlg->mrOLLog.Return(pOL);
						return;
					}
				}
			}
		}
		// 最大表示行数まで削除する。
		int maxraw = ::SendMessage(
			GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_SPIN_MAX_RAW)
			, UDM_GETPOS32
			, 0
			, NULL);
		int c = ListView_GetItemCount(hLv);
		int over = c - maxraw;
		for (; 0 < over; --over)
		{
			ListView_DeleteItem(hLv, 0);
		}
		// リストビューをスクロールさせるか判定
		if (ListView_GetNextItem(hLv, -1, LVNI_SELECTED) == -1)
		{
			ListView_EnsureVisible(hLv, ListView_GetItemCount(hLv) - 1, FALSE);
		}
		return;
	} }

	, pfnLvEndLog{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(Context);
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemLv.get(), 1);
		if (WaitResult == WAIT_TIMEOUT)
		{
			return;
		}
		if (pDlg->hLog)
		{
			WaitForThreadpoolIoCallbacks(pDlg->pTP_IOLog, TRUE);
			CloseHandle(pDlg->hLog);
			pDlg->hLog = NULL;
		}
	}}

	, pfnCompletedReadRoutine{
	[](
		 _Inout_     PTP_CALLBACK_INSTANCE Instance,
		 _Inout_opt_ PVOID                 Context,
		 _Inout_opt_ PVOID                 Overlapped,
		 _In_        ULONG                 IoResult,
		 _In_        ULONG_PTR             NumberOfBytesTransferred,
		 _Inout_     PTP_IO                Io
	)
	{
		CloseThreadpoolIo(Io);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg(reinterpret_cast<MainDialog*>(Context));
		if (!Overlapped)
		{
			ErrMes("Overlapped is NULL.");
			return;
		}
		OL_CON_HDL_ROUTINE* pOLCompletedReadRoutine(
			reinterpret_cast<OL_CON_HDL_ROUTINE*>(Overlapped));
		pOLCompletedReadRoutine->pTPIO = NULL;
		if (IoResult != NO_ERROR)
		{
			ErrOut(IoResult);
			return;
		}
		pOLCompletedReadRoutine->str.resize(NumberOfBytesTransferred);

		// Childへ転送
		{
			pDlgString_type* pDS = &((*pDlg->mrDlgStr.Lend())
				= { pDlg, pOLCompletedReadRoutine->str });

			TP_WAIT* pTPWait(NULL);
			if (!(pTPWait = CreateThreadpoolWait(
				pDlg->pfnSerializedWriteToChild
				, pDS
				, &*pDlg->pcbe)))
			{
				ErrOut(GetLastError());
				pDlg->mrDlgStr.Return(pDS);
				return;
			}
			SetThreadpoolWait(pTPWait, pDlg->hSemChildWrite.get(), NULL);
		}

		// 読み込んだものをログへ書き込む
		{
			pDlgString_type* pDS = &((*pDlg->mrDlgStr.Lend())
				= { pDlg, pOLCompletedReadRoutine->str });
			TP_WAIT* pTPWaitLog(NULL);
			if (!(pTPWaitLog = CreateThreadpoolWait(
				pDlg->pfnLvSerializedWriteFromParentToLog
				, pDS
				, &*pDlg->pcbe)))
			{
				ErrOut(GetLastError());
				pDlg->mrDlgStr.Return(pDS);
				return;
			}
			SetThreadpoolWait(pTPWaitLog, pDlg->hSemLv.get(), NULL);
		}
		
		// 切断して、再接続待ちをする。
		DisconnectNamedPipe(pOLCompletedReadRoutine->hPipe);
		pDlg->olConConnectedComp = {};
		pDlg->olConConnectedComp.hEvent = pDlg->hEvOL.get();
		ResetEvent(pDlg->olConConnectedComp.hEvent);
		if (!(pDlg->olConConnectedComp.pTPWait = CreateThreadpoolWait(
			pDlg->pfnCompletedConConnectRoutine
			, pDlg
			, &*pDlg->pcbe)))
		{
			ErrOut(GetLastError());
			return ;
		}
		SetThreadpoolWait(
			pDlg->olConConnectedComp.pTPWait
			, pDlg->olConConnectedComp.hEvent
			, NULL);

		if (!ConnectNamedPipe(pOLCompletedReadRoutine->hPipe, pOLCompletedReadRoutine)) {
			if (DWORD dw = GetLastError() != ERROR_IO_PENDING) {
				ErrOut(dw);
				MessageBox(pDlg->hDlgMain
					, _T("ConnectNamedPipe Err.")
					, _T("USIDebugStub")
					, MB_OK | MB_ICONEXCLAMATION);
				CancelIoEx(pOLCompletedReadRoutine->hPipe, NULL);
				CloseHandle(pOLCompletedReadRoutine->hPipe);
				pOLCompletedReadRoutine->hPipe = NULL;
				return;
			}
		}

	} }

	, pfnCompletedConConnectRoutine{
	[](
	 _Inout_     PTP_CALLBACK_INSTANCE Instance,
	 _Inout_opt_ PVOID                 Context,
	 _Inout_     PTP_WAIT              Wait,
	 _In_        TP_WAIT_RESULT        WaitResult
	) {
		CloseThreadpoolWait(Wait);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		if (WaitResult != WAIT_OBJECT_0)
		{
			ErrOut(WaitResult);
			return;
		}
		MainDialog* pDlg(reinterpret_cast<MainDialog*>(Context));
		if (pDlg->olConConnectedComp.bEnd)
		{
			_D("bEnd flag");
			return;
		}
		if (!pDlg->olConConnectedComp.hPipe)
		{
			ErrMes("pDlg->olcon.hPipe is NULL.");
			return;
		}

		pDlg->olConReadComp ={};
		pDlg->olConReadComp.hPipe = pDlg->olConConnectedComp.hPipe;
		// スレッドプールリードオブジェクト作成
		if (!(pDlg->olConReadComp.pTPIO = CreateThreadpoolIo(
			pDlg->olConConnectedComp.hPipe
			, pDlg->pfnCompletedReadRoutine
			, pDlg
			, &*pDlg->pcbe)))
		{
			ErrOut(GetLastError());
			return ;
		}

		StartThreadpoolIo(pDlg->olConReadComp.pTPIO);
		pDlg->olConReadComp.str.resize(BUFFER_SIZE,'\0');
		if (!ReadFile(	
			pDlg->olConReadComp.hPipe
			, pDlg->olConReadComp.str.data()
			, pDlg->olConReadComp.str.size()
			, NULL
			, &pDlg->olConReadComp)) {
			if (DWORD dw = GetLastError() != ERROR_IO_PENDING) {
				CancelThreadpoolIo(pDlg->olConReadComp.pTPIO);
				WaitForThreadpoolIoCallbacks(pDlg->olConReadComp.pTPIO, TRUE);

				ErrOut(dw);
				pDlg->olConReadComp.pTPIO = NULL;
				return;
			}
		}
	} }

	, pfnAdjustUILayoutW{
	[](
	 _Inout_     PTP_CALLBACK_INSTANCE Instance,
	 _Inout_opt_ PVOID                 Context,
	 _Inout_     PTP_WORK              Work
	) 
	{
		CloseThreadpoolWork(Work);
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		const MainDialog* pDlg(reinterpret_cast<MainDialog*>(Context));

		const RECT rcDlgCli;
		if (!GetClientRect(pDlg->hDlgMain, const_cast<RECT*>(&rcDlgCli)))
		{
			EOut;
			return;
		}

		// UIダイアログRECTを求める。
		const RECT rcDlgUI;
		UIDialog* pUI(NULL);
		{
			pUI = Button_GetCheck(GetDlgItem(pDlg->pNormalUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE))
				== BST_CHECKED ?
				pDlg->pMinimumUIDlg.get()
				: pDlg->pNormalUIDlg.get();
			GetWindowRect(pUI->hDlg, const_cast<RECT*>(&rcDlgUI));
		}
		if (!::MapWindowRect(
			HWND_DESKTOP
			, pDlg->hDlgMain
			, const_cast<RECT*>(&rcDlgUI)))
		{
			EOut;
			return;
		}
		OffsetRect(const_cast<RECT*>(&rcDlgUI), -rcDlgUI.left, -rcDlgUI.top);

		// UIダイアログ位置設定
		if (!SetWindowPos(
			pUI->hDlg
			, HWND_TOP
			, rcDlgUI.left
			, rcDlgCli.bottom - rcDlgUI.bottom
			, 0
			, 0
			,  SWP_NOSIZE
			|SWP_NOZORDER
			| SWP_ASYNCWINDOWPOS
		))
		{
			ErrOut(GetLastError());
			return;
		}

	// List1設定
		const HWND hList = GetDlgItem(pDlg->hDlgMain, IDC_LIST1);
		if (!hList) {
			EOut;
			return;
		}

		// List1、RECTを求める。
		const RECT rcListWin;
		if (!::GetWindowRect(hList, const_cast<RECT*>(&rcListWin)))
		{
			EOut;
			return;
		}
		if (!::MapWindowRect(
			HWND_DESKTOP
			, pDlg->hDlgMain
			, const_cast<RECT*>(&rcListWin)))
		{
			EOut;
			return;
		}

		// List1位置設定。
		if (!SetWindowPos(
			hList
			, HWND_TOP
			, rcListWin.left
			, rcListWin.top
			, rcDlgCli.right - rcDlgCli.left - (rcListWin.left) * 2
			, rcDlgCli.bottom - rcDlgUI.bottom - rcListWin.top
			, SWP_NOZORDER))
		{
			ErrOut(GetLastError());
			return;
		}

		const RECT rcListCliAfter{};
		if (!GetClientRect(hList, const_cast<RECT*>(&rcListCliAfter)))
		{
			ErrOut(GetLastError());
			return;
		}
		// リストビューヘッダー等の設定
		{
			// アラインヘッダー
			{
				const HWND hwndHeader = ListView_GetHeader(hList);
				// Line項目の幅の計算。
				const HDC hDC = GetDC(hList);
				const HFONT hfont = (HFONT)SendMessage(pDlg->hDlgMain, WM_GETFONT, 0, 0);
				const HFONT hfontprev = SelectFont(hDC, hfont);
				SIZE sz{};
				tstring strH0(_T("1000000"));
				GetTextExtentPoint32(hDC, strH0.c_str(), strH0.size(), &sz);
				SelectFont(hDC, hfontprev);
				ReleaseDC(hList, hDC);
				ListView_SetColumnWidth(hList, 0, sz.cx);
				const int cnt = Header_GetItemCount(hwndHeader);
				for (int i = 1; i < cnt - 1; ++i)
				{
					ListView_SetColumnWidth(hList, i, LVSCW_AUTOSIZE_USEHEADER);
				}

				// 最後の列の幅の計算
				LONG lDeduct(0);
				for (int i = 0; i < cnt - 1; ++i)
				{
					LONG lW = (LONG)ListView_GetColumnWidth(hList, i);
					lDeduct += lW;
				}
				// 縦スクロールバーが表示されているかいないかの判定。
				SCROLLINFO si = { sizeof(SCROLLINFO) };
				si.fMask = SIF_RANGE | SIF_PAGE;
				GetScrollInfo(hList, SB_VERT, &si);
				if (si.nMax < (int)si.nPage)
				{
					lDeduct += GetSystemMetrics(SM_CXVSCROLL);
				}

				// ヘッダーの最後の列の幅設定
				ListView_SetColumnWidth(hList, cnt - 1, rcListCliAfter.right - lDeduct);
				if (!::SetWindowPos(
					hwndHeader
					, HWND_TOP
					, 0
					, 0
					, 0
					, 0
					, SWP_NOSIZE
					| SWP_NOZORDER))
				{
					ErrOut(GetLastError());
					return;
				}
			}
		}

		// リドロー領域を求めてそこだけリドローする。
		{
			const RECT rcListWinAfter;
			if (!::GetWindowRect(hList, const_cast<RECT*>(&rcListWinAfter)))
			{
				EOut;
				return;
			}
			if (!::MapWindowRect(
				HWND_DESKTOP
				, pDlg->hDlgMain
				, const_cast<RECT*>(&rcListWinAfter)))
			{
				EOut;
				return;
			}

			if (rcListWinAfter.bottom - rcListWin.bottom)
			{
				RECT rcDef{
					0
					,rcListWin.bottom - rcListWin.top
					,rcListWinAfter.right - rcListWinAfter.left
					,rcListWinAfter.bottom - rcListWinAfter.top
				};
				RedrawWindow(
					hList
					, &rcDef
					, NULL
					, RDW_INVALIDATE);
			}
		}
	// リストビュー設定終わり。
	} }

	, pfnCompletedWriteLogMR{
	[] (
		 _Inout_     PTP_CALLBACK_INSTANCE Instance,
		 _Inout_opt_ PVOID                 Context,
		 _Inout_opt_ PVOID                 Overlapped,
		 _In_        ULONG                 IoResult,
		 _In_        ULONG_PTR             NumberOfBytesTransferred,
		 _Inout_     PTP_IO                Io
	)
	{
		if (!Context)
		{
			ErrMes("Context is NULL.");
			return;
		}
		MainDialog* pDlg(reinterpret_cast<MainDialog*>(Context));
		if (!Overlapped)
		{
			ErrMes("Overlapped is NULL.");
			return;
		}
		pDlg->mrOLLog.Return((OL_LOG*)Overlapped);
	} }

	, pfnCGCC{
	[] (
		 _Inout_opt_ PVOID ObjectContext,
		 _Inout_opt_ PVOID CleanupContext
	)
	{
			ObjectContext;
	}}

{
//**************コンストラクタ************************
	// EscCodeにmapChToStrのkeyを書き込む
	for (const auto& [key, value] : mapChToStrT)
	{
		const_cast<DUSI_COMMON::tstring*>(&EscCode)->push_back(key);
	}
	mrDlgStr.DebugString("DlgStr");
	//mrString.DebugString("OverLappedIo String");
	mrOLLog.DebugString("mrOLLog");
}

UsiDebugStb::MainDialog::~MainDialog()
{

}

tstring UsiDebugStb::MainDialog::MakeDispLogFileName()
{
	tstring tstrLogFileName(BUFFER_SIZE, _T('\0'));
	tstrLogFileName.resize(GetPrivateProfileString(
		_T("MAIN_DLG")
		, _T("LOG_FILE_NAME")
		, _T("test")
		, tstrLogFileName.data()
		, tstrLogFileName.size()
		, MakeIniPath().c_str()));
	DWORD err = GetLastError();
	tstring tstrLogFileExt(BUFFER_SIZE, _T('\0'));
	tstrLogFileExt.resize(GetPrivateProfileString(
		_T("MAIN_DLG")
		, _T("LOG_FILE_EXT")
		, _T(".log")
		, tstrLogFileExt.data()
		, tstrLogFileExt.size()
		, MakeIniPath().c_str()));
	// ファイルネームに時間が加えられているか調べる。
	string strTimeFormat;
	HWND hCheckAddTimeStamp = GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_ADD_TIME_STAMP);
	if (Button_GetCheck(hCheckAddTimeStamp) == BST_CHECKED)
	{
		strTimeFormat = MakeTimeFormat();
	}
	else {
		strTimeFormat = "";
	}
	return tstrLogFileName + AtoT(strTimeFormat) == _T("")
		? tstring(_T(""))
		: tstrLogFileExt == _T("") ?
		_T("")
		: tstrLogFileName + AtoT(strTimeFormat) + tstrLogFileExt;
}

BOOL UsiDebugStb::MainDialog::ParentRead()
{
	TP_WAIT* pTPWait(NULL);
	if (!(pTPWait = CreateThreadpoolWait(pfnParentReadRepeat, this, &*pcbe)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SetThreadpoolWait(pTPWait, hSemParentRead.get(), NULL);
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::ChildRead()
{
	TP_WAIT* pTPWait(NULL);
	if (!(pTPWait = CreateThreadpoolWait(pfnChildReadRepeat, this, &*pcbe)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SetThreadpoolWait(pTPWait, hSemChildRead.get(), NULL);
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::DoExecute()
{
	// 実行中でないか確認。
	if (PI.hProcess)
	{
		return FALSE;
	}
	// パスが設定されているか確認。
	{
		tstring tstrPath(MAX_PATH, _T('\0'));
		tstrPath.resize(
			::GetWindowText(
				GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_EXE_PATH_NAME)
				, tstrPath.data()
				, tstrPath.size()));
		if (tstrPath == _T(""))
		{
			MessageBox(hDlgMain
						  , _T("実行ファイルのパスが設定されていません。")
						  , _T(_STRINGIZE(APPNAME))
						  , MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	// フラグエンドリセット
	fEnd.store(0);
	INVALID_HANDLE_VALUE;
	// 現在の標準ハンドルを保存
//	_D("GetStdHandle(STD_INPUT_HANDLE)");
	if ((hPrevIn = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	if ((hPrevOut = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	// IOリダイレクトを受け入れる場合
	if (Button_GetCheck(
		GetDlgItem(
			pNormalUIDlg->hDlg
			, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT)) == BST_CHECKED)
	{
		// 標準入力取得
		if (!(hIn = GetStdHandle(STD_INPUT_HANDLE)))
		{
			ErrOut(GetLastError());
			MessageBox(hDlgMain
				, _T("標準入力を取得できませんでした。")
				, _T(_STRINGIZE(APPNAME))
				, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		// 標準出力取得
		if (!(hOut = GetStdHandle(STD_OUTPUT_HANDLE)))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
	}
	else
	{
		// IOリダイレクションを受け入れない場合、
		// コンソールを立ち上げる
		if (!AllocConsole())
		{
			if (!AttachConsole(ATTACH_PARENT_PROCESS))
			{
				ErrOut(GetLastError());
				return FALSE;
			}
		}
		if (!SetConsoleCtrlHandler(pfnConsoleHandlerRoutine, TRUE)) {
			ErrOut(GetLastError());
			return FALSE;
		}


		if (!(hwndCon = GetConsoleWindow()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		{
			RECT rect{};
			GetWindowRect(hwndCon, &rect);
			// 閉じるボタンを削除
			if (!(::DeleteMenu(::GetSystemMenu(hwndCon, FALSE), SC_CLOSE, MF_BYCOMMAND)))
			{
				ErrOut(GetLastError());
				return FALSE;
			}
		}

		// コンソールポジションを設定
		RECT rect{};
		if (GetPrivateProfileStruct(
			_T("CONSOLE")
			, _T("WINDOW_POS")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			if (!::SetWindowPos(hwndCon
				, HWND_TOPMOST
				, rect.left
				, rect.top
				, 0
				, 0
				, SWP_SHOWWINDOW | SWP_NOSIZE))
			{
				ErrOut(GetLastError());
				return FALSE;
			}
		}
		SetWindowAppropriatePositionEx(hwndCon);

// コンソール表示
		ShowWindow(
			hwndCon
			, Button_GetCheck(
				GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_PROMPT)
			) == BST_CHECKED ? SW_SHOW : SW_HIDE);

		if ((hIn = CreateFileA("CONIN$"
			, GENERIC_READ
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, NULL
			, NULL)) == INVALID_HANDLE_VALUE)
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		if ((hOut = CreateFileA("CONOUT$"
			, GENERIC_WRITE
			, FILE_SHARE_WRITE
			, NULL
			, OPEN_EXISTING
			, NULL
			, NULL)) == INVALID_HANDLE_VALUE)
		{
			ErrOut(GetLastError());
			return FALSE;
		}
	}

	//if (!SetConsoleMode(hIn, ENABLE_LINE_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT))
	//{
	//	EOut;
	//	return FALSE;
	//}

	// InputDialogからのメッセージを受け取る為の
	// NamedPipe作成し接続を待つ
	if(hwndCon)
	{
		// プロセスIDを取得する
		DWORD pid;
		if (!GetWindowThreadProcessId(hwndCon, &pid))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		olConConnectedComp = {};
		olConConnectedComp.hEvent = hEvOL.get();
		ResetEvent(olConConnectedComp.hEvent);
		// プロセスIDを名前に加える。
		string pipename(string("\\\\.\\pipe\\")\
			+ _STRINGIZE(NAMED_PIPE_PREFIX) + to_string((int)pid));
		if ((olConConnectedComp.hPipe = CreateNamedPipe(
			AtoT(pipename).c_str()// パイプ名
			, PIPE_ACCESS_DUPLEX // インバウンド
			| FILE_FLAG_OVERLAPPED // オーバーラップモード
			, PIPE_TYPE_BYTE
			| PIPE_READMODE_BYTE // バイト単位で受信
			| PIPE_ACCEPT_REMOTE_CLIENTS
			, 2// 最大同時接続数
			, 1024// 出力バッファサイズ
			, 1024// 入力バッファサイズ
			, 0// タイムアウト時間（0はデフォルト）
			, NULL // セキュリティ属性（NULLはデフォルト）
		)) == INVALID_HANDLE_VALUE)
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		// クライアントからの接続を待機する
		if (!(olConConnectedComp.pTPWait = CreateThreadpoolWait(pfnCompletedConConnectRoutine, this, &*pcbe)))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		SetThreadpoolWait(olConConnectedComp.pTPWait, olConConnectedComp.hEvent, NULL);
		olConConnectedComp.bEnd = TRUE;
		if (!ConnectNamedPipe(olConConnectedComp.hPipe, &olConConnectedComp)) {
			if (DWORD dw = GetLastError() != ERROR_IO_PENDING) {
				ErrOut(dw);
				CloseHandle(olConConnectedComp.hPipe);
				MessageBox(hDlgMain
					, _T("名前付きパイプを接続出来ませんでした。")
					, _T("USIDebugStub")
					, MB_OK | MB_ICONEXCLAMATION);
				::SendMessage(hwndCon, WM_CLOSE, 0, 0);
				return FALSE;
			}
		}
		olConConnectedComp.bEnd = FALSE;
	}

	// ボタン無効化
	Button_Enable(GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT), FALSE);
	Button_Enable(GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT), FALSE);

	SECURITY_ATTRIBUTES saAttr = {};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hDownRd, &hDownWt, &saAttr, 0))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	if (!SetHandleInformation(hDownWt, HANDLE_FLAG_INHERIT, 0))
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	if (!CreatePipe(&hUpRd, &hUpWt, &saAttr, 0))
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	if (!SetHandleInformation(hUpRd, HANDLE_FLAG_INHERIT, 0))
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	DWORD dwMode = PIPE_WAIT;
	if (!SetNamedPipeHandleState(hUpRd, &dwMode, NULL, NULL))
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	tstring tstrPath(MAX_PATH, _T('\0'));
	tstrPath.resize(::GetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_EXE_PATH_NAME)
		, tstrPath.data()
		, tstrPath.size()));

	tstring tstrOption(MAX_PATH - tstrPath.size(), _T('\0'));
	tstrOption.resize(::GetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_EDIT_EXE_OPTION_STRING)
		, tstrOption.data()
		, tstrOption.size()));
	if(tstrOption.size())
	{
		tstrPath += _T(" ") + tstrOption;
	}

	STARTUPINFO siStartInfo = {};
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = hUpWt;
	siStartInfo.hStdOutput = hUpWt;
	siStartInfo.hStdInput = hDownRd;
	siStartInfo.wShowWindow
		= Button_GetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_HIDE_CHILD))
		== BST_CHECKED ? SW_HIDE : SW_SHOWNORMAL;
	siStartInfo.dwFlags	= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	const filesystem::path Exepath(tstrPath) ;
	if (!CreateProcess(NULL,
		tstrPath.data(),     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		Exepath.parent_path().c_str(), // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&PI))  // receives PROCESS_INFORMATION
	{
		ErrOut(GetLastError());
		MessageBox(hDlgMain
			, _T("プロセスを作成できませんでした。")
			, _T("USIDebugStub")
			, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	// パイプ親プロセスでは使わないのでClose。
	CloseHandle(hDownRd);
	CloseHandle(hUpWt);

	// ChildProcessの起動状態監視用スレッド。
	TP_WAIT *pTPWait(NULL);
	if (!(pTPWait = CreateThreadpoolWait(pfnDetectEnd, this, &*pcbe)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SetThreadpoolWait(pTPWait, PI.hProcess, NULL);

	// ログファイル作成するかどうか。
	if (Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_MAKE_LOG_FILE)) == BST_CHECKED)
	{
		tstring tstrPath(BUFFER_SIZE, _T('\0'));
		tstrPath.resize(
			::GetWindowText(
				::GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_PATH)
				, tstrPath.data()
				, tstrPath.size()));
		BOOL bAppMode = Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_LOG_FILE_APP_MODE))== BST_CHECKED;
		if ((hLog = CreateFile(
			(tstrPath + MakeDispLogFileName()).c_str()
			, bAppMode ? FILE_APPEND_DATA : FILE_APPEND_DATA
			, 0
			, nullptr
			, bAppMode ? OPEN_ALWAYS : CREATE_ALWAYS
			, FILE_FLAG_OVERLAPPED
			, nullptr)) == INVALID_HANDLE_VALUE)
		{
			DWORD dwE = GetLastError();
			ErrOut(dwE);
			if (dwE == 3)
			{
				MessageBox(hDlgMain
					, _T("指定されたログパスが見つかりません。")
					, _T("USIDebugStub")
					, MB_OK | MB_ICONEXCLAMATION);
				hLog = NULL;
			}
			else {
				return FALSE;
			}
		}
		else {
			if (!(pTP_IOLog = CreateThreadpoolIo(
				hLog
				, pfnCompletedWriteLogMR
				, this
				, &*pcbe
			)))
			{
				EOut;
				return FALSE;
			}
		}
	}
	else {
		hLog = NULL;
	}

	// ブリンキングスタティック有効化
	BS.Attach(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_CHILD_IS_RUNNING)
		, IDC_STATIC_CHILD_IS_RUNNING);
	ChildRead();
	ParentRead();

	// タスクトレイティップ文字変更
	TCHAR	tszFileName[_MAX_PATH];
	TCHAR	drive[_MAX_DRIVE];
	TCHAR	dir[_MAX_DIR];
	TCHAR	fname[_MAX_FNAME];
	TCHAR	ext[_MAX_EXT];
	_tsplitpath_s(tstrPath.c_str()
					  , drive, _MAX_DRIVE
					  , dir, _MAX_DIR
					  , fname, _MAX_FNAME
					  , ext, _MAX_EXT);
	_tmakepath_s(tszFileName
					 , _MAX_PATH
					 , _T("")
					 , _T("")
					 , fname
					 , ext);
	tstring strTip =
		tstring(_T(_CRT_STRINGIZE(APPNAME)))
		+ _T("-")
		+ tstring(tszFileName);
	_tcscpy_s(nid.szTip, 128, strTip.c_str());
	if (Button_GetCheck(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_DISP_TASK_TRAY))
		== BST_CHECKED)
	{
		int ret = (int)Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
	// メインダイアログタイトルを変更。
	SetWindowText(hDlgMain, strTip.c_str());

	// Executeボタン無効
	Button_Enable(GetDlgItem(pNormalUIDlg->hDlg, IDC_BUTTON_EXECUTE), FALSE);

	// Terminateボタン有効
	Button_Enable(GetDlgItem(pNormalUIDlg->hDlg, IDC_BUTTON_TERMINATE_PROCESS), TRUE);

	// インプットダイアログ送信ボタン有効化
	pInputDlg->EnableSendButton(TRUE);
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::ChangeSimpleUIMode()
{
	// ダイアログアイテムをSW_HIDEにする。
	ShowWindow(pNormalUIDlg->hDlg, SW_HIDE);
	ShowWindow(pMinimumUIDlg->hDlg, SW_SHOW);
	RECT rcNormal;
	GetWindowRect(pNormalUIDlg->hDlg, &rcNormal);
	OffsetRect(const_cast<RECT*>(&rcNormal), -rcNormal.left, -rcNormal.top);
	RECT rcMinimum;
	GetWindowRect(pMinimumUIDlg->hDlg, &rcMinimum);
	OffsetRect(const_cast<RECT*>(&rcMinimum), -rcMinimum.left, -rcMinimum.top);
	LONG dest = rcNormal.bottom - rcMinimum.bottom;
	RECT rcDlg;
	GetWindowRect(hDlgMain, &rcDlg);
	SetWindowPos(
		hDlgMain
		, HWND_NOTOPMOST
		, 0
		, 0
		, rcDlg.right - rcDlg.left
		, rcDlg.bottom - rcDlg.top - (rcNormal.bottom - rcMinimum.bottom)
		, SWP_NOMOVE|SWP_NOZORDER);

	const RECT rcDlgNew;
	GetWindowRect(hDlgMain, const_cast<RECT*>(&rcDlgNew));
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("WINDOW_POS")
		, const_cast<RECT*>(&rcDlgNew)
		, sizeof(RECT)
		, MakeIniPath().c_str()))
	{
		EOut;
		MessageBox(hDlgMain
			, _T("WINDOW_POS書き込みエラー")
			, _T("USIDebugStub")
			, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	TP_WORK* pTPWork(NULL);
	if (!(pTPWork = CreateThreadpoolWork(pfnAdjustUILayoutW, this, &*pcbeUI)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SubmitThreadpoolWork(pTPWork);

	return 0;
}

BOOL UsiDebugStb::MainDialog::ChangeNormalUIMode()
{
	// ダイアログアイテムをSW_HIDEにする。
	ShowWindow(pNormalUIDlg->hDlg, SW_SHOW);
	ShowWindow(pMinimumUIDlg->hDlg, SW_HIDE);
	RECT rcNormal;
	GetWindowRect(pNormalUIDlg->hDlg, &rcNormal);
	OffsetRect(const_cast<RECT*>(&rcNormal), -rcNormal.left, -rcNormal.top);
	RECT rcMinimum;
	GetWindowRect(pMinimumUIDlg->hDlg, &rcMinimum);
	OffsetRect(const_cast<RECT*>(&rcMinimum), -rcMinimum.left, -rcMinimum.top);
	LONG dest = rcNormal.bottom - rcMinimum.bottom;
	const RECT rcDlg;
	GetWindowRect(hDlgMain, const_cast<RECT*>( & rcDlg));
	OffsetRect(const_cast<RECT*>(&rcDlg), -rcDlg.left, -rcDlg.top);
	SetWindowPos(
		hDlgMain
		, HWND_NOTOPMOST
		, 0
		, 0
		, rcDlg.right - rcDlg.left
		, rcDlg.bottom - rcDlg.top + (rcNormal.bottom - rcMinimum.bottom)
		, SWP_NOMOVE|SWP_NOZORDER);
	const RECT rcDlgNew;
	GetWindowRect(hDlgMain, const_cast<RECT*>(&rcDlgNew));
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("WINDOW_POS")
		, const_cast<RECT*>(& rcDlgNew)
		, sizeof(RECT)
		, MakeIniPath().c_str()))
	{
		EOut;
		MessageBox(hDlgMain
			, _T("WINDOW_POS書き込みエラー")
			, _T("USIDebugStub")
			, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	TP_WORK* pTPWork(NULL);
	if (!(pTPWork = CreateThreadpoolWork(pfnAdjustUILayoutW, this, &*pcbeUI)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SubmitThreadpoolWork(pTPWork);
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::SetInitialPlacement()
{
//	_D("SetInitialPlacement");
	// デザイン時のフォント取得
	LOGFONT lfBase;
	if (!GetPrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, &lfBase
		, sizeof(lfBase)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	HFONT hfont;
	if (!(hfont = CreateFontIndirect(&lfBase)))
	{
		_D("font error.");
		return FALSE;
	}

	// 各ダイアログアイテムにWM_SETFONT送信
	for (const auto& pair : vhrcDlgItemsBasePos)
	{
		::SendMessage(pair.first, WM_SETFONT, (WPARAM)hfont, TRUE);
	}
	
	//***********ベースサイズに戻す。***************
		// ダイアログをベースサイズに戻す。
	{
		RECT rect{};
		if (!GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_WINDOW_BASE_RECT")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		if (!::SetWindowPos(
			hDlgMain
			, 0
			, 0
			, 0
			, rect.right - rect.left
			, rect.bottom - rect.top
			, SWP_NOMOVE
			| SWP_NOZORDER
		))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
	}
	// ダイアログアイテムをベースポジションに戻す。
	for (const auto& dlgItem : vhrcDlgItemsBasePos)
	{
		if (!::SetWindowPos(
			dlgItem.first
			, 0
			, dlgItem.second.left
			, dlgItem.second.top
			, dlgItem.second.right - dlgItem.second.left
			, dlgItem.second.bottom - dlgItem.second.top
			, SWP_NOZORDER))
		{
			EOut;
			return FALSE;
		}
	}

	// UI子ダイアログのベースポジション移動
	{
	// 現在表示されているUIのRECTとハンドルを取得
		RECT rcUI;
		Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE))
			== BST_CHECKED ? GetWindowRect(pMinimumUIDlg->hDlg,&rcUI)
			: GetWindowRect(pNormalUIDlg->hDlg,&rcUI);
		MapWindowRect(HWND_DESKTOP, hDlgMain, &rcUI);
		OffsetRect(&rcUI, -rcUI.left, -rcUI.top);
		
		HWND hUIDlg = 
			Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE))
			== BST_CHECKED ? pMinimumUIDlg->hDlg : pNormalUIDlg->hDlg;


		// 子ダイアログをデザイン時のサイズ・位置に戻す。
		pNormalUIDlg->SetInitialPlacement();
		pMinimumUIDlg->SetInitialPlacement();
		RECT rcCli;
		if (!GetClientRect(hDlgMain, &rcCli))
		{
			ErrOut(GetLastError());
				return FALSE;
		}
		if (!::SetWindowPos(
			hUIDlg
			, HWND_TOP
			, rcCli.left
			, rcCli.bottom - rcUI.bottom
			, 0
			, 0
			, SWP_NOCOPYBITS
			| SWP_SHOWWINDOW
			| SWP_NOZORDER
			| SWP_NOSIZE
		))
		{
			EOut;
			return FALSE;
		}
	}
	//***********ベースサイズに戻す。終了***************

	// リストビューは特別設定
	{
		HWND hListView(NULL);
		if (!(hListView = GetDlgItem(hDlgMain, IDC_LIST1)))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		RECT rcListCli{};
		if (!GetClientRect(hListView, &rcListCli))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		RECT rcListWin;
		GetWindowRect(hListView, &rcListWin);
		MapWindowRect(HWND_DESKTOP, hListView, &rcListWin);

		// アラインヘッダー
		{
			HWND hwndHeader = ListView_GetHeader(hListView);
			// Line項目の幅の計算。
			HDC hDC = GetDC(hListView);
			HFONT hfontprev = SelectFont(hDC, hfont);
			SIZE sz{};
			tstring strH0(_T("1000000"));
			GetTextExtentPoint32(hDC, strH0.c_str(), strH0.size(), &sz);
			SelectFont(hDC, hfontprev);
			DeleteFont(hfont);
			ReleaseDC(hListView, hDC);
			ListView_SetColumnWidth(hListView, 0, sz.cx);
			int cnt = Header_GetItemCount(hwndHeader);
			for (int i = 1; i < cnt - 1; ++i)
			{
				ListView_SetColumnWidth(hListView, i, LVSCW_AUTOSIZE_USEHEADER);
			}

			LONG lDeduct(0);
			for (int i = 0; i < cnt - 1; ++i)
			{
				LONG lW = (LONG)ListView_GetColumnWidth(hListView, i);
				lDeduct += lW;
			}
			// 縦スクロールバーが表示されているかいないかの判定。
			SCROLLINFO si = { sizeof(SCROLLINFO) };
			si.fMask = SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hListView, SB_VERT, &si);
			if (si.nMax < (int)si.nPage)
			{
				lDeduct += GetSystemMetrics(SM_CXVSCROLL);
			}

			ListView_SetColumnWidth(hListView, cnt - 1, rcListCli.right - lDeduct);
		}
	} // リストビュー設定終わり。

	RedrawWindow(
		hDlgMain
		, NULL
		, NULL
		, RDW_ALLCHILDREN
		| RDW_ERASE
		| RDW_INVALIDATE
	);
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::ArrangeUI()
{
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::
OnNM_List1(HWND hDlg, WPARAM wParam, LPNMHDR pnmhdr)
{
	switch (pnmhdr->code) {
		// カスタムドロー関連
	case NM_CUSTOMDRAW: {
		LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pnmhdr;

		switch (lplvcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			return  CDRF_NOTIFYSUBITEMDRAW;

		case CDDS_POSTPAINT:
			_D("CDDS_POSTPAINT\r\n");
			return 0;

		case CDDS_PREERASE:
			_D("CDDS_PREERASE\r\n");
			return 0;

		case CDDS_POSTERASE:
			_D("CDDS_POSTERASE\r\n");
			return 0;

		case CDDS_ITEMPREPAINT:
			return CDRF_NOTIFYSUBITEMDRAW;

		case CDDS_ITEMPOSTPAINT:
			_D((string("CDDS_ITEMPOSTPAINT")\
				+ " dwItemSpec:"\
				+ to_string(lplvcd->nmcd.dwItemSpec)\
				+ " iSubItem:"
				+ to_string(lplvcd->iSubItem)\
				+ "\r\n").c_str());
			return CDRF_DODEFAULT;

		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			if (lplvcd->iSubItem == MainDialog::Text)
			{
				return CDRF_NOTIFYPOSTPAINT | CDRF_DOERASE | CDRF_NOTIFYPOSTERASE;
			}
			return  /*CDRF_NOTIFYPOSTPAINT*/0;

		case CDDS_SUBITEM | CDDS_ITEMPOSTPAINT:
			if (lplvcd->iSubItem == MainDialog::Text)
			{
				basic_string<TCHAR> tstr(BUFFER_SIZE, _T('\0'));
				LVITEM m = {};
				m =
				{ /*UINT   mask       */LVIF_TEXT
				, /*int    Lv_iPrevItem      */0
				, /*int    iSubItem   */lplvcd->iSubItem
				, /*UINT   state      */0
				, /*UINT   stateMask  */0
				, /*LPTSTR pszText    */tstr.data()
				, /*int    cchTextMax */static_cast<int>(tstr.capacity())
				, /*int    iImage     */0
				, /*LPARAM lParam     */0
				};
				// サブアイテムメッセージ取得
				tstr.resize(SendMessage(
					pnmhdr->hwndFrom
					, LVM_GETITEMTEXT
					, (WPARAM)lplvcd->nmcd.dwItemSpec
					, (LPARAM)&m));
				// テキストアラインモード設定
				UINT PrevTA = GetTextAlign(lplvcd->nmcd.hdc);
				MoveToEx(lplvcd->nmcd.hdc
					, lplvcd->nmcd.rc.left
					, lplvcd->nmcd.rc.top
					, NULL
				);
				SetTextAlign(lplvcd->nmcd.hdc
					, PrevTA & ~TA_UPDATECP | TA_UPDATECP);
				ParseStr(tstr, 0, EscCode, mapChToStrT, lplvcd);
				SetTextAlign(lplvcd->nmcd.hdc, PrevTA);
			}

			return CDRF_DODEFAULT;
		case CDDS_SUBITEM | CDDS_ITEMPREERASE:
			_D((string("CDDS_SUBITEM | CDDS_ITEMPREERASE  SubItem:")\
				+ to_string(lplvcd->iSubItem) + "\r\n").c_str());
			return /*CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT*/0;

		case CDDS_SUBITEM | CDDS_ITEMPOSTERASE:
			_D((string("CDDS_SUBITEM | CDDS_ITEMPOSTERASE  SubItem:")\
				+ to_string(lplvcd->iSubItem) + "\r\n").c_str());
			return /*CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT*/0;

		default:
			_D((to_string(lplvcd->nmcd.dwDrawStage)\
				+ "default\r\n").c_str());
			return 0;
		}
		return 0;
	}
							// セレクトされているアイテムがクリックされた場合、
							// セレクト解除する。
	case LVN_ITEMCHANGING: {
		NMLISTVIEW* pnmv = (LPNMLISTVIEW)pnmhdr;
		if (Lv_iPrevItem == pnmv->iItem)
		{
			if ((Lv_Status & LVNI_SELECTED) != (pnmv->uNewState & LVNI_SELECTED))
			{
				return 1;
			}
			else {
				return 0;
			}
		}
		return 0;
	}
	case LVN_ITEMCHANGED: {
		return 1;
	}
	case LVN_INSERTITEM: {
		return 1;
	}
	case LVN_GETEMPTYMARKUP: {
		NMLISTVIEW* pnmv = (NMLISTVIEW*)pnmhdr;
		//_D((string("LVN_GETEMPTYMARKUP")\
		//	+ " pnmv->Lv_iPrevItem:"\
		//	+ to_string(pnmv->iItem)\
		//	+ "\r\n").c_str());
		return 0;
	}
	case LVM_REMOVEALLGROUPS: {
		NMLISTVIEW* pnmv = (NMLISTVIEW*)pnmhdr;
		_D((string("LVM_REMOVEALLGROUPS")\
			+ " pnmv->Lv_iPrevItem:"\
			+ to_string(pnmv->iItem)\
			+ "\r\n").c_str());
		return 0;
	}
	case NM_CLICK: {
		LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmhdr;
		UINT state =
			ListView_GetItemState(
				pnmhdr->hwndFrom
				, lpnmitem->iItem
				, LVNI_SELECTED);
		Lv_iPrevItem = -1;
		Lv_Status = 0;
		ListView_SetItemState(
			pnmhdr->hwndFrom
			, -1
			, LVNI_SELECTED
			, ~LVNI_SELECTED);
		if (lpnmitem->iItem == Lv_iPrevItem && Lv_Status != state)
		{
			ListView_SetItemState(
				pnmhdr->hwndFrom
				, lpnmitem->iItem
				, LVNI_SELECTED
				, ~state);
		}
		Lv_iPrevItem = lpnmitem->iItem;
		Lv_Status = ~state;
		return 1;
	}

	case NM_RCLICK: {
		LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmhdr;
		HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);
		HMENU tmp = LoadMenu(hInst, (LPCTSTR)MAKEINTRESOURCE(IDC_CPE));
		HMENU hmenuR = GetSubMenu(tmp, 0);
		POINT pt;
		pt = lpnmitem->ptAction;
		ClientToScreen(lpnmitem->hdr.hwndFrom, &pt);
		SetForegroundWindow(hDlg);
		TrackPopupMenu(hmenuR, TPM_LEFTALIGN | TPM_BOTTOMALIGN,
			pt.x, pt.y, 0, hDlg, NULL);
		DestroyMenu(tmp);

		return 1;
	}

  // ListViewのNOTIFYメッセージデフォルト
	default:
		return 1;
	}
}

INT_PTR UsiDebugStb::MainDialog::
OnBnOK(HWND hwnd, HWND hclrl, UINT code)
{
	if (GetFocus() != hclrl)
	{
		return FALSE;
	}

	return PostMessage(pNormalUIDlg->hDlg
		, WM_COMMAND
		, MAKEWPARAM(IDCANCEL, BN_CLICKED)
		, (LPARAM)GetDlgItem(pNormalUIDlg->hDlg, IDCANCEL));
}

INT_PTR UsiDebugStb::MainDialog::
OnBnCancel(HWND hwnd, HWND hclrl, UINT code)
{
	unsigned thaddr(0);
	if (!(hEndThread = _beginthreadex(NULL
		, 0
		, pfnEndProc
		, this
		, CREATE_SUSPENDED
		, &thaddr)))
	{
		ErrOut_(WSAGetLastError(), __FILE__, __FUNCTION__, __LINE__);
		ExitProcess(-1);
	}
	ResumeThread((HANDLE)hEndThread);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnWmNotify(HWND hwnd, WPARAM wParam, LPNMHDR lParam)
{
	// このマクロは仮引数名に依存しています。
#define HDL_NOTIFY_CLS( ctlid, fn) \
		case ctlid: return this->fn(hwnd,wParam,lParam)
	switch (lParam->idFrom) {
		//HDL_NOTIFY_CLS(IDC_SPIN_MAX_RAW, OnNM_SpinMaxRaw);
		HDL_NOTIFY_CLS(IDC_LIST1, OnNM_List1);

		default:
			return 0;
	}
#undef HDL_NOTIFY_CLS
}

INT_PTR UsiDebugStb::MainDialog::
OnBnSelectLogFile(HWND hwnd, HWND hctrl, UINT code)
{
	TCHAR	tszIniPathName[_MAX_PATH + 1]{};

	tszIniPathName[
	::GetPrivateProfileString(
		_T("MAIN_DLG")
		, _T("LOG_PATH")
		, _T("")
		, tszIniPathName
		, _MAX_PATH
		, MakeIniPath().c_str())] = _T('\0');

	TCHAR	tszOriginFileName[_MAX_FNAME + 1]{};
	tszOriginFileName[
		::GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("LOG_FILE_NAME")
			, _T("")
			, tszOriginFileName
			, _MAX_FNAME
			, MakeIniPath().c_str())] = _T('\0');
		TCHAR	tszOriginExt[_MAX_EXT + 1]{};
		tszOriginExt[
			::GetPrivateProfileString(
				_T("MAIN_DLG")
				, _T("LOG_FILE_EXT")
				, _T("")
				, tszOriginExt
				, _MAX_EXT
				, MakeIniPath().c_str())] = _T('\0');
			_tcscat_s(tszOriginFileName, tszOriginExt);
			// common dialog box structure, setting all fields to 0 is important
			OPENFILENAME ofn{};
			// Initialize remaining fields of OPENFILENAME structure
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlgMain;
			ofn.lpstrFile = tszOriginFileName;
			ofn.nMaxFile = sizeof(tszOriginFileName);
			ofn.lpstrFilter = _T("Log\0*.log\0All\0*.*\0");
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = tszIniPathName;
			ofn.lpstrDefExt = _T(".log");
			ofn.Flags = OFN_PATHMUSTEXIST;
			if (GetOpenFileName(&ofn) != TRUE)
			{
				ErrOut_(CommDlgExtendedError(), __FILE__, __FUNCTION__, __LINE__);
				return FALSE;
			}
			TCHAR	fname[_MAX_FNAME];
			TCHAR	drive[_MAX_DRIVE];
			TCHAR	dir[_MAX_DIR];
			TCHAR	ext[_MAX_EXT];
			TCHAR   path[_MAX_PATH];
			TCHAR   fnameext[_MAX_FNAME + _MAX_EXT + 1]{};
			_tsplitpath_s(ofn.lpstrFile
							  , drive, _MAX_DRIVE
							  , dir, _MAX_DIR
							  , fname, _MAX_FNAME
							  , ext, _MAX_EXT);

			_tmakepath_s(path
							 , _MAX_PATH
							 , drive
							 , dir
							 , _T("")
							 , _T(""));
			::SetWindowText(
				GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_PATH)
				, path);

			// 拡張子が指定されているか否か。
			if (!_tcsicmp(ext, _T("")))
			{
				_tcscpy_s(ext, _T(".log"));
			}

			WritePrivateProfileString(_T("MAIN_DLG")
											  , _T("LOG_PATH")
											  , path
											  , MakeIniPath().c_str());
			WritePrivateProfileString(_T("MAIN_DLG")
											  , _T("LOG_FILE_NAME")
											  , fname
											  , MakeIniPath().c_str());
			WritePrivateProfileString(_T("MAIN_DLG")
											  , _T("LOG_FILE_EXT")
											  , ext
											  , MakeIniPath().c_str());
			//プロファイルへの書き込みの後の結果が反映される。
			::SetWindowText(
				GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_FILE_NAME)
				, MakeDispLogFileName().c_str());
			return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnLogPathClear(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(_T("MAIN_DLG")
									  , _T("LOG_PATH")
									  , _T("")
									  , MakeIniPath().c_str());
	WritePrivateProfileString(_T("MAIN_DLG")
									  , _T("LOG_FILE_NAME")
									  , _T("")
									  , MakeIniPath().c_str());
	WritePrivateProfileString(_T("MAIN_DLG")
									  , _T("LOG_FILE_EXT")
									  , _T("")
									  , MakeIniPath().c_str());
	// プロファイルへの書き込みの後の結果が反映される。
	::SetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_PATH)
		, _T(""));
	::SetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_FILE_NAME)
		, MakeDispLogFileName().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnDeleteAll(HWND hwnd, HWND hctrl, UINT code)
{
	ListView_DeleteAllItems(GetDlgItem(hDlgMain, IDC_LIST1));
	return FALSE;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnSelectFont(HWND hwnd, HWND hctrl, UINT code)
{
	HANDLE hThread(NULL);
	unsigned thaddr(0);
	if (!(hThread = (HANDLE)_beginthreadex(NULL
		, 0
		, pfnFontSelectProc
		, this
		, CREATE_SUSPENDED
		, &thaddr)))
	{
		ErrOut(errno);
		return FALSE;
	}
	ResumeThread((HANDLE)hThread);
	CloseHandle(hThread);
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::
OnMnCopy(HWND hwnd, HWND hctrl, UINT code)
{
	POINT pt{};
	GetCursorPos(&pt);
	ScreenToClient(hDlgMain, &pt);


	HWND hLv = GetDlgItem(hDlgMain, IDC_LIST1);
	LVITEMINDEX lvii{};
	lvii.iItem = -1;
	if (::SendMessage(hLv, LVM_GETNEXTITEMINDEX, (WPARAM)&lvii, LVNI_SELECTED))
	{
		tstring tstrz(BUFFER_SIZE, _T('\0'));
		LV_ITEM lvi{};
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = (MainDialog::Text);
		lvi.cchTextMax = tstrz.size();
		lvi.pszText = tstrz.data();
		tstrz.resize(::SendMessage(hLv, LVM_GETITEMTEXT, lvii.iItem, (LPARAM)&lvi));
		if (tstrz.empty())
		{
			return TRUE;
		}
		if (!OpenClipboard(hDlgMain))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		// Remove the current Clipboard contents
		if (!EmptyClipboard())
		{
			MessageBox(hDlgMain
						  , _T("Cannot empty the Clipboard")
						  , _T("DUSI_COMMON Stub")
						  , MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		string strz = TtoA(tstrz);
		// Get the currently selected data
		if (!(hgCopy = GlobalAlloc(GMEM_MOVEABLE, (strz.size() + 1) * sizeof(char))))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		char *lptstr(NULL);
		if (!(lptstr = (char *)GlobalLock(hgCopy))) {
			ErrMes("Err.Globall Memory");
			return FALSE;
		}
		errno_t err(0);
		if (err = memcpy_s(lptstr
									, (strz.size() + 1) * sizeof(char)
									, strz.data()
									, strz.size()))
		{
			MessageBox(hDlgMain
						  , _T("Cannot memcpy_s")
						  , _T("DUSI_COMMON Stub")
						  , MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		lptstr[strz.size()] = '\0';
		GlobalUnlock(hgCopy);
		// For the appropriate data formats...
		if (::SetClipboardData(CF_TEXT, hgCopy) == NULL)
		{
			ErrOut(GetLastError());
			MessageBox(hDlgMain
						  , _T("Cannot SetClipboardData")
						  , _T("DUSI_COMMON Stub")
						  , MB_OK | MB_ICONEXCLAMATION);
			CloseClipboard();
			GlobalFree(hgCopy);
			return FALSE;
		}
		CloseClipboard();
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnExecute(HWND hwnd, HWND hctrl, UINT code)
{
	DoExecute();
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnSelectExeFile(HWND hwnd, HWND hctrl, UINT code)
{
	tstring tstrPath(MAX_PATH, _T('\0'));
	tstrPath.resize(::GetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_EXE_PATH_NAME)
		, tstrPath.data()
		, tstrPath.size()));
	// common dialog box structure, setting all fields to 0 is important
	OPENFILENAME ofn{};
	TCHAR szFile[MAX_PATH + 1]{_T('\0')};
	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlgMain;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Exe\0*.EXE\0All\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = tstrPath.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) != TRUE)
	{
		return FALSE;
	}
	// use ofn.lpstrFile here
	::SetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_EXE_PATH_NAME)
		, ofn.lpstrFile);
	// EXEパスネームをプロファイルに書き込み。
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("EXE_PATH_NAME")
		, ofn.lpstrFile
		, MakeIniPath().c_str());

	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnTerminateProcess(HWND hwnd, HWND hctrl, UINT code)
{
	if (!PI.hProcess)
	{
		return true;
	}
	if (!TerminateProcess(PI.hProcess, 3))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnBnTest1(HWND hwnd, HWND hctrl, UINT code)
{
	(*pfnConsoleHandlerRoutine)(CTRL_C_EVENT);
	return INT_PTR();
}

INT_PTR UsiDebugStb::MainDialog::
OnChkDispTaskTray(HWND hwnd, HWND hctrl, UINT code)
{
	int fCheck(0);
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_DISP_TASK_TRAY")
		, (fCheck = Button_GetCheck(hctrl) == BST_CHECKED)
		? _T("1") : _T("0")
		, MakeIniPath().c_str());
	Shell_NotifyIcon(fCheck ? NIM_ADD : NIM_DELETE, &nid);
	if (!fCheck)
	{
		Button_SetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_HIDE_WINDOW_AT_START), BST_UNCHECKED);
		WritePrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_HIDE_WINDOW_AT_START")
			, _T("0")
			, MakeIniPath().c_str());
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkHideWindowAtStart(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_HIDE_WINDOW_AT_START")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	if (Button_GetCheck(hctrl) == BST_CHECKED)
	{
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_DISP_TASK_TRAY)
			, BST_CHECKED);
		OnChkDispTaskTray(hwnd
			, GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_DISP_TASK_TRAY)
			, code);
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkHideChild(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_HIDE_CHILD")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkMakeLogFile(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_MAKE_LOG_FILE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkAddTimeStamp(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_ADD_TIME_STAMP")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	// 表示切替
	SetWindowText(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_FILE_NAME)
		, MakeDispLogFileName().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkStartChildAtStartup(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_START_CHILD_AT_STARTUP")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkEndAtChild(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_END_AT_CHILD")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkShowPrompt(HWND hwnd, HWND hctrl, UINT code)
{
	int IsShow(1);
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_SHOW_PROMPT")
		, (IsShow=(Button_GetCheck(hctrl) == BST_CHECKED))
				? _T("1") : _T("0")
		, MakeIniPath().c_str());
	if (hwndCon)
	{
		ShowWindow(hwndCon, IsShow ? SW_SHOW:SW_HIDE);
	}
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkShowInputDlg(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("INPUT_DLG")
		, _T("CHECK_SHOW_INPUT_DLG")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	// 入力ダイアログを表示させるかどうか。
	ShowWindow(pInputDlg->hDlg,
		Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_INPUT_DLG))
		== BST_CHECKED ?
		SW_SHOW : SW_HIDE);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkSimpleUIMode(HWND hwnd, HWND hctrl, UINT code)
{
	tstring tstr = Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0");

	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("CHECK_SIMPLE_UI_MODE")
		, (tstr=Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")).c_str()
		, MakeIniPath().c_str());
	Button_SetCheck(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE)
		, tstr != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(
		GetDlgItem(pMinimumUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE)
		, tstr != _T("0") ? BST_CHECKED : BST_UNCHECKED);

	tstr!=_T("0") ? ChangeSimpleUIMode() : ChangeNormalUIMode();
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnEdtMaxRaw(HWND hwnd, HWND hctrl, UINT code)
{
	tstring str(BUFFER_SIZE, _T('\0'));
	switch (code)
	{
		case EN_CHANGE:
		{
			HWND hudMaxRaw = GetDlgItem(pNormalUIDlg->hDlg, IDC_SPIN_MAX_RAW);
			BOOL bFalse(0);
			int iPos = ::SendMessage(hudMaxRaw, UDM_GETPOS32, 0, (LPARAM) & bFalse);
			if (bFalse)
			{
				MessageBox(hDlgMain
					, _T("UP_DOWN_MAX_RAW_POSエラー")
					, _T("USIDebugStub")
					, MB_OK | MB_ICONEXCLAMATION);
			}
			if (!WritePrivateProfileStruct(
				_T("MAIN_DLG")
				, _T("UP_DOWN_MAX_RAW_POS")
				, &iPos
				, sizeof(int)
				, MakeIniPath().c_str()))
			{
				EOut;
				MessageBox(hDlgMain
					, _T("UP_DOWN_MAX_RAW_POSエラー")
					, _T("USIDebugStub")
					, MB_OK | MB_ICONEXCLAMATION);
				return true;
			}
			return true;
		}
		default:
			return true;
	}
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnRdoLogFileApp(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("RADIO_LOG_FILE_APP_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnRdoLogFileTrunc(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("RADIO_LOG_FILE_APP_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("0") : _T("1")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnRdoRejectIORedirect(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("RADIO_IO_REDIRECT_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("0") : _T("1")
		, MakeIniPath().c_str());

	Button_Enable(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_PROMPT)
		, Button_GetCheck(hctrl) == BST_CHECKED ? TRUE:FALSE);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnRdoAcceptIORedirect
(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("MAIN_DLG")
		, _T("RADIO_IO_REDIRECT_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());

	Button_Enable(
		GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_PROMPT)
		, Button_GetCheck(hctrl) == BST_CHECKED ? FALSE : TRUE);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnMnTasktrayClose
(HWND hwnd, HWND hctrl, UINT code)
{
	PostMessage(pNormalUIDlg->hDlg
		, WM_COMMAND
		, MAKEWPARAM(IDCANCEL,BN_CLICKED)
			, (LPARAM)GetDlgItem(hwnd, IDCANCEL));
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::OnWmSysCommand
(HWND hwnd, UINT cmd, int x, int y)
{

	switch (cmd)
	{
		case SC_CLOSE: {
			_D("SC_CLOSE");
			PostMessage(pNormalUIDlg->hDlg
							, WM_COMMAND
							, MAKEWPARAM(IDCANCEL, BN_CLICKED)
							, (LPARAM)GetDlgItem(hwnd, IDCANCEL));
			return 0;
		}
		case SC_RESTORE: {_D("SC_RESTORE"); }
		case SC_MINIMIZE: {_D("SC_MINIMIZE"); }
		case SC_DEFAULT: {_D("SC_DEFAULT"); }
		default:
			return DefWindowProc(hwnd, WM_SYSCOMMAND, cmd, MAKELPARAM(x, y));
	}
}

INT_PTR UsiDebugStb::MainDialog::OnWmCommand
(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
// マクロは仮引数の名前に依存しています。
#define HDL_CMD_CLS( ctlid, fn) \
		case ctlid: return this->fn(hwnd,hwndCtl,codeNotify)
	switch (id) {
		HDL_CMD_CLS(IDOK, OnBnOK);
		HDL_CMD_CLS(IDCANCEL, OnBnCancel);
		HDL_CMD_CLS(IDC_BUTTON_SELECT_LOG_FILE, OnBnSelectLogFile);
		HDL_CMD_CLS(IDC_BUTTON_LOG_PATH_CLEAR, OnBnLogPathClear);
		HDL_CMD_CLS(IDC_BUTTON_DELETE_ALL, OnBnDeleteAll);
		HDL_CMD_CLS(IDM_COPY, OnMnCopy);
		HDL_CMD_CLS(IDC_BUTTON_EXECUTE, OnBnExecute);
		HDL_CMD_CLS(IDC_BUTTON_SELECT_FILE, OnBnSelectExeFile);
		HDL_CMD_CLS(IDC_BUTTON_TERMINATE_PROCESS, OnBnTerminateProcess);
		HDL_CMD_CLS(IDC_CHECK_DISP_TASK_TRAY, OnChkDispTaskTray);
		HDL_CMD_CLS(IDC_CHECK_HIDE_WINDOW_AT_START, OnChkHideWindowAtStart);
		HDL_CMD_CLS(IDC_CHECK_HIDE_CHILD, OnChkHideChild);
		HDL_CMD_CLS(IDC_CHECK_MAKE_LOG_FILE, OnChkMakeLogFile);
		HDL_CMD_CLS(IDC_CHECK_ADD_TIME_STAMP, OnChkAddTimeStamp);
		HDL_CMD_CLS(IDC_CHECK_START_CHILD_AT_STARTUP, OnChkStartChildAtStartup);
		HDL_CMD_CLS(IDC_CHECK_END_AT_CHILD, OnChkEndAtChild);
		HDL_CMD_CLS(IDC_RADIO_LOG_FILE_APP_MODE, OnRdoLogFileApp);
		HDL_CMD_CLS(IDC_RADIO_LOG_FILE_TRUNC_MODE, OnRdoLogFileTrunc);
		HDL_CMD_CLS(IDC_RADIO_IO_REDIRECT_MODE_REJECT, OnRdoRejectIORedirect);
		HDL_CMD_CLS(IDC_RADIO_IO_REDIRECT_MODE_ACCEPT, OnRdoAcceptIORedirect);
		HDL_CMD_CLS(IDM_TASKTRAY_CLOSE, OnMnTasktrayClose);
		HDL_CMD_CLS(IDC_CHECK_SHOW_PROMPT, OnChkShowPrompt);
		HDL_CMD_CLS(IDC_CHECK_SHOW_INPUT_DLG, OnChkShowInputDlg);
		HDL_CMD_CLS(IDC_CHECK_SIMPLE_UI_MODE, OnChkSimpleUIMode);
		HDL_CMD_CLS(IDC_BUTTON_SELECT_FONT, OnBnSelectFont);
		HDL_CMD_CLS(IDC_BUTTON_TEST1, OnBnTest1);
		HDL_CMD_CLS(IDC_EDIT_MAX_RAW, OnEdtMaxRaw);
	default:
		return 0;
	}
#undef HDL_CMD_CLS
}

INT_PTR UsiDebugStb::MainDialog::OnWmEnd(HWND hwnd)
{
	::WaitForSingleObject((HANDLE)hEndThread, INFINITE);
	CloseHandle((HANDLE)hEndThread);
	//::ShowWindowAsync(hDlgMain, SW_HIDE);
	::SetWindowPos(
		hDlgMain
		, HWND_TOP
		, 0
		, 0
		, 0
		, 0
		,SWP_NOMOVE | SWP_NOSIZE  | SWP_FRAMECHANGED|SWP_NOZORDER);
	EndDialog(hDlgMain, S_OK);
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmWindowPosChanging
(HWND hwnd, LPWINDOWPOS lpwpos)
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
		//_D("SWP_SHOWWINDOW");
	}
	if ((lpwpos->flags & SWP_HIDEWINDOW) == SWP_HIDEWINDOW)
	{
		_D("SWP_HIDEWINDOW");
	}
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::
OnWmSize(HWND hwnd, UINT state, int cx, int cy)
{
	switch (state)
	{
	case SIZE_MINIMIZED: {
		//_D("SIZE_MINIMIZED");
		break;
	}
	case SIZE_RESTORED: {
//		_D("SIZE_RESTORED");
		//RePlaceDlgItem();
	}
	default:
		break;
	}
	return 1;
}

INT_PTR UsiDebugStb::MainDialog::OnWmSizing(HWND hwnd, UINT state, RECT *pRect)
{
	if (rcPrevDlgPos.right - rcPrevDlgPos.left == pRect->right - pRect->left
		&& rcPrevDlgPos.bottom - rcPrevDlgPos.top == pRect->bottom - pRect->top)
	{
		return TRUE;
	}

	// 最小のサイズがUIダイアログになる様にする為、UIのRECTを取得。
	const RECT rcDlgUI;
	UIDialog* pUI(NULL);
	{
		pUI = Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE))
			== BST_CHECKED ?
			pMinimumUIDlg.get()
			: pNormalUIDlg.get();
		GetWindowRect(pUI->hDlg, const_cast<RECT*>(&rcDlgUI));
		if (!::MapWindowRect(
			HWND_DESKTOP
			, hwnd
			, const_cast<RECT*>(&rcDlgUI)))
		{
			EOut;
			return FALSE;
		}
		OffsetRect(const_cast<RECT*>(&rcDlgUI), -rcDlgUI.left, -rcDlgUI.top);
	}

	// UIダイアログを最小サイズにした場合の、MainDialogの最小RECTを得る。
	const RECT rcWin = rcDlgUI;
	if (!AdjustWindowRect(const_cast<RECT*>( & rcWin), WS_OVERLAPPEDWINDOW, FALSE)) {
		EOut;
		return FALSE;
	}
	OffsetRect(const_cast<RECT*>(&rcWin), -rcWin.left, -rcWin.top);

	// 最小サイズを超えないようにする
	if (state == WMSZ_LEFT || state == WMSZ_TOPLEFT || state == WMSZ_BOTTOMLEFT)
		if (pRect->right - pRect->left < rcWin.right)
			pRect->left = pRect->right - rcWin.right;
	if (state == WMSZ_RIGHT || state == WMSZ_TOPRIGHT || state == WMSZ_BOTTOMRIGHT)
		if (pRect->right - pRect->left < rcWin.right)
			pRect->right = pRect->left + rcWin.right;
	if (state == WMSZ_TOP || state == WMSZ_TOPLEFT || state == WMSZ_TOPRIGHT)
		if (pRect->bottom - pRect->top < rcWin.bottom)
			pRect->top = pRect->bottom - rcWin.bottom;
	if (state == WMSZ_BOTTOM || state == WMSZ_BOTTOMLEFT || state == WMSZ_BOTTOMRIGHT)
		if (pRect->bottom - pRect->top < rcWin.bottom)
			pRect->bottom = pRect->top + rcWin.bottom;

	// 左と上のフレームを動かしたらInputDialogも移動。
	if (state == WMSZ_LEFT || state == WMSZ_TOPLEFT || state == WMSZ_TOP)
	{
		int offsetx = pRect->left-rcPrevDlgPos.left ;
		int offsety = pRect->top - rcPrevDlgPos.top;
		::PostMessage(pInputDlg->hDlg, WM_DO_OFFSET_MOVE, offsetx, offsety);
	}

	TP_WORK* pTPWork(NULL);
	if (!(pTPWork = CreateThreadpoolWork(pfnAdjustUILayoutW, this, &*pcbeUI)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SubmitThreadpoolWork(pTPWork);

	rcPrevDlgPos = *pRect;
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::OnWmDestroyClipBoard(HWND hwnd)
{
	if (hgCopy != NULL) {
		if (!::GlobalUnlock(hgCopy)) {
			DWORD dw;
			if (dw = GetLastError()) {
				ErrOut(dw);
			}
		}

		if (::GlobalFree(hgCopy)==hgCopy) {
			EOut;
		};
		hgCopy = NULL;
	}
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmNotifyIcon(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (wParam != reinterpret_cast<UINT>(hDlg)) {
		return FALSE;
	}
	switch (lParam) {
		case WM_LBUTTONDOWN:
		{
			if (fHide)
			{
				fHide = FALSE;
				RECT rect{};
				if (!GetPrivateProfileStruct(
					_T("MAIN_DLG")
					, _T("WINDOW_POS")
					, &rect
					, sizeof(rect)
					, MakeIniPath().c_str()))
				{
					ErrOut(GetLastError());
					return FALSE;
				}
				ShowWindow(hDlg, SW_SHOW);
				ShowWindow(pInputDlg->hDlg
					, Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_INPUT_DLG))
					== BST_CHECKED ? SW_SHOW : SW_HIDE);
				if (hwndCon)
				{
					ShowWindow(hwndCon, SW_SHOW);
				}
				break;
			}
			if (Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_HIDE_WINDOW_AT_START))
				== BST_CHECKED)
			{
				fHide = TRUE;
				ShowWindow(hDlg, SW_HIDE);
				ShowWindow(pInputDlg->hDlg, SW_HIDE);
				if (hwndCon)
				{
					ShowWindow(hwndCon, SW_HIDE);
				}
				break;
			}
			if (IsIconic(hDlg))
			{
				ShowWindow(hDlg, SW_RESTORE);
				ShowWindow(pInputDlg->hDlg
					, Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_INPUT_DLG))
					== BST_CHECKED ? SW_RESTORE : SW_HIDE);
				//ShowWindow(pInputDlg->hDlg, SW_RESTORE);
				if (hwndCon)
				{
					ShowWindow(hwndCon, SW_RESTORE);
				}
			}
			else {
				ShowWindow(hDlg, SW_MINIMIZE);
				ShowWindow(pInputDlg->hDlg, SW_MINIMIZE);
				if (hwndCon)
				{
					ShowWindow(hwndCon, SW_MINIMIZE);
				}
			}
			break;
		}
		case WM_RBUTTONDOWN: {
			RECT rc{};
			POINT pt{};
			HMENU tmp = LoadMenu(
				(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE)
				, (LPCTSTR)MAKEINTRESOURCE(IDC_CPE));
			GetCursorPos(&pt);
			HMENU hmenuR = GetSubMenu(tmp, 1);
			SetForegroundWindow(hDlg);
			TrackPopupMenu(hmenuR, TPM_LEFTALIGN | TPM_BOTTOMALIGN,
								pt.x, pt.y, 0, hDlg, NULL);
			DestroyMenu(tmp);
			return 0;
		}
		default:
			return true;
	}

	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmInputConfirmed
(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	string str = TtoA(pInputDlg->strCmd) + TtoA(pInputDlg->GetLB());
	// Childへ転送
	{
		pDlgString_type* pDS = &((*mrDlgStr.Lend())
			= { this, str});

		TP_WAIT* pTPWait(NULL);
		if (!(pTPWait = CreateThreadpoolWait(
			pfnSerializedWriteToChild
			, pDS
			, &*pcbe)))
		{
			ErrOut(GetLastError());
			mrDlgStr.Return(pDS);
			return FALSE;
		}
		SetThreadpoolWait(pTPWait, hSemChildWrite.get(), NULL);
	}

	// 読み込んだものをログへ書き込む
	{
		pDlgString_type* pDS = &((*mrDlgStr.Lend())
			= { this, str });
		TP_WAIT* pTPWaitLog(NULL);
		if (!(pTPWaitLog = CreateThreadpoolWait(
			pfnLvSerializedWriteFromParentToLog
			, pDS
			, &*pcbe)))
		{
			ErrOut(GetLastError());
			mrDlgStr.Return(pDS);
			return FALSE;
		}
		else {
			SetThreadpoolWait(pTPWaitLog, hSemLv.get(), NULL);
		}
	}
	return INT_PTR();
}

INT_PTR UsiDebugStb::MainDialog::OnWmMoving(HWND hDlg, LPRECT pRect)
{
	LONG dx = pRect->left - reinterpret_cast<POINT*>(& rcPrevDlgPos)->x;
	LONG dy = pRect->top - reinterpret_cast<POINT*>(&rcPrevDlgPos)->y;
	PostMessage(pInputDlg->hDlg, WM_DO_OFFSET_MOVE, (WPARAM)dx, (LPARAM)dy);
	rcPrevDlgPos = *pRect;
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmMove
(HWND hwnd, int x, int y)
{
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmExitSizeMove(HWND hwnd)
{
	RECT rcDlg;
	GetWindowRect(hwnd, &rcDlg);
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("WINDOW_POS")
		, &rcDlg
		, sizeof(RECT)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::
OnWmSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw)
{
	HFONT hfontnew(NULL);
	if (!GetPrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_INIT_FONT_HANDLE")
		, &hfontnew
		, sizeof(hfontnew)
		, MakeIniPath().c_str()))
	{
		if (!WritePrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_INIT_FONT_HANDLE")
			, &hfont
			, sizeof(HFONT)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		LOGFONT lf{};
		if (!GetObject(hfont, sizeof(lf), &lf))
		{
			ErrMes("Create logfont failed.");
			return TRUE;
		}
		if (!WritePrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_LOG_FONT_BASE")
			, &lf
			, sizeof(lf)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		_D("OnWmSetFont.lfHeight first time is " + to_string((int)lf.lfHeight));
		return TRUE;
	}

	// 各ダイアログアイテムにWM_SETFONT送信
	for (const auto& pair : vhrcDlgItemsBasePos)
	{
		::SendMessage(pair.first, WM_SETFONT, (WPARAM)hfont, TRUE);
	}

	LOGFONT lf;
	if (!GetObject(hfont, sizeof(LOGFONT), &lf))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	_D("OnWmSetFont.lfHeight is " + to_string((int)lf.lfHeight));
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_LOG_FONT_CURRENT")
		, &lf
		, sizeof(LOGFONT)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	::SendMessage(pNormalUIDlg->hDlg, WM_SETFONT, (WPARAM)hfont, 0L);
	::SendMessage(pMinimumUIDlg->hDlg, WM_SETFONT, (WPARAM)hfont, 0L);
	if (hCurrentFont)
	{
		DeleteFont(hCurrentFont);
	}
	hCurrentFont = hfont;
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::OnWmUserSetFont(HWND hwndCtl, HFONT hfont)
{
	// 基本に戻すレシオを求める。
	LOGFONT lfCur;
	if (!GetObject(hCurrentFont, sizeof(LOGFONT), &lfCur))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	LOGFONT lfBase;
	if (!GetPrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, &lfBase
		, sizeof(LOGFONT)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	RECT rcMainDlgPrev;
	if (!GetWindowRect(hDlgMain, &rcMainDlgPrev))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	RECT rcConvBaseRatioMain = rcMainDlgPrev;

	// 基本フォントでの大きさを求める。
	ScaleRect(& rcConvBaseRatioMain
		, (DOUBLE)lfBase.lfHeight / lfCur.lfHeight);

	// ベースサイズに戻す。
	SetInitialPlacement();

	// 各ダイアログアイテムにWM_SETFONT送信
	for (const auto& pair : vhrcDlgItemsBasePos)
	{
		::SendMessage(pair.first, WM_SETFONT, (WPARAM)hfont, TRUE);
	}

	// 新サイズのレシオを求める。
	DOUBLE NewRatio(0);
	{
		// ダイアログデザイン時のロジフォントを取得。
		LOGFONT lgBase{};
		if (!GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_LOG_FONT_BASE")
			, &lgBase
			, sizeof(lgBase)
			, MakeIniPath().c_str()))
		{
			_D("Err. DLG_LOG_FONT_BASE.");
			return FALSE;
		}

		// 新しくセットされたhfontのロジフォントの取得。
		LOGFONT lgNew{};
		if (!GetObject(hfont, sizeof(lgNew), &lgNew))
		{
			ErrMes("Err.");
			return FALSE;
		}
		NewRatio = (DOUBLE)lgNew.lfHeight / lgBase.lfHeight;
	}

	// メインダイアログのサイズの更新
	{
		const int cx = (rcConvBaseRatioMain.right - rcConvBaseRatioMain.left) * NewRatio;
		const int cy = (rcConvBaseRatioMain.bottom - rcConvBaseRatioMain.top) * NewRatio;
		if (!SetWindowPos(
			hwndCtl
			, HWND_TOP
			, 0
			, 0
			, cx
			, cy
			,  SWP_SHOWWINDOW
			| SWP_NOMOVE|SWP_NOZORDER
			));
		{
			EOut;
			// 何故かここに来るがエラーコードはC0
			//return FALSE;
		}
	}

	// ダイアログアイテムのサイズと位置変更。
	for (const auto& pair : vhrcDlgItemsBasePos)
	{
		if (!::SetWindowPos(
			pair.first
			, HWND_TOPMOST
			, pair.second.left * NewRatio
			, pair.second.top * NewRatio
			, (pair.second.right - pair.second.left) * NewRatio
			, (pair.second.bottom - pair.second.top) * NewRatio
			,
			SWP_NOOWNERZORDER
			| SWP_SHOWWINDOW
		))
		{
			EOut;
			return FALSE;
		}
	}

	// UIダイアログにフォント変更メッセージ
	::SendMessage(pNormalUIDlg->hDlg, WM_USER_SETFONT, (WPARAM)hfont, 0L);
	::SendMessage(pMinimumUIDlg->hDlg, WM_USER_SETFONT, (WPARAM)hfont, 0L);

	// セットされたフォントを保存。
	LOGFONT lgNew{};
	if (!GetObject(hfont, sizeof(lgNew), &lgNew))
	{
		ErrMes("Err.");
		return FALSE;
	}
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_LOG_FONT_CURRENT")
		, &lgNew
		, sizeof(lgNew)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	if (hCurrentFont != hfont)
	{
		DeleteFont(hCurrentFont);
	}
	hCurrentFont = hfont;

	TP_WORK* pTPWork(NULL);
	if (!(pTPWork = CreateThreadpoolWork(pfnAdjustUILayoutW, this, &*pcbeUI)))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	SubmitThreadpoolWork(pTPWork);
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::OnWmGetFont(HWND hwnd)
{
	if (!hCurrentFont)
	{
		LOGFONT lgCur{};
		if (GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_LOG_FONT_CURRENT")
			, &lgCur
			, sizeof(lgCur)
			, MakeIniPath().c_str())) {
			// ユーザー設定のフォントに変更
			if (!(hCurrentFont = CreateFontIndirect(&lgCur)))
			{
				ErrMes("Create Font Err");
				hCurrentFont = NULL;
			}
		}
		else if (GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_LOG_FONT_BASE")
			, &lgCur
			, sizeof(lgCur)
			, MakeIniPath().c_str())) {
			// ユーザー設定のフォントに変更
			if (!(hCurrentFont = CreateFontIndirect(&lgCur)))
			{
				ErrMes("Create Font Err");
				hCurrentFont = NULL;
			}
		}
	}
	SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR)hCurrentFont);
	return (INT_PTR)hCurrentFont;
}

INT_PTR UsiDebugStb::MainDialog::OnWmDestroy(HWND hwnd)
{
	if (hCurrentFont)
	{
		DeleteFont(hCurrentFont);
	}
	// 項目を削除
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_LOG_FONT_BASE")
		, NULL
		, NULL
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	// 項目を削除
	if (!WritePrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_INIT_FONT_HANDLE")
		, NULL
		, NULL
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}
	{
		RECT rect{};
		::GetWindowRect(hDlgMain, &rect);
		WritePrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("WINDOW_POS")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str());
	}
	// リストのRECT保存
	{
		RECT rect{};
		HWND hList = GetDlgItem(hwnd, IDC_LIST1);
		if (!::GetWindowRect(hList, &rect))
		{
			EOut;
		}
		if (!::MapWindowRect(HWND_DESKTOP, hwnd, &rect))
		{
			EOut;
		}
		if (!::WritePrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("LIST_RECT")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			EOut;
		}
	}
	return INT_PTR();
}

BOOL UsiDebugStb::MainDialog::
OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	_D("Start InitDialog");
	this->hDlgMain = hDlg;

	// ダイアログアイテムのハンドルと位置保存
	EnumChildWindows(hDlg, SaveChildWindowPosition, (LPARAM)&vhrcDlgItemsBasePos);


	// 最初のフォントハンドル取得。後で、保存されているフォントに変更。
	if (!GetPrivateProfileStruct(
		_T("MAIN_DLG")
		, _T("DLG_INIT_FONT_HANDLE")
		, &hCurrentFont
		, sizeof(hCurrentFont)
		, MakeIniPath().c_str()))
	{
		ErrOut(GetLastError());
		return FALSE;
	}

	// リストビュー初期設定
	// カラムの設定
	const auto hListView = GetDlgItem(hDlgMain, IDC_LIST1);
	unique_ptr< LVCOLUMN[]> cols = make_unique<LVCOLUMN[]>(vstrHeader.size());
	for (int i(0); i < vstrHeader.size(); ++i)
	{
		cols[i] =
		{ /*UINT   mask       */LVCF_TEXT
			, /*int    fmt        */0
			, /*int    cx         */0
			, /*LPTSTR pszText    */const_cast<LPTSTR>(vstrHeader[i].c_str())
			, /*int    cchTextMax */0
			, /*int    iSubItem   */0
			, /*int    iImage     */0
			, /*int    iOrder     */0
		};
	}

	ListView_SetExtendedListViewStyleEx(
		hListView
		, LVS_EX_FULLROWSELECT
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_AUTOSIZECOLUMNS
		, LVS_EX_FULLROWSELECT
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_AUTOSIZECOLUMNS
	);

	for (int i(0); i < vstrHeader.size(); ++i)
	{
		ListView_InsertColumn(hListView, i, &cols[i]);
		if (i == vstrHeader.size() - 1)
		{
			ListView_SetExtendedListViewStyleEx(
				hListView
				, LVS_EX_FULLROWSELECT
				| LVS_EX_ONECLICKACTIVATE
				| LVS_EX_AUTOSIZECOLUMNS
				, LVS_EX_FULLROWSELECT
				| LVS_EX_ONECLICKACTIVATE
				//	| LVS_EX_AUTOSIZECOLUMNS
			);
		}
		ListView_SetColumnWidth(hListView, i, LVSCW_AUTOSIZE_USEHEADER);
	}

	// ベースダイアログRECT保存。
	{
		RECT rect{};
		GetWindowRect(hDlg, &rect);
		if (!WritePrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_WINDOW_BASE_RECT")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
	}

	// UINormal子ダイアログ作成
	if (!pNormalUIDlg->CreateDialogX(
		GetModuleHandle(NULL)
		, hDlgMain
		, IDD_NORMALUI
		, _T("NORMAL_UI_DLG")))
	{
		ErrMes("Create NormalUIDlg fail.");
		return FALSE;
	}

	// UIMinimumh子ダイアログ作成
	if (!pMinimumUIDlg->CreateDialogX(
		GetModuleHandle(NULL)
		, hDlgMain
		, IDD_SIMPLEUI
		, _T("MINIMUM_UI_DLG")))
	{
		ErrMes("Create NormalUIDlg fail.");
		return FALSE;
	}

	SetInitialPlacement();

	// インプットダイアログ設定
	{
		pInputDlg.get()
			->CreateDialogX(
				(HINSTANCE)GetWindowLongPtr(
					hDlg
					, GWLP_HINSTANCE)
				, hDlg);
		RECT rect{};
		if (!GetPrivateProfileStruct(
			_T("INPUT_DLG")
			, _T("WINDOW_POS")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			// 初めての起動の場合初期位置を設定
			RECT rectMain{};
			GetWindowRect(this->hDlgMain, &rectMain);
			GetWindowRect(hDlg, &rect);
			int dx = rectMain.left - rect.left - (rect.right - rect.left);
			int dy = rectMain.top - rect.top;
			OffsetRect(&rect, dx, dy);
		}
		if (!::SetWindowPos(
			pInputDlg.get()->hDlg
			, HWND_TOP
			, rect.left
			, rect.top
			, 0
			, 0
			, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_SHOWWINDOW|SWP_NOZORDER))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		SetWindowAppropriatePositionEx(pInputDlg->hDlg);
	}

	// メインダイアログポジション設定
	{
		RECT rect{};
		if (GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("WINDOW_POS")
			, &rect
			, sizeof(rect)
			, MakeIniPath().c_str()))
		{
			SetWindowAppropriatePosition(hDlg, rect);
		}
		GetWindowRect(hDlg, &rect);
		rcPrevDlgPos = rect;
	}

	// ダイアログフォント設定。
	{
		LOGFONT lgCur{};
		if (GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("DLG_LOG_FONT_CURRENT")
			, &lgCur
			, sizeof(lgCur)
			, MakeIniPath().c_str())) {
			// ユーザー設定のフォントに変更
			HFONT hfont(NULL);
			if (!(hfont = CreateFontIndirect(&lgCur)))
			{
				ErrMes("Create Font Err");
				return FALSE;
			}
			DeleteFont(hCurrentFont);
			hCurrentFont = hfont;
		}
		PostMessage(hDlg, WM_USER_SETFONT, (WPARAM)hCurrentFont, (LPARAM)0);
	}
	//MessageBox(hDlgMain
	//	, _T("デバックアタッチポイント")
	//	, _T("USIDebugStub")
	//	, MB_OK | MB_ICONEXCLAMATION);

	// 最大行数UpDownボタン読み込み
	{
		int ipos;
		if (!GetPrivateProfileStruct(
			_T("MAIN_DLG")
			, _T("UP_DOWN_MAX_RAW_POS")
			, &ipos
			, sizeof(int)
			, MakeIniPath().c_str()))
		{
			ipos = 1000;
		}
		const auto hudBtn = GetDlgItem(pNormalUIDlg->hDlg, IDC_SPIN_MAX_RAW);
		const auto hudEdit = GetDlgItem(pNormalUIDlg->hDlg, IDC_EDIT_MAX_RAW);
		::SendMessage(hudBtn, UDM_SETBUDDY, (WPARAM)hudEdit, 0);
		::SendMessage(hudBtn, UDM_SETRANGE32, 0, MAXINT);
		::SendMessage(hudBtn, UDM_SETPOS32, 0, ipos);
	}

	// *************UI設定****************

	// EXEファイル名表示
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("EXE_PATH_NAME")
			, _T("")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		SetWindowText(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_EXE_PATH_NAME)
			, tstrReturn.c_str());
	}

	// ログファイル作成するか否か。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_MAKE_LOG_FILE")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_MAKE_LOG_FILE)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// ログファイル名にタイムスタンプを加えるか否か。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_ADD_TIME_STAMP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_ADD_TIME_STAMP)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// ログファイル名表示
	{
		SetWindowText(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_LOG_FILE_NAME)
			, MakeDispLogFileName().c_str());
	}
	// ログパス表示
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(::GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("LOG_PATH")
			, _T("")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		::SetWindowText(GetDlgItem(pNormalUIDlg->hDlg
			, IDC_STATIC_LOG_PATH)
			, tstrReturn.c_str());
	}
	// ログファイルオープンモードラジオボタン設定
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("RADIO_LOG_FILE_APP_MODE")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_LOG_FILE_APP_MODE)
			, tstrReturn == _T("1") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_LOG_FILE_TRUNC_MODE)
			, tstrReturn != _T("1") ? BST_CHECKED : BST_UNCHECKED);
	}
	// 標準IOモードラジオボタン設定
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("RADIO_IO_REDIRECT_MODE")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT)
			, tstrReturn == _T("0") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// タスクトレイにアイコンを表示するかどうか。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_DISP_TASK_TRAY")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_DISP_TASK_TRAY)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// タスクトレイ設定。nidは取り合えず初期化。
	{
		nid.cbSize = sizeof(nid);
		nid.hWnd = hDlg;
		// 複数のアイコンを表示したときの識別ID。なんでもいい。
		nid.uID = reinterpret_cast<UINT>(hDlg);
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_NOTIFYICON; //WM_USER以降の定数。
		nid.hIcon = (HICON)LoadIcon(NULL, IDI_APPLICATION);
		_tcscpy_s(nid.szTip, 128, _T(_CRT_STRINGIZE(APPNAME)));
		if (Button_GetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_DISP_TASK_TRAY))
			== BST_CHECKED)
		{
			int ret = (int)Shell_NotifyIcon(NIM_ADD, &nid);
		}
	}
	// 起動時ウインドウを隠すかどうか
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_HIDE_WINDOW_AT_START")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		BOOL fCheck(FALSE);
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_HIDE_WINDOW_AT_START)
			, fHide = tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
		ShowWindow(pInputDlg->hDlg, fHide ? SW_HIDE : SW_SHOW);
	}
	// 起動時子ウインドウを隠すかどうか
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_HIDE_CHILD")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_HIDE_CHILD)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// 起動時子プログラムを起動させるかどうか。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_START_CHILD_AT_STARTUP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_START_CHILD_AT_STARTUP)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// 子プログラム終了時プログラムを終了させるかどうか。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_END_AT_CHILD")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_END_AT_CHILD)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// UIモードの読み込み。
	{
		tstring tstr(BUFFER_SIZE, _T('\0'));
		tstr.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_SIMPLE_UI_MODE")
			, _T("0")
			, tstr.data()
			, tstr.size()
			, MakeIniPath().c_str()));
		if (tstr.empty())
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE)
			, tstr == _T("1") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(pMinimumUIDlg->hDlg, IDC_CHECK_SIMPLE_UI_MODE)
			, tstr == _T("1") ? BST_CHECKED : BST_UNCHECKED);
	}
	// プロンプトを表示させるかどうか。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("MAIN_DLG")
			, _T("CHECK_SHOW_PROMPT")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_PROMPT)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	// 入力ダイアログを表示させるかどうか。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("INPUT_DLG")
			, _T("CHECK_SHOW_INPUT_DLG")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_INPUT_DLG)
			, tstrReturn == _T("1") ? BST_CHECKED : BST_UNCHECKED);
	}
	// 終了ボタンを無効にする。
	{
		Button_Enable(GetDlgItem(pNormalUIDlg->hDlg, IDC_BUTTON_TERMINATE_PROCESS), FALSE);
	}
	// ダイアログタイトルを変更
	SetWindowText(hDlgMain, _T(_CRT_STRINGIZE(APPNAME)));

	// ブリンキングスタティックを非表示に
	ShowWindow(GetDlgItem(pNormalUIDlg->hDlg, IDC_STATIC_CHILD_IS_RUNNING), SW_HIDE);

	//***********************************************************************

	//「プロンプトを表示する」を有効にするかどうか
	{
		Button_Enable(
			GetDlgItem(pNormalUIDlg->hDlg, IDC_CHECK_SHOW_PROMPT)
			, Button_GetCheck(GetDlgItem(pNormalUIDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT))
			== BST_CHECKED ? TRUE : FALSE);
	}

	// UIの設定が終わったところで、入力ダイアログボックスを表示するか否か。
	{
		ShowWindow(pInputDlg->hDlg
			, Button_GetCheck(
				GetDlgItem(pNormalUIDlg->hDlg
					, IDC_CHECK_SHOW_INPUT_DLG))
			== BST_CHECKED ? SW_SHOW : SW_HIDE);
	}

	// UIの設定が終わったところで、どちらのUIを表示するか初期設定。
	{
		if (Button_GetCheck(GetDlgItem(
			pNormalUIDlg->hDlg
			, IDC_CHECK_SIMPLE_UI_MODE))
			== BST_CHECKED)
		{
			ShowWindow(pMinimumUIDlg->hDlg, SW_SHOW);
			ShowWindow(pNormalUIDlg->hDlg, SW_HIDE);
		}
		else {
			ShowWindow(pMinimumUIDlg->hDlg, SW_HIDE);
			ShowWindow(pNormalUIDlg->hDlg, SW_SHOW);
		}

		TP_WORK* pTPWork(NULL);
		if (!(pTPWork = CreateThreadpoolWork(pfnAdjustUILayoutW, this, &*pcbeUI)))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		SubmitThreadpoolWork(pTPWork);
	}

	//MessageBox(hDlgMain
	//	, _T("デバックアタッチポイント")
	//	, _T("USIDebugStub")
	//	, MB_OK | MB_ICONEXCLAMATION);

	// UIの設定が終わったところで、起動時子プログラムを起動するか否か。
	{
		if (Button_GetCheck(GetDlgItem(
			pNormalUIDlg->hDlg
			, IDC_CHECK_START_CHILD_AT_STARTUP))
			== BST_CHECKED)
		{
			DoExecute();
		}
	}

	_D("End InitDialog");
	return true;
}

INT_PTR UsiDebugStb::MainDialog::DialogBoxParamW
(_In_opt_ HINSTANCE hInstance
	, _In_opt_ HWND      hWndParent
) noexcept
{
	//	pcbe = pcbeIn;
	return /*WINUSERAPI INT_PTR WINAPI*/::DialogBoxParamW
	( /*_In_opt_ HINSTANCE hInstance     */hInstance
		, /*_In_     LPCWSTR   lpTemplateName*/MAKEINTRESOURCEW(IDD_MAIN_DLG1)
		, /*_In_opt_ HWND      hWndParent    */nullptr
		, /*_In_opt_ DLGPROC   lpDialogFunc  */
		[](
			HWND hDlg
			, UINT uMsg
			, WPARAM wParam
			, LPARAM lParam)->INT_PTR
		{
			/// マクロは仮引数の名前に依存しています。

			/// The macro depends on the name of the provisional Argument.
#include "DefAncestorDialogPtr.h"
#include "DefUserMessageCracker.h"
/// Newly added Windows message.
#include "DefNewWindowMessageCracker.h"
#define DEF_CLASS_PTR MainDialog
			switch (uMsg)
			{
				HDL_DLG_MSG_CLS(WM_NOTIFY, OnWmNotify);
				HDL_DLG_MSG_CLS(WM_SYSCOMMAND, OnWmSysCommand);
				HDL_DLG_MSG_CLS(WM_COMMAND, OnWmCommand);
				HDL_DLG_MSG_CLS(WM_END, OnWmEnd);
				HDL_DLG_MSG_CLS(WM_WINDOWPOSCHANGING, OnWmWindowPosChanging);
				HDL_DLG_MSG_CLS(WM_SIZE, OnWmSize);
				HDL_DLG_MSG_CLS(WM_SIZING, OnWmSizing);
				HDL_DLG_MSG_CLS(WM_DESTROYCLIPBOARD, OnWmDestroyClipBoard);
				HDL_DLG_MSG_CLS(WM_NOTIFYICON, OnWmNotifyIcon);
				HDL_DLG_MSG_CLS(WM_INPUT_CONFIRMED, OnWmInputConfirmed);
				HDL_DLG_MSG_CLS(WM_MOVING, OnWmMoving);
				HDL_DLG_MSG_CLS(WM_MOVE, OnWmMove);
				HDL_DLG_MSG_CLS(WM_INITDIALOG, OnWmInitDialog);
				HDL_DLG_MSG_CLS(WM_SETFONT, OnWmSetFont);
				HDL_DLG_MSG_CLS(WM_USER_SETFONT, OnWmUserSetFont);
				HDL_DLG_MSG_CLS(WM_GETFONT, OnWmGetFont);
				HDL_DLG_MSG_CLS(WM_DESTROY, OnWmDestroy);
				HDL_DLG_MSG_CLS(WM_EXITSIZEMOVE, OnWmExitSizeMove);

			default:
				return FALSE;
			}
			// undef Newly added Windows message.
#include "UndefNewWindowMessageCracker.h"
#include "UndefUserMessageCracker.h"
#include "UndefAncestorDialogPtr.h"
		}
		, /*_In_     LPARAM    dwInitParam   */{ (LPARAM)this }
		);
}

