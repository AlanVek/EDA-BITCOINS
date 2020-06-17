#include "GUIMsg.h"
#include "C:/Users/alanv/source/repos/EDA-BITCOINS/EDA - BITCOINS - FASE 3-3++/EDA - BITCOINS - FASE 3/Simulation/GUI.h"

GUIMsg::GUIMsg(const void* const gui) : gui(gui) {}

void GUIMsg::setMessage(const std::string& msg) { ((GUI*)gui)->updateMsg(msg); }