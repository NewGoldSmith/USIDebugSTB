//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#include "pch.h"
#include "SocketContextDS.h"

using namespace std;
//using namespace DUSIP;
using namespace DUSI_COMMON;
namespace UsiDebugStb {

	SocketContext::SocketContext()
		:hSocket(NULL)
		, hEvent({ []() {
			try {
				WSAEVENT hEvent = WSACreateEvent();
				if (!hEvent) {
					DWORD Err = WSAGetLastError();
					throw runtime_error("WSACreateEvent.Code:" + to_string(Err) + "\rn");
				}
				return hEvent;
			}
			catch (exception& e) {
			   MyTRACE(e.what());
			   exception_ptr pe = current_exception();
			   rethrow_exception(pe);
			}}()
			,WSACloseEvent
		})
		, SocketID(0)
		, SockRemString()
		, vReadLines()
		, vWriteLines()
//		, pTPWait(NULL)
	{
		SockRemString.reserve(BUFFER_SIZE);
	}

	SocketContext::~SocketContext()
	{
		//if (pTPWait)
		//{
		//	SetThreadpoolWait(pTPWait, NULL, 0);
		//	WaitForThreadpoolWaitCallbacks(pTPWait, TRUE);
		//	CloseThreadpoolWait(pTPWait);
		//}
		if (hSocket)
		{
			closesocket(hSocket);
		}

	}

	void SocketContext::ReInitialize()
	{
		//if (pTPWait)
		//{
		//	SetThreadpoolWait(pTPWait, NULL, 0);
		//	//WaitForThreadpoolWaitCallbacks(pTPWait, TRUE);
		//	CloseThreadpoolWait(pTPWait);
		//	pTPWait = NULL;
		//}
		if (hSocket)
		{
			closesocket(hSocket);
			hSocket = NULL;
		}
		SocketID = 0;
		SockRemString.clear();
	}

}