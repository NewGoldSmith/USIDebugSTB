/**
 * @file USICommonLib.h
 * @brief �ėp�I�ȋ��ʃ��C�u����
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
 * @brief �w�肳�ꂽ���������CRLF���s�R�[�h���A�w�肳�ꂽ������ɒu�������܂��B
 *
 * ���̊֐��́A�������2�̒u�����������͂Ƃ��Ď󂯎��A�V�����������Ԃ��܂��B
 * �����ŁACRLF���s�R�[�h�i"\\r\\n"�j�̂��ׂĂ̏o�����A�ŏ��̒u��������iCR�j��2�Ԗڂ̒u��������iLF�j�ɒu���������܂��B
 *
 * @param input �ϊ����镶����B
 * @param cr CRLF���s�R�[�h��CR�����i"\\r"�j��u�������镶����B
 * @param lf CRLF���s�R�[�h��LF�����i"\\n"�j��u�������镶����B
 * @return ���s�R�[�h���w�肳�ꂽ������ɕϊ����ꂽ�V����������B
 */
	string ConvertLineEndingsWithCustomStringsA(const string& strString, const string &strCR, const string& strLF);
	/**
 * @brief �w�肳�ꂽ���������CRLF���s�R�[�h���A�w�肳�ꂽ������ɒu�������܂��B
 *
 * ���̊֐��́A�������2�̒u�����������͂Ƃ��Ď󂯎��A�V�����������Ԃ��܂��B
 * �����ŁACRLF���s�R�[�h�iL"\\r\\n"�j�̂��ׂĂ̏o�����A�ŏ��̒u��������iCR�j��2�Ԗڂ̒u��������iLF�j�ɒu���������܂��B
 *
 * @param input �ϊ����镶����B
 * @param cr CRLF���s�R�[�h��CR�����iL"\\r"�j��u�������镶����B
 * @param lf CRLF���s�R�[�h��LF�����iL"\\n"�j��u�������镶����B
 * @return ���s�R�[�h���w�肳�ꂽ������ɕϊ����ꂽ�V����������B
 */
	wstring ConvertLineEndingsWithCustomStringsW(const wstring& strString, const wstring& strCR, const wstring& strLF);
	/**
	 * @brief �w�肳�ꂽ��������̉��s�R�[�h��CRLF����LF�ɕϊ����܂��B
	 *
	 * ���̊֐��́A���������͂Ƃ��Ď󂯎��A�V�����������Ԃ��܂��B
	 * �����ŁACRLF���s�R�[�h�i"\\r\\n"�j�̂��ׂĂ̏o����LF���s�R�[�h�i"\\n"�j�ɒu���������܂��B
	 *
	 * @param input �ϊ����镶����B
	 * @return ���s�R�[�h��CRLF����LF�ɕϊ����ꂽ�V����������B
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
 * @brief �w�肳�ꂽ�A�X�L�[��������̃G�X�P�[�v�V�[�P���X���������܂��B
 *
 * ���̊֐��́A�A�X�L�[���������͂Ƃ��Ď󂯎��A�V�����A�X�L�[�������Ԃ��܂��B
 * �����ŁA�G�X�P�[�v�V�[�P���X�i�Ⴆ�΁A"\\n"�A"\\t"�Ȃǁj�́A���ꂼ��Ή����镶���ɒu���������܂��B
 *
 * @param input ��������A�X�L�[������B
 * @return �G�X�P�[�v�V�[�P���X�����ꂼ��Ή����镶���ɒu��������ꂽ�V�����A�X�L�[������B
 */
	string UnescapeStringA(const string& input);
	/**
	 * @brief �w�肳�ꂽ���C�h��������̃G�X�P�[�v�V�[�P���X���������܂��B
	 *
	 * ���̊֐��́A���C�h���������͂Ƃ��Ď󂯎��A�V�������C�h�������Ԃ��܂��B
	 * �����ŁA�G�X�P�[�v�V�[�P���X�i�Ⴆ�΁A"\\n"�A"\\t"�Ȃǁj�́A���ꂼ��Ή����镶���ɒu���������܂��B
	 *
	 * @param input �������郏�C�h������B
	 * @return �G�X�P�[�v�V�[�P���X�����ꂼ��Ή����镶���ɒu��������ꂽ�V�������C�h������B
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
	/// hWnd�ł�DC�̃t�H���g�̃|�C���g���v�Z����BhWnd��NULL�̏ꍇ
	/// �f�X�N�g�b�vDC�B
	/// </summary>
	/// <param name="hWnd">�E�B���h�E�n���h��</param>
	/// <param name="hFont">�t�H���g�n���h��</param>
	/// <returns>�t�H���g�̃|�C���g</returns>
	DOUBLE GetPointSizeFromFontHandle(HWND hWnd, HFONT hFont);
	/// <summary>
	/// �w�肳�ꂽ�t�H���g�ƃ|�C���g�T�C�Y����V�����t�H���g���쐬����B
	/// </summary>
	/// <param name="hwnd">Window�n���h��</param>
	/// <param name="hFont">�w�肷��t�H���g�n���h��</param>
	/// <param name="points">�w�肷��|�C���g</param>
	/// <returns>�쐬���ꂽ�t�H���g�n���h���B���s�����ꍇ��NULL��Ԃ��B</returns>
	HFONT CreateFontWithPointSize(HWND hwnd, HFONT hFont, int points);
	/// <summary>
	/// �_�C�A���O�{�b�N�X�Ƃ��̒��̃R���g���[���̃T�C�Y��ύX����B
	/// </summary>
	/// <param name="hDlg">�_�C�A���O�{�b�N�X�̃n���h���B</param>
	/// <param name="ratio">�T�C�Y�ύX�̔䗦�B</param>
	/// <returns>TRUE �Ȃ琬���AFALSE �Ȃ玸�s�B</returns>
	BOOL ReSizeDlg(const HWND hDlg, const DOUBLE ratio);
	/// <summary>
	/// ��`�̃T�C�Y��ύX����B
	/// </summary>
	/// <param name="pRect">��`�̃|�C���^�B</param>
	/// <param name="ratio">�T�C�Y�ύX�̔䗦�B</param>
	void ScaleRect(RECT* pRect, DOUBLE ratio);
	/// <summary>
	/// �q�E�B���h�E�̃t�H���g��ݒ肵�܂��B
	/// </summary>
	/// <param name="hwnd">�q�E�B���h�E�̃n���h���B</param>
	/// <param name="lParam">�ݒ肷��t�H���g�̃n���h���B</param>
	/// <returns>�֐������������ꍇ��TRUE�A����ȊO�̏ꍇ��FALSE�B</returns>
	BOOL CALLBACK SetFontForChildWindow(HWND   hwnd,LPARAM lParam);
	/// <summary>
	/// �R�[���o�b�N�֐��B�_�C�A���O�{�b�N�X���̎q�E�B���h�E�i�R���g���[���j�̃T�C�Y�ƈʒu��ύX����B
	/// </summary>
	/// <param name="hCtrl">�q�E�B���h�E�i�R���g���[���j�̃n���h���B</param>
	/// <param name="lParam">�w�肳�ꂽ�䗦�Bstd::tuple&lt;HWND, DOUBLE&gt; �^�̃|�C���^�B</param>
	/// <returns>TRUE �Ȃ琬���AFALSE �Ȃ玸�s�B</returns>
	BOOL CALLBACK ResizeChildWindow(HWND hCtrl, LPARAM lParam);
	///
	/// \brief SaveChildWindowPosition
	/// \param std::vector<std::pair<HWND, RECT>>*
	/// \return TRUE��Ԃ��Ɨ񋓂����s����AFALSE��Ԃ��Ɨ񋓂��I�����܂��B
	///
	/// ���̊֐��́AEnumChildWindows�֐��ɂ���ČĂяo�����R�[���o�b�N�֐��ł��B
	/// �e�q�E�B���h�E�̃N���C�A���g���W���擾���A������ۑ����܂��B
	///
	BOOL CALLBACK SaveChildWindowPosition(HWND hwnd, LPARAM lParam);
	/// <summary>
	/// �q�E�B���h�E�̕\���܂��͔�\��
	/// </summary>
	/// <remarks>
	/// ���̊֐��́AEnumChildWindows�֐��ɂ���Ďg�p�����R�[���o�b�N�֐��ł��B
	/// �w�肳�ꂽ�e�E�B���h�E�̊e�q�E�B���h�E�ɑ΂��ČĂяo����AlParam�̒l�Ɋ�Â���
	/// �q�E�B���h�E�̉�����ύX���܂��B
	/// </remarks>
	/// <param name="hwnd">�q�E�B���h�E�̃n���h��</param>
	/// <param name="lParam">�q�E�B���h�E��\���܂��͔�\���ɂ��邩�ǂ������w�肷��u�[���l</param>
	/// <returns>�q�E�B���h�E�̗񋓂𑱂���ꍇ��TRUE�A��~����ꍇ��FALSE</returns>
	BOOL CALLBACK ShowHideChildWindow(HWND hwnd, LPARAM lParam);
	/// <summary>
	/// �_�C�A���O�{�b�N�X�̎q�E�B���h�E���ړ�����R�[���o�b�N�֐�
	/// </summary>
	/// <param name="hWnd">�q�E�B���h�E�̃n���h��</param>
	/// <param name="lParam">�_�C�A���O�{�b�N�X�̃n���h���ƃI�t�Z�b�g
	/// auto params = std::make_tuple(hDlg, offsetX, offsetY);</param>
	/// <returns>�񋓂𑱍s����ꍇ��TRUE�A����ȊO�̏ꍇ��FALSE</returns>
	BOOL CALLBACK OffsetMoveChildWindow(HWND hWnd, LPARAM lParam);
 }