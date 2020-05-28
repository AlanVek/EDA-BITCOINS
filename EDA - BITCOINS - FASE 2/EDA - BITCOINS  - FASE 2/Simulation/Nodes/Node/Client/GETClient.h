#pragma once
#include "Client.h"
class GETClient : public Client {
public:
	GETClient(const std::string&, const unsigned int, const std::string&, const unsigned int);

	virtual ~GETClient(void);

	virtual bool perform();

protected:
	virtual void configurateClient(void);

	std::string id;
	unsigned int count;
};