/**
 * @file MainDlgDS.h
 * @brief メインダイアログクラスの宣言
 * @author Gold Smith
 * @date 2022-2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#include "pch.h"
#include "UserDefinition.h"
#include "InputDialog.h"
#include "UIDialog.h"
#include "resource.h"

using namespace std;
using namespace DUSI_COMMON;
namespace UsiDebugStb {

//	**************設定*******************
//	メインダイアログクラスを保持する方法の設定。
// グローバルthisポインタを保持しているpgMainDialog使うか、
//	DWLP_USERを使うか。
#define USING_GROBAL_THISPOINTER
// ********* 設定ここまで。************

/// メインダイアログ
	class MainDialog
	{
		/// <summary>
		/// コンソールからダイアログに送信する制御コードの送信完了用
		/// </summary>
		struct OL_CON_HDL_ROUTINE : OVERLAPPED
		{
			//OL_CON_HDL_ROUTINE() :OVERLAPPED{} {};
			//OL_CON_HDL_ROUTINE(int i) = delete;
			PTP_WAIT pTPWait = NULL;
			PTP_IO pTPIO = NULL;
			HANDLE hPipe = NULL;
			string str = "";
			BOOL bEnd=FALSE;
		}olConConnectedComp,olConReadComp;
		/// <summary>
		/// ログ書き込み用のOVERLAPPED構造体
		/// </summary>
		struct OL_LOG :OVERLAPPED
		{
			string str;
		} olLog;
		HWND hDlgMain;
		const unique_ptr<TP_POOL, void(*)(PTP_POOL)>ptppUI;
		const unique_ptr<TP_CALLBACK_ENVIRON
			, void(*)(TP_CALLBACK_ENVIRON*)>pcbeUI;
		const unique_ptr<TP_CLEANUP_GROUP
			, void(*)(PTP_CLEANUP_GROUP)>ptpcgUI;
		const unique_ptr<TP_POOL, void(*)(PTP_POOL)>ptpp;
		const unique_ptr<TP_CALLBACK_ENVIRON
			, void(*)(TP_CALLBACK_ENVIRON*)>pcbe;
		const unique_ptr<TP_CLEANUP_GROUP
			, void(*)(PTP_CLEANUP_GROUP)>ptpcg;
		BOOL fHide;
		HANDLE hIn;
		HANDLE hOut;
		HANDLE hPrevIn;
		HANDLE hPrevOut;
		HANDLE hDownRd;
		HANDLE hDownWt;
		HANDLE hUpRd;
		HANDLE hUpWt;
		HWND hwndCon;
		HGLOBAL hgCopy;
		HFONT hCurrentFont;
		uintptr_t hEndThread;
		RECT rcPrevDlgPos;
		atomic<int> fEnd;
		NOTIFYICONDATA nid;
		PROCESS_INFORMATION PI;
		HANDLE hLog;
		ofstream ofLog;
		atomic<u_int> LineNum;
		UINT Lv_Status;
		int Lv_iPrevItem;
		PTP_IO pTP_IOLog;
		unique_ptr<UIDialog, void(*)(UIDialog*)> pNormalUIDlg;
		unique_ptr<UIDialog, void(*)(UIDialog*)> pMinimumUIDlg;
		string ParentReadBuf;
		string ParentRemBuf;
		string ChildReadBuf;
		string ChildRemBuf;
		BlinkingStatic BS;
		const tstring EscCode;
		const map < TCHAR, tstring> mapChToStrT;
		const unique_ptr<InputDialog
			, void(*)(InputDialog *)> pInputDlg;
		const unique_ptr< FILETIME
			, void (*)(FILETIME *)> pFT1sec;
		const unique_ptr <remove_pointer_t<HANDLE>
			, decltype(CloseHandle)*>	hEvOL;
		const unique_ptr <remove_pointer_t<HANDLE>
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
		{ _T("Line"),_T("Device") ,_T("Dir"),_T("Text") };
		enum enumHeader :const unsigned 
		{ Line, Device, Dir, Text };
		const unique_ptr<pDlgString_type, void(*)(pDlgString_type[])>
			pDlgStringArrayHeap;
		const vector<std::pair<const HWND, const RECT>> vhrcDlgItemsBasePos;
		MemoryRental<pDlgString_type> mrDlgStr;
		//const unique_ptr<string, void(*)(string[])>uqpString;
		//MemoryRental<string> mrString;
		const unique_ptr < OL_LOG, void(*)(OL_LOG[])> pOLLogArrayHeap;
		MemoryRental< OL_LOG>mrOLLog;

		basic_string<TCHAR> MakeDispLogFileName();
		BOOL ParentRead();
		BOOL ChildRead();
		BOOL DoExecute();
		BOOL ChangeSimpleUIMode();
		BOOL ChangeNormalUIMode();
		BOOL SetInitialPlacement();
		BOOL ArrangeUI();

		INT_PTR OnNM_List1(HWND hDlg, WPARAM wParam, LPNMHDR lParam);

		INT_PTR OnBnOK(HWND hwnd, HWND, UINT code);
		INT_PTR OnBnCancel(HWND hwnd, HWND, UINT code);
		INT_PTR OnBnLogPathClear(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnDeleteAll(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnExecute(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnSelectExeFile(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnSelectFont(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnSelectLogFile(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnTerminateProcess(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnBnTest1(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkDispTaskTray(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkHideWindowAtStart(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkHideChild(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkMakeLogFile(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkAddTimeStamp(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkStartChildAtStartup(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkEndAtChild(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkShowPrompt(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkShowInputDlg(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnChkSimpleUIMode(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnEdtMaxRaw(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoLogFileApp(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoLogFileTrunc(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoRejectIORedirect(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnRdoAcceptIORedirect(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnMnCopy(HWND hwnd, HWND hctrl, UINT code);
		INT_PTR OnMnTasktrayClose(HWND hwnd, HWND hctrl, UINT code);

		BOOL OnWmInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
		INT_PTR OnWmNotify(HWND hDlg, WPARAM wParam, LPNMHDR lParam);
		INT_PTR OnWmSysCommand(HWND hwnd, UINT cmd, int x, int y);
		INT_PTR OnWmCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		INT_PTR OnWmEnd(HWND hwnd);
		INT_PTR OnWmWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos);
		INT_PTR OnWmSize(HWND hwnd, UINT state, int cx, int cy);
		INT_PTR OnWmSizing(HWND hwnd, UINT state, RECT* pRect);
		INT_PTR OnWmDestroyClipBoard(HWND hwnd);
		INT_PTR OnWmNotifyIcon(HWND hDlg, WPARAM wParam, LPARAM lParam);
		INT_PTR OnWmInputConfirmed(HWND hDlg, WPARAM wParam, LPARAM lParam);
		INT_PTR OnWmMoving(HWND hDlg, LPRECT pRect);
		INT_PTR OnWmMove(HWND hwnd, int x, int y);
		INT_PTR OnWmExitSizeMove(HWND hwnd);
		INT_PTR OnWmSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw);
		INT_PTR OnWmUserSetFont(HWND hwndCtl, HFONT hfont);
		INT_PTR OnWmGetFont(HWND hwnd);
		INT_PTR OnWmDestroy(HWND hwnd);

		const _beginthreadex_proc_type pfnEndProc;
		const _beginthreadex_proc_type pfnFontSelectProc;
		void(* const pfnDSDeleter)(pDlgString_type*);
		/// <summary>
		/// コンソールハンドラのルーチン。
		/// "HandlerRoutine callback function"を参照。
		/// </summary>
		const PHANDLER_ROUTINE pfnConsoleHandlerRoutine;
		/// <summary>
		/// 遅延終了処理のコールバック関数
		/// </summary>
		const PTP_TIMER_CALLBACK pfnDelyedChildProcessTermination;
		/// <summary>
		/// 子プロセスの終了検知のコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnDetectEnd;
		/// <summary>
		/// 親プロセスからの読み取りを繰り返すコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnParentReadRepeat;
		/// <summary>
		/// 子プロセスからの読み取りを繰り返すコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnChildReadRepeat;
		/// <summary>
		/// 親プロセスへの書き込みをシリアル化するコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnSerializedWriteToParent;
		/// <summary>
		/// 子プロセスへの書き込みをシリアル化するコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnSerializedWriteToChild;
		/// <summary>
		/// 親プロセスからログへの書き込みをシリアル化するコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnLvSerializedWriteFromParentToLog;
		/// <summary>
		/// 子プロセスからログへの書き込みをシリアル化するコールバック関数
		/// </summary>
		const PTP_WAIT_CALLBACK pfnLvSerializedWriteFromChildToLog;
		/// <summary>
		/// ログファイルハンドルを閉じる。通常キューの最後に入れる。
		/// </summary>
		const PTP_WAIT_CALLBACK pfnLvEndLog;
		/// <summary>
		/// スレッドプールリードオブジェクトのコールバック関数。
		/// この関数は、スレッドプールリードオブジェクトが完了したときに呼び出されます。
		/// 非同期リード操作の結果を処理し、リードされたデータを子プロセスに転送し、
		/// ログに書き込み、名前付きパイプを切断し、再接続待ちを開始します。
		/// </summary>
		/// <param name="Instance">コールバックインスタンスを定義するTP_CALLBACK_INSTANCE構造体</param>
		/// <param name="Context">StartThreadpoolIo関数に渡されたアプリケーション定義のデータへのポインター</param>
		/// <param name="Overlapped">リード操作で使用されたOVERLAPPED構造体へのポインター</param>
		/// <param name="IoResult">リード操作の結果</param>
		/// <param name="NumberOfBytesTransferred">リード操作中に転送されたバイト数</param>
		/// <param name="Io">リードオブジェクトを定義するTP_IO構造体</param>
		const PTP_WIN32_IO_CALLBACK pfnCompletedReadRoutine;
		/// <summary>
		/// pfnConsoleHandlerRoutineからメッセージを受け取る為に
		/// NamedPipeを作成し、接続待機する。
		/// 接続されたらpfnCompletedConnectRoutineで
		/// リード待機をセットする。
		/// </summary>
		const PTP_WAIT_CALLBACK pfnCompletedConConnectRoutine;
		const PTP_WORK_CALLBACK pfnAdjustUILayoutW;
		const PTP_WIN32_IO_CALLBACK pfnCompletedWriteLogMR;
		/// <summary>
		/// CloseThreadpoolCleanupGroupMembers
		/// のコールバック関数。
		/// </summary>
		const PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfnCGCC;
	public:
		MainDialog();
		~MainDialog();
		INT_PTR DialogBoxParamW
		(_In_opt_ HINSTANCE hInstance
			, _In_opt_ HWND      hWndParent
		) noexcept;

		bool operator==(const MainDialog& other) const = delete;
};
#ifdef USING_GROBAL_THISPOINTER
	extern MainDialog* pgMainDialog;
#endif // USING_GROBAL_THISPOINTER

}