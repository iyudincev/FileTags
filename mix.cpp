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
	wchar_t separator[256];
	wcsncpy_s(separator, Opt.Separator.c_str(), ARRAYSIZE(separator)-1);
	
	PluginDialogBuilder Builder(::Info, MainGuid, ConfigDialogGuid, MConfigTitle, L"Config");
	Builder.AddText(MConfigSeparator);
	Builder.AddEditField(separator, ARRAYSIZE(separator), 30);
	Builder.AddCheckbox(MConfigStorePanelMode, &Opt.StorePanelMode);
	Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog()) {
		Opt.Separator = separator;
		PluginSettings settings(MainGuid, ::Info.SettingsControl);
		settings.Set(0, OptionSeparator, separator);
		settings.Set(0, OptionStorePanelMode, Opt.StorePanelMode);

		return 1;
	}

	return 0;
}
