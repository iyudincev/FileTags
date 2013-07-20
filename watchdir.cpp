#include "watchdir.hpp"
#include "guid.hpp"
#include "mix.hpp"


DirChangeNotifier::DirChangeNotifier(const wchar_t *path, HANDLE hPanel) :
	m_path(path), m_hPanel(hPanel), m_fTerminate(false), m_fModified(0)
{
	m_hTask = CreateThread(
		nullptr,
		0x10000,
		route,
		this,
		0,
		nullptr);
}

DirChangeNotifier::~DirChangeNotifier() {
	m_fTerminate = true;
	if (m_hChangeDir != INVALID_HANDLE_VALUE)
		FindCloseChangeNotification(m_hChangeDir);

	WaitForSingleObject(m_hTask, INFINITE);
	CloseHandle(m_hTask);
}

DWORD WINAPI DirChangeNotifier::route(void* arg) {
	return reinterpret_cast<DirChangeNotifier*>(arg)->task();
}

DWORD WINAPI DirChangeNotifier::task() {
	m_hChangeDir = FindFirstChangeNotification(
		m_path,                        // directory to watch
		FALSE,                         // do not watch subtree
		FILE_NOTIFY_CHANGE_FILE_NAME); // watch for file names

	if (m_hChangeDir == INVALID_HANDLE_VALUE) {
		OutputDebugStringW(L"ERROR: FindFirstChangeNotification");
		return 0;
	}

	while (TRUE) {
		DWORD dwWaitStatus = WaitForSingleObject(m_hChangeDir, INFINITE);
		if (dwWaitStatus == WAIT_OBJECT_0) {
			if (m_fTerminate) {
				break;
			}
			// A file was created, renamed, or deleted in the directory.
			// Refresh this directory and restart the notification.
			if (isChecked()) {
				::Info.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, (void*)m_hPanel);
			}

			if (!FindNextChangeNotification(m_hChangeDir)) {
				OutputDebugStringW(L"ERROR: FindNextChangeNotification");
				break;
			}
		}
		else {
			OutputDebugStringW(L"WaitForSingleObject");
			break;
		}
	}
	if (!m_fTerminate)
		FindCloseChangeNotification(m_hChangeDir);
	m_hChangeDir = INVALID_HANDLE_VALUE;
	return 0;
}
