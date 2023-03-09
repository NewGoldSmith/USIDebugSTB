//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "MainDS.h"


using namespace UsiDebugStb;
using namespace std;
int APIENTRY wWinMain
(_In_ HINSTANCE hInstance
	, _In_opt_ HINSTANCE hPrevInstance
	, _In_ LPWSTR    lpCmdLine
	, _In_ int       nCmdShow
)
{
	int result(0);
	{


		UNREFERENCED_PARAMETER(hPrevInstance);
		UNREFERENCED_PARAMETER(lpCmdLine);

		const std::unique_ptr
			< WSADATA
			, void(*)(WSADATA*)
			> pwsadata
		{ []()
			{
				auto pwsadata = new WSADATA;
					if (WSAStartup(MAKEWORD(2, 2), pwsadata))
					{
						throw std::runtime_error("error!");
					}
			return pwsadata;
			}()
		, [](_Inout_ WSADATA* pwsadata)
			{
				WSACleanup();
				delete pwsadata;
			}
		};

		const std::unique_ptr
			< TP_POOL
			, decltype(CloseThreadpool)*
			> ptpp{ /*WINBASEAPI Must_inspect_result PTP_POOL WINAPI*/CreateThreadpool
				( /*Reserved PVOID reserved*/nullptr
				)
				, /*WINBASEAPI VOID WINAPI */CloseThreadpool/*(_Inout_ PTP_POOL ptpp)*/
		};

		const std::unique_ptr
			< TP_CALLBACK_ENVIRON
			, decltype(DestroyThreadpoolEnvironment)*
			> pcbe{
				[]()
				{
					const auto pcbe = new TP_CALLBACK_ENVIRON;
					/*FORCEINLINE VOID*/InitializeThreadpoolEnvironment
					( /*Out PTP_CALLBACK_ENVIRON pcbe*/pcbe
					);
					return pcbe;
					}()
				,
				[](_Inout_ PTP_CALLBACK_ENVIRON pcbe) {
						/*FORCEINLINE VOID*/DestroyThreadpoolEnvironment
						( /*Inout PTP_CALLBACK_ENVIRON pcbe*/pcbe
						);
						delete pcbe;
				}
		};

		/*WINBASEAPI VOID WINAPI*/SetThreadpoolThreadMaximum
		( /*Inout PTP_POOL ptpp     */&*ptpp
			, /*In    DWORD    cthrdMost*/MAX_THREAD
		);
		(void)/*WINBASEAPI BOOL WINAPI*/SetThreadpoolThreadMinimum
		( /*Inout PTP_POOL ptpp    */&*ptpp
			, /*In    DWORD    cthrdMic*/MIN_THREAD
		);
		/*FORCEINLINE VOID*/SetThreadpoolCallbackPool
		( /*Inout PTP_CALLBACK_ENVIRON pcbe*/&*pcbe
			, /*In    PTP_POOL             ptpp*/&*ptpp
		);

		INITCOMMONCONTROLSEX cc;
		cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
		cc.dwICC =
			ICC_LISTVIEW_CLASSES
			| ICC_INTERNET_CLASSES
			| ICC_UPDOWN_CLASS
			| ICC_STANDARD_CLASSES
			| ICC_UPDOWN_CLASS;
		InitCommonControlsEx(&cc);
		{

			MainDialog MainDlg;
#ifdef USING_GROBAL_THISPOINTER
			pgMainDialog = &MainDlg;
#endif // USING_GROBAL_THISPOINTER
			result = MainDlg.DialogBoxParamW(hInstance, NULL, pcbe.get());
		}
	}
	_CrtDumpMemoryLeaks();
	return result;
}
