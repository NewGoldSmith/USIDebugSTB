//Copyright (c) 20220 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "framework.h"
#include "USICommonLib.h"

using namespace std;

namespace DUSI_COMMON {
	string ConvertCRLFA(const string& strString, const string& strCR, const string& strLF)
	{
		string strRet;
		string strCRLF("\r\n");
		string::const_iterator  ite = strString.begin();
		string::const_iterator  iteEnd = strString.end();

		if (0 < strString.size()) {

			unsigned char bNextChar = *ite++;

			while (1) {
				if (strCRLF[0] == bNextChar) {

					// 改行確定
					strRet += strCR;

					// EOF判定
					if (ite == iteEnd) {
						break;
					}

					// 1文字取得
					bNextChar = *ite++;

					if (strCRLF[1] == bNextChar) {
						// 改行確定
						strRet += strLF;

						// EOF判定
						if (ite == iteEnd) {
							break;
						}

						// 1文字取得
						bNextChar = *ite++;
					}
				}
				else if (strCRLF[1] == bNextChar) {

					// 改行確定
					strRet += strLF;

					// EOF判定
					if (ite == iteEnd) {
						break;
					}

					// 1文字取得
					bNextChar = *ite++;
					continue;
				}
				else {

					// 改行以外
					strRet += bNextChar;

					// EOF判定
					if (ite == iteEnd) {
						break;
					}

					// 1文字取得
					bNextChar = *ite++;
				}
			};
		}
		return(strRet);
	}


	wstring ConvertCRLFW(const wstring& strString, const wstring& strCR, const wstring& strLF)
	{
		wstring strRet;
		wstring strCRLF(L"\r\n");
		wstring::const_iterator ite = strString.begin();
		wstring::const_iterator iteEnd = strString.end();

		if (0 < strString.size()) {

			wchar_t bNextChar = *ite++;

			while (1) {
				if (strCRLF[0] == bNextChar) {

					// 改行確定
					strRet += strCR;

					// EOF判定
					if (ite == iteEnd) {
						break;
					}

					// 1文字取得
					bNextChar = *ite++;

					if (strCRLF[1] == bNextChar) {
						// 改行確定
						strRet += strLF;

						// EOF判定
						if (ite == iteEnd) {
							break;
						}

						// 1文字取得
						bNextChar = *ite++;
					}
				}
				else if (strCRLF[1] == bNextChar) {

					// 改行確定
					strRet += strLF;

					// EOF判定
					if (ite == iteEnd) {
						break;
					}

					// 1文字取得
					bNextChar = *ite++;
					continue;
				}
				else {

					// 改行以外
					strRet += bNextChar;

					// EOF判定
					if (ite == iteEnd) {
						break;
					}

					// 1文字取得
					bNextChar = *ite++;
				}
			};
		}
		return(strRet);
	}

	string SeparatePhrase(string& str)
	{
		string strsub;
		string dem = " \n";
		auto pos = str.find_first_of(dem);
		if (pos == string::npos)
		{
			strsub = str;
			str.clear();
			return strsub;
		}

		strsub = str.substr(0, pos);
		str.erase(0, ++pos);
		if (pos - 2 > 0 && strsub[pos - 2] == '\r')
		{
			strsub.erase(pos - 2);
		}
		size_t numpos(0);
		for (;;)
		{
			if (str[0] == string::npos)
				break;
			if (str[0] == ' ' || str[0] == '\n')
			{
				str.erase(0, 1);
				continue;
			}
			else {
				break;
			}
		}
		return strsub;
	}

	string SplitFirstLineBreak(string& str)
	{
		std::string strsub;
		auto pos = str.find("\r\n");
		if (pos == string::npos)
		{
			pos = str.find("\n");
			if (pos != string::npos)
			{
				strsub = str.substr(0, pos);
				str.erase(0, pos + 1);
			}
		}
		else {
			strsub = str.substr(0, pos);
			str.erase(0, pos + 2);
		}
		return strsub;
	}

	string SplitFirstSpace(string& str)
	{
		std::string strsub;
		auto pos = str.find(" ");
		if (pos == string::npos)
		{
			return strsub;
		}
		else {
			strsub = str.substr(0, pos);
			str.erase(0, pos + 1);
		}
		return strsub;
	}

	string SplitLastLineBreakWithLBA(string& str)
	{
		size_t pos = str.rfind('\r');
		if (pos < str.size())
		{
			if (pos + 1 < str.size())
			{
				if (str[pos + 1] == '\n')
				{
					string strsub = str.substr(0, pos + 2);
					str.erase(0, pos + 2);
					return strsub;
				}
				else {
					string strsub = str.substr(0, pos + 1);
					str.erase(0, pos + 1);
					return strsub;
				}
			}
			else {
				string strsub = str.substr(0, pos + 1);
				str.erase(0, pos + 1);
				return strsub;
			}
		}
		else {
			pos = str.rfind('\n');
			if (pos < str.size())
			{
				string strsub = str.substr(0, pos + 1);
				str.erase(0, pos + 1);
				return strsub;
			}
			else {
				return string();
			}
		}
	}

	u8string SplitLastLineBreakWithLBU8(u8string& str)
	{
		size_t pos = str.rfind(u8'\r');
		if (pos < str.size())
		{
			if (pos + 1 < str.size())
			{
				if (str[pos + 1] == u8'\n')
				{
					u8string strsub = str.substr(0, pos + 2);
					str.erase(0, pos + 2);
					return strsub;
				}
				else {
					u8string strsub = str.substr(0, pos + 1);
					str.erase(0, pos + 1);
					return strsub;
				}
			}
			else {
				u8string strsub = str.substr(0, pos + 1);
				str.erase(0, pos + 1);
				return strsub;
			}
		}
		else {
			pos = str.rfind(u8'\n');
			if (pos < str.size())
			{
				u8string strsub = str.substr(0, pos + 1);
				str.erase(0, pos + 1);
				return strsub;
			}
			else {
				return u8string();
			}
		}
	}

	string SplitLastLineBreakWithOutLB(string& str)
	{
		auto pos = str.rfind("\n");
		if (pos == str.npos)
		{
			return string();
		}
		if (pos - 1 > 0)
		{
				if (str[pos - 1] == '\r')
				{
				}
				string strsub = str.substr(0, pos - 1);
				str.erase(0, pos + 1);
				return strsub;
		}
		pos = str.rfind("\r");
		if (pos == string::npos)
		{
			return string();
		}
		string strsub = str.substr(0, pos - 1);
		str.erase(0, pos + 1);
		return strsub;
	}

	vector<string> vSplitLineBreakWithLBCodeA(string& str)
	{
		vector<string> vs;
		const string fstr("\r\n");
		size_t szRem(0);
		if (szRem = vSplitLineBreakWithLBCodeInternalA(str, 0,fstr, vs))
		{
			str.erase(0, szRem);
		}
		return vs;
	}

	size_t vSplitLineBreakWithLBCodeInternalA(const string& str, const size_t pos, const string& fstr, vector<string>& vs)
	{
		size_t posLB = str.find_first_of(fstr, pos);

		if (posLB == string::npos)
		{
			return pos;
		}
		if (str[posLB] == '\r')
		{
			if (posLB + 1 >= str.size())
			{
				vs.push_back(str.substr(pos, posLB + 1 - pos));
				return posLB + 1;
			}
			else {
				if (str[posLB + 1] == '\n')
				{
					vs.push_back(str.substr(pos, posLB + 2 - pos));
					if (posLB + 2 >= str.size())
					{
						return posLB + 2;
					}
					else {
						return vSplitLineBreakWithLBCodeInternalA(str, posLB + 2, fstr, vs);
					}
				}
				else {
					return posLB + 1;
				}
			}
		}
		else //if(str[posLB] == '\n')
		{
			vs.push_back(str.substr(pos, posLB + 1 - pos));
			if (posLB + 1 < str.size())
			{
				return vSplitLineBreakWithLBCodeInternalA(str, posLB + 1, fstr, vs);
			}
			else {
				return posLB + 1;
			}
		}
	}

	vector<u8string> vSplitLineBreakWithLBCodeU8(u8string& str)
	{
		vector<u8string> vs;
		size_t szRem(0);
		const u8string fstr(u8"\r\n");
		if (szRem = vSplitLineBreakWithLBCodeInternalU8(str, 0,fstr, vs))
		{
			str.erase(0, szRem);
		}
		return vs;
	}

	size_t vSplitLineBreakWithLBCodeInternalU8(
		const u8string& str
		, const size_t pos
		, const u8string& fstr
		, vector<u8string>& vs)
	{
		size_t posLB = str.find_first_of(fstr, pos);

		if (posLB == u8string::npos)
		{
			return pos;
		}
		if (str[posLB] == u8'\r')
		{
			if (posLB + 1 >= str.size())
			{
				vs.push_back(str.substr(pos, posLB + 1 - pos));
				return posLB + 1;
			}
			else {
				if (str[posLB + 1] == u8'\n')
				{
					vs.push_back(str.substr(pos, posLB + 2 - pos));
					if (posLB + 2 >= str.size())
					{
						return posLB + 2;
					}
					else {
						return vSplitLineBreakWithLBCodeInternalU8(str, posLB + 2, fstr, vs);
					}
				}
				else {
					return posLB + 1;
				}
			}
		}
		else// if (str[posLB] == u8'\n')
		{
			vs.push_back(str.substr(pos, posLB + 1 - pos));
			if (posLB + 1 < str.size())
			{
				return vSplitLineBreakWithLBCodeInternalU8(str, posLB + 1, fstr, vs);
			}
			else {
				return posLB + 1;
			}
		}
	}

	wstring AtoW(const string& str)
	{
		if (str.empty())
		{
			return wstring();
		}
		wstring wstr(str.size(), L'\0');
		//ANSIからユニコードへ変換
		int size(0);
		if (!(size = MultiByteToWideChar(
			CP_ACP
			, 0
			, str.data()
			, static_cast<int>(str.size())
			, wstr.data()
			, static_cast<int>(wstr.size()))))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			wstr.resize(0);
			return wstr;
		}
		wstr.resize(size);
		return wstr;
	}

	string AtoA(const string& str)
	{
		return str;
	}

	string WtoA(const wstring& wstr)
	{
		int size = WideCharToMultiByte(
			CP_ACP
			, 0
			, wstr.data()
			, static_cast<int>(wstr.size())
			, NULL
			, NULL
			, NULL
			, NULL);
		string str(size, '\0');
		str.resize(WideCharToMultiByte(
			CP_ACP
			, 0
			, wstr.data()
			, static_cast<int>(wstr.size())
			, str.data()
			, static_cast<int>(str.size())
			, NULL
			, NULL));
		return str;
	}

	wstring WtoW(const wstring& wstr)
	{
		return wstr;
	}

	u8string WtoU8(const wstring& wstr)
	{
		int size = WideCharToMultiByte(
			CP_UTF8
			, 0
			, wstr.data()
			, static_cast<int>(wstr.size())
			, NULL
			, NULL
			, NULL
			, NULL);
		u8string str(size, u8'\0');
		str.resize(WideCharToMultiByte(
			CP_UTF8
			, 0
			, wstr.data()
			, static_cast<int>(wstr.size())
			, (LPSTR)str.data()
			, static_cast<int>(str.size())
			, NULL
			, NULL));
		return str;
	}

	wstring U8toW(const u8string& u8str)
	{
		if (u8str.empty())
		{
			return wstring();
		}
		wstring wstr(u8str.size(), L'\0');
		//ANSIからユニコードへ変換
		int size(0);
		if (!(size = MultiByteToWideChar(
			CP_UTF8
			, 0
			, (LPCCH)u8str.data()
			, static_cast<int>(u8str.size())
			, wstr.data()
			, static_cast<int>(wstr.size()))))
		{
			ErrOut(GetLastError(), __FILE__, __FUNCTION__, __LINE__);
			wstr.resize(0);
			return wstr;
		}
		wstr.resize(size);
		return wstr;
	}

	u8string AtoU8(const string& str)
	{
		return WtoU8(move(AtoW(str)));
	}

	string U8toA(const u8string& u8str)
	{
		return WtoA(U8toW(u8str));
	}

	const string ErrOut(
		DWORD dw
		, LPCSTR lpszFile
		, LPCSTR lpszFunction
		, DWORD dwLine
		, LPCSTR lpszOpMessage)
	{
		LPVOID lpMsgBuf;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		stringstream ss;
		ss << lpszFile << "(" << dwLine << "): error C" << dw << ": "\
			<< TtoA((LPTSTR)lpMsgBuf)
			<< "function name: " << lpszFunction
			<< " User Message: " << lpszOpMessage << "\r\n";
		MyTRACE(ss.str().c_str());
		cerr << ss.str();
		LocalFree(lpMsgBuf);

		return ss.str().c_str();
	}

	const string ErrMes(const string& str)
	{
		cerr << str;
		OutputDebugStringA(str.c_str());
		return str;
	}

	void ParseStr(tstring const& str, const size_t pos,const tstring strEscCodes, const map<TCHAR, tstring>& mapChToStr, const LPNMLVCUSTOMDRAW lplvcd)
	{
		if (str.empty())
		{
			return;
		}
		const size_t fpos = str.find_first_of(strEscCodes, pos);
		if (fpos == basic_string<TCHAR>::npos)
		{
			Draw(str.substr(pos, str.size() - pos), lplvcd);
			return;
		}
		Draw(str.substr(pos, fpos - pos), lplvcd);
		ConvAndDraw(str.at(fpos), mapChToStr,lplvcd);
		if (fpos + 1 == basic_string<TCHAR>::npos)
		{
			return;
		}
		ParseStr(str, fpos + 1, strEscCodes, mapChToStr, lplvcd);
		return;
	}

	inline void Draw(const tstring& tstr, const LPNMLVCUSTOMDRAW& lplvcd)
	{
		ExtTextOut(lplvcd->nmcd.hdc
			, 0
			, 0
			, ETO_CLIPPED
			, &lplvcd->nmcd.rc
			, tstr.data()
			, tstr.size()
			, NULL);
	}

	inline void ConvAndDraw(
		const TCHAR ch
		, const map<TCHAR, tstring>& mapChToStr
		, LPNMLVCUSTOMDRAW lplvcd)
	{
		DrawBlue(mapChToStr.at(ch), lplvcd);
	}

	inline void DrawBlue(
		const tstring& str
		, const LPNMLVCUSTOMDRAW& lplvcd)
	{
		COLORREF DefCol = GetTextColor(lplvcd->nmcd.hdc);
		SetTextColor(lplvcd->nmcd.hdc, RGB(0, 192, 192));
		Draw(str, lplvcd);
		SetTextColor(lplvcd->nmcd.hdc, DefCol);
	}

	BOOL LoadDlgPosition(RECT& rect)
	{
		tstring tstrFileName = MakeIniPath();
		tstring tstrReturn(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_LEFT")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.left = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		tstrReturn.resize(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_TOP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.top = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		tstrReturn.resize(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_RIGHT")
			, _T("0"), tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.right = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		tstrReturn.resize(DUSI_COMMON::BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_BOTTOM")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.bottom = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		return IsRectEmpty(&rect) ? FALSE : TRUE;

	}

	void SaveDlgPosition(HWND hDlg)
	{
		tstring tstrFileName = MakeIniPath();
		tstring tstrSection(_T("SYSTEM"));
		RECT rect{};
		TCHAR buf[MAX_PATH];
		::GetWindowRect(hDlg, &rect);
		_itot_s(rect.left, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_LEFT")
			, buf
			, tstrFileName.c_str());
		_itot_s(rect.top, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_TOP")
			, buf
			, tstrFileName.c_str());
		_itot_s(rect.right, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_RIGHT")
			, buf
			, tstrFileName.c_str());
		_itot_s(rect.bottom, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_MAIN_BOTTOM")
			, buf
			, tstrFileName.c_str());
	}

	tstring MakeIniPath()
	{
		HMODULE hInstance = GetModuleHandle(NULL);
		tstring	szModulePathName(MAX_PATH, _T('\0'));
		szModulePathName.resize(
			::GetModuleFileName(
				hInstance
				, szModulePathName.data()
				, MAX_PATH));
		TCHAR	tszIniPathName[_MAX_PATH];
		TCHAR	drive[_MAX_DRIVE];
		TCHAR	dir[_MAX_DIR];
		TCHAR	fname[_MAX_FNAME];
		TCHAR	ext[_MAX_EXT];
		_tsplitpath_s(szModulePathName.c_str()
			, drive, _MAX_DRIVE
			, dir, _MAX_DIR
			, fname, _MAX_FNAME
			, ext, _MAX_EXT);
		_tmakepath_s(tszIniPathName
			, _MAX_PATH
			, drive
			, dir
			, fname
			, _T("ini"));
		tstring tstrProfileName(tszIniPathName);
		if (tstrProfileName.empty())
		{
			MessageBox(NULL, _T("メモリ不足エラー")
				, _T("DUSI_COMMON")
				, MB_OK | MB_ICONEXCLAMATION);
			return tstring();
		}
		return tstrProfileName;
	}

	BOOL LoadConsoleWindowPosition(RECT& rect)
	{
		tstring tstrFileName = MakeIniPath();
		tstring tstrReturn(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_LEFT")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.left = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_TOP")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.top = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_RIGHT")
			, _T("0"), tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.right = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		tstrReturn.resize(BUFFER_SIZE, _T('\0'));
		tstrReturn.resize(GetPrivateProfileString(
			_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_BOTTOM")
			, _T("0")
			, tstrReturn.data()
			, tstrReturn.size()
			, tstrFileName.c_str()));
		if (tstrReturn.size())
		{
			rect.bottom = _ttoi(tstrReturn.c_str());
		}
		else {
			return FALSE;
		}
		return !IsRectEmpty(&rect);
	}

	void SaveConsoleWindowPosition()
	{
		HWND hCon(NULL);
		if (!(hCon = GetConsoleWindow()))
		{
			return;
		}
		tstring tstrFileName = MakeIniPath();
		tstring tstrSection(_T("SYSTEM"));
		RECT rect{};
		TCHAR buf[MAX_PATH];
		::GetWindowRect(hCon, &rect);
		_itot_s(rect.left, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_LEFT")
			, buf
			, tstrFileName.c_str());
		_itot_s(rect.top, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_TOP")
			, buf
			, tstrFileName.c_str());
		_itot_s(rect.right, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_RIGHT")
			, buf
			, tstrFileName.c_str());
		_itot_s(rect.bottom, buf, MAX_PATH, 10);
		WritePrivateProfileString(_T("SYSTEM")
			, _T("WINDOW_POS_CONSOLE_BOTTOM")
			, buf
			, tstrFileName.c_str());
	}

	BOOL SetWindowAppropriatePosition(HWND hWnd, RECT& rect)
	{
		if (IsRectEmpty(&rect))
		{
			return FALSE;
		}
		else {
			HMONITOR hMonitor = NULL;
			MONITORINFO MonitorInfo{ 0 };
			MonitorInfo.cbSize = sizeof(MONITORINFO);
			hMonitor = MonitorFromRect(&rect, MONITOR_DEFAULTTOPRIMARY);
			GetMonitorInfo(hMonitor, &MonitorInfo);
			if (PtInRect(&MonitorInfo.rcWork, POINT{ rect.left, rect.top }))
			{
				RECT nowrect{};
				GetWindowRect(hWnd, &nowrect);
				SetWindowPos(hWnd, NULL, rect.left, rect.top
					, nowrect.right - nowrect.left, nowrect.bottom - nowrect.top
					, SWP_SHOWWINDOW);
				return TRUE;
			}
			return FALSE;
		}
	}

	BOOL SetWindowAppropriatePositionEx(HWND hWnd)
	{
		RECT rcWnd{};
		GetWindowRect(hWnd, &rcWnd);
		MONITORINFO MonitorInfo{};
		MonitorInfo.cbSize = sizeof(MONITORINFO);
		HMONITOR hMonitor;
		if (!(hMonitor = 
			MonitorFromPoint(POINT{ rcWnd.left,rcWnd.top }, MONITOR_DEFAULTTONULL)))
		{
			hMonitor =MonitorFromPoint(POINT{ rcWnd.left,rcWnd.top }
			, MONITOR_DEFAULTTONEAREST);
			GetMonitorInfo(hMonitor, &MonitorInfo);
			LONG x = MonitorInfo.rcWork.left > rcWnd.left
				? MonitorInfo.rcWork.left
				: rcWnd.left;
			LONG y = MonitorInfo.rcWork.top > rcWnd.top
				? MonitorInfo.rcWork.top
				: rcWnd.top;

			SetWindowPos(hWnd, HWND_TOP
				, x
				, y
				, rcWnd.right - rcWnd.left
				, rcWnd.bottom - rcWnd.top
				, SWP_SHOWWINDOW);
			return FALSE;
		}
		GetMonitorInfo(hMonitor, &MonitorInfo);
		{
			LONG x = MonitorInfo.rcWork.left > rcWnd.left
				? MonitorInfo.rcWork.left
				: rcWnd.left;
			LONG y = MonitorInfo.rcWork.top > rcWnd.top
				? MonitorInfo.rcWork.top
				: rcWnd.top;
			x = MonitorInfo.rcWork.right < rcWnd.right
				? MonitorInfo.rcWork.right - (rcWnd.right - rcWnd.left)
				: x;
			y = MonitorInfo.rcWork.bottom < rcWnd.bottom
				? MonitorInfo.rcWork.bottom - (rcWnd.bottom - rcWnd.top)
				: y;
			SetWindowPos(hWnd, HWND_TOP
				, x
				, y
				, rcWnd.right - rcWnd.left
				, rcWnd.bottom - rcWnd.top
				, SWP_SHOWWINDOW);
			return TRUE;
		}
		return FALSE;
	}

	string MakeTimeFormat(const string& strFormat)
	{
		char tmpbuf[128];
		time_t ltime;
		struct tm today;
		errno_t err;
		time(&ltime);
		// Convert to time structure and adjust for PM if necessary.
		err = _localtime64_s(&today, &ltime);
		if (err)
		{
			char ch[256];
			strerror_s(ch, err);
			stringstream ss;
			ss << ch << " LINE:" << __LINE__ << endl;
			cerr << ss.str();
			MyTRACE(ss.str().c_str());
		}
		// Use strftime to build a customized time string.
		strftime(tmpbuf, 128,
			strFormat.c_str(), &today);
		return string(tmpbuf);
	}

}