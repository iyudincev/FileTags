#pragma once
#pragma warning(disable: 4503)

#include <string>
#include <unordered_map>
#include <vector>
#include "panelitemslist.hpp"


/// directory contents
class Folder {
public:
	Folder();
	Folder(const std::wstring &dir);

	const std::wstring &getDirName() const {
		return dirName;
	}
	const PluginPanelItems &getItems() const {
		return items;
	}

	bool read();
	const PluginPanelItem *findItem(const std::wstring &name) const {
		Name2item::const_iterator it = name2item.find(name);
		return (it != name2item.end()) ? it->second : nullptr;
	}
	bool contains(const std::wstring &name) const {
		return name2item.find(name) != name2item.end();
	}

private:
	typedef std::unordered_map<std::wstring, const PluginPanelItem *> Name2item;

	Name2item name2item;
	std::wstring dirName;
	PluginPanelItems items;
};
