/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include <boost/optional.hpp>
#include <optional>
#include <IteratorView.h>
#include <map>
#include <regex>
#include <VFSManager.h>
#include <NormalizePath.h>

static std::string getDirectory(const std::string& in)
{
	auto i = in.find_last_of('/');

	if (i != std::string::npos)
	{
		return in.substr(0, i);
	}
	else
	{
		return _xor_(".").c_str();
	}
}

struct Match
{
	void replace_all(std::string& str, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}

	Match(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
	{
		auto slashPos = pattern.find_last_of('/');
		auto root = pattern.substr(0, slashPos) + _xor_("/").c_str();
		auto after = pattern.substr(slashPos + 1);

		this->findHandle = device->FindFirst(root, &findData);

		this->device = device;
		this->pattern = after;
		this->root = root;
		this->end = (this->findHandle == INVALID_DEVICE_HANDLE);

		auto patternCopy = after;

		replace_all(patternCopy, _xor_("\\").c_str(), _xor_("\\\\").c_str());
		replace_all(patternCopy, _xor_("^").c_str(), _xor_("\\^").c_str());
		replace_all(patternCopy, _xor_(".").c_str(), _xor_("\\.").c_str());
		replace_all(patternCopy, _xor_("$").c_str(), _xor_("\\$").c_str());
		replace_all(patternCopy, _xor_("|").c_str(), _xor_("\\|").c_str());
		replace_all(patternCopy, _xor_("(").c_str(), _xor_("\\(").c_str());
		replace_all(patternCopy, _xor_(")").c_str(), _xor_("\\)").c_str());
		replace_all(patternCopy, _xor_("[").c_str(), _xor_("\\[").c_str());
		replace_all(patternCopy, _xor_("]").c_str(), _xor_("\\]").c_str());
		replace_all(patternCopy, _xor_("*").c_str(), _xor_("\\*").c_str());
		replace_all(patternCopy, _xor_("+").c_str(), _xor_("\\+").c_str());
		replace_all(patternCopy, _xor_("?").c_str(), _xor_("\\?").c_str());
		replace_all(patternCopy, _xor_("/").c_str(), _xor_("\\/").c_str());
		replace_all(patternCopy, _xor_("\\?").c_str(), _xor_(".").c_str());
		replace_all(patternCopy, _xor_("\\*").c_str(), _xor_(".*").c_str());

		this->re = std::regex{ _xor_("^").c_str() + patternCopy + _xor_("$").c_str() };

		while (!Matches() && !end)
		{
			FindNext();
		}

		this->has = false;

		if (Matches())
		{
			this->has = true;
		}
	}

	const vfs::FindData& Get()
	{
		return findData;
	}

	bool Matches()
	{
		if (findData.name != _xor_(".").c_str() && findData.name != _xor_("..").c_str())
		{
			return std::regex_match(findData.name, re);
		}

		return false;
	}

	void Next()
	{
		if (!end)
		{
			do
			{
				FindNext();
			} while (!Matches() && !end);

			has = !end && Matches();
		}
		else
		{
			has = false;
		}
	}

	operator bool()
	{
		return findHandle != INVALID_DEVICE_HANDLE && has;
	}

	~Match()
	{
		if (findHandle != INVALID_DEVICE_HANDLE)
		{
			device->FindClose(findHandle);
		}

		findHandle = INVALID_DEVICE_HANDLE;
	}

private:
	void FindNext()
	{
		end = !device->FindNext(findHandle, &findData);
	}

private:
	fwRefContainer<vfs::Device> device;
	std::string root;
	std::string pattern;
	vfs::Device::THandle findHandle;
	vfs::FindData findData;
	std::regex re;
	bool end;
	bool has;
};

static std::vector<std::string> MatchFiles(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
{
	auto patternNorm = path_normalize(pattern);

	auto starPos = patternNorm.find('*');
	auto before = getDirectory((starPos != std::string::npos) ? patternNorm.substr(0, starPos) : patternNorm);
	auto slashPos = (starPos != std::string::npos) ? patternNorm.find('/', starPos) : std::string::npos;
	auto after = (slashPos != std::string::npos) ? patternNorm.substr(slashPos + 1) : "";

	bool recurse = (starPos != std::string::npos &&
		patternNorm.substr(starPos + 1, 1) == "*" &&
		(starPos == 0 || patternNorm.substr(starPos - 1, 1) == "/"));

	std::set<std::string> results;

	if (recurse)
	{
		// 1 is correct behavior, 2 is legacy behavior we have to retain(...)
		for (auto submaskOff : { 1, 2 })
		{
			auto submask = patternNorm.substr(0, starPos) + patternNorm.substr(starPos + submaskOff);

			auto rawResults = MatchFiles(device, submask);

			for (auto& result : rawResults)
			{
				results.insert(std::move(result));
			}
		}

		auto findPattern = patternNorm.substr(0, starPos + 1);

		for (Match match{ device, findPattern }; match; match.Next())
		{
			if (match.Get().attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				auto matchPath = before + _xor_("/").c_str() + match.Get().name + _xor_("/").c_str() + patternNorm.substr(starPos);
				auto resultsSecond = MatchFiles(device, matchPath);

				for (auto& result : resultsSecond)
				{
					results.insert(std::move(result));
				}
			}
		}
	}
	else
	{
		auto findPattern = patternNorm.substr(0, slashPos != std::string::npos ? slashPos - 1 : std::string::npos);

		for (Match match{ device, findPattern }; match; match.Next())
		{
			bool isfile = !(match.Get().attributes & FILE_ATTRIBUTE_DIRECTORY);
			bool hasSlashPos = slashPos != std::string::npos;

			if (!(hasSlashPos && isfile))
			{
				auto matchPath = before + _xor_("/").c_str() + match.Get().name;

				if (!after.empty())
				{
					auto resultsSecond = MatchFiles(device, matchPath + _xor_("/").c_str() + after);

					for (auto& result : resultsSecond)
					{
						results.insert(std::move(result));
					}
				}
				else if (isfile)
				{
					results.insert(matchPath);
				}
			}
		}
	}

	std::vector<std::string> resultsVec{ results.begin(), results.end() };
	return resultsVec;
}

 // GUID type
struct guid_t
{
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];
};

namespace fx
{
	class Resource;

	class ResourceMetaDataComponent;

	class ResourceMetaDataLoader : public fwRefCountable
	{
	public:
		virtual boost::optional<std::string> LoadMetaData(ResourceMetaDataComponent* component, const std::string& resourcePath) = 0;
	};

	class ResourceMetaDataComponent : public fwRefCountable
	{
	private:
		Resource* m_resource;

		std::multimap<std::string, std::string> m_metaDataEntries;

		fwRefContainer<ResourceMetaDataLoader> m_metaDataLoader;

	public:
		ResourceMetaDataComponent(Resource* resourceRef);

		boost::optional<std::string> LoadMetaData(const std::string& resourcePath);

		std::optional<bool> IsManifestVersionBetween(const guid_t& lowerBound, const guid_t& upperBound);

		std::optional<bool> IsManifestVersionBetween(const std::string& lowerBound, const std::string& upperBound);

		inline Resource* GetResource()
		{
			return m_resource;
		}

		inline void SetMetaDataLoader(fwRefContainer<ResourceMetaDataLoader> loader)
		{
			m_metaDataLoader = loader;
		}

		inline std::map<std::string, std::vector<std::string>> GetAllEntries()
		{
			std::map<std::string, std::vector<std::string>> newMap;

			for (auto it = m_metaDataEntries.begin(), end = m_metaDataEntries.end(); it != end; it++)
			{
				newMap[it->first].push_back(it->second);
			}

			return newMap;
		}

		inline auto GetEntries(const std::string& key)
		{
			return GetIteratorView(m_metaDataEntries.equal_range(key));
		}

		void GlobEntries(const std::string& key, const std::function<void(const std::string&)>& entryCallback)
		{
			for (auto& entry : GetEntries(key))
			{
				GlobValue(entry.second, entryCallback);
			}
		}

		template<typename OutputIterator>
		inline void GlobEntriesIterator(const std::string& key, OutputIterator out)
		{
			GlobEntries(key, [&out](const std::string& entry) mutable
			{
				*out = entry;
				++out;
			});
		}

		void GlobValue(const std::string& value, const std::function<void(const std::string&)>& entryCallback)
		{
			if (value.empty())
			{
				return;
			}

			const auto& rootPath = m_resource->GetPath() + _xor_("/").c_str();
			fwRefContainer<vfs::Device> device = vfs::GetDevice(rootPath);

			if (!device.GetRef())
			{
				return;
			}

			auto relRoot = path_normalize(rootPath);

			std::string pattern = value;

			// @ prefixes for files are special and handled later on
			if (pattern.length() >= 1 && pattern[0] == '@')
			{
				entryCallback(pattern);
				return;
			}

			auto mf = MatchFiles(device, rootPath + pattern);

			for (auto& file : mf)
			{
				if (file.length() < (relRoot.length() + 1))
				{
					continue;
				}

				entryCallback(file.substr(relRoot.length() + 1));
			}
		}

		template<typename OutputIterator>
		inline void GlobValueIterator(const std::string& value, OutputIterator out)
		{
			GlobValue(value, [&out](const std::string& entry) mutable
				{
					*out = entry;
			++out;
				});
		}

		inline std::vector<std::string> GlobValueVector(const std::string& value)
		{
			std::vector<std::string> retval;
			GlobValueIterator(value, std::back_inserter(retval));

			return retval;
		}

		inline void AddMetaData(const std::string& key, const std::string& value)
		{
			m_metaDataEntries.insert({ key, value });
		}
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceMetaDataComponent);