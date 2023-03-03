#pragma once

#include <string>
#include <vector>
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

std::vector<char> ucs2oem(const std::wstring &s);
std::vector<char> ucs2utf(const std::wstring &s);
std::wstring oem2ucs(const char *buf, size_t nChars);
std::wstring oem2ucs(const std::vector<char> &buf);
std::wstring utf2ucs(const char *buf, size_t nChars);
std::wstring utf2ucs(const std::vector<char> &buf);
