#pragma once

#include <vector>
#include <fstream>

class Path;

class FileManager {
public:
  virtual ~FileManager() {}

  virtual void getFileContents(const Path &relative_path, std::ifstream &result,
                               bool binary_mode = false) const = 0;

  virtual void getFileContents(const Path &relative_path, std::vector<char> &result,
                               bool binary_mode = false) const = 0;

  virtual void getFileContents(const Path &relative_path, std::vector<unsigned char> &result,
                               bool binary_mode) const = 0;
};
