/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "Resource.h"

namespace fx
{
	class ResourceImpl;

	class ResourceImpl final : public Resource
	{
	private:
		std::string m_name;

		std::string m_rootPath;

		ResourceManager* m_manager;

		ResourceState m_state;

	public:
		ResourceImpl(const std::string& name, ResourceManager* manager);

		virtual bool LoadFrom(const std::string& rootPath, std::string* errorState) override;

		virtual const std::string& GetName() override;

		virtual const std::string& GetIdentifier() override;

		virtual const std::string& GetPath() override;

		virtual ResourceState GetState() override;

		virtual bool Start()
		{
			m_manager->MakeCurrent();

			m_state = ResourceState::Started;

			return true;
		}

		virtual bool Stop()
		{
			m_manager->MakeCurrent();

			m_state = ResourceState::Stopped;

			return true;
		}

		virtual void Run(std::function<void()>&& fn) override;

		virtual ResourceManager* GetManager() override;

	public:
		void Destroy()
		{
			m_state = ResourceState::Uninitialized;

			OnRemove();
		}
	};
}