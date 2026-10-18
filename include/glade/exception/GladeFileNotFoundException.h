#pragma once

#include "GladeException.h"

class GladeFileNotFoundException : public GladeException {
public:
  GladeFileNotFoundException(const char *message_) : GladeException(message_) {}
};
