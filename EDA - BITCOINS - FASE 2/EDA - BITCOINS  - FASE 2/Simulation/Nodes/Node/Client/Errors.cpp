#include "Errors.h"

Error::Error(const char* message_) : message(message_) {};

const char* Error::what(void) { return message; }

Error::~Error() {};