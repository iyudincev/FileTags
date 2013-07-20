#include "panelitemslist.hpp"


void PluginPanelItems::clear() {
	for (iterator it = begin(); it != end(); ++it) {
		PluginPanelItem &ppi = *it;
		free((void*)ppi.FileName);
		free((void*)ppi.AlternateFileName);
		free((void*)ppi.Description);
		free((void*)ppi.Owner);
	}
	std::vector<PluginPanelItem>::clear();
}

const PluginPanelItem &PluginPanelItems::add(const PluginPanelItem &item) {
	PluginPanelItem ppi = item;
	ppi.FileName          = _wcsdup(item.FileName);
	ppi.AlternateFileName = _wcsdup(item.AlternateFileName);
	ppi.Description       = _wcsdup(item.Description);
	ppi.Owner             = _wcsdup(item.Owner);
	ppi.CustomColumnData  = nullptr;

	push_back(ppi);
	return at(size()-1);
}

PluginPanelItems &PluginPanelItems::operator=(const PluginPanelItems &rhs) {
	if (this != &rhs) {
		clear();
		reserve(rhs.size());
		for (PluginPanelItems::const_iterator it = rhs.begin(); it != rhs.end(); ++it)
			add(*it);
	}
	return *this;
}
