#pragma once

#include "plugin.hpp"
#include "tags.hpp"


/// tags selection dialog (CtrlZ)
class DescrDialog {
public:
	/// User's choice
	enum Result {
		CANCEL,  ///< do nothing
		SET,     ///< set and clear tags
		REPLACE  ///< replace tags
	};

	DescrDialog(const Tags &tags, Tags &tagsToSet, Tags &tagsToClear);
	virtual ~DescrDialog();
	Result show();

private:
	enum ItemIndex {
		IX_TAGS_SET     = 2,
		IX_TAGS_REPLACE = 3,
		IX_TAGS_CLEAR_T = 4,
		IX_TAGS_CLEAR   = 5,
		IX_OK           = 7,
		IX_CANCEL       = 8,
		IX_TAGS_SELECT  = 9,
	};

	const Tags &m_tags;
	Tags &m_tagsToSet;
	Tags &m_tagsToClear;
	HANDLE m_hDlg;

	intptr_t dlgProc (intptr_t Msg, intptr_t Param1, void* Param2);
	static intptr_t WINAPI dlgWrapper(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void setEditFields();
	void applyEditFields();
	std::wstring getEditedText(ItemIndex index);
};
