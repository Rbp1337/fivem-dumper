#pragma once
#include <cstdint>
#include <iostream>

class NativeContext :
	public scrNativeCallContext
{
private:
	// Configuration
	enum
	{
		MaxNativeParams = 32,
		ArgSize = 8,
	};

	// Anything temporary that we need
	uint8_t m_TempStack[MaxNativeParams * ArgSize];

public:
	inline NativeContext()
	{
		m_pArgs = &m_TempStack;
		m_pReturn = &m_TempStack;		// It's okay to point both args and return at 
										// the same pointer. The game should handle this.
		m_nArgCount = 0;
		m_nDataCount = 0;

		memset(m_TempStack, 0, sizeof(m_TempStack));
	}

	template <typename T>
	inline void Push(T value)
	{
		static_assert(sizeof(T) <= ArgSize, "Argument has an invalid size");

		if constexpr (sizeof(T) < ArgSize)
		{
			// Ensure we don't have any stray data
			*reinterpret_cast<uintptr_t*>(m_TempStack + ArgSize * m_nArgCount) = 0;
		}

		*reinterpret_cast<T*>(m_TempStack + ArgSize * m_nArgCount) = value;
		m_nArgCount++;
	}

	template <typename T>
	inline T GetResult()
	{
		return *reinterpret_cast<T*>(m_TempStack);
	}
};

struct pass { template<typename ...T> pass(T...) {} };

class NativeInvoke
{
private:
	static inline void Invoke1(NativeContext* cxt, __int64 hash)
	{


		//((__int64(__fastcall*)(unsigned __int64, scrNativeCallContext*))(Addresses::CallNativeHandlerRage))(hash, cxt);

		//typedef void(__cdecl* NativeHandler)(scrNativeCallContext* context);
		//using GetNativeHandler_t = NativeHandler(__fastcall*)(__int64 hash);
		//using CallNativeHandler_t = void(__fastcall*)(NativeContext*);

		NativeHandler fn = invoker::GetNativeHandler(hash, invoker::MapNative(hash));

		if (fn != 0)
		{
			spoof(fn)(cxt);
		}
	}
public:

	template<typename R, typename... Args>
	static inline R Invoke(__int64 Hash, Args... args)
	{
		NativeContext cxt;

		pass{ ([&]()
		{
			cxt.Push(args);
		}(), 1)... };

		Invoke1(&cxt, Hash);

		return cxt.GetResult<R>();
	}
};