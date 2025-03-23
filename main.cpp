#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>

#include <ResourceCache.h>
#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceImpl.h>

#include <VFSManager.h>

#include <json.hpp>

fx::ResourceManager* resource_manager = nullptr;
std::vector<fx::Resource*> client_resources = {};

std::vector<std::string> glob_entries_vector(fwRefContainer<fx::ResourceMetaDataComponent> metadata, const std::string& key)
{
	std::vector<std::string> entries;
	metadata->GlobEntriesIterator(key, std::back_inserter(entries));
	return entries;
}

nlohmann::json fetch_metadata()
{
	static nlohmann::json metadata;
	for (fx::Resource* resource : client_resources)
	{
		try
		{
			std::vector<std::string> files;
			fwRefContainer<fx::ResourceMetaDataComponent> meta_data = resource->GetComponent<fx::ResourceMetaDataComponent>();

			auto shared_scripts = glob_entries_vector(meta_data, _xor_("shared_script").c_str());
			auto client_scripts = glob_entries_vector(meta_data, _xor_("client_script").c_str());
			auto file_scripts = glob_entries_vector(meta_data, _xor_("file").c_str());

			for (auto& list : { shared_scripts, client_scripts, file_scripts })
				for (auto& script : list)
					if (script.find(_xor_("@").c_str()) == std::string::npos)
						files.push_back(const_cast<char*>(script.c_str()));

			fwRefContainer<vfs::Stream> fxmanifest = vfs::OpenRead(resource->GetPath() + _xor_("fxmanifest.lua").c_str());
			fwRefContainer<vfs::Stream> __resource = vfs::OpenRead(resource->GetPath() + _xor_("__resource.lua").c_str());

			if (fxmanifest.GetRef())
				files.push_back(_xor_("fxmanifest.lua").c_str());
			else if (__resource.GetRef())
				files.push_back(_xor_("__resource.lua").c_str());

			if (files.size() > 0)
				metadata[resource->GetName().c_str()] = files;
		} catch (...) {}
	}

	return metadata;
}

void dump_server(std::string dump_path)
{
	lazy(MessageBoxA).safe_cached()(0, _xor_("Started dumping. Please wait!").c_str(), _xor_("fivem dumper by @febbraio").c_str(), MB_ICONINFORMATION);

	resource_manager = Instance<fx::ResourceManager>::Get();
	if (!resource_manager) return;

	resource_manager->ForAllResources([](const fwRefContainer<fx::Resource>& res) {
		client_resources.push_back(res.GetRef());
	});

	nlohmann::json metadata = fetch_metadata();

	try
	{
		for (nlohmann::json::iterator it = metadata.begin(); it != metadata.end(); ++it)
		{
			for (auto& resource : it.value())
			{
				fwRefContainer<vfs::Stream> resource_data = vfs::OpenRead(resource_manager->GetResource(it.key())->GetPath() + resource.get<std::string>());
				if (resource_data.GetRef())
				{
					std::vector<uint8_t> data = resource_data->ReadToEnd(false);

					std::string output(dump_path);
					if (!std::filesystem::exists(output.append(_xor_("\\").c_str()).append(it.key())))
						std::filesystem::create_directory(output);

					std::string resources_list(resource.get<std::string>());

					size_t pos = 0;
					while ((pos = resources_list.find(_xor_("/").c_str())) != std::string::npos)
					{
						std::string token = resources_list.substr(0, pos);
						output.append(_xor_("\\").c_str()).append(token);

						if (!std::filesystem::exists(output))
							std::filesystem::create_directory(output);

						resources_list.erase(0, pos + std::string(_xor_("/").c_str()).length());
					}

					std::ofstream file((pos == 0) ? output.append(_xor_("\\").c_str()).append(resource.get<std::string>()) : output.append(_xor_("\\").c_str()).append(resources_list), std::ios::binary);
					file.write(reinterpret_cast<char*>(&data[0]), data.size() * sizeof(data[0]));
					file.close();
				}
			}
		}
	}
	catch (...) {}

	lazy(MessageBoxA).safe_cached()(0, _xor_("Dumping done! Saved to C:\\dump").c_str(), _xor_("cfx.re (fivem+redm) dumper by @febbraio").c_str(), MB_ICONINFORMATION);
}

bool __stdcall DllMain(void* hinstDLL, std::uint32_t fdwReason, void* reserved)
{
	if (fdwReason != DLL_PROCESS_ATTACH) return true;

	if (!std::filesystem::exists(_xor_("C:\\dump").c_str()))
		std::filesystem::create_directory(_xor_("C:\\dump").c_str());
	
	dump_server(_xor_("C:\\dump").c_str());

	return true;
}