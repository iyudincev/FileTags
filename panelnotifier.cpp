#include "panelnotifier.hpp"
#include "panel.hpp"


void PanelNotifier::addListener(void *listener) {
	m_listeners.insert(listener);
}

void PanelNotifier::removeListener(void *listener) {
	m_listeners.erase(listener);
}

void PanelNotifier::notify() {
	for (auto pPanel: m_listeners) {
		reinterpret_cast<Panel*>(pPanel)->notify();
	}
}
