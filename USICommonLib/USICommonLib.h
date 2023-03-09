//Copyright (c) 20220 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#pragma once
#include "pch.h"
using namespace std;
namespace DUSI_COMMON {
	constexpr UINT BUFFER_SIZE = 0x100;
 	typedef basic_string<TCHAR> tstring;
	string ConvertCRLFA(const string& strString, const string &strCR, const string& strLF);
	wstring ConvertCRLFW(const wstring &strString,const wstring& strCR,const wstring& strLF);
	string SeparatePhrase(string& str);
	string SplitFirstLineBreak(string& str);
	string SplitFirstSpace(string& str);
	string SplitLastLineBreakWithLBA(string& str);
	u8string SplitLastLineBreakWithLBU8(u8string& str);
	string SplitLastLineBreakWithOutLB(string& str);
	vector<string>vSplitLineBreakWithLBCodeA(string& str);
	vector<u8string>vSplitLineBreakWithLBCodeU8(u8string& str);
	size_t vSplitLineBreakWithLBCodeInternalA(
		const string& str
		, const size_t pos
		, const string &fstr
		, vector<string>& vs);
	size_t vSplitLineBreakWithLBCodeInternalU8(
		const u8string& str
		, const size_t pos
		, const u8string& fstr
		, vector<u8string>& vs);

	string AtoA(const string& str);
	wstring AtoW(const string& str);
	string WtoA(const wstring& wstr);
	wstring WtoW(const wstring& wstr);
	u8string WtoU8(const wstring& wstr);
	wstring U8toW(const u8string& u8str);
	u8string AtoU8(const string& str);
	string U8toA(const u8string& u8str);

#if _UNICODE
#define ConvertCRLF ConvertCRLFW
#define AtoT AtoW
#define TtoA WtoA
#define WtoT WtoW
#define TtoW WtoW
#define TtoU8 WtoU8
#define U8toT U8toW
#else
#define ConvertCRLF ConvertCRLFA
#define AtoT AtoA
#define TtoA AtoA
#define WtoT WtoA
#define TtoW AtoW
#define TtoU8 AtoU8
#define U8toT U8toA
#endif // _UNICODE

	const string ErrOut(
		DWORD dw
		, LPCSTR lpszFile
		, LPCSTR lpszFunction
		, DWORD dwLine
		, LPCSTR lpszOpMessage = "");
	const string ErrMes(const string& str);

#if _DEBUG
#define    MyTRACE(lpstr) ErrMes(lpstr)
#else
#define MyTRACE __noop
#endif

	template<class T>
	inline basic_string<T> ToUpper(const basic_string<T>& tstr)
	{
		basic_string<T> tret(tstr);
		transform(tstr.begin(), tstr.end(), tret.begin(), toupper);
		return tret;
	}

	void ParseStr(tstring const& str, const size_t pos, const tstring strEscCodes, const map<TCHAR, tstring>& mapChToStr, const LPNMLVCUSTOMDRAW lplvcd);
	inline void Draw(const tstring& tstr, const LPNMLVCUSTOMDRAW& lplvcd);
	inline void ConvAndDraw(const TCHAR ch
		, const map< TCHAR, tstring>& mapChToStr
		,LPNMLVCUSTOMDRAW lplvcd);
	inline void DrawBlue(const tstring& str, const LPNMLVCUSTOMDRAW& lplvcd);

	BOOL LoadDlgPosition(RECT& rect);
	void SaveDlgPosition(HWND hDlg);
	tstring MakeIniPath();
	BOOL LoadConsoleWindowPosition(RECT& rect);
	void SaveConsoleWindowPosition();
	BOOL SetWindowAppropriatePosition(HWND hWnd, RECT& rect);
	BOOL SetWindowAppropriatePositionEx(HWND hWnd);
	string MakeTimeFormat
	(const string& strFormat = string("%y-%m-%d-%H-%M-%S"));

}