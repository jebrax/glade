#pragma once

class FPSMeter {
private:
  float fps;

public:
  /**
     *
     * @param deltaTime Time in milliseconds that was taken to draw 1 frame
     */
  void updateFps(double delta_time) {
    if (delta_time <= 0) {
      fps = -1;
    } else {
      fps = (float)(1000.0 / delta_time);
    }
  }

  float getFps() { return fps; }
};
