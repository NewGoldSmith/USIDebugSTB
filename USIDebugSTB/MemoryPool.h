// Copyright (c) 2022, Gold Smith
// Released under the MIT license
// https ://opensource.org/licenses/mit-license.php
#pragma once
// ********�g�p������ݒ�***********
#define USING_CRITICAL_SECTION// �N���e�B�J���Z�N�V�����g�p����ꍇ
//#define NO_CONFIRM_RINGBUF    // �m�F�K�v�Ȃ��̏ꍇ
//#define USING_MUTEX_RINGBUF		// RingBuf���g���r������������ꍇ
#define USING_DEBUG_STRING		// �f�X�g���N�^�Ő������m�F
//#define DESTRACTOR_ACTIVATION	// �f�X�g���N�^�L����
// ******�����ݒ�I���*************

#ifdef USING_CRITICAL_SECTION
#ifdef USING_MUTEX_RINGBUF
#undef USING_MUTEX_RINGBUF
#endif
#endif


#ifdef USING_DEBUG_STRING
#define NOMINMAX
#include <Windows.h>
#include < algorithm >
#endif // USING_DEBUG_STRING

#ifdef USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // USING_CRITICAL_SECTION

#include <exception>
#include <mutex>
#include <atomic>
#include <string>
#include <iostream>

template <class T>class MemoryPool
{
public:
	MemoryPool() = delete;
	MemoryPool(T* const pBufIn
		, size_t sizeIn
#ifdef DESTRACTOR_ACTIVATION
		, int da = 0
#endif // DESTRACTOR_ACTIVATION

	)
		:ppBuf(nullptr)
		, size(sizeIn)
		, front(0)
		, end(0)
		, mask(sizeIn - 1)
#ifdef USING_CRITICAL_SECTION
		, cs{}
#endif // USING_CRITICAL_SECTION
#ifdef DESTRACTOR_ACTIVATION
		, da{ da }
#endif // DESTRACTOR_ACTIVATION
	{
		try {

			if ((sizeIn & mask) != 0)
			{
				throw std::invalid_argument(
					"Err! The MemoryPool must be Power of Two.\r\n");
			}
		}
		catch (std::invalid_argument& e)
		{
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(std::string(e.what()).c_str());
#endif// USING_DEBUG_STRING
			std::cerr << e.what();
			std::rethrow_exception(std::current_exception());
		}

#ifdef USING_CRITICAL_SECTION
		(void)InitializeCriticalSectionAndSpinCount(&cs, 3000);
#endif // USING_CRITICAL_SECTION

		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryPool(MemoryPool& obj) = delete;
	MemoryPool(MemoryPool&& obj) = delete;
	~MemoryPool()
	{
#ifdef USING_CRITICAL_SECTION
		DeleteCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
		std::stringstream ss;
		ss << "MemoryPool "
			<< "\"" << strID << "\" "
			<< "log. front;" << std::to_string(front)
			<< " end:" << std::to_string(end)
			<< " end-front:" << std::to_string(end - front)
			<< " size:" << std::to_string(size)
			<< " Maximum peak usage:" << std::to_string(max_using)
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
#endif // USING_DEBUG_STRING
		delete[]ppBuf;
	}

	void ReInitialize(T* pBufIn, size_t sizeIn)
	{
		delete[]ppBuf;
		ppBuf = nullptr;
		size = sizeIn;
		front = 0;
		end = 0;
		mask = sizeIn - 1;

#ifdef USING_MUTEX_RINGBUF
		mtx.unlock();
#endif// USING_MUTEX_RINGBUF
		try {
			if ((sizeIn & mask) != 0)
			{
				throw std::invalid_argument(
					"Err! The MemoryPool must be Power of Two.\r\n");
			}
		}
		catch (std::invalid_argument& e)
		{
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(e.what().c_str());
#endif// USING_DEBUG_STRING
			std::cerr << e.what();
			std::exception_ptr ep = std::current_exception();
			std::rethrow_exception(ep);
		}

		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}

	T* Pull()
	{
#ifdef USING_CRITICAL_SECTION
		EnterCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
#ifdef USING_MUTEX_RINGBUF
		std::unique_lock<std::mutex> lock(mtx);
#endif // USING_MUTEX_RINGBUF
#ifndef NO_CONFIRM_RINGBUF
		try {
			if (front + size < end)
			{
				throw std::runtime_error(
					"Err! \"" + strID
					+ "\" MemoryPool.Pull (front("
					+ std::to_string(front)
					+ ")& mask) + 1 == (end("
					+ std::to_string(end)
					+ " & mask)\r\n"); // ��O���o
			}
		}
		catch (std::exception& e) {
			// ��O��ߑ��B�G���[���R���o�͂���B
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(std::string(e.what()).c_str());
#endif// USING_DEBUG_STRING
			std::cerr << e.what();
			std::rethrow_exception(std::current_exception());
		}
#endif // !NO_CONFIRM_RINGBUF
		T** ppT = &ppBuf[end & mask];
		++end;
#ifdef USING_CRITICAL_SECTION
		LeaveCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
#ifdef USING_DEBUG_STRING
		max_using.store(
			(std::max)(end.load() - front.load(), max_using.load()));
#endif // USING_DEBUG_STRING
		return *ppT;
	}

	void Push(T* pT)
	{
#ifdef USING_CRITICAL_SECTION
		EnterCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
#ifdef USING_MUTEX_RINGBUF
		std::unique_lock<std::mutex> lock(mtx);
#endif
#ifndef NO_CONFIRM_RINGBUF
		try {
			if (front == end + size)
			{
				throw std::runtime_error(
					"Err! \"" + strID
					+ "\" MemoryPool.Push(front("
					+ std::to_string(front)
					+ ") & mask) + 1 == ( end("
					+ std::to_string(end)
					+ ") & mask)\r\n"); // ��O���o
			}
		}
		catch (std::exception& e) {
			// ��O��ߑ��B�G���[���R���o�͂���B
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(std::string(e.what()).c_str());
#endif// USING_DEBUG_STRING
			std::cerr << e.what();
			std::rethrow_exception(std::current_exception());
		}
#endif // !NO_CONFIRM_RINGBUF
#ifdef DESTRACTOR_ACTIVATION
		da ? pT->~pT : __noop;
#endif // DESTRACTOR_ACTIVATION
		ppBuf[front & mask] = pT;
		++front;
#ifdef USING_CRITICAL_SECTION
		LeaveCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
	}
	void DebugString(const std::string str)
	{
#ifdef USING_DEBUG_STRING
		strID = str;
#endif // USING_DEBUG_STRING
	}

protected:
	T** ppBuf;
	std::atomic_size_t size;
	std::atomic_size_t front;
	std::atomic_size_t end;
	std::atomic_size_t mask;
#ifdef USING_CRITICAL_SECTION
	CRITICAL_SECTION cs;
#endif // USING_CRITICAL_SECTION
#ifdef DESTRACTOR_ACTIVATION
	std::atomic_int da;
#endif // DESTRACTOR_ACTIVATION

#ifdef USING_MUTEX_RINGBUF
	std::mutex mtx;
#endif// USING_MUTEX_RINGBUF
#ifdef USING_DEBUG_STRING
	std::atomic_size_t max_using;
	std::string strID;
#endif // USING_DEBUG_STRING
};


#ifdef USING_CRITICAL_SECTION
#undef USING_CRITICAL_SECTION
#endif // USING_CRITICAL_SECTION

#ifdef NO_CONFIRM_RINGBUF
#undef NO_CONFIRM_RINGBUF
#endif // NO_CONFIRM_RINGBUF

#ifdef USING_MUTEX_RINGBUF
#undef USING_MUTEX_RINGBUF
#endif // USING_MUTEX_RINGBUF

#ifdef USING_DEBUG_STRING
#undef NOMINMAX
#undef USING_DEBUG_STRING
#endif // USING_DEBUG_STRING

#ifdef DESTRACTOR_ACTIVATION
#undef DESTRACTOR_ACTIVATION
#endif // DESTRACTOR_ACTIVATION
