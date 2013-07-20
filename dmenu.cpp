#include "dmenu.hpp"
#include "filetagslng.hpp"
#include "guid.hpp"
#include "mix.hpp"


bool DescrMenu::show() {
	intptr_t breakCode;
	intptr_t cursor = 0;
	std::wstring cursorTag;

	do {
		FarMenuItem *items = new FarMenuItem[m_tags.size()];
		memset(items, 0, sizeof(FarMenuItem) * m_tags.size());
		size_t i = 0;
		for (Tags::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it) {
			const std::wstring &tag = *it;
			if (m_tagsToSet.find(tag) != m_tagsToSet.end())
				items[i].Flags = '+';
			if (m_tagsToClear.find(tag) != m_tagsToClear.end())
				items[i].Flags = '-';
			if (i == cursor)
				items[i].Flags |= MIF_SELECTED;
			items[i].Text = tag.c_str();
			i++;
		}

		struct FarKey breakKeys[4] = {{VK_ADD, 0}, {VK_SUBTRACT, 0}, {VK_SPACE, 0}, {0, 0}};
		cursor = ::Info.Menu(&MainGuid, &DescrMenuGuid, -1, -1, 0, FMENU_WRAPMODE,
							 GetMsg(MDescrMenuCaption), GetMsg(MDescrMenuHint), L"DescriptionMenu",
							 breakKeys, &breakCode,
							 items, m_tags.size());
		if (cursor >= 0) {
			cursorTag = std::wstring(items[cursor].Text);
			switch (breakCode) {
			case 0:  //gray +
				m_tagsToSet.insert(cursorTag);
				m_tagsToClear.erase(cursorTag);
				break;
			case 1:  //gray -
				m_tagsToSet.erase(cursorTag);
				m_tagsToClear.insert(cursorTag);
				break;
			case 2:  //Space
				m_tagsToSet.erase(cursorTag);
				m_tagsToClear.erase(cursorTag);
				break;
			case -1: //Enter
				if (items[cursor].Flags | MIF_SELECTED &&
					(items[cursor].Flags & 0xFFFF) != '-')
				{
					m_tagsToSet.insert(cursorTag);
					m_tagsToClear.erase(cursorTag);
				}
				delete[] items;
				return true;
			default:
				break;
			}
		}

		delete[] items;
	} while (cursor >= 0);

	//Escape
	return false;
}
