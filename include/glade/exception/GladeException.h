#pragma once

#include <string>

class GladeException {
private:
  std::string message;

public:
  GladeException() {}
  GladeException(std::string &message_param) : message(message_param) {}
  GladeException(const char *message_param) : message(message_param) {}
  const char *getMessage() { return message.c_str(); }
};
