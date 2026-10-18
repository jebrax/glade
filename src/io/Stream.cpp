#include <cassert>
#include <cstring>
#include <glade/io/Stream.h>
#include <glade/debug/log.h>
#include <algorithm>

unsigned int Glade::MemoryInputStream::read(void *out_data, unsigned int size) {
  if (size == 0) {
    ::log("Warning: tried to read 0 bytes from a stream. Why would you?");
    return 0;
  }

  if (mHead >= mCapacity) {
    assert(mHead == mCapacity);
    return 0;
  }

  unsigned int unread_bytes_left = mCapacity - mHead;
  unsigned int bytes_to_read = std::min<unsigned int>(unread_bytes_left, size);

  std::memcpy(out_data, mBuffer + mHead, bytes_to_read);
  mHead += bytes_to_read;
  assert(mHead <= mCapacity);

  return bytes_to_read;
}

unsigned int Glade::MemoryOutputStream::write(const void *data, unsigned int size) {
  if (size == 0) {
    ::log("Warning: tried to write 0 bytes into stream. Why would you?");
    return 0;
  }

  unsigned int bytes_left = mCapacity - mTail;

  if (bytes_left < size) {
    unsigned int reallocate_bytes = std::max<unsigned int>(mCapacity * 2, mTail + size);
    mBuffer = (unsigned char *)std::realloc(mBuffer, reallocate_bytes);
    assert(mBuffer && "Failed to reallocate stream buffer");
    mCapacity = reallocate_bytes;

    bytes_left = mCapacity - mTail;
    assert(bytes_left >= size);
  }

  std::memcpy(mBuffer + mTail, data, size);
  mTail += size;
  assert(mTail <= mCapacity);

  return size;
}
