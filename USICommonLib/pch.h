//Copyright (c) 20220 2023, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php

#ifndef PCH_H
#define PCH_H

// プリコンパイルするヘッダーをここに追加します
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <tchar.h>
#include <strsafe.h>
#include <CommCtrl.h>
#include <wincrypt.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include "MemoryRental.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")


#endif //PCH_H
