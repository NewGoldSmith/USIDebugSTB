//Copyright (c) 2022 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#pragma once
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#include <Windows.h>

namespace UsiDebugStb {
	constexpr auto HOST_BASE_ADDR = "127.0.0.8";
	constexpr u_short HOST_PORT = 0;
	constexpr auto PEER_ADDR = "127.0.0.6";
	constexpr u_short PEER_PORT = 50000;
	constexpr u_int SOCK_ELM_SIZE = 16;//Ç◊Ç´èÊ
	constexpr u_int DLG_STRING_ARRAY_SIZE = 0x10000;//Ç◊Ç´èÊ
	constexpr int BUFFER_SIZE = 1024;
	constexpr auto DEFAULT_HOST_ID = "ID01";
}