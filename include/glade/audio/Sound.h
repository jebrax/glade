#pragma once

#include <vector>

class Sound {
public:
  enum Request { STOP, PLAY, PAUSE, NONE };

  std::vector<char> data;

private:
  Request request;

public:
  Sound(std::vector<char> &data_) : data(data_), request(NONE) {}
  Sound() : request(NONE) {}
  void play() { request = PLAY; }
  void stop() { request = STOP; }
  void pause() { request = PAUSE; }
  void resetRequest() { request = NONE; }
  bool isPlayRequested() { return request == PLAY; }
  bool isStopRequested() { return request == STOP; }
  bool isPauseRequested() { return request == PAUSE; }
};
