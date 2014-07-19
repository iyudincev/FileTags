#pragma once

#include <string>
#include "plugin.hpp"

struct Options {
	std::wstring Separator;
	int StorePanelMode;
	intptr_t PanelMode;
};

const wchar_t OptionSeparator[] = L"Separator";
const wchar_t OptionStorePanelMode[] = L"StorePanelMode";
const wchar_t OptionPanelMode[] = L"PanelMode";

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;
extern struct Options Opt;

int Config(const ConfigureInfo* CInfo);
const wchar_t *GetMsg(int MsgId);
