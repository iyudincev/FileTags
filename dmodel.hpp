#pragma once

#include <map>
#include "tags.hpp"


/// file descriptions database
class DescrDb {
public:
	static const wchar_t DIZ_FILENAME[];

	static const std::wstring extractFileName(const std::wstring &path) {
		size_t iSeparator = path.find_last_of(L'\\');
		if (iSeparator != std::wstring::npos)
			return path.substr(iSeparator+1);
		return path;
	}

	void clear() {
		m_tags.clear();
		m_items.clear();
	}
	/// get all tags
	const Tags &getTags() const {
		return m_tags;
	}
	/// get tags of a file
	const Tags &getTagsByName(const std::wstring &item) {
		return m_items[item].tags;
	}
	/// get file description
	const std::wstring &getDizByName(const std::wstring &item) {
		return m_items[item].text;
	}
	/// get text for file description column
	std::wstring getColumnTextByName(const std::wstring &item);
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
		bool bModifyText, const std::wstring &text,
		const Tags &tagsToSet, const Tags &tagsToClear);
	void modify(const std::set<std::wstring> &selectedFiles, 
		bool bModifyText, const std::wstring &text,
		const Tags &newTags);
	void removeTags(const std::set<std::wstring> &selectedFiles,
		const Tags &tags);

private:
	struct ItemData {
		std::wstring text;
		Tags tags;

		void clear() {
			text.empty();
			tags.empty();
		}
		bool empty() const;
	};
	typedef std::map<std::wstring, ItemData> Items;

	Tags m_tags;
	Items m_items;
};
