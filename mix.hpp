#pragma once

#include <string>
#include "plugin.hpp"

struct Options {
	std::wstring TagMarker;
	int StorePanelMode;
	intptr_t PanelMode;
};

const wchar_t OptionTagMarker[] = L"TagMarker";
const wchar_t OptionStorePanelMode[] = L"StorePanelMode";
const wchar_t OptionPanelMode[] = L"PanelMode";

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;
extern struct Options Opt;

int Config(const ConfigureInfo* CInfo);
const wchar_t *GetMsg(int MsgId);
