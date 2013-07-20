#pragma once

#include <set>
#include <string>


class Tags: public std::set<std::wstring> {
public:
	static bool isValidTag(const std::wstring &tag) {
		return tag != L"." && tag != L".." && tag != L"\\";
	}

	void load(const std::wstring &descr);
	void add(std::wstring::const_iterator itBegin, std::wstring::const_iterator itEnd);
	std::wstring toString() const;
	bool includes(const Tags& tags) const;
	Tags& operator+=(const Tags& rhs);
	Tags& operator-=(const Tags& rhs);
	Tags& operator*=(const Tags& rhs);
};
