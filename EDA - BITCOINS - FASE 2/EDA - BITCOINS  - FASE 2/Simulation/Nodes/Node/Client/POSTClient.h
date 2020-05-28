#pragma once
#include "Client.h"
class POSTClient : public Client {
public:
	POSTClient(const std::string&, const unsigned int, const json&);

	virtual ~POSTClient(void);

	virtual bool perform();

protected:
	virtual void configurateClient(void);

	json data;
};
