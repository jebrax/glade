#pragma once

#include <cassert>

#include <memory>

namespace Glade {

class MemoryInputStream {
public:
  MemoryInputStream(const unsigned char *buffer, unsigned int size)
      : mBuffer(buffer), mCapacity(size), mHead(0) {
    assert(buffer != nullptr);
    assert(size != 0);
  }

  ~MemoryInputStream() {}
  unsigned int read(void *out_data, unsigned int size);

  // POD only plz
  template <typename T> unsigned int read(T &out_data) { return read(&out_data, sizeof(T)); };
  const unsigned char *getBuffer() { return mBuffer; }
  const unsigned int getBufferSize() { return mCapacity; }
  const unsigned int getUnreadSize() { return mCapacity - mHead; }

private:
  unsigned int mCapacity, mHead;
  const unsigned char *mBuffer;
};

class MemoryOutputStream {
public:
  static constexpr unsigned int DEFAULT_BUFFER_SIZE = 32;

  MemoryOutputStream() : mBuffer(nullptr), mCapacity(DEFAULT_BUFFER_SIZE), mTail(0) {
    mBuffer = (unsigned char *)std::malloc(mCapacity);
  }

  ~MemoryOutputStream() { std::free(mBuffer); }
  unsigned int write(const void *data, unsigned int size);

  // POD only plz
  template <typename T> unsigned int write(const T &data) { return write(&data, sizeof(T)); };
  const unsigned char *getBuffer() { return mBuffer; }
  const unsigned int getBufferSize() { return mCapacity; }
  const unsigned int getWrittenSize() { return mTail; }

private:
  unsigned int mCapacity, mTail;
  unsigned char *mBuffer;
};

}
