#pragma once

#include <string>

#ifdef _WIN32
#define DIRECTORY_SEPARATOR '\\'
#else
#define DIRECTORY_SEPARATOR '/'
#endif // _WIN32

class Path {
private:
  std::string pathString;

public:
  static const class Hasher {
  public:
    size_t operator()(const Path &subject) const {
      std::hash<std::string> str_hasher;
      return str_hasher(subject.pathString);
    }
  } hasher;

  Path() {}
  Path(const std::string &path_string) : pathString(path_string) { normalize(); }
  Path(const char *path_c_string) : pathString(path_c_string) { normalize(); }
  Path(const Path &other) : pathString(other.pathString) {}

  Path &operator=(const Path &other) {
    pathString = other.pathString;
    return *this;
  }

  Path &operator=(const char *path_string) {
    pathString = path_string;
    return *this;
  }

  bool operator==(const Path &other) const { return pathString == other.pathString; }

  Path operator+(const Path &other) const {
    return Path(pathString + DIRECTORY_SEPARATOR + other.pathString);
  }

  Path operator+(const char *string) const {
    return Path(pathString + DIRECTORY_SEPARATOR + std::string(string));
  }

  bool operator<(const Path &other) const { return pathString < other.pathString; }
  std::string toString() const { return pathString; }
  const char *cString() const { return pathString.c_str(); }

  Path base() {
    int i = pathString.rfind(DIRECTORY_SEPARATOR);

    if (i != std::string::npos) {
      return Path(pathString.substr(0, i + 1));
    } else {
      return Path();
    }
  }

  void append(Path &other) {
    pathString.append(1, DIRECTORY_SEPARATOR);
    pathString.append(other.cString());
  }

  void append(const char *string) {
    pathString.append(1, DIRECTORY_SEPARATOR);
    pathString.append(string);
  }

private:
  void normalize() {
    std::string::iterator i = pathString.begin();

    while (i != pathString.end()) {
      if (*i == '/' || *i == '\\') {
        *i = DIRECTORY_SEPARATOR;
      }

      ++i;
    }

    if (*pathString.rbegin() == DIRECTORY_SEPARATOR) {
      pathString.pop_back();
    }
  }
};
