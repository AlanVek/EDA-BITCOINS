#pragma once
#include <string>

class GUIMsg
{
public:
	GUIMsg(const void* const gui);

	void setMessage(const std::string& msg);

private:
	const void* const gui;
};
