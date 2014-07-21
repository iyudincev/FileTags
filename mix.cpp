#include "windows.h"
#include "mix.hpp"
#include "filetagslng.hpp"
#include "DlgBuilder.hpp"
#include "PluginSettings.hpp"
#include "guid.hpp"


const wchar_t *GetMsg(int MsgId) {
	return ::Info.GetMsg(&MainGuid,MsgId);
}

int Config(const ConfigureInfo* CInfo) {
	wchar_t tagMarker[256];
	wcsncpy_s(tagMarker, Opt.TagMarker.c_str(), ARRAYSIZE(tagMarker)-1);

	PluginDialogBuilder Builder(::Info, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");
	Builder.AddText(MConfigTagMarker);
	Builder.AddEditField(tagMarker, ARRAYSIZE(tagMarker), 30);
	Builder.AddCheckbox(MConfigStorePanelMode, &Opt.StorePanelMode);
	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog()) {
		Opt.TagMarker = tagMarker;
		PluginSettings settings(MainGuid, ::Info.SettingsControl);
		settings.Set(0, OptionTagMarker, tagMarker);
		settings.Set(0, OptionStorePanelMode, Opt.StorePanelMode);

		return 1;
	}

	return 0;
}

std::string ucs2oem(const std::wstring &s) {
	std::string result;

	int sizeRequired = WideCharToMultiByte(CP_OEMCP, 0, s.c_str(), (int)s.size(),
		nullptr, 0, nullptr, nullptr);
	if (sizeRequired > 0) {
		result.resize(sizeRequired);
		WideCharToMultiByte(CP_OEMCP, 0, s.c_str(), (int)s.size(),
			&(*result.begin()), sizeRequired, nullptr, nullptr);
	}

	return result;
}

std::wstring oem2ucs(const char *s, size_t nChars) {
	std::wstring result;
	if (nChars != 0) {
		result.resize(nChars);
		if (MultiByteToWideChar(CP_OEMCP, 0, s, (int)nChars,
			&(*result.begin()), (int)nChars) == 0)
		{
			result.clear();
		}
	}
	return result;
}

std::wstring oem2ucs(const std::string &s) {
	return oem2ucs(s.c_str(), s.size());
}
