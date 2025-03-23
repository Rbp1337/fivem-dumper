/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include <array>

#include <optional>
#include <shared_mutex>
#include <map>

#include <Resource.h>

#include <boost/algorithm/string.hpp>
#include <msgpack.hpp>

struct IgnoreCaseLess
{
	using is_transparent = int;

	template< class T, class U>
	inline auto operator()(T&& lhs, U&& rhs) const
		-> decltype(std::forward<T>(lhs) < std::forward<U>(rhs))
	{
		return boost::ilexicographical_compare(lhs, rhs);
	}
};

class ResourceCache
{
private:
	std::unique_ptr<uintptr_t> m_indexDatabase;

	std::string m_cachePath;

	std::string m_physCachePath;

public:
	class Entry
	{
	private:
		std::map<std::string, std::string> m_metaData;

		std::array<uint8_t, 20> m_hash;

		std::string m_localPath;

	public:
		Entry(const std::string& entryData);

	public:
		inline const std::string& GetLocalPath() const
		{
			return m_localPath;
		}

		inline const std::map<std::string, std::string>& GetMetaData() const
		{
			return m_metaData;
		}

		inline const std::array<uint8_t, 20>& GetHash() const
		{
			return m_hash;
		}

		std::string GetHashString() const;
	};

public:
	ResourceCache(const std::string& cachePath, const std::string& physCachePath);

	void AddEntry(const std::string& localFileName, const std::map<std::string, std::string>& metaData);

	void AddEntry(const std::string& localFileName, const std::array<uint8_t, 20>& hashData, const std::map<std::string, std::string>& metaData);

	std::optional<Entry> GetEntryFor(const std::string& hashString);

	std::optional<Entry> GetEntryFor(const std::array<uint8_t, 20>& hash);

	inline const std::string& GetCachePath()
	{
		return m_cachePath;
	}

	inline const std::string& GetPhysCachePath()
	{
		return m_physCachePath;
	}

private:
	void OpenDatabase();

	std::unordered_map<std::string, std::optional<std::optional<Entry>>> m_entryCache;
	std::shared_mutex m_entryCacheMutex;
};
