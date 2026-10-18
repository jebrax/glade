#pragma once

#include <chrono>

class Timer {
private:
  using Clock = std::chrono::steady_clock;

  bool paused, autoReset, started;
  Clock::time_point baseTimestamp;

public:
  Timer(bool auto_reset = false)
      : paused(false), autoReset(auto_reset), started(false), baseTimestamp() {}

  /**
   * Returns elapsed wall-clock milliseconds since reset(), or 0 if paused or
   * never started. Auto-reset timers start on their first query and reset on
   * every subsequent query, including while paused.
   */
  float getDeltaTime(void) {
    if (!started) {
      if (autoReset) reset();

      return 0;
    }

    const auto now = Clock::now();
    const float milliseconds =
        std::chrono::duration<float, std::milli>(now - baseTimestamp).count();

    if (autoReset) baseTimestamp = now;

    return paused ? 0.0f : milliseconds;
  }

  void reset(void) {
    baseTimestamp = Clock::now();
    started = true;
  }

  void pause(void) { paused = true; }

  void unpause(void) {
    // Preserve the original semantics: unpausing does not reset elapsed time.
    paused = false;
  }
};
