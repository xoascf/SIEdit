#ifndef FILE_IO_H
#define FILE_IO_H

/**
 * Platform-agnostic file I/O abstraction layer.
 * 
 * This abstraction provides a uniform interface for file operations across
 * different Windows platforms (desktop, UWP, Xbox) and non-Windows platforms.
 * 
 * Desktop Windows uses Win32 HANDLE-based APIs for optimal performance.
 * UWP/Xbox and other platforms use standard C++ streams.
 */

#include "types.h"

#if defined(_WIN32)
  #include "platform_win.h"
  #if SIEDIT_WIN_DESKTOP
    // Desktop Windows: Use Win32 HANDLE APIs
    #include <windows.h>
  #endif
#endif

namespace si {

/**
 * Platform-specific file handle abstraction.
 * 
 * On desktop Windows: HANDLE
 * On UWP/Xbox and other platforms: std::fstream*
 */
class FileHandle
{
public:
  enum Mode {
    ModeRead,
    ModeWrite
  };

  FileHandle();
  ~FileHandle();

  // Open file using narrow char path
  bool Open(const char *path, Mode mode);

#if defined(_WIN32)
  // Open file using wide char path (Windows only)
  bool Open(const wchar_t *path, Mode mode);
#endif

  // Get current file position
  uint64_t GetPosition() const;

  // Get total file size
  uint64_t GetSize() const;

  // Seek to position
  enum SeekMode {
    SeekBegin,
    SeekCurrent,
    SeekEnd
  };
  void Seek(uint64_t position, SeekMode mode);

  // Read data from file
  uint64_t Read(void *buffer, uint64_t size);

  // Write data to file
  uint64_t Write(const void *buffer, uint64_t size);

  // Close file
  void Close();

  // Check if file is open
  bool IsOpen() const;

private:
  void *m_Handle;
  Mode m_Mode;

  // Disable copy
  FileHandle(const FileHandle&);
  FileHandle& operator=(const FileHandle&);
};

}

#endif // FILE_IO_H
