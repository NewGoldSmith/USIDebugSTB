// Copyright (c) 2022, Gold Smith
// Released under the MIT license
// https ://opensource.org/licenses/mit-license.php
#pragma once
// ���̃N���X��Windows��p�ł��B
// ********�g�p������ݒ�***********
#define USING_CRITICAL_SECTION// �N���e�B�J���Z�N�V�����g�p����ꍇ
#define CONFIRM_POINT    // �m�F�K�v�̏ꍇ
#define USING_DEBUG_STRING		// �f�X�g���N�^�Ő������m�F
// ******�����ݒ�I���*************

#ifdef USING_DEBUG_STRING
#define NOMINMAX
#include <Windows.h>
#include < algorithm >
#include <sstream>
#endif // USING_DEBUG_STRING

#ifdef USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // USING_CRITICAL_SECTION

#include <exception>
#include <atomic>
#include <string>
#include <iostream>

template <class T>class MemoryRental
{
public:
	MemoryRental() = delete;
	//sizeIn��2�ׂ̂���Ŗ����Ă͂Ȃ�܂���B
	MemoryRental(T* const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, size(sizeIn)
		, front(0)
		, end(0)
		, mask(sizeIn - 1)
#ifdef USING_CRITICAL_SECTION
		, cs{}
#endif // USING_CRITICAL_SECTION
	{
		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryRental must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
			std::cerr << estr;
			throw std::invalid_argument(estr);
		}

#ifdef USING_CRITICAL_SECTION
		(void)InitializeCriticalSectionAndSpinCount(&cs, 0);
#endif // USING_CRITICAL_SECTION

		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryRental(MemoryRental& obj) = delete;
	MemoryRental(MemoryRental&& obj) = delete;
	MemoryRental operator ()(MemoryRental& obj) = delete;
	MemoryRental operator =(MemoryRental& obj) = delete;
	~MemoryRental()
	{
#ifdef USING_CRITICAL_SECTION
		DeleteCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
		std::stringstream ss;
		ss << "MemoryRental "
			<< "\"" << strID.c_str() << "\" "
			<< "front:" << std::to_string(front)
			<< " end:" << std::to_string(end)
			<< " Difference:" << std::to_string(end - front)
			<< " Unit size:" << std::to_string(size)
			<< " Maximum peak usage:" << std::to_string(max_using)
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
#endif // USING_DEBUG_STRING
		delete[]ppBuf;
	}

	//sizeIn��2�ׂ̂���Ŗ����Ă͂Ȃ�܂���B
	void ReInitialize(T* pBufIn, size_t sizeIn)
	{
		delete[]ppBuf;
		ppBuf = nullptr;
		size = sizeIn;
		front = 0;
		end = 0;
		const_cast<size_t>(mask) = sizeIn - 1;

		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryRental must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
			std::cerr << estr;
			throw std::invalid_argument(estr);
		}

		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}

	inline T* Lend()
	{
#ifdef USING_CRITICAL_SECTION
		std::unique_ptr< CRITICAL_SECTION, void(*)(CRITICAL_SECTION*)> qcs
			= { [&]() {EnterCriticalSection(&cs); return &cs; }()
			,[](CRITICAL_SECTION* pcs) {LeaveCriticalSection(pcs); } };
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_POINT
		if (front + size < end)
		{
			std::stringstream ss;
			ss << __FILE__ << "("<<__LINE__<<"): " <<"Err!\""
				<<strID<< "\" MemoryRental.Lend. Underflow."
				<<"front[" << std::to_string(front)
				<< "] end["	<< std::to_string(end)
				<< "] size["<< std::to_string(size)
				<< "] end-front="	<< std::to_string(end-front)
				<<"\r\n";
			std::cerr << ss.str();
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::runtime_error(ss.str().c_str()); // ��O���o
		}
#endif // !CONFIRM_POINT
		T** ppT = &ppBuf[end & mask];
		++end;
#ifdef USING_DEBUG_STRING
		max_using=std::max<size_t>(end - front, max_using);
#endif // USING_DEBUG_STRING
		return *ppT;
	}

	inline void Return(T* pT)
	{
#ifdef USING_CRITICAL_SECTION
		std::unique_ptr< CRITICAL_SECTION, void(*)(CRITICAL_SECTION*)> qcs
			= { [&]() {EnterCriticalSection(&cs); return &cs; }()
			,[](CRITICAL_SECTION* pcs) {LeaveCriticalSection(pcs); }};
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_POINT
		if (front == end + size)
		{
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "): " << "Err!\""
				<< strID << "\" MemoryRental.Return.  Overflow."
				<< "front[" << std::to_string(front)
				<< "] end[" << std::to_string(end)
				<< "] size[" << std::to_string(size)
				<< "] end+size=" << std::to_string(end + size)
				<< "\r\n";
			std::cerr << ss.str();
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::runtime_error(ss.str()); // ��O���o
		}

#endif // !CONFIRM_POINT
		ppBuf[front & mask] = pT;
		++front;
	}

	void DebugString(const std::string str)
	{
#ifdef USING_DEBUG_STRING
		strID = str;
#endif // USING_DEBUG_STRING
	}

protected:
	T** ppBuf;
	//std::atomic_size_t size;
	//std::atomic_size_t front;
	//std::atomic_size_t end;
	size_t size;
	size_t front;
	size_t end;
	const size_t mask;
#ifdef USING_CRITICAL_SECTION
	CRITICAL_SECTION cs;
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
	//std::atomic_size_t max_using;
	size_t max_using;
	std::string strID;
#endif // USING_DEBUG_STRING
};


#ifdef USING_CRITICAL_SECTION
#undef USING_CRITICAL_SECTION
#endif // USING_CRITICAL_SECTION

#ifdef CONFIRM_POINT
#undef CONFIRM_POINT
#endif // CONFIRM_POINT

#ifdef USING_DEBUG_STRING
#undef NOMINMAX
#undef USING_DEBUG_STRING
#endif // USING_DEBUG_STRING

