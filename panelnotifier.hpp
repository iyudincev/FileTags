#pragma once

#include <unordered_set>


/// notify panels on settings change
class PanelNotifier {
public:
	void addListener(void *listener);
	void removeListener(void *listener);
	void notify();

private:
	std::unordered_set<void*> m_listeners;
};
