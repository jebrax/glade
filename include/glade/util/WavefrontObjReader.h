#pragma once

#include <vector>
#include <string>

namespace Glade {

class Mesh;

}

class WavefrontObjReader {
private:
  static bool putFace(std::string &face, Glade::Mesh *mesh);

public:
  static bool read(const std::vector<char> &in, Glade::Mesh *mesh);
};
