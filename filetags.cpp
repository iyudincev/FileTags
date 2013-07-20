#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4121)
#endif // _MSC_VER

#include <windows.h>
#include <Shlobj.h>
#include <initguid.h>
#include <string.h>
#include <plugin.hpp>
#include <CRT/crt.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>

#include "mix.hpp"
#include "filetagslng.hpp"
#include "guid.hpp"
#include "version.hpp"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	(void) hDll;
	(void) dwReason;
	(void) lpReserved;
	return TRUE;
}
#endif


struct Options Opt;

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;


void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=MAKEFARVERSION(3, 0, 0, 2927, VS_RELEASE);
	Info->Version=MAKEFARVERSION(PLUGIN_MAJOR, PLUGIN_MINOR, 0, 0, VS_RELEASE);
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;

	PluginSettings settings(MainGuid, ::Info.SettingsControl);
	Opt.StorePanelMode = settings.Get(0, OptionStorePanelMode, 1);
	Opt.PanelMode = settings.Get(0, OptionPanelMode, 6);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* CInfo)
{
	return Config(CInfo);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=0;
	Info->CommandPrefix = L"tags";

	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MPluginsMenuString);
	Info->PluginMenu.Guids=&ConfigMenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);

	static const wchar_t *PluginCfgStrings[1];
	PluginCfgStrings[0]=GetMsg(MPluginsCfgMenuString);
	Info->PluginConfig.Guids=&ConfigMenuGuid;
	Info->PluginConfig.Strings=PluginCfgStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginCfgStrings);
}
