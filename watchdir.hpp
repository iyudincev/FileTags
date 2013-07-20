#include <string>
#include "windows.h"


/// watch for the current directory file list change
class DirChangeNotifier {
public:
	DirChangeNotifier(const wchar_t *path, HANDLE hPanel);
	virtual ~DirChangeNotifier();
	bool isModified() {
		return InterlockedBitTestAndReset(&m_fModified, 0) != 0;
	}

private:
	const wchar_t *m_path;
	HANDLE m_hPanel;
	HANDLE m_hTask;
	HANDLE m_hChangeDir;
	volatile LONG m_fModified;
	bool m_fTerminate;

	bool isChecked() {
		return InterlockedBitTestAndSet(&m_fModified, 0) == 0;
	}

	DWORD WINAPI task();
	static DWORD WINAPI route(void* arg);
};
