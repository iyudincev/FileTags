#pragma once

#include <vector>
#include "plugin.hpp"


class PluginPanelItems : public std::vector<PluginPanelItem> {
public:
	~PluginPanelItems() {
		clear();
	}
	void clear();
	const PluginPanelItem &add(const PluginPanelItem &item);
	PluginPanelItems &operator=(const PluginPanelItems &rhs);
};
