#pragma once

// FIXME get rid of windows.h here
#include <windows.h>

// Little endian
#define FOURCC_RIFF 'FFIR'
#define FOURCC_DATA 'atad'
#define FOURCC_FMT ' tmf'
#define FOURCC_WAVE 'EVAW'
#define FOURCC_XWMA 'AMWX'
#define FOURCC_DPDS 'sdpd'

class RIFFReader {
public:
  static HRESULT findChunk(HANDLE h_file, unsigned long fourcc, unsigned long &dw_chunk_size,
                           unsigned long &dw_chunk_data_position) {
    HRESULT hr = S_OK;

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h_file, 0, NULL, FILE_BEGIN))
      return HRESULT_FROM_WIN32(::GetLastError());

    unsigned long chunk_type;
    unsigned long chunk_data_size;
    unsigned long riff_data_size = 0;
    unsigned long file_type;
    unsigned long bytes_read = 0;
    unsigned long offset = 0;

    while (hr == S_OK) {
      unsigned long how_many_was_read;

      if (0 == ::ReadFile(h_file, &chunk_type, sizeof(unsigned long), &how_many_was_read, NULL))
        hr = HRESULT_FROM_WIN32(::GetLastError());

      if (0 ==
          ::ReadFile(h_file, &chunk_data_size, sizeof(unsigned long), &how_many_was_read, NULL))
        hr = HRESULT_FROM_WIN32(::GetLastError());

      switch (chunk_type) {
      case FOURCC_RIFF:
        riff_data_size = chunk_data_size;
        chunk_data_size = 4;

        if (0 == ::ReadFile(h_file, &file_type, sizeof(unsigned long), &how_many_was_read, NULL))
          hr = HRESULT_FROM_WIN32(::GetLastError());

        break;

      default:
        if (INVALID_SET_FILE_POINTER ==
            ::SetFilePointer(h_file, chunk_data_size, NULL, FILE_CURRENT))
          return HRESULT_FROM_WIN32(::GetLastError());
      }

      offset += sizeof(unsigned long) * 2;

      if (chunk_type == fourcc) {
        dw_chunk_size = chunk_data_size;
        dw_chunk_data_position = offset;

        return S_OK;
      }

      offset += chunk_data_size;

      if (bytes_read >= riff_data_size) return S_FALSE;
    }

    return S_OK;
  }

  static HRESULT readChunkData(HANDLE h_file, void *buffer, unsigned long buffersize,
                               unsigned long bufferoffset) {
    HRESULT hr = S_OK;

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h_file, bufferoffset, NULL, FILE_BEGIN))
      return HRESULT_FROM_WIN32(::GetLastError());

    unsigned long how_many_was_read;

    if (0 == ::ReadFile(h_file, buffer, buffersize, &how_many_was_read, NULL))
      hr = HRESULT_FROM_WIN32(::GetLastError());

    return hr;
  }
};
