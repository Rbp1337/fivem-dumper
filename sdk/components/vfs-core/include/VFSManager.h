/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include "VFSDevice.h"
#include "VFSStream.h"

namespace vfs
{
	class Manager : public fwRefCountable
	{
	public:
		virtual fwRefContainer<Stream> OpenRead(const std::string& path);

		virtual fwRefContainer<Stream> OpenWrite(const std::string& path, bool append = false);

		virtual bool RemoveFile(const std::string& path);

		virtual bool RenameFile(const std::string& from, const std::string& to);

		virtual fwRefContainer<Stream> Create(const std::string& path, bool createIfExists, bool append);

		virtual fwRefContainer<Device> GetDevice(const std::string& path) = 0;

		virtual fwRefContainer<Device> FindDevice(const std::string& absolutePath, std::string& transformedPath) = 0;

		virtual void Mount(fwRefContainer<Device> device, const std::string& path) = 0;

		virtual void Unmount(const std::string& path) = 0;

		virtual fwRefContainer<Device> GetNativeDevice(void* nativeDevice);
	};

	fwRefContainer<Stream> OpenRead(const std::string& path);

	fwRefContainer<Device> GetDevice(const std::string& path);

	fwRefContainer<Device> GetNativeDevice(void* nativeDevice);

	void Mount(fwRefContainer<Device> device, const std::string& path);

	void Unmount(const std::string& path);
}

DECLARE_INSTANCE_TYPE(vfs::Manager);