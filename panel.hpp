#pragma once

#include <set>
#include "folder.hpp"
#include "dmodel.hpp"
#include "panelitemslist.hpp"
#include "watchdir.hpp"


class Panel {
public:
	Panel();
	virtual ~Panel();

	bool openFromMainMenu();
	bool openFromCommandLine(const std::wstring &cmd);
	const wchar_t *buildPrompt();
	const wchar_t *buildCaption();
	void capture(size_t *nItems, PluginPanelItem **items) {
		capturedItems = visibleItems;
		*nItems = capturedItems.size();
		*items = (capturedItems.empty()) ? nullptr : &capturedItems[0];
	}
	void release() {
		capturedItems.clear();
	}
	intptr_t changeDir(const wchar_t *delta);
	intptr_t ProcessKey(const INPUT_RECORD *Rec);
	intptr_t ProcessEvent(intptr_t Event, void *Param);
	intptr_t ProcessSync(intptr_t Event);

private:
	Tags filterTags;
	std::vector<std::wstring> relPath; //tags from filterTags in order they were selected
	PluginPanelItems visibleItems;
	PluginPanelItems capturedItems;
	std::wstring prompt;
	std::wstring caption;
	DescrDb db;
	Folder *folder;
	DirChangeNotifier *dcn;
	size_t cursorPos;

	void saveDescr() const;
	bool read();
	std::wstring addTagToFilter(const std::wstring &tag);
	std::wstring followTags(const std::wstring &delta);
	void updateVisibleItems();
	void setCursor(const std::wstring &cursor);
	std::wstring getPath() const;
	std::wstring getDescrPath() const;
	void createTag();
	void editTags();
	bool processDeletion();
	void removeTags(const Tags &tags);
};