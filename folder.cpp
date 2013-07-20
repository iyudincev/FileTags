#include "folder.hpp"
#include "mix.hpp"


Folder::Folder() {
	size_t bufSize = ::Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELDIRECTORY, 0, nullptr);
	FarPanelDirectory* dirInfo=(FarPanelDirectory*)malloc(bufSize);
	dirInfo->StructSize = sizeof(FarPanelDirectory);
	::Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELDIRECTORY, bufSize, dirInfo);
	dirName = std::wstring(dirInfo->Name);
	free(dirInfo);
}

Folder::Folder(const std::wstring &dir) {
	dirName = dir;
}

bool Folder::read() {
	items.clear();
	name2item.clear();

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(
		(dirName + L"\\*.*").c_str(),
		&fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				PluginPanelItem ppi = {0};
				ppi.CreationTime = fd.ftCreationTime;
				ppi.LastAccessTime = fd.ftLastAccessTime;
				ppi.LastWriteTime = fd.ftLastWriteTime;
				ppi.FileSize = (unsigned long long)fd.nFileSizeLow |
							   (unsigned long long)fd.nFileSizeHigh << 32;
				ppi.FileName = fd.cFileName;
				ppi.AlternateFileName = fd.cAlternateFileName;
				ppi.FileAttributes = fd.dwFileAttributes;
				items.add(ppi);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);

		for (PluginPanelItems::const_iterator it = items.begin(); it != items.end(); ++it) {
			const PluginPanelItem &ppi = *it;
			name2item[ppi.FileName] = &ppi;
		}
	}

	return true;
}
