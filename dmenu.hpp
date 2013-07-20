#include "windows.h"
#include "tags.hpp"


/// tags selection menu (CtrlZ - Select...)
class DescrMenu {
public:
	DescrMenu(const Tags &tags, Tags &tagsToSet, Tags &tagsToClear) :
	  m_tags(tags), m_tagsToSet(tagsToSet), m_tagsToClear(tagsToClear) {}

	bool show();
private:
	const Tags &m_tags;
	Tags &m_tagsToSet;
	Tags &m_tagsToClear;
};
