#pragma once

#include "FileManager.h"

#include <vector>
#include <fstream>

class AAssetManager;
class Path;

class AndroidFileManager : public FileManager {
private:
  AAssetManager *androidAssetManager;

public:
  AndroidFileManager(AAssetManager *android_asset_manager);

  virtual void getFileContents(const Path &relative_path, std::ifstream &result,
                               bool binary_mode = false) const;

  virtual void getFileContents(const Path &relative_path, std::vector<char> &result,
                               bool binary_mode = false) const;

  virtual void getFileContents(const Path &relative_path, std::vector<unsigned char> &result,
                               bool binary_mode) const;
};
