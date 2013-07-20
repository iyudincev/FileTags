#include "windows.h"
#include "tags.hpp"


void Tags::load(const std::wstring &descr) {
	clear();
	bool bInTag = false;
	std::wstring::const_iterator itTag = descr.begin();
	for (std::wstring::const_iterator it = descr.begin(); it != descr.end(); ++it) {
		if (*it == L',') {
			if (bInTag) {
				bInTag = false;
				add(itTag, it);
			}
		}
		else if (!iswspace(*it)) {
			if (!bInTag) {
				bInTag = true;
				itTag = it;
			}
		}
	}
	if (bInTag) {
		add(itTag, descr.end());
	}
}

void Tags::add(std::wstring::const_iterator itBegin, std::wstring::const_iterator itEnd) {
	while (iswspace(*(itEnd-1)) && itEnd != itBegin)
		--itEnd;

	if (itEnd != itBegin) {
		std::wstring tag(itBegin, itEnd);

		wchar_t* tagBuf = new wchar_t[itEnd-itBegin+1];
		tag.copy(tagBuf, itEnd-itBegin);
		tagBuf[itEnd-itBegin] = 0;
		CharLower(tagBuf);
		std::wstring tagLow(tagBuf);
		delete[] tagBuf;

		if (isValidTag(tagLow))
			insert(tagLow);
	}
}

std::wstring Tags::toString() const {
	std::wstring res;
	for (const_iterator it = begin(); it != end(); ++it) {
		if (it != begin()) res += L',';
		res += *it;
	}
	return res;
}

bool Tags::includes(const Tags& tags) const {
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
		if (find(*it) == end())
			return false;

	return true;
}

Tags& Tags::operator+=(const Tags& rhs) {
	for (Tags::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
		insert(*it);
	}
	return *this;
}

Tags& Tags::operator-=(const Tags& rhs) {
	for (Tags::iterator it = begin(); it != end(); ) {
		if (rhs.find(*it) != rhs.end())
			erase(it++);
		else
			++it;
	}
	return *this;
}

Tags& Tags::operator*=(const Tags& rhs) {
	for (Tags::iterator it = begin(); it != end(); ) {
		if (rhs.find(*it) == rhs.end())
			erase(it++);
		else
			++it;
	}
	return *this;
}
