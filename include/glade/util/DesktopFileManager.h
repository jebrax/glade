#pragma once

#include "FileManager.h"
#include "glade/util/Path.h"

#include <vector>
#include <fstream>

class DesktopFileManager : public FileManager {
private:
  Path basePath;

public:
  DesktopFileManager(const Path &base_path) : basePath(base_path) {}
  DesktopFileManager() {}
  Path getBasePath() const { return basePath; }
  void setBasePath(const Path &base_path) { basePath = base_path; }
  void setBasePath(const char *base_path) { basePath = base_path; }
  Path getAbsolutePath(const Path &relative_path) const { return basePath + relative_path; }

  virtual void getFileContents(const Path &relative_path, std::ifstream &result,
                               bool binary_mode = false) const;

  virtual void getFileContents(const Path &relative_path, std::vector<char> &result,
                               bool binary_mode = false) const;

  virtual void getFileContents(const Path &relative_path, std::vector<unsigned char> &result,
                               bool binary_mode) const;
};
