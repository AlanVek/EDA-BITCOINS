#pragma once
#include <string>

typedef struct {
	std::string host;
	std::string path;
} Location;

bool getData(char**, int, void*);
