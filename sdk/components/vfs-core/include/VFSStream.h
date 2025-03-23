/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include "VFSDevice.h"
#include <vector>

namespace vfs
{
	class Stream : public fwRefCountable
	{
	private:
		fwRefContainer<Device> m_device;

		Device::THandle m_handle;

	public:
		Stream(fwRefContainer<Device> device, Device::THandle handle);

		virtual ~Stream();

		std::vector<uint8_t> Read(size_t length, bool isStreaming);

		size_t Read(void* buffer, size_t length, bool isStreaming);

		template<typename TAlloc>
		inline size_t Read(std::vector<uint8_t, TAlloc>& buffer, bool isStreaming)
		{
			return Read(&buffer[0], buffer.size(), isStreaming);
		}

		size_t Write(const void* buffer, size_t length);

		template<typename TAlloc>
		inline size_t Write(const std::vector<uint8_t, TAlloc>& buffer)
		{
			return Write(&buffer[0], buffer.size());
		}

		void Close();

		uint64_t GetLength();

		size_t Seek(intptr_t offset, int seekType);

		std::vector<uint8_t> ReadToEnd(bool isStreaming);

		inline Device::THandle GetHandle()
		{
			return m_handle;
		}
	};
}
