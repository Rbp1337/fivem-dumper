#pragma once

class ScriptContext
	{
	public:
		enum
		{
			MaxArguments = 32,
			ArgumentSize = sizeof(void*)
		};

	protected:
		void* m_argumentBuffer;

		int m_numArguments;
		int m_numResults;

	protected:
		inline ScriptContext() = default;

	public:
		template<typename T>
		inline const T& GetArgument(int index)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			return *reinterpret_cast<T*>(&functionData[index]);
		}

		template<typename T>
		inline void SetArgument(int index, const T& value)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&functionData[index]) = 0;
			}

			*reinterpret_cast<T*>(&functionData[index]) = value;
		}

		template<typename T>
		inline const T& CheckArgument(int index)
		{
			const auto& argument = GetArgument<T>(index);

			if (argument == T())
			{
				return T();
			}

			return argument;
		}

		inline int GetArgumentCount()
		{
			return m_numArguments;
		}

		template<typename T>
		inline void Push(const T& value)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&functionData[m_numArguments]) = 0;
			}

			*reinterpret_cast<T*>(&functionData[m_numArguments]) = value;
			m_numArguments++;
		}

		template<typename T>
		inline void SetResult(const T& value)
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			if (sizeof(T) < ArgumentSize)
			{
				*reinterpret_cast<uintptr_t*>(&functionData[0]) = 0;
			}

			*reinterpret_cast<T*>(&functionData[0]) = value;

			m_numResults = 1;
			m_numArguments = 0;
		}

		template<typename T>
		inline T GetResult()
		{
			auto functionData = (uintptr_t*)m_argumentBuffer;

			return *reinterpret_cast<T*>(functionData);
		}

		inline void* GetArgumentBuffer()
		{
			return m_argumentBuffer;
		}
	};