#pragma once

#include <map>
#include "tags.hpp"


/// file-to-tags mapping
class Descr {
public:
	static const wchar_t FILENAME[];

	static const std::wstring extractFileName(const std::wstring &path) {
		size_t iSeparator = path.find_last_of(L'\\');
		if (iSeparator != std::wstring::npos)
			return path.substr(iSeparator+1);
		return path;
	}

	void clear() {
		m_tags.clear();
		m_item2tags.clear();
	}
	/// get all tags
	const Tags &getTags() const {
		return m_tags;
	}
	/// get tags of a file
	const Tags &getTagsByName(const std::wstring &item) {
		return m_item2tags[item];
	}
	/// sync m_tags with modified m_item2tags
	void refreshTags();
	/// read descript.ion
	void load(const std::wstring &filename);
	/// write descript.ion
	bool save(const std::wstring &filename) const;
	/// add a new entry; refreshTags() must be called after adding/deleting entries
	void insert(const std::wstring &filename, const Tags &fileTags);
	/// delete an entry; refreshTags() must be called after adding/deleting entries
	void erase(const std::wstring &filename);
	/// apply changes in tags
	void modify(const std::set<std::wstring> &selectedFiles, 
		const Tags &tagsToSet, const Tags &tagsToClear);
	void modify(const std::set<std::wstring> &selectedFiles, 
		const Tags &newTags);

private:
	typedef std::map<std::wstring, Tags> Item2tags;

	Tags m_tags;
	Item2tags m_item2tags;
};
