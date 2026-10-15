#include <cassert>
#include <cstring>
#include <glade/io/Stream.h>
#include <glade/debug/log.h>
#include <algorithm>

unsigned int Glade::MemoryInputStream::Read(void *outData, unsigned int size)
{
  if (size == 0) {
    log("Warning: tried to read 0 bytes from a stream. Why would you?");
    return 0;
  }

  if (mHead >= mCapacity) {
    assert(mHead == mCapacity);
    return 0;
  }

  unsigned int unreadBytesLeft = mCapacity - mHead;
  unsigned int bytesToRead = std::min<unsigned int>(unreadBytesLeft, size);

  std::memcpy(outData, mBuffer + mHead, bytesToRead);
  mHead += bytesToRead;
  assert(mHead <= mCapacity);

  return bytesToRead;
}

unsigned int Glade::MemoryOutputStream::Write(const void *data, unsigned int size)
{
  if (size == 0) {
    log("Warning: tried to write 0 bytes into stream. Why would you?");
    return 0;
  }

  unsigned int bytesLeft = mCapacity - mTail;

  if (bytesLeft < size) {
    unsigned int reallocateBytes = std::max<unsigned int>(mCapacity * 2, mTail + size);
    mBuffer = (unsigned char*) std::realloc(mBuffer, reallocateBytes);
    assert(mBuffer && "Failed to reallocate stream buffer");
    mCapacity = reallocateBytes;

    bytesLeft = mCapacity - mTail;
    assert(bytesLeft >= size);

  }

  std::memcpy(mBuffer + mTail, data, size);
  mTail += size;
  assert(mTail <= mCapacity);

  return size;
}
