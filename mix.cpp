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
