//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#pragma once
#include "pch.h"
using namespace std;
namespace UsiDebugStb {

	class SocketContext
	{
		friend class MainDialog;
	public:
		SocketContext();
		~SocketContext();
		void ReInitialize();
	protected:
		atomic<SOCKET>  hSocket;
		//SOCKET hSocketShutDown;
		u_int SocketID;
		const unique_ptr<remove_pointer_t< WSAEVENT >, decltype(WSACloseEvent)*> hEvent;
		string SockRemString;
		vector<string> vReadLines;
		vector<string> vWriteLines;
		mutex mtxWrite;
		mutex mtxRead;
//		PTP_WAIT pTPWait;
	};
}