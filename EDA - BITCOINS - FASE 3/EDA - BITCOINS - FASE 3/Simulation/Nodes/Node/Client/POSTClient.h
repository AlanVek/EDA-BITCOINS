#pragma once
#include "Client.h"
class POSTClient : public Client {
public:
	POSTClient(const std::string&, const unsigned int, const unsigned int, const json&);

	virtual ~POSTClient(void);

protected:
	virtual void configurateClient(void);

	json data;

	std::string strData;
};
