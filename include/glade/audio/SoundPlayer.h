#pragma once

class SoundPlayer {
public:
  virtual void load(Sound &sound) = 0;
  virtual void hold(Sound &sound) = 0;
  virtual void unholdAll() = 0;
  virtual void clear(bool force) = 0;
  virtual void process() = 0;
  //void hold(Collection<Sound> sounds);
};
