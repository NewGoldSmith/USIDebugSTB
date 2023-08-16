/**
 * @file MainDlgDS.h
 * @brief ���C���_�C�A���O�N���X�̐錾
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

//	**************�ݒ�*******************
//	���C���_�C�A���O�N���X��ێ�������@�̐ݒ�B
// �O���[�o��this�|�C���^��ێ����Ă���pgMainDialog�g�����A
//	DWLP_USER���g�����B
#define USING_GROBAL_THISPOINTER
// ********* �ݒ肱���܂ŁB************

/// ���C���_�C�A���O
	class MainDialog
	{
		/// <summary>
		/// �R���\�[������_�C�A���O�ɑ��M���鐧��R�[�h�̑��M�����p
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
		/// ���O�������ݗp��OVERLAPPED�\����
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
		/// �R���\�[���n���h���̃��[�`���B
		/// "HandlerRoutine callback function"���Q�ƁB
		/// </summary>
		const PHANDLER_ROUTINE pfnConsoleHandlerRoutine;
		/// <summary>
		/// �x���I�������̃R�[���o�b�N�֐�
		/// </summary>
		const PTP_TIMER_CALLBACK pfnDelyedChildProcessTermination;
		/// <summary>
		/// �q�v���Z�X�̏I�����m�̃R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnDetectEnd;
		/// <summary>
		/// �e�v���Z�X����̓ǂݎ����J��Ԃ��R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnParentReadRepeat;
		/// <summary>
		/// �q�v���Z�X����̓ǂݎ����J��Ԃ��R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnChildReadRepeat;
		/// <summary>
		/// �e�v���Z�X�ւ̏������݂��V���A��������R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnSerializedWriteToParent;
		/// <summary>
		/// �q�v���Z�X�ւ̏������݂��V���A��������R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnSerializedWriteToChild;
		/// <summary>
		/// �e�v���Z�X���烍�O�ւ̏������݂��V���A��������R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnLvSerializedWriteFromParentToLog;
		/// <summary>
		/// �q�v���Z�X���烍�O�ւ̏������݂��V���A��������R�[���o�b�N�֐�
		/// </summary>
		const PTP_WAIT_CALLBACK pfnLvSerializedWriteFromChildToLog;
		/// <summary>
		/// ���O�t�@�C���n���h�������B�ʏ�L���[�̍Ō�ɓ����B
		/// </summary>
		const PTP_WAIT_CALLBACK pfnLvEndLog;
		/// <summary>
		/// �X���b�h�v�[�����[�h�I�u�W�F�N�g�̃R�[���o�b�N�֐��B
		/// ���̊֐��́A�X���b�h�v�[�����[�h�I�u�W�F�N�g�����������Ƃ��ɌĂяo����܂��B
		/// �񓯊����[�h����̌��ʂ��������A���[�h���ꂽ�f�[�^���q�v���Z�X�ɓ]�����A
		/// ���O�ɏ������݁A���O�t���p�C�v��ؒf���A�Đڑ��҂����J�n���܂��B
		/// </summary>
		/// <param name="Instance">�R�[���o�b�N�C���X�^���X���`����TP_CALLBACK_INSTANCE�\����</param>
		/// <param name="Context">StartThreadpoolIo�֐��ɓn���ꂽ�A�v���P�[�V������`�̃f�[�^�ւ̃|�C���^�[</param>
		/// <param name="Overlapped">���[�h����Ŏg�p���ꂽOVERLAPPED�\���̂ւ̃|�C���^�[</param>
		/// <param name="IoResult">���[�h����̌���</param>
		/// <param name="NumberOfBytesTransferred">���[�h���쒆�ɓ]�����ꂽ�o�C�g��</param>
		/// <param name="Io">���[�h�I�u�W�F�N�g���`����TP_IO�\����</param>
		const PTP_WIN32_IO_CALLBACK pfnCompletedReadRoutine;
		/// <summary>
		/// pfnConsoleHandlerRoutine���烁�b�Z�[�W���󂯎��ׂ�
		/// NamedPipe���쐬���A�ڑ��ҋ@����B
		/// �ڑ����ꂽ��pfnCompletedConnectRoutine��
		/// ���[�h�ҋ@���Z�b�g����B
		/// </summary>
		const PTP_WAIT_CALLBACK pfnCompletedConConnectRoutine;
		const PTP_WORK_CALLBACK pfnAdjustUILayoutW;
		const PTP_WIN32_IO_CALLBACK pfnCompletedWriteLogMR;
		/// <summary>
		/// CloseThreadpoolCleanupGroupMembers
		/// �̃R�[���o�b�N�֐��B
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