#pragma once

#include <string>
#include <iostream>
#include <vector>

class CSVReader {
public:
  static void read(std::istream &input, std::vector<std::vector<std::string>> &output);
  static void read(std::vector<char> &input, std::vector<std::vector<std::string>> &output);
};
