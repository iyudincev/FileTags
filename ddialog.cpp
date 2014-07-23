#include "plugin.hpp"
#include "ddialog.hpp"
#include "dmenu.hpp"
#include "filetagslng.hpp"
#include "guid.hpp"
#include "mix.hpp"


DescrDialog::DescrDialog(const Tags &tags, bool dizEnabled,
	Tags &tagsToSet, Tags &tagsToClear, std::wstring &text) :
	m_tags(tags), m_dizEnabled(dizEnabled),
	m_tagsToSet(tagsToSet), m_tagsToClear(tagsToClear),
	m_text(text),
	m_hDlg(INVALID_HANDLE_VALUE)
{
	static FarDialogItem DlgItems[]={
		/* 0*/ {DI_DOUBLEBOX,3, 1,67,11, {0},nullptr     ,nullptr   ,0                                ,GetMsg(MDescrTitle)},
		/* 1*/ {DI_TEXT     ,6, 2, 0, 0, {0},nullptr     ,nullptr   ,0                                ,GetMsg(MDescrSet)},
		/* 2*/ {DI_EDIT     ,5, 3,64, 0, {1},L"Tags"     ,nullptr   ,DIF_HISTORY                      ,L""},
		/* 3*/ {DI_CHECKBOX ,5, 4, 0, 0, {0},0           ,nullptr   ,0                                ,GetMsg(MDescrReplace)},
		/* 4*/ {DI_TEXT     ,6, 5, 0, 0, {0},nullptr     ,nullptr   ,0                                ,GetMsg(MDescrClear)},
		/* 5*/ {DI_EDIT     ,5, 6,64, 0, {1},L"Tags"     ,nullptr   ,DIF_HISTORY                      ,L""},
		/* 6*/ {DI_TEXT     ,6, 7, 0, 0, {0},nullptr     ,nullptr   ,0                                ,GetMsg(MDescrText)},
		/* 7*/ {DI_EDIT     ,5, 8,64, 0, {0},L"DizText"  ,nullptr   ,DIF_HISTORY                      ,L""},
		/* 8*/ {DI_TEXT     ,0, 9, 0, 0, {0},nullptr     ,nullptr   ,DIF_SEPARATOR                    ,L""},
		/* 9*/ {DI_BUTTON   ,0,10, 0, 0, {0},0           ,nullptr   ,DIF_CENTERGROUP|DIF_DEFAULTBUTTON,GetMsg(MOk)},
		/*10*/ {DI_BUTTON   ,0,10, 0, 0, {0},0           ,nullptr   ,DIF_CENTERGROUP                  ,GetMsg(MCancel)},
		/*11*/ {DI_BUTTON   ,0,10, 0, 0, {0},0           ,nullptr   ,DIF_CENTERGROUP                  ,GetMsg(MDescrSelect)},
	};

	m_hDlg = ::Info.DialogInit(&MainGuid, &DescrDlgGuid,
		-1, -1, 71, 13,
		L"Description", DlgItems, ARRAYSIZE(DlgItems),
		0, 0,
		dlgWrapper, this);

	setEditFields();
}

DescrDialog::~DescrDialog() {
	::Info.DialogFree(m_hDlg);
}

void DescrDialog::setEditFields() {
	std::wstring sSet = m_tagsToSet.toString();
	::Info.SendDlgMessage(m_hDlg, DM_SETTEXTPTR, IX_TAGS_SET, (void *)sSet.c_str());

	std::wstring sClear = m_tagsToClear.toString();
	::Info.SendDlgMessage(m_hDlg, DM_SETTEXTPTR, IX_TAGS_CLEAR, (void *)sClear.c_str());

	if (m_dizEnabled)
		::Info.SendDlgMessage(m_hDlg, DM_SETTEXTPTR, IX_DIZ, (void *)m_text.c_str());

	::Info.SendDlgMessage(m_hDlg, DM_ENABLE, IX_DIZ_T, (void *)((m_dizEnabled) ? TRUE : FALSE));
	::Info.SendDlgMessage(m_hDlg, DM_ENABLE, IX_DIZ, (void *)((m_dizEnabled) ? TRUE : FALSE));
}

void DescrDialog::applyEditFields() {
	m_tagsToSet.load(getEditedText(IX_TAGS_SET));
	m_tagsToClear.load(getEditedText(IX_TAGS_CLEAR));
	if (m_dizEnabled)
		m_text = getEditedText(IX_DIZ);
}

std::wstring DescrDialog::getEditedText(DescrDialog::ItemIndex index) {
	FarDialogItemData data = {sizeof(FarDialogItemData)};
	size_t nChars;
	wchar_t *text;

	nChars = ::Info.SendDlgMessage(m_hDlg, DM_GETTEXT, (intptr_t)index, nullptr);
	text = new wchar_t[nChars+1];
	data.PtrData = text;
	::Info.SendDlgMessage(m_hDlg, DM_GETTEXT, (intptr_t)index, &data);
	std::wstring res(data.PtrData, data.PtrLength);
	delete[] text;
	return res;
}

intptr_t WINAPI DescrDialog::dlgWrapper(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2) {
	DescrDialog *dlg = reinterpret_cast<DescrDialog *>(::Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0));
	return dlg->dlgProc(Msg, Param1, Param2);
}

DescrDialog::Result DescrDialog::show() {
	bool bReplace;

	switch (::Info.DialogRun(m_hDlg)) {
	case IX_OK:
		applyEditFields();
		bReplace = ::Info.SendDlgMessage(m_hDlg, DM_GETCHECK, IX_TAGS_REPLACE, 0) == BSTATE_CHECKED;
		return (bReplace) ? REPLACE : SET;

	default:
		return CANCEL;
	}
}

intptr_t DescrDialog::dlgProc(intptr_t Msg, intptr_t Param1, void* Param2) {
	bool bReplace;

	switch (Msg) {
	case DN_BTNCLICK:
		switch (Param1) {
		case IX_TAGS_REPLACE:
			bReplace = Param2!=0;
			::Info.SendDlgMessage(m_hDlg, DM_ENABLE, IX_TAGS_CLEAR_T, (void *)((bReplace) ? FALSE : TRUE));
			::Info.SendDlgMessage(m_hDlg, DM_ENABLE, IX_TAGS_CLEAR, (void *)((bReplace) ? FALSE : TRUE));
			return TRUE;

		case IX_TAGS_SELECT:
			{
				applyEditFields();
				Tags tagsToSet(m_tagsToSet);
				Tags tagsToClear(m_tagsToClear);
				DescrMenu menu(m_tags, tagsToSet, tagsToClear);
				if (menu.show()) {
					m_tagsToSet = tagsToSet;
					m_tagsToClear = tagsToClear;
					setEditFields();
					::Info.SendDlgMessage(m_hDlg, DM_SETFOCUS, IX_TAGS_SET, nullptr);
				}
				return TRUE;
			}
		}
		break;
	}
	return ::Info.DefDlgProc(m_hDlg, Msg, Param1, Param2);
}
