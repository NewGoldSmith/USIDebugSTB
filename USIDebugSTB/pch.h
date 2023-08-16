// pch.h: プリコンパイル済みヘッダー ファイルです。
// 次のファイルは、その後のビルドのビルド パフォーマンスを向上させるため 1 回だけコンパイルされます。
// コード補完や多くのコード参照機能などの IntelliSense パフォーマンスにも影響します。
// ただし、ここに一覧表示されているファイルは、ビルド間でいずれかが更新されると、すべてが再コンパイルされます。
// 頻繁に更新するファイルをここに追加しないでください。追加すると、パフォーマンス上の利点がなくなります。
#pragma once
#ifndef PCH_H
#define PCH_H

// プリコンパイルするヘッダーをここに追加します
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <tchar.h>
#include <strsafe.h>
#include <CommCtrl.h>
#include <wincrypt.h>
#include <string>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <queue>
#include <map>
#include <numeric>
#include <filesystem>
#include <sal.h>
#include "../USICommonLib/MemoryRental.h"
//#include "..\common\DUSIP.h"
#include "..\USICommonLib\USICommonLib.h"
#include "..\USICommonLib\BlinkingStatic.h"
#include "DUSIDS.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib, "comctl32")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
 language='*'\"")
#pragma comment(lib, _CRT_STRINGIZE(..\\"USICommonLib"\\\
CONFIGURATION\\"USICommonLib"-CONFIGURATION-PLATFORM_TARGET\
-CHARACTER_SET.lib))


#endif //PCH_H
