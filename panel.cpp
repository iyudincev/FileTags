#include "panel.hpp"
#include "PluginSettings.hpp"
#include "ddialog.hpp"
#include "filetagslng.hpp"
#include "guid.hpp"
#include "mix.hpp"


Panel::Panel() :
	cursorPos(0)
{
	folder = new Folder();
	dcn = new DirChangeNotifier(folder->getDirName().c_str(), this);
}

Panel::~Panel() {
	delete dcn;
	delete folder;
}

void Panel::saveDescr() const {
	if (!db.save(getDescrPath())) {
		static const wchar_t * items[] = {
			GetMsg(MSaveError),
			GetMsg(MOk),
		};
		::Info.Message(&MainGuid, &SaveErrorGuid,
			FMSG_WARNING | FMSG_ERRORTYPE, nullptr,
			items, ARRAYSIZE(items), 1);
	}
}

std::wstring Panel::getDescrPath() const {
	return folder->getDirName() + L"\\" + DescrDb::DIZ_FILENAME;
}

bool Panel::openFromMainMenu() {
	db.load(getDescrPath());
	read();
	return true;
}

bool Panel::openFromCommandLine(const std::wstring &cmd) {
	db.load(getDescrPath());
	followTags(cmd);
	read();
	return true;
}

intptr_t Panel::changeDir(const wchar_t *delta) {
	if (lstrcmp(delta, L"..") == 0 && filterTags.empty()) {
		::Info.PanelControl(this, FCTL_CLOSEPANEL, 0, (void *)folder->getDirName().c_str());
	}
	else {
		std::wstring cursor = followTags(std::wstring(delta));
		updateVisibleItems();
		setCursor(cursor);
	}
	return TRUE;
}

bool Panel::read() {
	folder->read();
	updateVisibleItems();
	return true;
}

std::wstring Panel::addTagToFilter(const std::wstring &tag) {
	std::wstring res;
	if (tag == L"..") {
		if (!filterTags.empty()) {
			res = relPath[relPath.size()-1];
			relPath.pop_back();
			filterTags.erase(res);
		}
	}
	else if (tag == L"\\") {
		relPath.clear();
		filterTags.clear();
	}
	else if (Tags::isValidTag(tag)) {
		if (filterTags.find(tag) == filterTags.end()) {
			filterTags.insert(tag);
			relPath.push_back(tag);
		}
	}
	return res;
}

std::wstring Panel::followTags(const std::wstring &delta) {
	std::wstring res;
	size_t prev = 0;
	size_t curr;
	do {
		curr = delta.find(L',', prev);
		std::wstring tag(delta, prev, curr);
		res = addTagToFilter(tag);
		prev = curr+1;
	} while (curr != std::wstring::npos);
	return res;
}

void Panel::updateVisibleItems() {
	visibleItems.clear();
	visibleItems.reserve(folder->getItems().size());

	Tags tmpTags;

	//populate the panel with files
	for (PluginPanelItems::const_iterator it=folder->getItems().begin(); it != folder->getItems().end(); ++it) {
		const PluginPanelItem &ppi = *it;

		std::wstring filename(ppi.FileName);
		std::wstring path = folder->getDirName() + L"\\" + filename;
		const Tags &fileTags = db.getTagsByName(filename);

		if (fileTags.includes(filterTags)) {
			PluginPanelItem shownItem = ppi;
			shownItem.FileName = _wcsdup(path.c_str());
			shownItem.AlternateFileName = _wcsdup(ppi.AlternateFileName);
			shownItem.Description = _wcsdup(db.getColumnTextByName(filename).c_str());
			shownItem.Owner = _wcsdup(ppi.Owner);
			shownItem.CustomColumnData = nullptr;

			visibleItems.add(shownItem);
			for (Tags::const_iterator it = fileTags.begin(); it != fileTags.end(); ++it)
				if (filterTags.find(*it) == filterTags.end())
					tmpTags.insert(*it);
		}
	}

	//add the tags to the top of the panel
	PluginPanelItem emptyItem = {0};
	visibleItems.insert(visibleItems.begin(), tmpTags.size(), emptyItem);
	size_t i = 0;
	for (Tags::const_iterator it = tmpTags.begin(); it != tmpTags.end(); ++it) {
		const std::wstring &tag = *it;

		PluginPanelItem *ppi = &visibleItems[i];
		ppi->FileAttributes = FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_VIRTUAL;
		ppi->FileName = _wcsdup(tag.c_str());
		i++;
	}
}

void Panel::setCursor(const std::wstring &cursor) {
	cursorPos = 0;
	for (size_t i = 0; i < visibleItems.size(); i++) {
		if (lstrcmp(visibleItems[i].FileName, cursor.c_str()) == 0) {
			cursorPos = i + 1;
			break;
		}
	}
}

std::wstring Panel::getPath() const {
	std::wstring res;
	for (std::vector<std::wstring>::const_iterator it = relPath.begin(); it != relPath.end(); ++it) {
		if (!res.empty()) res.append(L",");
		res.append(*it);
	}
	return res;
}

const wchar_t *Panel::buildPrompt() {
	prompt = folder->getDirName();
	return prompt.c_str();
}

const wchar_t *Panel::buildCaption() {
	caption = L"Tags:" + getPath();
	return caption.c_str();
}

intptr_t Panel::ProcessKey(const INPUT_RECORD *Rec) {
	if (!(Rec->EventType == KEY_EVENT || Rec->EventType == FARMACRO_KEY_EVENT))
		return FALSE;

	WORD key = Rec->Event.KeyEvent.wVirtualKeyCode;
	DWORD dwControl = Rec->Event.KeyEvent.dwControlKeyState;
	bool bNone = dwControl == 0;
	bool bCtrl = (dwControl & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
	bool bAlt = (dwControl & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED)) != 0;
	bool bShift = (dwControl & SHIFT_PRESSED) != 0;

	if (key == L'Z' && bCtrl) {
		editTags();
		return 1;
	}
	if (key == VK_F7 && bNone) {
		createTag();
		return 1;
	}
	if (key == VK_F8 && (bNone || bShift) ||
		key == VK_DELETE && (bAlt || bShift))
	{
		return (processDeletion()) ? 1 : 0;
	}
	return 0;
}

intptr_t Panel::ProcessEvent(intptr_t Event, void *Param) {
	if (Event == FE_CHANGEVIEWMODE) {
		PanelInfo pi = {sizeof(PanelInfo)};
		::Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);
		if (Opt.StorePanelMode) {
			Opt.PanelMode = pi.ViewMode;
			PluginSettings settings(MainGuid, ::Info.SettingsControl);
			settings.Set(0, OptionPanelMode, Opt.PanelMode);
		}
	}
	else if (Event == FE_REDRAW) {
		if (cursorPos != 0) {
			//user went one level up, set the cursor and redraw again
			PanelRedrawInfo ri = {sizeof(PanelRedrawInfo), cursorPos, 0};
			cursorPos = 0;
			::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, &ri);
			return FALSE;
		}

		if (dcn->isModified()) {
			Folder* newFolder = new Folder(folder->getDirName());
			newFolder->read();

			bool bModified = false;

			//remove the descriptions of deleted files from descript.ion
			for (PluginPanelItems::const_iterator it = folder->getItems().begin(); it != folder->getItems().end(); ++it) {
				const PluginPanelItem &ppi = *it;
				std::wstring filename(ppi.FileName);
				if (!newFolder->contains(filename)) {
					db.erase(filename);
					bModified = true;
				}
			}

			//assign descriptions to new files
			if (!filterTags.empty()) {
				for (PluginPanelItems::const_iterator it = newFolder->getItems().begin(); it != newFolder->getItems().end(); ++it) {
					const PluginPanelItem &ppi = *it;
					std::wstring filename(ppi.FileName);
					if (!folder->contains(filename)) {
						db.insert(filename, filterTags);
						bModified = true;
					}
				}
			}

			std::swap(folder, newFolder);
			delete newFolder;

			if (bModified) {
				db.refreshTags();
				saveDescr();
			}

			updateVisibleItems();
			::Info.PanelControl(this, FCTL_UPDATEPANEL, 1, nullptr);
		}
	}
	return FALSE;
}

intptr_t Panel::ProcessSync(intptr_t Event) {
	if (Event == SE_COMMONSYNCHRO) { //file list has changed
		::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, nullptr);
	}
	return 0;
}

void Panel::createTag() {
	wchar_t buf[1024];
	if (::Info.InputBox(&MainGuid, &CreateTagGuid, GetMsg(MCreateTagTitle),
						GetMsg(MCreateTagSubTitle), L"Tags", nullptr,
						buf, ARRAYSIZE(buf), L"CreateTag",
						FIB_NOUSELASTHISTORY | FIB_BUTTONS))
	{
		std::wstring tag(buf);
		if (Tags::isValidTag(tag)) {
			followTags(tag);
			updateVisibleItems();
			::Info.PanelControl(this, FCTL_UPDATEPANEL, 1, nullptr);
			::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, nullptr);
		}
	}
}

void Panel::editTags() {
	std::set<std::wstring> selectedFiles;
	Tags commonTags = db.getTags();
	Tags removedTags;
	std::wstring dizText;

	PanelInfo pi = {sizeof(PanelInfo)};
	::Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

	if (pi.SelectedItemsNumber == 0)
		return;

	//leave only tags that appear in each selected item
	for (size_t i=0; i<pi.SelectedItemsNumber; i++) {
		size_t size = ::Info.PanelControl(this, FCTL_GETSELECTEDPANELITEM, i, 0);
		PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(malloc(size));
		if (ppi) {
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), size, ppi};
			::Info.PanelControl(this, FCTL_GETSELECTEDPANELITEM, i, &gpi);

			if (ppi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				//extend the action to all files labeled with this tag
				std::wstring tag(std::wstring(ppi->FileName));
				for (PluginPanelItems::const_iterator it=visibleItems.begin(); it != visibleItems.end(); ++it) {
					if (!((*it).FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						std::wstring path((*it).FileName);
						std::wstring filename = DescrDb::extractFileName(path); //visibleItems contain full paths to files
						const Tags &itemTags = db.getTagsByName(filename);
						if (itemTags.find(tag) != itemTags.end()) {
							selectedFiles.insert(filename);
							if (selectedFiles.size() == 1)
								dizText = db.getDizByName(filename);
							else
								dizText.clear();
							commonTags *= itemTags;
						}
					}
				}
			}
			else {
				std::wstring path(ppi->FileName);
				std::wstring filename = DescrDb::extractFileName(path);
				selectedFiles.insert(filename);
				if (selectedFiles.size() == 1)
					dizText = db.getDizByName(filename);
				else
					dizText.clear();
				commonTags *= db.getTagsByName(filename);
			}

			free(ppi);
		}
	}

	bool bModifyText = !Opt.TagMarker.empty() && selectedFiles.size() == 1;
	DescrDialog dlg(db.getTags(), bModifyText,
		commonTags, removedTags, dizText);
	switch (dlg.show()) {
	case DescrDialog::SET:
		db.modify(selectedFiles, bModifyText, dizText, commonTags, removedTags);
		updateVisibleItems();
		::Info.PanelControl(this, FCTL_UPDATEPANEL, 1, nullptr);
		::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, nullptr);
		saveDescr();
		break;
	case DescrDialog::REPLACE:
		db.modify(selectedFiles, bModifyText, dizText, commonTags);
		updateVisibleItems();
		::Info.PanelControl(this, FCTL_UPDATEPANEL, 1, nullptr);
		::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, nullptr);
		saveDescr();
		break;
	}
}

bool Panel::processDeletion() {
	Tags tags;
	std::vector<size_t> tagItemNumbers;
	std::vector<size_t> fileItemNumbers;

	PanelInfo pi = {sizeof(PanelInfo)};
	::Info.PanelControl(this, FCTL_GETPANELINFO, 0, &pi);

	if (pi.SelectedItemsNumber == 0)
		return false;

	if (pi.SelectedItemsNumber == 1) {
		bool res = false;
		size_t size = ::Info.PanelControl(this, FCTL_GETSELECTEDPANELITEM, 0, nullptr);
		PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(malloc(size));
		if (ppi) {
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), size, ppi};
			::Info.PanelControl(this, FCTL_GETSELECTEDPANELITEM, 0, &gpi);

			if (ppi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				tags.insert(std::wstring(ppi->FileName));
				removeTags(tags);
				res = true;
			}

			free(ppi);
		}
		return res;
	}

	//more than one item selected
	tagItemNumbers.reserve(pi.SelectedItemsNumber);
	fileItemNumbers.reserve(pi.SelectedItemsNumber);

	for (size_t i=0; i<pi.ItemsNumber; i++) {
		size_t size = ::Info.PanelControl(this, FCTL_GETPANELITEM, i, 0);
		PluginPanelItem* ppi = reinterpret_cast<PluginPanelItem*>(malloc(size));
		if (ppi) {
			FarGetPluginPanelItem gpi = {sizeof(FarGetPluginPanelItem), size, ppi};
			::Info.PanelControl(this, FCTL_GETPANELITEM, i, &gpi);

			if (ppi->Flags & PPIF_SELECTED) {
				if (ppi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					tags.insert(std::wstring(ppi->FileName));
					tagItemNumbers.push_back(i);
				}
				else {
					fileItemNumbers.push_back(i);
				}
			}

			free(ppi);
		}
	}

	if (tagItemNumbers.empty())      //no selected tags, pass the deletion to FAR
		return false;

	if (fileItemNumbers.empty()) {   //no selected files, handle the deletion
		removeTags(tags);
		return true;
	}

	//both tags and files are selected, clear selection from tags and pass the deletion to FAR
	::Info.PanelControl(this, FCTL_BEGINSELECTION, 0, nullptr);
	for (std::vector<size_t>::const_iterator it = tagItemNumbers.begin(); it != tagItemNumbers.end(); ++it) {
		::Info.PanelControl(this, FCTL_SETSELECTION, *it, nullptr);
	}
	::Info.PanelControl(this, FCTL_ENDSELECTION, 0, nullptr);
	::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, nullptr);

	return false;
}

void Panel::removeTags(const Tags &tags) {
	static const wchar_t * items[] = {
		GetMsg(MDeleteCaption),
		GetMsg(MDeletePrompt),
	};
	if (::Info.Message(&MainGuid, &DeletePromptGuid, FMSG_MB_OKCANCEL, nullptr, items, ARRAYSIZE(items), 2) == 0) {
		std::set<std::wstring> affectedFiles;
		for (PluginPanelItems::const_iterator it=visibleItems.begin(); it != visibleItems.end(); ++it) {
			if (!((*it).FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::wstring path((*it).FileName);
				std::wstring filename = DescrDb::extractFileName(path); //visibleItems contain full paths to files
				affectedFiles.insert(filename);
			}
		}
		db.removeTags(affectedFiles, tags);
		updateVisibleItems();
		::Info.PanelControl(this, FCTL_UPDATEPANEL, 1, nullptr);
		::Info.PanelControl(this, FCTL_REDRAWPANEL, 0, nullptr);
		saveDescr();
	}
}
