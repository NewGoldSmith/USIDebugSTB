/**
 * @file USICommonLib.cpp
 * @brief 共通ライブラリ実装
 * @author Gold Smith
 * @date 2022-2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#include "pch.h"
#include "framework.h"
#include "USICommonLib.h"

using namespace std;

namespace DUSI_COMMON {
	string ConvertLineEndingsWithCustomStringsA(const string& strString, const string& strCR, const string& strLF)
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


	wstring ConvertLineEndingsWithCustomStringsW(const wstring& strString, const wstring& strCR, const wstring& strLF)
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

	string ReplaceCRLFWithLFA(const string& strString)
	{
		std::string str = strString;
		std::string::size_type pos = 0;
		while ((pos = str.find("\r\n", pos)) != std::string::npos) {
			str.replace(pos, 2, "\n");
			pos++;
		}
		return str;
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
			ErrOut(GetLastError());
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
			ErrOut(GetLastError());
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

	string UnescapeStringA(const string& input)
	{
		std::map<std::string, char> escape_sequences = {
			 {"\\a", '\a'},
			 {"\\b", '\b'},
			 {"\\f", '\f'},
			 {"\\n", '\n'},
			 {"\\r", '\r'},
			 {"\\t", '\t'},
			 {"\\v", '\v'},
			 {"\\\\", '\\'},
			 {"\\'", '\''},
			 {"\\\"", '\"'},
			 {"\\?", '\?'}
		};

		std::string output;
		for (size_t i = 0; i < input.size(); ++i) {
			if (input[i] == '\\' && i + 1 < input.size()) {
				if (input[i + 1] == 'x' && i + 3 < input.size()) {
					char c = static_cast<char>(std::stoi(input.substr(i + 2, 2), nullptr, 16));
					output += c;
					i += 3;
				}
				else {
					std::string key = input.substr(i, 2);
					if (escape_sequences.count(key)) {
						output += escape_sequences[key];
						++i;
					}
					else {
						output += input[i];
					}
				}
			}
			else {
				output += input[i];
			}
		}

		return output;
	}

	wstring UnescapeStringW(const wstring& input)
	{
		std::map<std::wstring, wchar_t> escape_sequences = {
			 {L"\\a", L'\a'},
			 {L"\\b", L'\b'},
			 {L"\\f", L'\f'},
			 {L"\\n", L'\n'},
			 {L"\\r", L'\r'},
			 {L"\\t", L'\t'},
			 {L"\\v", L'\v'},
			 {L"\\\\", L'\\'},
			 {L"\\'", L'\''},
			 {L"\\\"", L'\"'},
			 {L"\\?", L'\?'}
		};

		std::wstring output;
		for (size_t i = 0; i < input.size(); ++i) {
			if (input[i] == L'\\' && i + 1 < input.size()) {
				if (input[i + 1] == L'x' && i + 3 < input.size()) {
					wchar_t c = static_cast<wchar_t>(std::stoi(input.substr(i + 2, 2), nullptr, 16));
					output += c;
					i += 3;
				}
				else {
					std::wstring key = input.substr(i, 2);
					if (escape_sequences.count(key)) {
						output += escape_sequences[key];
						++i;
					}
					else {
						output += input[i];
					}
				}
			}
			else {
				output += input[i];
			}
		}

		return output;
	}

	const string ErrOut_(
		DWORD dw
		, LPCSTR lpszFile
		, LPCSTR lpszFunction
		, DWORD dwLine
		, LPCSTR lpszOpMessage)
	{
		LPVOID lpMsgBuf;

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_IGNORE_INSERTS
			| FORMAT_MESSAGE_MAX_WIDTH_MASK
			, NULL
			, dw
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, (LPTSTR)&lpMsgBuf
			, 0, NULL);
		string strOpMessage;
		if (strlen(lpszOpMessage))
		{
			strOpMessage =(string) "User Message:\"" + lpszOpMessage + "\"";
		}
		stringstream ss;
		ss << lpszFile << "(" << dwLine << "): error C" << dw << ": "\
			<< TtoA((LPTSTR)lpMsgBuf)
			<< "function name: " << lpszFunction
			<< strOpMessage << "\r\n";
		OutputDebugStringA(ss.str().c_str());
		cerr << ss.str();
		LocalFree(lpMsgBuf);

		return ss.str().c_str();
	}

	const string ErrMes_(
		const string & lpszMessage
		, const string &lpszFile
		, const string &lpszFunction
		, const DWORD dwLine)
	{
		string strFileName;
		if (lpszFile == "")
		{
			strFileName = "";
		}
		else {
			strFileName = lpszFile;
		}
		string strdwLine;
		if (dwLine == MAXDWORD)
		{
			strdwLine = "";
		}
		else {
			strdwLine = "(" + to_string((int)dwLine) + "): ";
		}
		string strFunction;
		if (lpszFunction == "")
		{
			strFunction = "";
		}
		else {
			strFunction = string(": ") + "function name: " + lpszFunction;
		}
		stringstream ss;
		ss << strFileName << strdwLine << "\""<<lpszMessage<<"\""
			<< strFunction
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
		cerr << ss.str();
		return ss.str();
	}

	std::string ToUpper(const std::string& str)
	{
		std::string result;
		result.resize(str.size());
		std::transform(str.begin(), str.end(), result.begin(), ::toupper);
		return result;
	}

	std::wstring ToUpper(const std::wstring& str)
	{
		std::wstring result;
		result.resize(str.size());
		std::transform(str.begin(), str.end(), result.begin(), ::towupper);
		return result;
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
		COLORREF PrevColRef=SetTextColor(lplvcd->nmcd.hdc, RGB(0, 192, 192));
		Draw(str, lplvcd);
		SetTextColor(lplvcd->nmcd.hdc, PrevColRef);
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

	BOOL SetWindowAppropriatePosition(HWND hWnd, RECT& rect)
	{
		if (IsRectEmpty(&rect))
		{
			return FALSE;
		}

		MONITORINFO MI{};
		MI.cbSize = sizeof(MONITORINFO);
		HMONITOR hMonitor = MonitorFromRect(&rect, MONITOR_DEFAULTTOPRIMARY);
		GetMonitorInfo(hMonitor, &MI);
		if (PtInRect(&MI.rcWork, POINT{ rect.left, rect.top }))
		{
			SetWindowPos(hWnd, NULL, rect.left, rect.top
				, rect.right - rect.left, rect.bottom - rect.top
				, SWP_SHOWWINDOW);
			return TRUE;
		}
		else {
			// モニターのサイズを取得する
			int screenWidth = MI.rcWork.right - MI.rcWork.left;
			int screenHeight = MI.rcWork.bottom - MI.rcWork.top;

			// ダイアログボックスのサイズを取得する
			RECT rect;
			GetWindowRect(hWnd, &rect);
			int dialogWidth = rect.right - rect.left;
			int dialogHeight = rect.bottom - rect.top;

			// ダイアログボックスの位置を計算する
			int x = (screenWidth - dialogWidth) / 2;
			int y = (screenHeight - dialogHeight) / 2;

			// ダイアログボックスの位置を設定する
			SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
			return TRUE;
		}
	}

	BOOL SetWindowAppropriatePositionEx(HWND hWnd)
	{
		RECT rcWnd{};
		GetWindowRect(hWnd, &rcWnd);
		MONITORINFO MonitorInfo{};
		MonitorInfo.cbSize = sizeof(MONITORINFO);
		HMONITOR hMonitor;
		if (!MonitorFromPoint(POINT{ rcWnd.left,rcWnd.top }, MONITOR_DEFAULTTONULL)
			|| !MonitorFromPoint(POINT{ rcWnd.left,rcWnd.bottom }, MONITOR_DEFAULTTONULL)
			|| !MonitorFromPoint(POINT{ rcWnd.right,rcWnd.top }, MONITOR_DEFAULTTONULL)
			|| !MonitorFromPoint(POINT{ rcWnd.right,rcWnd.bottom }, MONITOR_DEFAULTTONULL))
		{
			hMonitor =MonitorFromPoint(POINT{ rcWnd.left,rcWnd.top }
			, MONITOR_DEFAULTTONEAREST);
			GetMonitorInfo(hMonitor, &MonitorInfo);
			LONG x = MonitorInfo.rcWork.left > rcWnd.left
				? MonitorInfo.rcWork.left
				: rcWnd.left;
			x = MonitorInfo.rcWork.right-(rcWnd.right-rcWnd.left) < rcWnd.right
				? MonitorInfo.rcWork.right - (rcWnd.right - rcWnd.left)
				: x;
			LONG y = MonitorInfo.rcWork.top > rcWnd.top
				? MonitorInfo.rcWork.top
				: rcWnd.top;
			y= MonitorInfo.rcWork.bottom - (rcWnd.bottom - rcWnd.top) < rcWnd.bottom
				? MonitorInfo.rcWork.bottom - (rcWnd.bottom - rcWnd.top)
				: y;
			SetWindowPos(hWnd, HWND_TOP
				, x
				, y
				, 0
				, 0
				, 
			 SWP_NOSIZE
			| SWP_NOZORDER);
			return FALSE;
		}
		return FALSE;
	}

	HWND GetMainWindowHandle()
	{
		DWORD pid = GetCurrentProcessId();
		HWND hwnd = GetTopWindow(0);
		while (hwnd) {
			DWORD p = 0;
			DWORD ok = GetWindowThreadProcessId(hwnd, &p);
			if (ok && p == pid) {
				return hwnd;
			}
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}
		return 0;
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
			_D(ss.str().c_str());
		}
		// Use strftime to build a customized time string.
		strftime(tmpbuf, 128,
			strFormat.c_str(), &today);
		return string(tmpbuf);
	}

	DOUBLE GetPointSizeFromFontHandle(HWND hDlg, HFONT hFontIn)
	{
		LOGFONTW lf;
		if (GetObjectW(HGDIOBJ(hFontIn), sizeof lf, &lf)) {
			UINT point(0);
			if (lf.lfHeight > 0)
			{
				HDC hdc = GetDC(hDlg);
				HFONT hFont = CreateFontIndirect(&lf);
				HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
				TEXTMETRIC tm;
				GetTextMetrics(hdc, &tm);
				int ilpy=GetDeviceCaps(hdc, LOGPIXELSY);
				SelectObject(hdc, hOldFont);
				DeleteObject(hFont);
				point=MulDiv((tm.tmHeight - tm.tmInternalLeading)
					, 72
					, ilpy);
				ReleaseDC(hDlg, hdc);
			}
			else if (lf.lfHeight < 0) {
				//フォントに関する情報が d に得られた
				const auto hDC = GetDC(NULL);
				int const ilpy = GetDeviceCaps(hDC, LOGPIXELSY);
				point = -MulDiv(lf.lfHeight, 72, ilpy);
				ReleaseDC(hDlg, hDC);
			}
			return point < 0 ? 0 : point;
			//この時点で point にポイントサイズが設定されている
		}
		return 0;
	}

	HFONT CreateFontWithPointSize(HWND hwnd, HFONT hFont, int points)
	{
		LOGFONTW lf;
		if (!GetObject(hFont, sizeof(LOGFONTW), &lf))
		{
			return NULL;
		}
		HDC hdc(NULL);
		if (!(hdc=GetDC(hwnd)))
		{
			return NULL;
		}
		int logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(hwnd, hdc);
		lf.lfHeight = -MulDiv(points, logpixelsy, 72);
		return CreateFontIndirect(&lf);
	}

	BOOL ReSizeDlg(const HWND hDlg, const DOUBLE ratio)
	{
		RECT rcPrevDlg{};
		if (!GetWindowRect(hDlg, &rcPrevDlg))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		if (!SetWindowPos(hDlg, NULL
			, 0
			, 0
			, (rcPrevDlg.right - rcPrevDlg.left) * ratio
			, (rcPrevDlg.bottom - rcPrevDlg.top) * ratio
			, SWP_NOMOVE | SWP_NOZORDER))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		auto data = std::make_tuple(hDlg, ratio);
		if (!EnumChildWindows(hDlg, ResizeChildWindow, (LPARAM)&data)) {
			return FALSE;
		}
		RECT rect(0, 0, (rcPrevDlg.right - rcPrevDlg.left) * ratio, (rcPrevDlg.bottom - rcPrevDlg.top) * ratio);
		InvalidateRect(hDlg, &rect,TRUE);
		return TRUE;
	}

	void ScaleRect(RECT* pRect, DOUBLE ratio)
	{
		pRect->left = (LONG)(pRect->left * ratio);
		pRect->top = (LONG)(pRect->top * ratio);
		pRect->right = (LONG)(pRect->right * ratio);
		pRect->bottom = (LONG)(pRect->bottom * ratio);
	}

	BOOL CALLBACK SetFontForChildWindow(HWND   hwnd, LPARAM lParam)
	{
		auto pData = (std::tuple<HFONT, BOOL>*)lParam;
		HFONT hFont = std::get<0>(*pData);
		BOOL bRedraw = std::get<1>(*pData);
		SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)bRedraw);
		return TRUE;
	}

	BOOL CALLBACK ResizeChildWindow(HWND hCtrl, LPARAM lParam)
	{
		auto pData = (std::tuple<HWND, DOUBLE>*)lParam;
		HWND hDlg = std::get<0>(*pData);
		DOUBLE ratio = std::get<1>(*pData);

		// コントロールに表示されるテキストのサイズを計算する
		RECT rcItem{};
		if (!GetWindowRect(hCtrl, &rcItem))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		// スクリーン座標からクライアント座標に変換する
		ScreenToClient(hDlg, reinterpret_cast<POINT*>(&rcItem.left));
		ScreenToClient(hDlg, reinterpret_cast<POINT*>(&rcItem.right));

		// コントロールのサイズ・ポジションを変更する
		ScaleRect(&rcItem, ratio);
		if (!SetWindowPos(
			hCtrl
			, HWND_TOP
			, rcItem.left
			, rcItem.top
			, rcItem.right - rcItem.left
			, rcItem.bottom - rcItem.top
			, SWP_NOZORDER))
		{
			ErrOut(GetLastError());
			return FALSE;
		}

		return TRUE;
	}

	BOOL CALLBACK SaveChildWindowPosition(HWND hwnd, LPARAM lParam)
	{
		if (!lParam||!hwnd)
		{
			return FALSE;
		}
		RECT rect;
		if (!GetWindowRect(hwnd, &rect))
		{
			ErrOut(GetLastError());
			return FALSE;
		}
		MapWindowPoints(HWND_DESKTOP,GetParent(hwnd), (POINT*) & rect,2);
		reinterpret_cast<std::vector<std::pair<HWND, RECT>> *>(lParam)->
			emplace_back(hwnd, rect);
		return TRUE;
	}

	BOOL CALLBACK ShowHideChildWindow(HWND hwnd, LPARAM lParam)
	{
		ShowWindow(hwnd, (BOOL)lParam ? SW_SHOW : SW_HIDE);
		return TRUE;
	}

	BOOL OffsetMoveChildWindow(HWND hWnd, LPARAM lParam)
	{
		// ダイアログボックスのハンドルとオフセットを取得
		auto params = *(std::tuple<HWND, int, int>*)lParam;
		HWND hDlg = std::get<0>(params);
		int cx = std::get<1>(params);
		int cy = std::get<2>(params);

		// ダイアログアイテムの位置を取得
		RECT rc;
		GetWindowRect(hWnd, &rc);

		// ダイアログボックスのクライアント座標に変換
		MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rc, 2);

		// オフセットを加算
		rc.left += cx;
		rc.right += cx;
		rc.top += cy;
		rc.bottom += cy;

		// ダイアログアイテムの位置を設定
		SetWindowPos(hWnd
			, 0
			,rc.left
			, rc.top
			, rc.right - rc.left
			, rc.bottom - rc.top
			, SWP_ASYNCWINDOWPOS
			| SWP_NOOWNERZORDER);
		return TRUE;
	}

}