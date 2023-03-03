#include <vector>
#include "dmodel.hpp"
#include "mapping.hpp"
#include "mix.hpp"


const wchar_t DescrDb::DIZ_FILENAME[] = L"Descript.ion";

static void rtrim(std::wstring &s) {
	s.erase(s.find_last_not_of(L" \n\r\t") + 1);
}

static bool isStringOemSafe(std::wstring &s) {
	return oem2ucs(ucs2oem(s)) == s;
}

static void decodeBytes(const char *buf, size_t nBytes, std::wstring &result, bool &bUtf8) {
	bUtf8 = nBytes >= 3 && buf[0] == '\xEF' && buf[1] == '\xBB' && buf[2] == '\xBF';
	if (bUtf8)
		result = utf2ucs(buf + 3, nBytes - 3);
	else
		result = oem2ucs(buf, nBytes);
}

static std::vector<char> convertToBytes(const std::wstring &text, bool bUtf8) {
	if (bUtf8) 
		return ucs2utf(L"\xFEFF" + text);
	return ucs2oem(text);
}

bool DescrDb::ItemData::empty() const {
	return (Opt.TagMarker.empty()) ?
		tags.empty() :
		text.empty() && tags.empty();
}

std::wstring DescrDb::getColumnTextByName(const std::wstring &item) {
	const ItemData &data = m_items[item];
	std::wstring result;
	bool bStore = false;
	if (Opt.TagMarker.empty()) {
		if (!data.tags.empty()) {
			result.append(data.tags.toString());
		}
	}
	else {
		if (!data.text.empty()) {
			result.append(data.text);
		}
		if (!data.tags.empty()) {
			if (!result.empty())
				result.push_back(L' ');
			result.append(Opt.TagMarker);
			result.push_back(L' ');
			result.append(data.tags.toString());
		}
	}
	return result;
}

void DescrDb::load(const std::wstring &filename) {
	clear();

	KFileMapping m;
	if (!m.open(filename.c_str()))
		return;

	LONGLONG llFileSize = m.getSize();
	if (llFileSize >= 0x40000000ULL)
		return;
	size_t fileSize = static_cast<size_t>(llFileSize);

	std::wstring buf;
	decodeBytes(m.value(), fileSize, buf, m_bLoadedUtf8);
	if (buf.empty())
		return;

	enum {FILENAME, QUOTE, SPACE, DESCR, TAGS} state = FILENAME;
	std::wstring item;
	ItemData data;
	std::wstring::const_iterator pItem = buf.begin(); //the beginning of currently parsed item
	std::wstring::const_iterator pMarker;             //current matching character in marker
	for (std::wstring::const_iterator pBuf = buf.begin(); pBuf != buf.end(); ++pBuf) {
		wchar_t c = *pBuf;
		switch (state) {
		case FILENAME:
			switch (c) {
			case L'"':
				state = QUOTE;
				pItem = pBuf+1; //skip an opening quote
				break;
			case L' ':
			case L'\t':
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
			switch (c) {
			case L'"':
				state = SPACE;
				item = std::wstring(pItem, pBuf);
				break;
			case L'\x0D':
			case L'\x0A':
				state = FILENAME;
				data.clear();
				pItem = pBuf+1; //skip EOL
				break;
			}
			break;

		case SPACE:
			switch (c) {
			case L'\x0D':
			case L'\x0A':
				state = FILENAME;
				data.clear();
				pItem = pBuf+1; //skip EOL
				break;
			case L' ':
			case L'\t':
				break;
			default:
				pItem = pBuf;
				if (Opt.TagMarker.empty()) {
					state = TAGS;
				}
				else {
					state = DESCR;
					pMarker = Opt.TagMarker.begin();
					goto testMarkerChar;
				}
				break;
			}
			break;

		case DESCR:
			switch (c) {
			case L'\x0D':
			case L'\x0A':
				data.text = std::wstring(pItem, pBuf);
				rtrim(data.text);
				m_items[item] = data;
				state = FILENAME;
				data.clear();
				pItem = pBuf + 1; //skip EOL
				break;
			default:
testMarkerChar:
				if (c == *pMarker) {
					if (++pMarker == Opt.TagMarker.end()) {
						data.text = std::wstring(pItem, pBuf - Opt.TagMarker.size() + 1);
						rtrim(data.text);
						state = TAGS;
						pItem = pBuf + 1;
					}
				}
				else {
					pMarker = Opt.TagMarker.begin();
				}
				break;
			}
			break;

		case TAGS:
			if (c == L'\x0D' || c == L'\x0A') {
				data.tags.load(std::wstring(pItem, pBuf));
				m_items[item] = data;
				m_tags += data.tags;

				state = FILENAME;
				data.clear();
				pItem = pBuf + 1; //skip EOL
			}
			break;

		}//state
	}//for i
}

bool DescrDb::save(const std::wstring &filename) const {
	bool bSaveUtf8 = m_bLoadedUtf8 || !isStringOemSafe(Opt.TagMarker); //force UTF-8 if the marker can't be saved in an OEM encoded file
	std::wstring text;
	for (Items::const_iterator it = m_items.begin(); it != m_items.end(); ++it) {
		std::wstring item = (*it).first;
		const ItemData &data = (*it).second;
		if (item.find(L' ') != std::wstring::npos)
			item = L"\"" + item + L"\"";
		std::wstring line = item;
		bool bStore = false;
		if (Opt.TagMarker.empty()) {
			if (!data.tags.empty()) {
				line.push_back(L' ');
				line.append(data.tags.toString());
				bStore = true;
			}
		}
		else {
			if (!data.text.empty()) {
				line.push_back(L' ');
				line.append(data.text);
				bStore = true;
			}
			if (!data.tags.empty()) {
				line.push_back(L' ');
				line.append(Opt.TagMarker);
				line.push_back(L' ');
				line.append(data.tags.toString());
				bStore = true;
			}
		}
		if (bStore) {
			text.append(line + L"\r\n");
		}
	}

	std::vector<char> buf = convertToBytes(text, bSaveUtf8);

	DWORD fileAttributes = GetFileAttributes(filename.c_str());
	bool bFileExists = fileAttributes != DWORD(-1);
	if (text.empty()) {
		if (bFileExists) {
			return DeleteFileW(filename.c_str()) != FALSE;
		}
		return true;
	}

	HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0, nullptr,
							(bFileExists) ? TRUNCATE_EXISTING : CREATE_NEW,
							0, nullptr);
	bool res = false;
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD nWritten;
		res = WriteFile(hFile, &(*buf.begin()), (DWORD)buf.size(), &nWritten, nullptr) != FALSE;
		CloseHandle(hFile);
		if (fileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			SetFileAttributes(filename.c_str(), FILE_ATTRIBUTE_HIDDEN);
		}
	}
	return res;
}

void DescrDb::insert(const std::wstring &filename, const Tags &fileTags) {
	m_items.insert(Items::value_type(filename, { L"", fileTags }));
}

void DescrDb::erase(const std::wstring &filename) {
	m_items.erase(filename);
}

void DescrDb::modify(const std::set<std::wstring> &selectedFiles,
	bool bModifyText, const std::wstring &text,
	const Tags &tagsToSet, const Tags &tagsToClear)
{
	for (std::set<std::wstring>::const_iterator it = selectedFiles.begin();
		it != selectedFiles.end(); ++it)
	{
		Items::iterator itEntry = m_items.find(*it);
		ItemData &data = (*itEntry).second;
		if (bModifyText)
			data.text = text;
		data.tags += tagsToSet;
		data.tags -= tagsToClear;
		if (data.empty())
			m_items.erase(itEntry);
	}
	refreshTags();
}

void DescrDb::modify(const std::set<std::wstring> &selectedFiles,
	bool bModifyText, const std::wstring &text,
	const Tags &newTags)
{
	for (std::set<std::wstring>::const_iterator it = selectedFiles.begin();
		it != selectedFiles.end(); ++it)
	{
		Items::iterator itEntry = m_items.find(*it);
		ItemData &data = (*itEntry).second;
		if (bModifyText)
			data.text = text;
		data.tags = newTags;
		if (data.empty())
			m_items.erase(itEntry);
	}
	refreshTags();
}

void DescrDb::removeTags(const std::set<std::wstring> &selectedFiles,
	const Tags &tags)
{
	for (std::set<std::wstring>::const_iterator it = selectedFiles.begin();
		it != selectedFiles.end(); ++it)
	{
		Items::iterator itEntry = m_items.find(*it);
		ItemData &data = (*itEntry).second;
		data.tags -= tags;
		if (data.empty())
			m_items.erase(itEntry);
	}
	refreshTags();
}

void DescrDb::refreshTags() {
	m_tags.clear();
	for (Items::const_iterator it = m_items.begin(); it != m_items.end(); ++it) {
		const Tags &itemTags = (*it).second.tags;
		m_tags += itemTags;
	}
}
