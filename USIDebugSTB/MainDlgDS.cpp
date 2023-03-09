//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "MainDlgDS.h"
using namespace std;
using namespace DUSI_COMMON;
using namespace UsiDebugStb;

#ifdef USING_GROBAL_THISPOINTER
	//グローバルメインダイアログポインタ
MainDialog* UsiDebugStb::pgMainDialog(NULL);
#endif //USING_GROBAL_THISPOINTER

UsiDebugStb::MainDialog::
MainDialog()
	:hDlg(NULL)
	, fHide(FALSE)
	, pcbe(NULL)
	, ptpcg(NULL)
	, hIn(NULL)
	, hOut(NULL)
	, hDownRd(NULL)
	, hDownWt(NULL)
	, hUpRd(NULL)
	, hUpWt(NULL)
	, hCon(NULL)
	, hPrevConParent(NULL)
	, hgCopy(NULL)
	, hEndThread(NULL)
	, ptPrevDlgPos{}
	, fEnd(0)
	, nid{}
	, PI{}
	, LineNum(0)
	, Lv_Status(0)
	, Lv_iPrevItem(-1)

	, pInputDlg
	{
		new InputDialog
	,[](InputDialog* p)
	{
		delete p;
	} }

	//ファイルタイム1秒
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

		{_T('\x0'),_T("NUL")}
		,{_T('\x03'),_T("ETX")}
		,{_T('\x08'),_T("BS")}
		,{_T('\x09'),_T("HT")}
		,{_T('\x1b'),_T("ESC")}
		,{_T('\r'),_T("CR")}
		,{_T('\n'),_T("LF")}
		,{_T('\x7'),_T("BEL")}
		,{_T('\x7F'),_T("DEL")}

	}

	, pDlgStringArrayHeap
	{
		new pDlgString_type[DLG_STRING_ARRAY_SIZE]
		,
		[](pDlgString_type p[])
		{
			delete[]p;
		}
	}

	, hSemParentRead{ [this]()
	{
		try {
			HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
			if (!hSem) {
				string str = ErrOut(GetLastError()
					, __FILE__\
					, __FUNCTION__\
					, __LINE__);
				throw runtime_error(str);
			}
			return hSem;
		}
		catch (exception& e) {
		   MyTRACE(e.what());
		   exception_ptr pe = current_exception();
		   rethrow_exception(pe);
		}
		}()
		,CloseHandle
	}

	, hSemParentWrite
	{
		[this]()
		{
			try 
			{
				HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
				if (!hSem) {
					string str = ErrOut(GetLastError()
						, __FILE__\
						, __FUNCTION__\
						, __LINE__);
					throw runtime_error(str);
				}
				return hSem;
			}
			catch (exception& e) {
				ErrMes(e.what());
			   exception_ptr pe = current_exception();
			   rethrow_exception(pe);
			}
		}()
		,
		CloseHandle
	}

	, hSemChildWrite
	{
		[this]()
		{
			try {
				HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
				if (!hSem) {
					string str = ErrOut(GetLastError()
						, __FILE__\
						, __FUNCTION__\
						, __LINE__);
					throw runtime_error(str);
				}
				return hSem;
			}
			catch (exception& e) {
				ErrMes(e.what());
			   exception_ptr pe = current_exception();
			   rethrow_exception(pe);
			}
		}()
		,CloseHandle
	}

	, hSemChildRead
	{
		[]()
		{
			try {
				HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
				if (!hSem) {
					DWORD Err = GetLastError();
					throw runtime_error("CreateSemaphore.Code:" + to_string(Err) + "\r\n");
				}
				return hSem;
			}
			catch (exception& e) {
			   MyTRACE(e.what());
			   exception_ptr pe = current_exception();
			   rethrow_exception(pe);
			}
		}()
		,CloseHandle
	}

	,hSemLv{ [this]()
	{
		try {
			HANDLE hSem = CreateSemaphore(NULL,1,1,NULL);
			if (!hSem) {
				string str = ErrOut(GetLastError()
					, __FILE__\
					, __FUNCTION__\
					, __LINE__);
				throw runtime_error(str);
			}
			return hSem;
		}
		catch (exception& e) {
			ErrMes(e.what());
		   exception_ptr pe = current_exception();
		   rethrow_exception(pe);
		}
		}()
		,CloseHandle
	}

	, pDlgStringPool(pDlgStringArrayHeap.get(), DLG_STRING_ARRAY_SIZE)

	, pfnEndProc{ [](void* pvoid)->unsigned {
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(pvoid);
		pDlg->fEnd.store(1);
		if (pDlg->hgCopy)
		{
			EmptyClipboard();
		}
		pDlg->SaveProfile();
		if (pDlg->PI.hProcess)
		{
			::SendMessage(
				pDlg->hDlg
				, WM_COMMAND
				, MAKEWPARAM( IDC_BUTTON_TERMINATE_PROCESS,BN_CLICKED)
				, (LPARAM)GetDlgItem(
					pDlg->hDlg
					, IDC_BUTTON_TERMINATE_PROCESS));
		}
		CancelIoEx(pDlg->hIn, NULL);
		CancelIoEx(pDlg->hUpRd, NULL);
		CloseThreadpoolCleanupGroupMembers(pDlg->ptpcg, TRUE, NULL);
		CloseThreadpoolCleanupGroup(pDlg->ptpcg);
		Shell_NotifyIcon(NIM_DELETE, &pDlg->nid);
		DestroyIcon(pDlg->nid.hIcon);
		//自分で立てたコンソールウィンドウなら解放する。
		if (pDlg->hCon)
		{
			SaveConsoleWindowPosition();
			FreeConsole();
		}
		pDlg->SaveInputDlgPosition(pDlg->pInputDlg.get()->hDlg);

		::PostMessage(pDlg->hDlg, WM_END, NULL, NULL);
		return unsigned();
	} }

	, pfnDSDeleter{ [](pDlgString_type* pDS) {
		MainDialog* pDlg = get<0>(*pDS);
		pDlg->pDlgStringPool.Push(pDS);
	} }

	, pfnDelayEndProcesing{[]
	(
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_TIMER             Timer
	){
		CloseThreadpoolTimer(Timer);
		MainDialog* pDlg = reinterpret_cast<MainDialog*>(Context);
		pDlg->fEnd.store(1);
		//コンソール解放
		if (pDlg->hCon)
		{
			SaveConsoleWindowPosition();
			//pDlg->ConWindow.Detach();
			FreeConsole();
			::SendMessage(pDlg->hCon, WM_CLOSE, 0L, 0L);
			pDlg->hCon = NULL;
		}
		//キャンセルIO
		CancelIoEx(pDlg->hIn, NULL);
		CancelIoEx(pDlg->hUpRd,NULL);
		CloseHandle(pDlg->hUpRd);
		pDlg->hUpRd = NULL;
		CloseHandle(pDlg->hDownWt);
		pDlg->hDownWt = NULL;
		pDlg->hDownRd = NULL;
		pDlg->hUpWt = NULL;
		WaitForSingleObject(pDlg->PI.hThread, INFINITE);
		CloseHandle(pDlg->PI.hThread);
		pDlg->PI.hThread = NULL;
		WaitForSingleObject(pDlg->PI.hProcess, INFINITE);
		CloseHandle(pDlg->PI.hProcess);
		pDlg->PI.hProcess = NULL;

		pDlg->ofLog.close();

		//タスクトレイのティップを元に戻す。
		pDlg->nid.hIcon = (HICON)LoadIcon(NULL, IDI_APPLICATION);
		_tcscpy_s(pDlg->nid.szTip, 128, _T(_CRT_STRINGIZE(APPNAME)));
		int ret = (int)Shell_NotifyIcon(NIM_MODIFY, &pDlg->nid);
		//ダイアログタイトルを変更
		SetWindowText(pDlg->hDlg, _T(_CRT_STRINGIZE(APPNAME)));
		//ブリンキングスタティックをデタッチ。
		pDlg->BS.Detach();
		//Executeボタン有効
		Button_Enable(GetDlgItem(pDlg->hDlg, IDC_BUTTON_EXECUTE), TRUE);
		//Terminateボタン無効。
		Button_Enable(GetDlgItem(pDlg->hDlg, IDC_BUTTON_TERMINATE_PROCESS), FALSE);
		//リダイレクション選択ラジオボタン有効化
		Button_Enable(GetDlgItem(pDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT), TRUE);
		Button_Enable(GetDlgItem(pDlg->hDlg, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT), TRUE);
		//Inputダイアログの送信ボタン無効
		pDlg->pInputDlg->EnableSendButton(FALSE);


		//IOリダイレクトを受け入れないをチェックの場合
		//ハンドルをCloseする。(Close必要か要確認）
		if (Button_GetCheck(
			GetDlgItem(
				pDlg->hDlg
				, IDC_RADIO_IO_REDIRECT_MODE_REJECT)) == BST_CHECKED)
		{
			CloseHandle(pDlg->hIn);
			pDlg->hIn = NULL;
			CloseHandle(pDlg->hOut);
			pDlg->hOut = NULL;
		}
		//リダイレクトの場合ハンドルはCloseしない。
		else {
			pDlg->hIn = NULL;
			pDlg->hOut = NULL;
		}

		//子プログラム終了時にこのプログラムも終了するか確認。
		if (Button_GetCheck(::GetDlgItem(pDlg->hDlg, IDC_CHECK_END_AT_CHILD)) == BST_CHECKED)
		{
			PostMessage(pDlg->hDlg
				, WM_COMMAND
				, MAKEWPARAM(IDCANCEL,BN_CLICKED)
				, (LPARAM)GetDlgItem(pDlg->hDlg, IDCANCEL));
		}

		//標準ハンドルを最初の状態にする。
		if (!SetStdHandle(STD_INPUT_HANDLE, pDlg->hPrevIn))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return;
		}
		if (!SetStdHandle(STD_OUTPUT_HANDLE, pDlg->hPrevOut))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return;
		}

		//ブリンキングスタティックを非表示
		ShowWindow(
			GetDlgItem(pDlg->hDlg, IDC_STATIC_CHILD_IS_RUNNING)
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
		MainDialog* pDlg;
		if (!(pDlg = reinterpret_cast<MainDialog*>(Context)))
		{
			return;
		}
		PTP_TIMER pTPTimer(NULL);
		if(!(pTPTimer= CreateThreadpoolTimer(pDlg->pfnDelayEndProcesing,Context,pDlg->pcbe)
		))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
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
		MainDialog* pDlg;
		if (!(pDlg = reinterpret_cast<MainDialog*>(Context)))
		{
			CloseThreadpoolWait(Wait);
			return;
		}
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemParentRead.get(), 1);
		CallbackMayRunLong(Instance);

		//CancelIoされた場合
		if (pDlg->fEnd)
		{
			CloseThreadpoolWait(Wait);
			return;
		}
		pDlg->ParentReadBuf.resize(BUFFER_SIZE,'\0');
		DWORD cbRead(0);
		if (!ReadFile(pDlg->hIn, pDlg->ParentReadBuf.data(), pDlg->ParentReadBuf.size(), &cbRead, NULL))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			CloseThreadpoolWait(Wait);
			return;
		}
		//CancelIoされた場合
		if (pDlg->fEnd)
		{
			CloseThreadpoolWait(Wait);
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
				pDlgString_type* pDS = &((*pDlg->pDlgStringPool.Pull())
					= { pDlg, str });

				TP_WAIT* pTPWait(NULL);
				if (!(pTPWait = CreateThreadpoolWait(
					pDlg->pfnSerializedWriteToChild, pDS, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
					pDlg->pDlgStringPool.Push(pDS);
					CloseThreadpoolWait(Wait);
					return;
				}
				else {
					SetThreadpoolWait(pTPWait, pDlg->hSemChildWrite.get(), NULL);
				}
			}

			//読み込んだものをログへ書き込む
			{
				pDlgString_type* pDS = &((*pDlg->pDlgStringPool.Pull())
					= { pDlg, str });
				TP_WAIT* pTPWaitLog(NULL);
				if (!(pTPWaitLog = CreateThreadpoolWait(
					pDlg->pfnLvSerializedWriteFromParentToLog
					, pDS
					, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
					pDlg->pDlgStringPool.Push(pDS);
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
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
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
				//親へ書き込む
				pDlgString_type* pDSWr
					= &((*pDlg->pDlgStringPool.Pull())
						= { pDlg,str });
				TP_WAIT* pTPWait(NULL);
				if (!(pTPWait = CreateThreadpoolWait(
					pDlg->pfnSerializedWriteToParent, pDSWr, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
					CloseThreadpoolWait(Wait);
					return;
				}
				//排他制御で親へ書き込む。
				SetThreadpoolWait(pTPWait, pDlg->hSemParentWrite.get(), NULL);
			}
			{
				//読み込んだものをログへ書き込む
				pDlgString_type* pDSWr
					= &((*pDlg->pDlgStringPool.Pull())
						= { pDlg,str });
				TP_WAIT* pTPWaitLog(NULL);
				if (!(pTPWaitLog = CreateThreadpoolWait(
					pDlg->pfnLvSerializedWriteFromChildToLog, pDSWr, &*pDlg->pcbe)))
				{
					ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
					CloseThreadpoolWait(Wait);
					return;
				}
				//リストビュー操作になる為そのセマフォで制御
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
			return;
		MainDialog* pDlg = get<0>(*reinterpret_cast<pDlgString_type*>(Context));
		if (!pDlg)
			return;
		string str = move(get<1>(*reinterpret_cast<pDlgString_type*>(Context)));
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemParentWrite.get(), 1);
		DWORD Writen(0);
		if (!WriteFile(pDlg->hOut, str.data(), str.size(), &Writen, NULL))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		}
		pDlg->pDlgStringPool.Push((pDlgString_type* )Context);
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
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return;
		}
	} }

	, pfnLvSerializedWriteFromParentToLog{
	[] (
	_Inout_     PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID                 Context,
	_Inout_     PTP_WAIT              Wait,
	_In_        TP_WAIT_RESULT        WaitResult
	)
	{
		CloseThreadpoolWait(Wait);
		unique_ptr<remove_pointer_t<pDlgString_type>
			, decltype(MainDialog::pfnDSDeleter)>puDS
			= { reinterpret_cast<pDlgString_type*>(Context)
			,get<0>(*reinterpret_cast<pDlgString_type*>(Context))->pfnDSDeleter };
		_Post_ _Notnull_ MainDialog* pDlg = get<0>(*puDS.get());
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemLv.get(), 1);

		//ListViewに書き込み。
		HWND hLv = GetDlgItem(pDlg->hDlg, IDC_LIST1);
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

			//ログファイルに書き込み。
			pDlg->ofLog << right << setw(6) << TtoA(strNumLine)
				<< noshowpos << " Parent > "
				<< ConvertCRLFA(str, "\\r", "\\n")
				<< "\r\n";
		}

		//文字が残っていたらそれも書き込む
		string str;
		if ((str=move(get<1>(*puDS.get()))).size())
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

			//ログファイルに書き込み。
			pDlg->ofLog << right << setw(6) << TtoA(strNumLine)
				<< noshowpos << " Parent > "
				<< ConvertCRLFA(str, "\\r", "\\n")
				<< "\r\n";
		}
		//最大表示行数まで削除する。
		int maxraw = ::SendMessage(
			GetDlgItem(pDlg->hDlg, IDC_SPIN_MAX_RAW)
			, UDM_GETPOS32
			, 0
			, NULL);
		int c = ListView_GetItemCount(hLv);
		int over = c - maxraw;
		for (; 0 < over; --over)
		{
			ListView_DeleteItem(hLv, 0);
		}
		//リストビューをスクロールさせるか判定
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
		unique_ptr<remove_pointer_t<pDlgString_type>
			, decltype(MainDialog::pfnDSDeleter)>puDS
			= { reinterpret_cast<pDlgString_type*>(Context)
			,get<0>(*reinterpret_cast<pDlgString_type*>(Context))->pfnDSDeleter };
		_Post_ _Notnull_ MainDialog* pDlg = get<0>(*puDS.get());
		ReleaseSemaphoreWhenCallbackReturns(Instance, pDlg->hSemLv.get(), 1);

		vector<string> vs = move(vSplitLineBreakWithLBCodeA(get<1>(*puDS.get())));
		//ListViewに書き込み。
		HWND hLv = GetDlgItem(pDlg->hDlg, IDC_LIST1);
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
				, const_cast<LPTSTR>(_T("Child")));
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Dir
				, const_cast<LPTSTR>(_T("<")));
			tstring tstr = AtoT(str);
			ListView_SetItemText(hLv, Index, pDlg->enumHeader::Text
				, const_cast<LPTSTR>(tstr.c_str()));
			//ログファイルに書き込み。
			pDlg->ofLog << right << setw(6) << TtoA(strNumLine)
				<< noshowpos << "  Child < "
				<< ConvertCRLFA(str, "\\r", "\\n")
				<< "\r\n";
		}
		//最大表示行数まで削除する。
		int maxraw = ::SendMessage(
			GetDlgItem(pDlg->hDlg, IDC_SPIN_MAX_RAW)
			, UDM_GETPOS32
			, 0
			, NULL);
		int c = ListView_GetItemCount(hLv);
		int over = c - maxraw;
		for (; 0 < over; --over)
		{
			ListView_DeleteItem(hLv, 0);
		}
		//リストビューをスクロールさせるか判定
		if (ListView_GetNextItem(hLv, -1, LVNI_SELECTED) == -1)
		{
			ListView_EnsureVisible(hLv, ListView_GetItemCount(hLv) - 1, FALSE);
		}
		return;
	} }

{
	//EscCodeにmapChToStrのkeyを書き込む
	for (const auto& [key, value] : mapChToStrT)
	{
		const_cast<DUSI_COMMON::tstring*>(&EscCode)->push_back(key);
	}
}

UsiDebugStb::MainDialog::~MainDialog()
{

}

tstring UsiDebugStb::MainDialog::MakeDispLogFileName()
{
	tstring tstrLogFileName(BUFFER_SIZE, _T('\0'));
	tstrLogFileName.resize(GetPrivateProfileString(
		_T("APP")
		, _T("LOG_FILE_NAME")
		, _T("test")
		, tstrLogFileName.data()
		, tstrLogFileName.size()
		, MakeIniPath().c_str()));
	DWORD err = GetLastError();
	tstring tstrLogFileExt(BUFFER_SIZE, _T('\0'));
	tstrLogFileExt.resize(GetPrivateProfileString(
		_T("APP")
		, _T("LOG_FILE_EXT")
		, _T(".log")
		, tstrLogFileExt.data()
		, tstrLogFileExt.size()
		, MakeIniPath().c_str()));
	//ファイルネームに時間が加えられているか調べる。
	string strTimeFormat;
	HWND hCheckAddTimeStamp = GetDlgItem(hDlg, IDC_CHECK_ADD_TIME_STAMP);
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

void UsiDebugStb::MainDialog::SaveProfile()
{

	WritePrivateProfileString(_T("SYSTEM")
		, _T("INI_EXISTING")
		, _T("1")
		, MakeIniPath().c_str());

	//ウィンドウ位置を保存
	SaveDlgPosition(hDlg);

	{
		//UpDown最大行数コントロール位置を保存
		int maxraw = ::SendMessage(
			GetDlgItem(hDlg, IDC_SPIN_MAX_RAW)
			, UDM_GETPOS32
			, 0
			, NULL);
		TCHAR buf[32]{};
		_itot_s(maxraw, buf, 32, 10);
		WritePrivateProfileString(
			_T("SYSTEM")
			, _T("UP_DOWN_MAX_RAW_POS")
			, buf
			, MakeIniPath().c_str());
	}
}

void UsiDebugStb::MainDialog::LoadProfile()
{
}

BOOL UsiDebugStb::MainDialog::LoadInputDlgPosition(HWND hDlg, RECT& rect)
{
	//初めての起動かどうか確認
	tstring tstrReturn(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
	tstrReturn.resize(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
	tstrReturn.resize(GetPrivateProfileString(
		_T("SYSTEM")
		, _T("INI_EXISTING")
		, _T("0")
		, tstrReturn.data()
		, tstrReturn.size()
		, MakeIniPath().c_str()));
	if (!_ttoi(tstrReturn.c_str()))
	{
		//初めての起動の場合初期位置を設定
		{
			RECT rectMain{};
			GetWindowRect(this->hDlg, &rectMain);
			GetWindowRect(hDlg, &rect);
			int dx = rectMain.left-rect.left-(rect.right-rect.left);
			int dy = rectMain.top-rect.top;
			OffsetRect(&rect, dx, dy);
		}
		return TRUE;
	}
	else {
		//2回目以降の場合
		RECT InputRect{};
		GetWindowRect(
			pInputDlg->hDlg
			, &rect);

		tstring tstrReturn(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("INPUT_DLG")
			, _T("WINDOW_POS_INPUT_LEFT")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		int x = _ttoi(tstrReturn.c_str());
		tstrReturn.resize(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("INPUT_DLG")
			, _T("WINDOW_POS_INPUT_TOP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		int y = _ttoi(tstrReturn.c_str());
		OffsetRect(&rect, -rect.left, -rect.top);
		OffsetRect(&rect, x, y);
		SetWindowAppropriatePosition(pInputDlg->hDlg, rect);
		return TRUE;
	}
}

BOOL UsiDebugStb::MainDialog::SaveInputDlgPosition(HWND hDlg)
{
	tstring tstrFileName = MakeIniPath();
	RECT rect{};
	TCHAR buf[MAX_PATH];
	::GetWindowRect(hDlg, &rect);
	_itot_s(rect.left, buf, MAX_PATH, 10);
	WritePrivateProfileString(_T("INPUT_DLG")
		, _T("WINDOW_POS_INPUT_LEFT")
		, buf
		, tstrFileName.c_str());
	_itot_s(rect.top, buf, MAX_PATH, 10);
	WritePrivateProfileString(_T("INPUT_DLG")
		, _T("WINDOW_POS_INPUT_TOP")
		, buf
		, tstrFileName.c_str());
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::ParentRead()
{
	TP_WAIT* pTPWait(NULL);
	if (!(pTPWait = CreateThreadpoolWait(pfnParentReadRepeat, this, &*pcbe)))
	{
		ErrOut(WSAGetLastError(), __FILE__, __FUNCTION__, __LINE__);
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
		ErrOut(WSAGetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	SetThreadpoolWait(pTPWait, hSemChildRead.get(), NULL);
	return TRUE;
}

BOOL UsiDebugStb::MainDialog::DoExecute()
{
	//実行中でないか確認。
	if (PI.hProcess)
	{
		return FALSE;
	}
	//パスが設定されているか確認。
	{
		tstring tstrPath(MAX_PATH, _T('\0'));
		tstrPath.resize(
			::GetWindowText(
				GetDlgItem(hDlg, IDC_STATIC_EXE_PATH_NAME)
				, tstrPath.data()
				, tstrPath.size()));
		if(tstrPath==_T(""))
		{
			MessageBox(hDlg
				, _T("実行ファイルのパスが設定されていません。")
				, _T(_STRINGIZE(APPNAME))
				, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	//フラグエンドリセット
	fEnd.store(0);
	INVALID_HANDLE_VALUE;
	//現在の標準ハンドルを保存
	if ((hPrevIn = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	if ((hPrevOut = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	//IOリダイレクトを受け入れる場合
	if (Button_GetCheck(
		GetDlgItem(
			hDlg
			, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT)) == BST_CHECKED)
	{
		//標準入力取得
		if (!(hIn = GetStdHandle(STD_INPUT_HANDLE)))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__
				, ("HANDLE is " + to_string((INT)hIn) + "\r\n").c_str());
			MessageBox(hDlg
				, _T("標準入力を取得できませんでした。")
				, _T(_STRINGIZE(APPNAME))
				, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		//標準出力取得
		if (!(hOut = GetStdHandle(STD_OUTPUT_HANDLE)))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}
	}
	else
	{
		//IOリダイレクションを受け入れない場合、
		//コンソールを立ち上げる
		if (!AllocConsole())
		{
			if (!AttachConsole(ATTACH_PARENT_PROCESS))
			{
				ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
				return FALSE;
			}
		}
		if (!(hCon = GetConsoleWindow()))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}
		{
			RECT rect{};
			GetWindowRect(hCon, &rect);
			//閉じるボタンを削除
			if (!(::DeleteMenu(::GetSystemMenu(hCon, FALSE), SC_CLOSE, MF_BYCOMMAND)))
			{
				ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
				return FALSE;
			}
		}
		//ConWindow.Attach(hCon);
		//コンソールポジションを設定
		RECT rect{};
		if (LoadConsoleWindowPosition(rect))
		{
			RECT nowrect{};
			GetWindowRect(hCon, &nowrect);
			SetWindowPos(hCon, NULL, rect.left, rect.top
				, nowrect.right - nowrect.left, nowrect.bottom - nowrect.top
				, SWP_SHOWWINDOW);
			SetWindowAppropriatePositionEx(hCon);
		}

//コンソール表示
		ShowWindow(
			hCon
			, Button_GetCheck(
				GetDlgItem(hDlg, IDC_CHECK_SHOW_PROMPT)
			) == BST_CHECKED ? SW_SHOW : SW_HIDE);

		if ((hIn = CreateFileA("CONIN$"
			, GENERIC_READ
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, NULL
			, NULL)) == INVALID_HANDLE_VALUE)
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
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
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}
	}

	//ボタン無効化
	Button_Enable(GetDlgItem(hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT), FALSE);
	Button_Enable(GetDlgItem(hDlg, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT), FALSE);

	SECURITY_ATTRIBUTES saAttr = {};
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hDownRd, &hDownWt, &saAttr, 0))
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	if (!SetHandleInformation(hDownWt, HANDLE_FLAG_INHERIT, 0))
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (!CreatePipe(&hUpRd, &hUpWt, &saAttr, 0))
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (!SetHandleInformation(hUpRd, HANDLE_FLAG_INHERIT, 0))
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	DWORD dwMode = PIPE_WAIT;
	if (!SetNamedPipeHandleState(hUpRd, &dwMode, NULL, NULL))
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}

	tstring tstrPath(MAX_PATH, _T('\0'));
	tstrPath.resize(::GetWindowText(
		GetDlgItem(hDlg, IDC_STATIC_EXE_PATH_NAME)
		, tstrPath.data()
		, tstrPath.size()));

	tstring tstrOption(MAX_PATH - tstrPath.size(), _T('\0'));
	tstrOption.resize(::GetWindowText(
		GetDlgItem(hDlg, IDC_EDIT_EXE_OPTION_STRING)
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
			GetDlgItem(hDlg, IDC_CHECK_HIDE_CHILD))
		== BST_CHECKED ? SW_HIDE : SW_SHOWNORMAL;
	siStartInfo.dwFlags	= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	if (!CreateProcess(NULL,
		tstrPath.data(),     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&PI))  // receives PROCESS_INFORMATION
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		MessageBox(hDlg
			, _T("プロセスを作成できませんでした。")
			, _T("USIDebugStub")
			, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	//パイプ親プロセスでは使わないのでClose。
	CloseHandle(hDownRd);
	CloseHandle(hUpWt);

	// ChildProcessの起動状態監視用スレッド。
	TP_WAIT* pTPWait(NULL);
	if (!(pTPWait = CreateThreadpoolWait(pfnDetectEnd, this, &*pcbe)))
	{
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	SetThreadpoolWait(pTPWait, PI.hProcess, NULL);

	//ログファイル作成するかどうか。
	if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_MAKE_LOG_FILE)) == BST_CHECKED)
	{
		tstring tstrPath(BUFFER_SIZE, _T('\0'));
		tstrPath.resize(
			::GetWindowText(
				::GetDlgItem(hDlg, IDC_STATIC_LOG_PATH)
				, tstrPath.data()
				, tstrPath.size()));
		ofLog.open(TtoA(tstrPath + MakeDispLogFileName())
			, ios_base::out
			| ios_base::binary
			| (Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_LOG_FILE_APP_MODE))
				== BST_CHECKED ? ios_base::app : ios_base::trunc));
		if (!ofLog)
		{
			MessageBox(hDlg
				, _T("ログファイルを開けませんでした。")
				, _T("DUSIDebugStub")
				, MB_OK | MB_ICONEXCLAMATION);
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			::PostMessage(
				hDlg
				, WM_COMMAND
				, MAKEWPARAM(IDC_BUTTON_TERMINATE_PROCESS, BN_CLICKED)
				, (LPARAM)GetDlgItem(hDlg, IDC_BUTTON_TERMINATE_PROCESS));
			return FALSE;
		}
	}
	else {
		//NULLデバイスを開く。
		ofLog.open("NUL");
		if (!ofLog)
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			::PostMessage(
				hDlg
				, WM_COMMAND
				, MAKEWPARAM(IDC_BUTTON_TERMINATE_PROCESS, BN_CLICKED)
				, (LPARAM)GetDlgItem(hDlg, IDC_BUTTON_TERMINATE_PROCESS));
			return FALSE;
		}
	}

	//ブリンキングスタティック有効化
	BS.Attach(
		GetDlgItem(hDlg, IDC_STATIC_CHILD_IS_RUNNING)
		, IDC_STATIC_CHILD_IS_RUNNING);
	ChildRead();
	ParentRead();

	//タスクトレイティップ文字変更
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
		GetDlgItem(hDlg, IDC_CHECK_DISP_TASK_TRAY))
		== BST_CHECKED)
	{
		int ret = (int)Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
	//メインダイアログタイトルを変更。
	SetWindowText(hDlg, strTip.c_str());

	//Executeボタン無効
	Button_Enable(GetDlgItem(hDlg, IDC_BUTTON_EXECUTE), FALSE);

	//Terminateボタン有効
	Button_Enable(GetDlgItem(hDlg, IDC_BUTTON_TERMINATE_PROCESS), TRUE);
	
	//インプットダイアログ送信ボタン有効化
	pInputDlg->EnableSendButton(TRUE);
	return TRUE;
}

void UsiDebugStb::MainDialog::CleanupExecute()
{
}

INT_PTR UsiDebugStb::MainDialog::
OnNM_List1(HWND hDlg, WPARAM wParam, LPNMHDR pnmhdr)
{
	switch (pnmhdr->code) {
	//カスタムドロー関連
	case NM_CUSTOMDRAW: {
		LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pnmhdr;

		switch (lplvcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			return  CDRF_NOTIFYSUBITEMDRAW;

		case CDDS_POSTPAINT:
			MyTRACE("CDDS_POSTPAINT\r\n");
			return 0;

		case CDDS_PREERASE:
			MyTRACE("CDDS_PREERASE\r\n");
			return 0;

		case CDDS_POSTERASE:
			MyTRACE("CDDS_POSTERASE\r\n");
			return 0;

		case CDDS_ITEMPREPAINT:
			return CDRF_NOTIFYSUBITEMDRAW;

		case CDDS_ITEMPOSTPAINT:
			MyTRACE((string("CDDS_ITEMPOSTPAINT")\
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
				//サブアイテムメッセージ取得
				tstr.resize(SendMessage(
					pnmhdr->hwndFrom
					, LVM_GETITEMTEXT
					, (WPARAM)lplvcd->nmcd.dwItemSpec
					, (LPARAM)&m));
				//テキストアラインモード設定
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
			MyTRACE((string("CDDS_SUBITEM | CDDS_ITEMPREERASE  SubItem:")\
				+ to_string(lplvcd->iSubItem) + "\r\n").c_str());
			return /*CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT*/0;

		case CDDS_SUBITEM | CDDS_ITEMPOSTERASE:
			MyTRACE((string("CDDS_SUBITEM | CDDS_ITEMPOSTERASE  SubItem:")\
				+ to_string(lplvcd->iSubItem) + "\r\n").c_str());
			return /*CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT*/0;

		default:
			MyTRACE((to_string(lplvcd->nmcd.dwDrawStage)\
				+ "defult\r\n").c_str());
			return 0;
		}
		return 0;
	}
	//セレクトされているアイテムがクリックされた場合、
	//セレクト解除する。
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
		MyTRACE((string("LVN_GETEMPTYMARKUP")\
			+ " pnmv->Lv_iPrevItem:"\
			+ to_string(pnmv->iItem)\
			+ "\r\n").c_str());
		return 0;
	}
	case LVM_REMOVEALLGROUPS: {
		NMLISTVIEW* pnmv = (NMLISTVIEW*)pnmhdr;
		MyTRACE((string("LVM_REMOVEALLGROUPS")\
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

  //ListViewのNOTIFYメッセージデフォルト
	default:
		return 1;
	}
}

INT_PTR UsiDebugStb::MainDialog::
OnBnOK(HWND hwnd, HWND hclrl, UINT code)
{
	if (GetFocus() != GetDlgItem(hwnd, IDOK))
	{
		::PostMessage(hwnd, WM_NEXTDLGCTL, 0, 0);
		return false;
	}
	return PostMessage(hDlg
		, WM_COMMAND
		, MAKEWPARAM(IDCANCEL,BN_CLICKED)
		, (LPARAM)GetDlgItem(hDlg, IDCANCEL));
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
		ErrOut(WSAGetLastError(), __FILE__, __FUNCTION__, __LINE__);
		ExitProcess(-1);
	}
	ResumeThread((HANDLE)hEndThread);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnWmNotify(HWND hwnd, WPARAM wParam, LPNMHDR lParam)
{
	//このマクロは仮引数名に依存しています。
#define HDL_NOTIFY_CLS( ctlid, fn) \
		case ctlid: return this->fn(hwnd,wParam,lParam)
	switch (lParam->idFrom) {
		HDL_NOTIFY_CLS(IDC_LIST1, OnNM_List1);
	default:
		return 0;
	}
#undef HDL_NOTIFY_CLS
}

INT_PTR UsiDebugStb::MainDialog::
OnBnSelectLogFile(HWND hwnd, HWND hctrl, UINT code)
{
	TCHAR	tszIniPathName[_MAX_PATH+1]{};

	tszIniPathName[
		::GetPrivateProfileString(
			_T("APP")
			, _T("LOG_PATH")
			, _T("")
			, tszIniPathName
			, _MAX_PATH
			, MakeIniPath().c_str())] = _T('\0');

	TCHAR	tszOriginFileName[_MAX_FNAME + 1]{};
	tszOriginFileName[
		::GetPrivateProfileString(
			_T("APP")
			, _T("LOG_FILE_NAME")
			, _T("")
			, tszOriginFileName
			, _MAX_FNAME
			, MakeIniPath().c_str())] = _T('\0');
		TCHAR	tszOriginExt[_MAX_EXT + 1]{};
		tszOriginExt[
			::GetPrivateProfileString(
				_T("APP")
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
	ofn.hwndOwner = hDlg;
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
		DWORD Err = CommDlgExtendedError();
		ErrOut(Err, __FILE__, __FUNCTION__, __LINE__);
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
		GetDlgItem(hDlg, IDC_STATIC_LOG_PATH)
		, path);

	//拡張子が指定されているか否か。
	if (!_tcsicmp(ext, _T("")))
	{
		_tcscpy_s(ext, _T(".log"));
	}

	WritePrivateProfileString(_T("APP")
		, _T("LOG_PATH")
		, path
		, MakeIniPath().c_str());
	WritePrivateProfileString(_T("APP")
		, _T("LOG_FILE_NAME")
		, fname
		, MakeIniPath().c_str());
	WritePrivateProfileString(_T("APP")
		, _T("LOG_FILE_EXT")
		, ext
		, MakeIniPath().c_str());
	//プロファイルへの書き込みの後の結果が反映される。
	::SetWindowText(
		GetDlgItem(hDlg, IDC_STATIC_LOG_FILE_NAME)
		, MakeDispLogFileName().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnLogPathClear(HWND hwnd, HWND hctrl, UINT code)
{
	MainDialog* const pDlg = reinterpret_cast<MainDialog*>
		(GetWindowLongPtr(hDlg, DWLP_USER));
	WritePrivateProfileString(_T("APP")
		, _T("LOG_PATH")
		, _T("")
		, MakeIniPath().c_str());
	WritePrivateProfileString(_T("APP")
		, _T("LOG_FILE_NAME")
		, _T("")
		, MakeIniPath().c_str());
	WritePrivateProfileString(_T("APP")
		, _T("LOG_FILE_EXT")
		, _T("")
		, MakeIniPath().c_str());
	//プロファイルへの書き込みの後の結果が反映される。
	::SetWindowText(
		GetDlgItem(hDlg, IDC_STATIC_LOG_PATH)
		, _T(""));
	::SetWindowText(
		GetDlgItem(hDlg, IDC_STATIC_LOG_FILE_NAME)
		, pDlg->MakeDispLogFileName().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnDeleteAll(HWND hwnd, HWND hctrl, UINT code)
{
	ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST1));
	return FALSE;
}

INT_PTR UsiDebugStb::MainDialog::
OnCopy(HWND hwnd, HWND hctrl, UINT code)
{
	POINT pt{};
	GetCursorPos(&pt);
	ScreenToClient(hDlg, &pt);


	HWND hLv = GetDlgItem(hDlg, IDC_LIST1);
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
		if (!OpenClipboard(hDlg))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			return FALSE;
		}
		// Remove the current Clipboard contents
		if (!EmptyClipboard())
		{
			MessageBox(hDlg
				, _T("Cannot empty the Clipboard")
				, _T("DUSI_COMMON Stub")
				, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

		string strz = TtoA(tstrz);
		// Get the currently selected data
		hgCopy = GlobalAlloc(GMEM_MOVEABLE, (strz.size() + 1) * sizeof(char));
		char* lptstr = (char*)GlobalLock(hgCopy);
		if (memcpy_s(lptstr
			, (strz.size() + 1) * sizeof(char)
			, strz.data()
			, strz.size()))
		{
			MessageBox(hDlg
				, _T("Cannot _tcscpy_s")
				, _T("DUSI_COMMON Stub")
				, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		lptstr[strz.size()] = '\0';
		GlobalUnlock(hgCopy);
		// For the appropriate data formats...
		if (::SetClipboardData(CF_TEXT, hgCopy) == NULL)
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			MessageBox(hDlg
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
OnBnSelectFile(HWND hwnd, HWND hctrl, UINT code)
{
	tstring tstrPath(MAX_PATH, _T('\0'));
	tstrPath.resize(::GetWindowText(
		GetDlgItem(hDlg, IDC_STATIC_EXE_PATH_NAME)
		, tstrPath.data()
		, tstrPath.size()));
	// common dialog box structure, setting all fields to 0 is important
	OPENFILENAME ofn {};
	TCHAR szFile[MAX_PATH+1]{ _T('\0') };
	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
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
		GetDlgItem(hDlg, IDC_STATIC_EXE_PATH_NAME)
		, ofn.lpstrFile);
	//EXEパスネームをプロファイルに書き込み。
	WritePrivateProfileString(
		_T("APP")
		, _T("EXE_PATH_NAME")
		, ofn.lpstrFile
		, MakeIniPath().c_str());

	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnBnTerminateProcess(HWND hwnd, HWND hctrl, UINT code)
{
	const auto pThis = reinterpret_cast<MainDialog*>
		(GetWindowLongPtr(hDlg, DWLP_USER));
	if (!pThis->PI.hProcess)
	{
		return true;
	}
	if (!TerminateProcess(pThis->PI.hProcess, 3))
	{
		ErrOut(WSAGetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return FALSE;
	}
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkDispTaskTray(HWND hwnd, HWND hctrl, UINT code)
{
	int fCheck(0);
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("CHECK_DISP_TASK_TRAY")
		, (fCheck=Button_GetCheck(hctrl) == BST_CHECKED)
		? _T("1") : _T("0")
		, MakeIniPath().c_str());
	Shell_NotifyIcon(fCheck ? NIM_ADD : NIM_DELETE, &nid);
	if (!fCheck)
	{
		Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK_HIDE_WINDOW_AT_START), BST_UNCHECKED);
		WritePrivateProfileString(
			_T("SYSTEM")
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
		_T("SYSTEM")
		, _T("CHECK_HIDE_WINDOW_AT_START")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	if (Button_GetCheck(hctrl) == BST_CHECKED)
	{
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_DISP_TASK_TRAY)
			, BST_CHECKED);
		OnChkDispTaskTray(hwnd
			, GetDlgItem(hDlg, IDC_CHECK_DISP_TASK_TRAY)
			, code);
	}
	return TRUE;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkHideChild(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("CHECK_HIDE_CHILD")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkMakeLogFile(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("CHECK_MAKE_LOG_FILE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnChkAddTimeStamp
(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("CHECK_ADD_TIME_STAMP")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	//表示切替
	SetWindowText(
		GetDlgItem(hDlg, IDC_STATIC_LOG_FILE_NAME)
		, MakeDispLogFileName().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkStartChildAtStartup(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("CHECK_START_CHILD_AT_STARTUP")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnChkEndAtChild(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("CHECK_END_AT_CHILD")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnChkShowPrompt
(HWND hwnd, HWND hctrl, UINT code)
{
	int IsShow(1);
	WritePrivateProfileString(
		_T("APP")
		, _T("CHECK_SHOW_PROMPT")
		, (IsShow=(Button_GetCheck(hctrl) == BST_CHECKED))
				? _T("1") : _T("0")
		, MakeIniPath().c_str());
	if (hCon)
	{
		ShowWindow(hCon, IsShow ? SW_SHOW:SW_HIDE);
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
	//入力ダイアログを表示させるかどうか。
	ShowWindow(pInputDlg->hDlg,
		Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_SHOW_INPUT_DLG))
		== BST_CHECKED ?
		SW_SHOW : SW_HIDE);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnRdoLogFileApp(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("RADIO_LOG_FILE_APP_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnRdoLogFileTrunc(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("SYSTEM")
		, _T("RADIO_LOG_FILE_APP_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("0") : _T("1")
		, MakeIniPath().c_str());
	return true;
}

INT_PTR UsiDebugStb::MainDialog::
OnRdoRejectIORedirect(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("APP")
		, _T("RADIO_IO_REDIRECT_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("0") : _T("1")
		, MakeIniPath().c_str());

	Button_Enable(
		GetDlgItem(hDlg, IDC_CHECK_SHOW_PROMPT)
		, Button_GetCheck(hctrl) == BST_CHECKED ? TRUE:FALSE);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnRdoAcceptIORedirect
(HWND hwnd, HWND hctrl, UINT code)
{
	WritePrivateProfileString(
		_T("APP")
		, _T("RADIO_IO_REDIRECT_MODE")
		, Button_GetCheck(hctrl) == BST_CHECKED ? _T("1") : _T("0")
		, MakeIniPath().c_str());

	Button_Enable(
		GetDlgItem(hDlg, IDC_CHECK_SHOW_PROMPT)
		, Button_GetCheck(hctrl) == BST_CHECKED ? FALSE : TRUE);
	return true;
}

INT_PTR UsiDebugStb::MainDialog::OnMnTasktrayClose
(HWND hwnd, HWND hctrl, UINT code)
{
	PostMessage(hDlg
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
		MyTRACE("SC_CLOSE\r\n");
		PostMessage(hDlg
			, WM_COMMAND
			, MAKEWPARAM(IDCANCEL, BN_CLICKED)
			, (LPARAM)GetDlgItem(hwnd, IDCANCEL));
		return 0;
	}
	case SC_RESTORE: {MyTRACE("SC_RESTORE\r\n"); }
	case SC_MINIMIZE:{MyTRACE("SC_MINIMIZE\r\n"); }
	case SC_DEFAULT:{MyTRACE("SC_DEFAULT\r\n"); }
	default:
		return DefWindowProc(hwnd, WM_SYSCOMMAND, cmd, MAKELPARAM(x, y));
	}
}

INT_PTR UsiDebugStb::MainDialog::OnWmCommand
(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
//マクロは仮引数の名前に依存しています。
#define HDL_CMD_CLS( ctlid, fn) \
		case ctlid: return this->fn(hwnd,hwndCtl,codeNotify)
	switch (id) {
		HDL_CMD_CLS(IDOK, OnBnOK);
		HDL_CMD_CLS(IDCANCEL, OnBnCancel);
		HDL_CMD_CLS(IDC_BUTTON_SELECT_LOG_FILE, OnBnSelectLogFile);
		HDL_CMD_CLS(IDC_BUTTON_LOG_PATH_CLEAR, OnBnLogPathClear);
		HDL_CMD_CLS(IDC_BUTTON_DELETE_ALL, OnBnDeleteAll);
		HDL_CMD_CLS(IDM_COPY, OnCopy);
		HDL_CMD_CLS(IDC_BUTTON_EXECUTE, OnBnExecute);
		HDL_CMD_CLS(IDC_BUTTON_SELECT_FILE, OnBnSelectFile);
		HDL_CMD_CLS(IDC_BUTTON_TERMINATE_PROCESS, OnBnTerminateProcess);
		HDL_CMD_CLS(IDC_CHECK_DISP_TASK_TRAY, OnChkDispTaskTray);
		HDL_CMD_CLS(IDC_CHECK_HIDE_WINDOW_AT_START, OnChkHideWindowAtStart);
		HDL_CMD_CLS(IDC_CHECK_HIDE_CHILD, OnChkHideChild);
		HDL_CMD_CLS(IDC_CHECK_MAKE_LOG_FILE, OnChkMakeLogFile);
		HDL_CMD_CLS(IDC_CHECK_ADD_TIME_STAMP, OnChkAddTimeStamp);
		HDL_CMD_CLS
		(IDC_CHECK_START_CHILD_AT_STARTUP, OnChkStartChildAtStartup);
		HDL_CMD_CLS(IDC_CHECK_END_AT_CHILD, OnChkEndAtChild);
		HDL_CMD_CLS(IDC_RADIO_LOG_FILE_APP_MODE, OnRdoLogFileApp);
		HDL_CMD_CLS(IDC_RADIO_LOG_FILE_TRUNC_MODE, OnRdoLogFileTrunc);
		HDL_CMD_CLS(IDC_RADIO_IO_REDIRECT_MODE_REJECT, OnRdoRejectIORedirect);
		HDL_CMD_CLS(IDC_RADIO_IO_REDIRECT_MODE_ACCEPT, OnRdoAcceptIORedirect);
		HDL_CMD_CLS(IDM_TASKTRAY_CLOSE, OnMnTasktrayClose);
		HDL_CMD_CLS(IDC_CHECK_SHOW_PROMPT, OnChkShowPrompt);
		HDL_CMD_CLS(IDC_CHECK_SHOW_INPUT_DLG, OnChkShowInputDlg);
	default:
		return 0;
	}
#undef HDL_CMD_CLS
}

INT_PTR UsiDebugStb::MainDialog::OnWmEnd(HWND hwnd)
{
	::WaitForSingleObject((HANDLE)hEndThread, INFINITE);
	CloseHandle((HANDLE)hEndThread);
	EndDialog(hDlg, S_OK);
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
		MyTRACE("SWP_SHOWWINDOW\r\n");
	}
	if ((lpwpos->flags & SWP_HIDEWINDOW) == SWP_HIDEWINDOW)
	{
		MyTRACE("SWP_HIDEWINDOW\r\n");
	}
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::
OnWmSize(HWND hwnd, UINT state, int cx, int cy)
{
	HWND hLv = GetDlgItem(hDlg, IDC_LIST1);
	RECT rcLv{};
	::GetWindowRect(hLv, &rcLv);
	RECT rcDlgCl{};
	::GetClientRect(hDlg, &rcDlgCl);
	::MapWindowPoints(NULL, hDlg, (POINT*)&rcLv, 2);
	::MoveWindow(hLv, rcLv.left, rcLv.top, cx - 20, rcLv.bottom - rcLv.top, TRUE);
	switch (state)
	{
	case SIZE_MINIMIZED: {
		MyTRACE("SIZE_MINIMIZED\r\n");
		break;
	}
	case SIZE_RESTORED: {
		MyTRACE("SIZE_RESTORED\r\n");
		break;
	}
	default:
		break;
	}
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmDestroyClipBoard(HWND hwnd)
{
	if (hgCopy != NULL) {
		::GlobalUnlock(hgCopy);
		::GlobalFree(hgCopy);
		hgCopy = NULL;
	}
	return 0;
}

INT_PTR MainDialog::OnWmNotifyIcon(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == reinterpret_cast<UINT>(hDlg)) {
		switch (lParam) {
		case WM_LBUTTONDOWN:
		{
			if (fHide)
			{
				fHide = FALSE;
				RECT rect{};
				LoadDlgPosition(rect);
				ShowWindow(hDlg, SW_SHOW);
				ShowWindow(pInputDlg->hDlg, SW_SHOW);
				if (hCon)
				{
					ShowWindow(hCon, SW_SHOW);
				}
				break;
			}
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK_HIDE_WINDOW_AT_START))
				== BST_CHECKED)
			{
				fHide = TRUE;
				ShowWindow(hDlg, SW_HIDE);
				ShowWindow(pInputDlg->hDlg, SW_HIDE);
				if (hCon)
				{
					ShowWindow(hCon, SW_HIDE);
				}
				break;
			}
			if (IsIconic(hDlg))
			{
				ShowWindow(hDlg, SW_RESTORE);
				ShowWindow(pInputDlg->hDlg, SW_RESTORE);
				if (hCon)
				{
					ShowWindow(hCon, SW_RESTORE);
				}
			}
			else {
				ShowWindow(hDlg, SW_MINIMIZE);
				ShowWindow(pInputDlg->hDlg, SW_MINIMIZE);
				if (hCon)
				{
					ShowWindow(hCon, SW_MINIMIZE);
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
	}
	return 0;
}

INT_PTR UsiDebugStb::MainDialog::OnWmInputConfirmed
(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	string str = TtoA(pInputDlg->strCmd) + TtoA(pInputDlg->GetLB());
	//Childへ転送
	{
		pDlgString_type* pDS = &((*pDlgStringPool.Pull())
			= { this, str});

		TP_WAIT* pTPWait(NULL);
		if (!(pTPWait = CreateThreadpoolWait(
			pfnSerializedWriteToChild
			, pDS
			, &*pcbe)))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			pDlgStringPool.Push(pDS);
			return FALSE;
		}
		SetThreadpoolWait(pTPWait, hSemChildWrite.get(), NULL);
	}

	//読み込んだものをログへ書き込む
	{
		pDlgString_type* pDS = &((*pDlgStringPool.Pull())
			= { this, str });
		TP_WAIT* pTPWaitLog(NULL);
		if (!(pTPWaitLog = CreateThreadpoolWait(
			pfnLvSerializedWriteFromParentToLog
			, pDS
			, &*pcbe)))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			pDlgStringPool.Push(pDS);
			return FALSE;
		}
		else {
			SetThreadpoolWait(pTPWaitLog, hSemLv.get(), NULL);
		}
	}
	return INT_PTR();
}

INT_PTR MainDialog::OnWmMoving(HWND hDlg, LPRECT pRect)
{
	if (ptPrevDlgPos.x==0&&ptPrevDlgPos.y==0)
	{
		ptPrevDlgPos=((POINT*)pRect)[0];
	}
	else {
		LONG x = pRect->left - ptPrevDlgPos.x;
		LONG y = pRect->top - ptPrevDlgPos.y;
		MyTRACE(
			"x="+to_string(x) + " "
			"y="+to_string(y)+"\r\n");
		RECT rcInput{};
		GetWindowRect(pInputDlg->hDlg, &rcInput);
		OffsetRect(&rcInput, x, y);
		MoveWindow(pInputDlg->hDlg
			, rcInput.left
			, rcInput.top
			, rcInput.right - rcInput.left
			, rcInput.bottom - rcInput.top
			, TRUE);
		SetWindowAppropriatePositionEx(pInputDlg->hDlg);
		ptPrevDlgPos = ((POINT*)pRect)[0];
	}
	return 0;
}

BOOL UsiDebugStb::MainDialog::OnWmInitDialog
(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	MyTRACE("Start InitDialog\r\n");
	//スレッドプールクリーンナップ設定
	if (!(ptpcg = CreateThreadpoolCleanupGroup())) {
		ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
		return false;
	}
	SetThreadpoolCallbackCleanupGroup(pcbe, ptpcg, NULL);

	this->hDlg = hDlg;
	//リストビュー初期設定
	try {

		const auto hListView = GetDlgItem(hDlg, IDC_LIST1);
		int i(0);
		static const LVCOLUMN cols[]
		{
			{ /*UINT   mask       */LVCF_TEXT | LVCF_WIDTH
			, /*int    fmt        */0
			, /*int    cx         */40
			, /*LPTSTR pszText    */const_cast<LPTSTR>(vstrHeader[enumHeader::Line].c_str())
			, /*int    cchTextMax */0
			, /*int    iSubItem   */0
			, /*int    iImage     */0
			, /*int    iOrder     */0
			}
			,
			{ /*UINT   mask       */LVCF_TEXT | LVCF_WIDTH
			, /*int    fmt        */0
			, /*int    cx         */50
			, /*LPTSTR pszText    */const_cast<LPTSTR>(vstrHeader[enumHeader::Device].c_str())
			, /*int    cchTextMax */0
			, /*int    iSubItem   */0
			, /*int    iImage     */0
			, /*int    iOrder     */0
			}
			,
			{ /*UINT   mask       */LVCF_TEXT | LVCF_WIDTH
			, /*int    fmt        */0
			, /*int    cx         */30
			, /*LPTSTR pszText    */const_cast<LPTSTR>(vstrHeader[enumHeader::Dir].c_str())
			, /*int    cchTextMax */0
			, /*int    iSubItem   */0
			, /*int    iImage     */0
			, /*int    iOrder     */0
			}
			,
			{ /*UINT   mask       */LVCF_TEXT | LVCF_WIDTH
			, /*int    fmt        */0
			, /*int    cx         */560
			, /*LPTSTR pszText    */const_cast<LPTSTR>(vstrHeader[enumHeader::Text].c_str())
			, /*int    cchTextMax */0
			, /*int    iSubItem   */0
			, /*int    iImage     */0
			, /*int    iOrder     */0
			}
		};
		ListView_SetExtendedListViewStyleEx(hListView
			, LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE
			, LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
		for (const auto& r : cols)
			ListView_InsertColumn(hListView, static_cast<int>(&r - cols), &r);
	}
	catch (const std::exception& e) {
		OutputDebugStringA(__FILE__ "(" _CRT_STRINGIZE(__LINE__) \
			"): " __FUNCTION__ "で例外が投入されました。...");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
		EndDialog(hDlg, IDCANCEL);
		return true;
	}

	//メインダイアログポジション設定
	RECT rect{};
	if (!LoadDlgPosition(rect))
	{
		SetWindowAppropriatePositionEx( hDlg);
		SaveDlgPosition(hDlg);
	}
	else {
		SetWindowAppropriatePosition(hDlg, rect);
	}
	//MessageBox(hDlg
	//	, _T("一時停止")
	//	, _T("DUSI Stub")
	//	, MB_OK | MB_ICONEXCLAMATION);

	{
		//インプットダイアログ作成
		pInputDlg.get()
			->CreateDialogX(
				(HINSTANCE)GetWindowLongPtr(
					hDlg
					, GWLP_HINSTANCE)
				, hDlg);
		RECT rect{};
		LoadInputDlgPosition(pInputDlg->hDlg, rect);
			::SetWindowPos(
				pInputDlg.get()->hDlg
				, HWND_TOP
				, rect.left
				, rect.top
				, rect.right - rect.left
				, rect.bottom - rect.top
				, SWP_ASYNCWINDOWPOS);
		SetWindowAppropriatePositionEx(pInputDlg->hDlg);
	}

	//最大行数UpDownボタン読み込み
	{
		tstring tstr(BUFFER_SIZE, _T('\0'));
		tstr.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("UP_DOWN_MAX_RAW_POS")
			, _T("100")
			, tstr.data()
			, tstr.size()
			, MakeIniPath().c_str()));
		const auto hUpDownButton = GetDlgItem(hDlg, IDC_SPIN_MAX_RAW);
		const auto hMaxRawEdit = GetDlgItem(hDlg, IDC_EDIT_MAX_RAW);
		::SendMessage(hUpDownButton, UDM_SETBUDDY, (WPARAM)hMaxRawEdit, 0);
		::SendMessage(hUpDownButton, UDM_SETRANGE32, 0, MAXINT);
		::SendMessage(hUpDownButton, UDM_SETPOS32, 0, _tstoi64(tstr.c_str()));
	}
	
	//プロファイル読み込み
	LoadProfile();

	//*************UI設定****************

	{
		//EXEファイル名表示
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("APP")
			, _T("EXE_PATH_NAME")
			, _T("")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		SetWindowText(
			GetDlgItem(hDlg, IDC_STATIC_EXE_PATH_NAME)
			, tstrReturn.c_str());
	}

	//ログファイル作成するか否か。
	{
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_MAKE_LOG_FILE")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_MAKE_LOG_FILE)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//ログファイル名にタイムスタンプを加えるか否か。
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_ADD_TIME_STAMP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_ADD_TIME_STAMP)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//ログファイル名表示
		SetWindowText(
			GetDlgItem(hDlg, IDC_STATIC_LOG_FILE_NAME)
			, MakeDispLogFileName()
			.c_str());
	}
	{
		//ログパス表示
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(::GetPrivateProfileString(
			_T("APP")
			, _T("LOG_PATH")
			, _T("")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		::SetWindowText(GetDlgItem(hDlg
			, IDC_STATIC_LOG_PATH)
			, tstrReturn.c_str());
	}
	{
		//ログファイルオープンモードラジオボタン設定
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("RADIO_LOG_FILE_APP_MODE")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_LOG_FILE_APP_MODE)
			, tstrReturn == _T("1") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_LOG_FILE_TRUNC_MODE)
			, tstrReturn != _T("1") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//標準IOモードラジオボタン設定
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("APP")
			, _T("RADIO_IO_REDIRECT_MODE")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT)
			, tstrReturn == _T("0") ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_RADIO_IO_REDIRECT_MODE_ACCEPT)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//タスクトレイにアイコンを表示するかどうか。
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_DISP_TASK_TRAY")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_DISP_TASK_TRAY)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//タスクトレイ設定。nidは取り合えず初期化。
		nid.cbSize = sizeof(nid);
		nid.hWnd = hDlg;
		//複数のアイコンを表示したときの識別ID。なんでもいい。
		nid.uID = reinterpret_cast<UINT>(hDlg);
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_NOTIFYICON; //WM_USER以降の定数。
		nid.hIcon = (HICON)LoadIcon(NULL, IDI_APPLICATION);
		_tcscpy_s(nid.szTip, 128, _T(_CRT_STRINGIZE(APPNAME)));
		if (Button_GetCheck(
			GetDlgItem(hDlg, IDC_CHECK_DISP_TASK_TRAY))
			== BST_CHECKED)
		{
			int ret = (int)Shell_NotifyIcon(NIM_ADD, &nid);
		}
	}
	{
		//起動時ウインドウを隠すかどうか
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_HIDE_WINDOW_AT_START")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		BOOL fCheck(FALSE);
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_HIDE_WINDOW_AT_START)
			, fHide=tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
		ShowWindow(pInputDlg->hDlg, fHide ? SW_HIDE:SW_SHOW);
	}
	{
		//起動時子ウインドウを隠すかどうか
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_HIDE_CHILD")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_HIDE_CHILD)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//起動時子プログラムを起動させるかどうか。
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_START_CHILD_AT_STARTUP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_START_CHILD_AT_STARTUP)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//子プログラム終了時プログラムを終了させるかどうか。
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("CHECK_END_AT_CHILD")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_END_AT_CHILD)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//プロンプトを表示させるかどうか。
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("APP")
			, _T("CHECK_SHOW_PROMPT")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_SHOW_PROMPT)
			, tstrReturn != _T("0") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//入力ダイアログを表示させるかどうか。
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("INPUT_DLG")
			, _T("CHECK_SHOW_INPUT_DLG")
			, _T("1")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		Button_SetCheck(
			GetDlgItem(hDlg, IDC_CHECK_SHOW_INPUT_DLG)
			, tstrReturn != _T("1") ? BST_CHECKED : BST_UNCHECKED);
	}
	{
		//終了ボタンを無効にする。
		Button_Enable(GetDlgItem(hDlg, IDC_BUTTON_TERMINATE_PROCESS), FALSE);
	}
	//ダイアログタイトルを変更
	SetWindowText(hDlg, _T(_CRT_STRINGIZE(APPNAME)));

	//ブリンキングスタティックを非表示に
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CHILD_IS_RUNNING), SW_HIDE);

	//***********************************************************************
	{
	//「プロンプトを表示する」を有効にするかどうか
		Button_Enable(
			GetDlgItem(hDlg, IDC_CHECK_SHOW_PROMPT)
			, Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_IO_REDIRECT_MODE_REJECT))
			== BST_CHECKED ? TRUE : FALSE);
	}
	{
		//UIの設定が終わったところで、
		//入力ダイアログボックスを表示するか否か。
		ShowWindow(GetDlgItem(
			hDlg
			, IDC_CHECK_SHOW_INPUT_DLG)
			, Button_GetCheck(
				GetDlgItem(pInputDlg->hDlg
				,IDC_CHECK_SHOW_INPUT_DLG))
			== BST_CHECKED ?SW_SHOW:SW_HIDE);
	}
	{
		//UIの設定が終わったところで、起動時子プログラムを起動するか否か。
		if (Button_GetCheck(GetDlgItem(
			hDlg
			, IDC_CHECK_START_CHILD_AT_STARTUP))
			== BST_CHECKED)
		{
			DoExecute();
		}
	}
	MyTRACE("End InitDialog\r\n");
	return true;
}

INT_PTR UsiDebugStb::MainDialog::DialogBoxParamW
(_In_opt_ HINSTANCE hInstance
	, _In_opt_ HWND      hWndParent
	, PTP_CALLBACK_ENVIRON pcbeIn
) noexcept
{
	pcbe = pcbeIn;
	return /*WINUSERAPI INT_PTR WINAPI*/::DialogBoxParamW
	( /*_In_opt_ HINSTANCE hInstance     */hInstance
		, /*_In_     LPCWSTR   lpTemplateName*/MAKEINTRESOURCEW(IDD_MAIN_DLG)
		, /*_In_opt_ HWND      hWndParent    */nullptr
		, /*_In_opt_ DLGPROC   lpDialogFunc  */
		[](
			HWND hDlg
			, UINT uMsg
			, WPARAM wParam
			, LPARAM lParam)->INT_PTR
		{
			//マクロは仮引数の名前に依存しています。
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
#define HANDLE_WM_NOTIFYICON(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(wParam),(LPARAM)(lParam)))
#define HANDLE_WM_END(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd)))
#define HANDLE_WM_INPUT_CONFIRMED(hwnd, wParam,lParam,fn) \
((fn)((hwnd),(WPARAM)(wParam),(LPARAM)(lParam)))
#define HANDLE_WM_MOVING(hwnd, wParam,lParam,fn) \
((fn)((hwnd),(LPRECT)(lParam)))

			switch (uMsg)
			{
				HDL_DLG_MSG_CLS(WM_NOTIFY, OnWmNotify);
				HDL_DLG_MSG_CLS(WM_SYSCOMMAND, OnWmSysCommand);
				HDL_DLG_MSG_CLS(WM_COMMAND, OnWmCommand);
				HDL_DLG_MSG_CLS(WM_END, OnWmEnd);
				HDL_DLG_MSG_CLS(WM_WINDOWPOSCHANGING,
					OnWmWindowPosChanging);
				HDL_DLG_MSG_CLS(WM_SIZE, OnWmSize);
				HDL_DLG_MSG_CLS(WM_DESTROYCLIPBOARD,
					OnWmDestroyClipBoard);
				HDL_DLG_MSG_CLS(WM_NOTIFYICON, OnWmNotifyIcon);
				HDL_DLG_MSG_CLS(WM_INPUT_CONFIRMED, OnWmInputConfirmed);
				HDL_DLG_MSG_CLS(WM_MOVING, OnWmMoving);
				HDL_DLG_MSG_CLS(WM_INITDIALOG, OnWmInitDialog);
			default:
				return FALSE;
			}
#undef HANDLE_WM_MOVING
#undef HANDLE_WM_INPUT_CONFIRMED
#undef HANDLE_WM_END
#undef HANDLE_WM_NOTIFYICON

#undef HDL_DLG_MSG_CLS
#undef GET_THIS_PTR_AT_INIT
#undef GET_THIS_PTR
		}
		, /*_In_     LPARAM    dwInitParam   */{ (LPARAM)this }
		);
}
