#pragma once

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

#include <glade/debug/log.h>
#include <glade/exception/GladeException.h>

class ShaderProgram {
private:
  std::map<std::string, unsigned> uniformHandles;

public:
  int gpuHandle;

  std::vector<char> vertexShaderSource;
  std::vector<char> fragmentShaderSource;

  ShaderProgram() : gpuHandle(-1) {}

  unsigned getUniformHandle(const std::string &name) const {
    unsigned handle = 0;

    try {
      handle = uniformHandles.at(name);
    } catch (std::out_of_range &e) {
      ::log("There is no uniform '%s' in the shader", name.c_str());
      throw GladeException("There is no uniform in the shader");
    }

    return handle;
  }

  void saveUniformHandle(const std::string &name, unsigned handle) {
    uniformHandles[name] = handle;
  }

  void erase() {
    vertexShaderSource.clear();
    fragmentShaderSource.clear();
  }

  bool hasGpuHandle() const { return gpuHandle >= 0; }
};
