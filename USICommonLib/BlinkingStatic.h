//Copyright (c) 2022, Gold Smith
//Released under the MIT license
//https ://opensource.org/licenses/mit-license.php
#pragma once
#include <Windows.h>



class BlinkingStatic
{
	UINT_PTR   nIDEvent;
	HWND hwnd;
	int stShow;
	const TIMERPROC TimerPrc;
public:
	BlinkingStatic()noexcept;
	BOOL Attach(HWND hctrl, UINT_PTR   ID);
	void Detach();
};

