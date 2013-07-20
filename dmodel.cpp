#include <vector>
#include "dmodel.hpp"
#include "mapping.hpp"


const wchar_t Descr::FILENAME[] = L"Descript.ion";


void Descr::load(const std::wstring &filename) {
	clear();

	KFileMapping m;
	if (!m.open(filename.c_str()))
		return;

	LONGLONG llFileSize = m.getSize();
	if (llFileSize >= 0x40000000ULL)
		return;
	size_t fileSize = static_cast<size_t>(llFileSize);

	std::vector<wchar_t> buf;
	buf.resize(fileSize * 2);
	if (MultiByteToWideChar(CP_OEMCP, 0, m.value(), (int)fileSize, &buf[0], (int)buf.size()) == 0)
		return;

	enum {FILENAME, QUOTE, SPACE, DESCR} state = FILENAME;
	std::wstring item;
	const wchar_t *pBuf = &buf[0];
	const wchar_t *pItem = pBuf;
	size_t nBuf = buf.size();
	for (size_t i=0; i<nBuf; i++, pBuf++) {
		switch (state) {
		case FILENAME:
			switch (*pBuf) {
			case L'"':
				state = QUOTE;
				pItem = pBuf+1; //skip an opening quote
				break;
			case L' ':
				state = SPACE;
				item = std::wstring(pItem, pBuf);
				break;
			case L'\x0D':
			case L'\x0A':
				pItem = pBuf+1; //skip EOL
				break;
			}
			break;

		case QUOTE:
			switch (*pBuf) {
			case L'"':
				state = SPACE;
				item = std::wstring(pItem, pBuf);
				break;
			case L'\x0D':
			case L'\x0A':
				state = FILENAME;
				pItem = pBuf+1; //skip EOL
				break;
			}
			break;

		case SPACE:
			if (*pBuf == L'\x0D' || *pBuf == L'\x0A') {
				state = FILENAME;
				pItem = pBuf+1; //skip EOL
			}
			else if (!iswspace(*pBuf)) {
				state = DESCR;
				pItem = pBuf;
			}
			break;

		case DESCR:
			if (*pBuf == L'\x0D' || *pBuf == L'\x0A') {
				std::wstring description(pItem, pBuf);

				state = FILENAME;

				Tags itemTags;
				itemTags.load(description);
				m_item2tags[item] = itemTags;
				m_tags += itemTags;

				pItem = pBuf+1; //skip EOL
			}
			break;

		}//state
	}//for i
}

bool Descr::save(const std::wstring &filename) const {
	std::wstring text;
	for (Item2tags::const_iterator it = m_item2tags.begin(); it != m_item2tags.end(); ++it) {
		std::wstring item = (*it).first;
		const Tags &tags = (*it).second;
		if (!tags.empty()) {
			if (item.find(L' ') != std::wstring::npos)
				item = L"\"" + item + L"\"";
			std::wstring line = item + L" " + tags.toString() + L"\r\n";
			text.append(line);
		}
	}

	bool bFileExists = GetFileAttributes(filename.c_str()) != DWORD(-1);
	if (text.empty() && bFileExists) {
		return DeleteFileW(filename.c_str()) != FALSE;
	}

	int sizeRequired = WideCharToMultiByte(CP_OEMCP, 0, text.c_str(), (int)text.size(),
										   nullptr, 0, nullptr, nullptr);
	if (sizeRequired <= 0)
		return true;

	char *buf = new char[sizeRequired];
	WideCharToMultiByte(CP_OEMCP, 0, text.c_str(), (int)text.size(),
						buf, sizeRequired, nullptr, nullptr);

	HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0, nullptr, 
							(bFileExists) ? TRUNCATE_EXISTING : CREATE_NEW, 
							0, nullptr);
	bool res = false;
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD nWritten;
		res = WriteFile(hFile, buf, sizeRequired, &nWritten, nullptr) != FALSE;
		CloseHandle(hFile);
	}
	delete[] buf;
	return res;
}

void Descr::insert(const std::wstring &filename, const Tags &fileTags) {
	m_item2tags.insert(Item2tags::value_type(filename, fileTags));
}

void Descr::erase(const std::wstring &filename) {
	m_item2tags.erase(filename);
}

void Descr::modify(const std::set<std::wstring> &selectedFiles, 
	const Tags &tagsToSet, const Tags &tagsToClear)
{
	for (std::set<std::wstring>::const_iterator it = selectedFiles.begin(); 
		it != selectedFiles.end(); ++it) 
	{
		Item2tags::iterator itEntry = m_item2tags.find(*it);
		Tags &itemTags = (*itEntry).second;
		itemTags += tagsToSet;
		itemTags -= tagsToClear;
		if (itemTags.empty())
			m_item2tags.erase(itEntry);
	}
	refreshTags();
}

void Descr::modify(const std::set<std::wstring> &selectedFiles, 
	const Tags &newTags)
{
	for (std::set<std::wstring>::const_iterator it = selectedFiles.begin(); 
		it != selectedFiles.end(); ++it) 
	{
		Item2tags::iterator itEntry = m_item2tags.find(*it);
		if (newTags.empty())
			m_item2tags.erase(itEntry);
		else
			(*itEntry).second = newTags;
	}
	refreshTags();
}

void Descr::refreshTags() {
	m_tags.clear();
	for (Item2tags::const_iterator it = m_item2tags.begin(); it != m_item2tags.end(); ++it) {
		const Tags &itemTags = (*it).second;
		m_tags += itemTags;
	}
}
