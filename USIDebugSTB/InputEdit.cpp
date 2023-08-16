//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "InputEdit.h"
using namespace UsiDebugStb;
using namespace DUSI_COMMON;

LRESULT UsiDebugStb::InputEdit::OnWmChar(HWND hwnd, TCHAR ch, int cRepeat)
{
	if (_T('\r') == ch || _T('\n') == ch)
	{
// �e�L�X�g��ǂ݂����B
		tstring str;
		str.resize(BUFFER_SIZE, _T('\0'));
		str.resize(GetWindowText(hwnd, str.data(), str.size()));
		if (GetLastError() != ERROR_SUCCESS)
		{
			throw runtime_error(ErrOut(GetLastError()));
			return FALSE;
		}

		// �X�g�����O�T�C�Y��0�ȏ�Ȃ�q�X�g���[�̏���
		if (str.size() > 0)
		{
			// �q�X�g���[�T�C�Y��numHistory�ȏ�Ȃ�Â�������폜����B
			if (vsHistory.size() >= numHistory) {
				vsHistory.erase(vsHistory.begin());
			}

			auto it = std::find(vsHistory.begin(), vsHistory.end(), str);
			if (it != vsHistory.end()) {
				vsHistory.erase(it);
			}
			vsHistory.push_back(str);

			// �C���f�b�N�X���Ō��+1�ɃZ�b�g
			Index = vsHistory.size();
		}
		// �e�_�C�A���O�֒ʒm
		::SendMessage(GetParent(hWnd), WM_KEYDOWN_ENTER, 0UL, 0L);
		return LRESULT();
	}
	FORWARD_WM_CHAR(hwnd, ch, cRepeat, pfnEditOriginPrc);
	return LRESULT();
}

LRESULT UsiDebugStb::InputEdit::OnWmKeydown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk) {
		case VK_UP: {
			// �q�X�g���[�����邩�m�F�B
			if (vsHistory.size())
			{
				Index++;
				if (Index >= 0 && Index < vsHistory.size()) {
					SetWindowText(hwnd, vsHistory[Index].c_str());
				}
				else {
					Index = vsHistory.size();
					SetWindowText(hwnd, _T(""));
				}
				return TRUE;
			}
		}
		case VK_DOWN: {
			// �q�X�g���[�����邩�m�F�B
			if (vsHistory.size())
			{
				Index--;
				if (Index >= 0 && Index < vsHistory.size()) {
					SetWindowText(hwnd, vsHistory[Index].c_str());
				}
				else {
					Index = -1;
					SetWindowText(hwnd, _T(""));
				}
				return TRUE;
			}
		}
		case VK_DELETE: {
			// �q�X�g���[�����邩�m�F�B
			if (vsHistory.size())
			{
				tstring text(BUFFER_SIZE, _T('\0'));
				text.resize(GetWindowText(hwnd, text.data(), text.size()));
				auto it = std::find(vsHistory.begin(), vsHistory.end(), text);
				if (it != vsHistory.end()) {
					vsHistory.erase(it);
					Index--;
				}
			}
		}
	}
	FORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, pfnEditOriginPrc);
	return TRUE;
}

LRESULT UsiDebugStb::InputEdit::OnWmDestroy(HWND hwnd)
{
	if (pfnEditOriginPrc)
	{
		LONG_PTR prevValue;
		SetLastError(0);
		prevValue = SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnEditOriginPrc);
		DWORD dwE;
		if (prevValue == 0 && (dwE = GetLastError()) != 0) {
			ErrOut(dwE);
			return FALSE;
		}
		if (!::SetWindowPos(hWnd
			, HWND_TOP
			, 0
			, 0
			, 0
			, 0
			, SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		pfnEditOriginPrc = NULL;
		hWnd = NULL;
	}
	return LRESULT();
}

InputEdit::InputEdit()
	:hWnd(NULL)
	, Index(0)
	, pfnEditSubPrc(
		[](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
{
	//�}�N���͉������̖��O�Ɉˑ����Ă��܂��B
#define GET_THIS_PTR \
		reinterpret_cast<InputEdit *>(GetWindowLongPtr(hwnd, GWLP_USERDATA))
#define GET_THIS_PTR_AT_INIT \
		reinterpret_cast<InputEdit*>(\
[hwnd,lParam]()\
{\
	SetWindowLongPtr(\
		hwnd\
		, GWLP_USERDATA\
		, reinterpret_cast<LONG_PTR>\
			(reinterpret_cast<LPCREATESTRUCT>\
				(lParam)->lpCreateParams));\
	return reinterpret_cast<LPCREATESTRUCT>\
			(lParam)->lpCreateParams;\
}())
#define HDL_WND_MSG_CLS( msg, fn) \
    case msg:\
	{\
	 return (HANDLE_##msg(hwnd, wParam, lParam,\
			reinterpret_cast<InputEdit*>\
			(msg==WM_CREATE ?\
			GET_THIS_PTR_AT_INIT :\
			GET_THIS_PTR)\
			->fn));\
	}
	switch (uMsg)
	{
		HDL_WND_MSG_CLS(WM_CHAR, OnWmChar);
		HDL_WND_MSG_CLS(WM_KEYDOWN, OnWmKeydown);
		HDL_WND_MSG_CLS(WM_DESTROY, OnWmDestroy);
	default:
			return CallWindowProc(
				GET_THIS_PTR->pfnEditOriginPrc
				, hwnd
				, uMsg
				, wParam
				, lParam);
	}
#undef HDL_WND_MSG_CLS
#undef GET_THIS_PTR_AT_INIT
#undef GET_THIS_PTR
	return LONG_PTR();
}
)
, pfnEditOriginPrc(NULL)
{
}

InputEdit::~InputEdit()
{
	UnSubclasssWindowX();
}

void InputEdit::SubclassWindowX(HWND hwnd)
{
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
	pfnEditOriginPrc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
	hWnd = hwnd;
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnEditSubPrc);
	if (!::SetWindowPos(hwnd
		, HWND_TOP
		, 0
		, 0
		, 0
		, 0
		, SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE))
	{
		ErrOut(GetLastError());
		return;
	}
	{
		// ����ǂݍ���
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("INPUT_EDIT")
			, _T("HISTORY")
			, _T("")
			, tstrReturn.data()
			, tstrReturn.size()
			, MakeIniPath().c_str()));
		std::stringstream ss(TtoA(tstrReturn));
		std::string item;
		while (std::getline(ss, item, ',')) {
			vsHistory.push_back(AtoT(item));
		}
		Index = vsHistory.size();
	}
}

void InputEdit::UnSubclasssWindowX()
{
	if (!vsHistory.empty()) {
		tstring history_str =
			std::accumulate(
				std::next(vsHistory.begin())
				, vsHistory.end()
				, vsHistory[0]
				, [](tstring a, tstring b) {return a + _T(',') + b; });
		WritePrivateProfileString(
			_T("INPUT_EDIT")
			, _T("HISTORY")
			, history_str.c_str()
			, MakeIniPath().c_str());
	}
	if (pfnEditOriginPrc)
	{
		LONG_PTR prevValue;
		SetLastError(0);
		prevValue=SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnEditOriginPrc);
		DWORD dwE;
		if (prevValue == 0 && (dwE=GetLastError()) != 0) {
			ErrOut(dwE);
			return ;
		}
		if (!::SetWindowPos(hWnd
			, HWND_TOP
			, 0
			, 0
			, 0
			, 0
			, SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE))
		{
			ErrOut(GetLastError());
			return;
		}
		pfnEditOriginPrc = NULL;
		hWnd = NULL;
	}
}
