#include <string>
#include <windows.h>
#include "mix.hpp"
#include "filetagslng.hpp"
#include "panel.hpp"


/// FAR Manager is opening the plugin's panel
HANDLE WINAPI OpenW(const OpenInfo *OInfo)
{
	if (OInfo->StructSize < sizeof(OpenInfo))
		return nullptr;

	//open from a file panel only
	PanelInfo pi = {sizeof(PanelInfo)};
	::Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi);
	if (pi.PluginHandle != 0)
		return nullptr;

	switch (OInfo->OpenFrom) {
	case OPEN_PLUGINSMENU:
		{
			Panel *panel = new Panel();
			if (!panel->openFromMainMenu()) {
				delete panel;
				return INVALID_HANDLE_VALUE;
			}
			return static_cast<HANDLE>(panel);
		}
	case OPEN_COMMANDLINE:
		{
			Panel *panel = new Panel();
			std::wstring cmd = reinterpret_cast<OpenCommandLineInfo*>(OInfo->Data)->CommandLine;
			if (!panel->openFromCommandLine(cmd)) {
				delete panel;
				return INVALID_HANDLE_VALUE;
			}
			return static_cast<HANDLE>(panel);
		}
	}
	return nullptr;
}

/// FAR Manager is closing the plugin's panel
void WINAPI ClosePanelW(const ClosePanelInfo *Info)
{
	if(Info->StructSize < sizeof(ClosePanelInfo))
		return;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	delete panel;
}

/// FAR Manager is requesting the list of plugin panel items
intptr_t WINAPI GetFindDataW(GetFindDataInfo *Info)
{
	if(Info->StructSize < sizeof(GetFindDataInfo))
		return FALSE;

	if (Info->OpMode & OPM_FIND) // skipping the plugin call from file search
		return FALSE;

	if(!Info->hPanel || INVALID_HANDLE_VALUE == Info->hPanel)
		return FALSE;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	panel->capture(&Info->ItemsNumber, &Info->PanelItem);
	return TRUE;
}

/// FAR Manager no longer needs plugin panel items, free them
void WINAPI FreeFindDataW(const FreeFindDataInfo *Info)
{
	if(Info->StructSize < sizeof(FreeFindDataInfo))
		return;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	panel->release();
}

/// FAR Manager is requesting info on the panel being opened
void WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info)
{
	if(Info->StructSize < sizeof(OpenPanelInfo))
		return;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	Info->Flags = OPIF_ADDDOTS|OPIF_REALNAMES|OPIF_SHOWNAMESONLY|OPIF_USEATTRHIGHLIGHTING;
	Info->CurDir = panel->buildPrompt();
	Info->PanelTitle=panel->buildCaption();
	if (Opt.StorePanelMode)
		Info->StartPanelMode = Opt.PanelMode + '0';
}

/// FAR Manager is requesting the current folder change.
intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo *Info)
{
	if(Info->StructSize < sizeof(SetDirectoryInfo))
		return FALSE;

	if (Info->OpMode & OPM_FIND) // skipping the plugin call from file search
		return FALSE;

	if(!Info->hPanel || INVALID_HANDLE_VALUE == Info->hPanel)
		return FALSE;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	return panel->changeDir(Info->Dir);
}

/// process keys
intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo *Info)
{
	if (Info->StructSize < sizeof(ProcessPanelInputInfo))
		return 0;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	return panel->ProcessKey(&Info->Rec);
}

/// process redraw events
intptr_t WINAPI ProcessPanelEventW(const ProcessPanelEventInfo *Info)
{
	if (Info->StructSize < sizeof(ProcessPanelEventInfo))
		return 0;

	Panel *panel = reinterpret_cast<Panel *>(Info->hPanel);
	return panel->ProcessEvent(Info->Event, Info->Param);
}

/// process sync events (they are fired by the plugin on directory contents change)
intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo *Info)
{
	if (Info->StructSize < sizeof(ProcessSynchroEventInfo))
		return 0;

	Panel *panel = reinterpret_cast<Panel *>(Info->Param);
	return panel->ProcessSync(Info->Event);
}
