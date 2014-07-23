#pragma once
#include <windows.h>


class KFile {
private:
	HANDLE hFile;
public:
	KFile() : hFile(INVALID_HANDLE_VALUE) {}

	~KFile() {
		close();
	}

	HANDLE handle() const {
		return hFile;
	}

	bool isOpened() const {
		return hFile != INVALID_HANDLE_VALUE;
	}

	bool open(const wchar_t* path) {
		hFile = CreateFile (path,
			GENERIC_READ,
			FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0);
		return isOpened();
	}

	void close() {
		if(hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	LONGLONG getSize() const {
		LARGE_INTEGER size={0};
		if (isOpened())
			GetFileSizeEx(hFile, &size);
		return size.QuadPart;
	}
};

class KMapping{
private:
	HANDLE hMapping;
public:
	KMapping() : hMapping(0) {}

	~KMapping() {
		close();
	}

	HANDLE handle() const {
		return hMapping;
	}

	bool open(const KFile& file) {
		hMapping = CreateFileMapping(file.handle(), 0, PAGE_READONLY, 0, 0, 0);
		return hMapping!=0;
	}

	void close() {
		if(hMapping != 0)
			CloseHandle(hMapping);
		hMapping = 0;
	}
};

class KViewOfFile {
private:
	const char* MappingAddr;
public:
	KViewOfFile() : MappingAddr(nullptr) {}

	~KViewOfFile() {
		close();
	}

	const char* value() const {
		return MappingAddr;
	}

	bool isOpened() {
		return MappingAddr != nullptr;
	}

	bool open(const KMapping& mapping) {
		MappingAddr = (const char*)MapViewOfFile(mapping.handle(), FILE_MAP_READ, 0, 0, 0);
		return isOpened();
	}

	void close() {
		if(MappingAddr != nullptr)
			UnmapViewOfFile(MappingAddr);
		MappingAddr = nullptr;
	}
};

class KFileMapping {
private:
	KFile file;
	KMapping mapping;
	KViewOfFile view;

public:
	KFileMapping() {}

	~KFileMapping() {
		close();
	}

	void close() {
		view.close();
		mapping.close();
		file.close();
	}

	bool open(const wchar_t* path) {
		if(file.open(path) && mapping.open(file) && view.open(mapping))
			return true;
		close();
		return false;
	}

	const char* value() const {
		return view.value();
	}

	LONGLONG getSize() const {
		return file.getSize();
	}
};
