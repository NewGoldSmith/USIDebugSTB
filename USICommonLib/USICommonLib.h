/**
 * @file USICommonLib.h
 * @brief 汎用的な共通ライブラリ
 * @author Gold Smith
 * @date 2022-2023
 *
 * Released under the MIT license
 * https: //opensource.org/licenses/mit-license.php
 */
#pragma once
#include "pch.h"
using namespace std;
namespace DUSI_COMMON {
	constexpr UINT BUFFER_SIZE = 0x100;
 	typedef basic_string<TCHAR> tstring;
	/**
 * @brief 指定された文字列内のCRLF改行コードを、指定された文字列に置き換えます。
 *
 * この関数は、文字列と2つの置換文字列を入力として受け取り、新しい文字列を返します。
 * ここで、CRLF改行コード（"\\r\\n"）のすべての出現が、最初の置換文字列（CR）と2番目の置換文字列（LF）に置き換えられます。
 *
 * @param input 変換する文字列。
 * @param cr CRLF改行コードのCR部分（"\\r"）を置き換える文字列。
 * @param lf CRLF改行コードのLF部分（"\\n"）を置き換える文字列。
 * @return 改行コードが指定された文字列に変換された新しい文字列。
 */
	string ConvertLineEndingsWithCustomStringsA(const string& strString, const string &strCR, const string& strLF);
	/**
 * @brief 指定された文字列内のCRLF改行コードを、指定された文字列に置き換えます。
 *
 * この関数は、文字列と2つの置換文字列を入力として受け取り、新しい文字列を返します。
 * ここで、CRLF改行コード（L"\\r\\n"）のすべての出現が、最初の置換文字列（CR）と2番目の置換文字列（LF）に置き換えられます。
 *
 * @param input 変換する文字列。
 * @param cr CRLF改行コードのCR部分（L"\\r"）を置き換える文字列。
 * @param lf CRLF改行コードのLF部分（L"\\n"）を置き換える文字列。
 * @return 改行コードが指定された文字列に変換された新しい文字列。
 */
	wstring ConvertLineEndingsWithCustomStringsW(const wstring& strString, const wstring& strCR, const wstring& strLF);
	/**
	 * @brief 指定された文字列内の改行コードをCRLFからLFに変換します。
	 *
	 * この関数は、文字列を入力として受け取り、新しい文字列を返します。
	 * ここで、CRLF改行コード（"\\r\\n"）のすべての出現がLF改行コード（"\\n"）に置き換えられます。
	 *
	 * @param input 変換する文字列。
	 * @return 改行コードがCRLFからLFに変換された新しい文字列。
	 */	
	string ReplaceCRLFWithLFA(const string& strString);
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
/**
 * @brief 指定されたアスキー文字列内のエスケープシーケンスを処理します。
 *
 * この関数は、アスキー文字列を入力として受け取り、新しいアスキー文字列を返します。
 * ここで、エスケープシーケンス（例えば、"\\n"、"\\t"など）は、それぞれ対応する文字に置き換えられます。
 *
 * @param input 処理するアスキー文字列。
 * @return エスケープシーケンスがそれぞれ対応する文字に置き換えられた新しいアスキー文字列。
 */
	string UnescapeStringA(const string& input);
	/**
	 * @brief 指定されたワイド文字列内のエスケープシーケンスを処理します。
	 *
	 * この関数は、ワイド文字列を入力として受け取り、新しいワイド文字列を返します。
	 * ここで、エスケープシーケンス（例えば、"\\n"、"\\t"など）は、それぞれ対応する文字に置き換えられます。
	 *
	 * @param input 処理するワイド文字列。
	 * @return エスケープシーケンスがそれぞれ対応する文字に置き換えられた新しいワイド文字列。
	 */
	wstring UnescapeStringW(const wstring& input);

#if _UNICODE
#define ConvertLineEndingsWithCustomStrings ConvertLineEndingsWithCustomStringsW
#define AtoT AtoW
#define TtoA WtoA
#define WtoT WtoW
#define TtoW WtoW
#define TtoU8 WtoU8
#define U8toT U8toW
#define UnescapeString UnescapeStringW
#else
#define ConvertLineEndingsWithCustomStrings ConvertLineEndingsWithCustomStringsA
#define AtoT AtoA
#define TtoA AtoA
#define WtoT WtoA
#define TtoW AtoW
#define TtoU8 AtoU8
#define U8toT U8toA
#define UnescapeString UnescapeStringA

#endif // _UNICODE
#define ErrOut__(dw,file,func,line) ErrOut_((dw),(file),(func),(line))
#define	ErrOut(dw) ErrOut__(dw,__FILE__,__FUNCTION__,__LINE__)
	const string ErrOut_(
		DWORD dw
		, LPCSTR lpszFile
		, LPCSTR lpszFunction
		, DWORD dwLine
		, LPCSTR lpszOpMessage = "");

#define ErrMes(lpszMessage) ErrMes_(lpszMessage,__FILE__,__FUNCTION__,__LINE__) 
	const string ErrMes_(
		 const string &lpszMessage
		, const string & lpszFile =""
		, const string & lpszFunction=""
		, DWORD dwLine =MAXDWORD
		);
#define EOut ErrOut(GetLastError())
#if _DEBUG
#define    MyTRACE(lpstr) ErrMes(lpstr)
#define _D(lpstr) ErrMes(lpstr)
#else
#define MyTRACE __noop
#define _D __noop
#endif // _DEBUG

	inline std::string ToUpper(const std::string& str);
	inline std::wstring ToUpper(const std::wstring& str);

	void ParseStr(tstring const& str, const size_t pos, const tstring strEscCodes, const map<TCHAR, tstring>& mapChToStr, const LPNMLVCUSTOMDRAW lplvcd);
	inline void Draw(const tstring& tstr, const LPNMLVCUSTOMDRAW& lplvcd);
	inline void ConvAndDraw(const TCHAR ch
		, const map< TCHAR, tstring>& mapChToStr
		,LPNMLVCUSTOMDRAW lplvcd);
	inline void DrawBlue(const tstring& str, const LPNMLVCUSTOMDRAW& lplvcd);

	tstring MakeIniPath();
	BOOL SetWindowAppropriatePosition(HWND hWnd, RECT& rect);
	BOOL SetWindowAppropriatePositionEx(HWND hWnd);
	HWND GetMainWindowHandle();
	string MakeTimeFormat
	(const string& strFormat = string("%y-%m-%d-%H-%M-%S"));
	/// <summary>
	/// hWndでのDCのフォントのポイントを計算する。hWndがNULLの場合
	/// デスクトップDC。
	/// </summary>
	/// <param name="hWnd">ウィンドウハンドル</param>
	/// <param name="hFont">フォントハンドル</param>
	/// <returns>フォントのポイント</returns>
	DOUBLE GetPointSizeFromFontHandle(HWND hWnd, HFONT hFont);
	/// <summary>
	/// 指定されたフォントとポイントサイズから新しいフォントを作成する。
	/// </summary>
	/// <param name="hwnd">Windowハンドル</param>
	/// <param name="hFont">指定するフォントハンドル</param>
	/// <param name="points">指定するポイント</param>
	/// <returns>作成されたフォントハンドル。失敗した場合はNULLを返す。</returns>
	HFONT CreateFontWithPointSize(HWND hwnd, HFONT hFont, int points);
	/// <summary>
	/// ダイアログボックスとその中のコントロールのサイズを変更する。
	/// </summary>
	/// <param name="hDlg">ダイアログボックスのハンドル。</param>
	/// <param name="ratio">サイズ変更の比率。</param>
	/// <returns>TRUE なら成功、FALSE なら失敗。</returns>
	BOOL ReSizeDlg(const HWND hDlg, const DOUBLE ratio);
	/// <summary>
	/// 矩形のサイズを変更する。
	/// </summary>
	/// <param name="pRect">矩形のポインタ。</param>
	/// <param name="ratio">サイズ変更の比率。</param>
	void ScaleRect(RECT* pRect, DOUBLE ratio);
	/// <summary>
	/// 子ウィンドウのフォントを設定します。
	/// </summary>
	/// <param name="hwnd">子ウィンドウのハンドル。</param>
	/// <param name="lParam">設定するフォントのハンドル。</param>
	/// <returns>関数が成功した場合はTRUE、それ以外の場合はFALSE。</returns>
	BOOL CALLBACK SetFontForChildWindow(HWND   hwnd,LPARAM lParam);
	/// <summary>
	/// コールバック関数。ダイアログボックス内の子ウィンドウ（コントロール）のサイズと位置を変更する。
	/// </summary>
	/// <param name="hCtrl">子ウィンドウ（コントロール）のハンドル。</param>
	/// <param name="lParam">指定された比率。std::tuple&lt;HWND, DOUBLE&gt; 型のポインタ。</param>
	/// <returns>TRUE なら成功、FALSE なら失敗。</returns>
	BOOL CALLBACK ResizeChildWindow(HWND hCtrl, LPARAM lParam);
	///
	/// \brief SaveChildWindowPosition
	/// \param std::vector<std::pair<HWND, RECT>>*
	/// \return TRUEを返すと列挙が続行され、FALSEを返すと列挙が終了します。
	///
	/// この関数は、EnumChildWindows関数によって呼び出されるコールバック関数です。
	/// 各子ウィンドウのクライアント座標を取得し、それらを保存します。
	///
	BOOL CALLBACK SaveChildWindowPosition(HWND hwnd, LPARAM lParam);
	/// <summary>
	/// 子ウィンドウの表示または非表示
	/// </summary>
	/// <remarks>
	/// この関数は、EnumChildWindows関数によって使用されるコールバック関数です。
	/// 指定された親ウィンドウの各子ウィンドウに対して呼び出され、lParamの値に基づいて
	/// 子ウィンドウの可視性を変更します。
	/// </remarks>
	/// <param name="hwnd">子ウィンドウのハンドル</param>
	/// <param name="lParam">子ウィンドウを表示または非表示にするかどうかを指定するブール値</param>
	/// <returns>子ウィンドウの列挙を続ける場合はTRUE、停止する場合はFALSE</returns>
	BOOL CALLBACK ShowHideChildWindow(HWND hwnd, LPARAM lParam);
	/// <summary>
	/// ダイアログボックスの子ウィンドウを移動するコールバック関数
	/// </summary>
	/// <param name="hWnd">子ウィンドウのハンドル</param>
	/// <param name="lParam">ダイアログボックスのハンドルとオフセット
	/// auto params = std::make_tuple(hDlg, offsetX, offsetY);</param>
	/// <returns>列挙を続行する場合はTRUE、それ以外の場合はFALSE</returns>
	BOOL CALLBACK OffsetMoveChildWindow(HWND hWnd, LPARAM lParam);
 }