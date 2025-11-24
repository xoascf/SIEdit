#include "file_io.h"

#if defined(_WIN32)
  #include "platform_win.h"
  #if SIEDIT_WIN_DESKTOP
    // Desktop Windows - use Win32 HANDLE APIs
    #include <windows.h>
  #else
    // UWP/Xbox - use standard C++ streams
    #include <fstream>
    #define FSTR(x) static_cast<std::fstream*>(x)
  #endif
#else
  // Non-Windows platforms - use standard C++ streams
  #include <fstream>
  #define FSTR(x) static_cast<std::fstream*>(x)
#endif

namespace si {

FileHandle::FileHandle()
  : m_Handle(NULL)
  , m_Mode(ModeRead)
{
}

FileHandle::~FileHandle()
{
  Close();
}

bool FileHandle::Open(const char *path, Mode mode)
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use Win32 CreateFileA
  m_Handle = CreateFileA(path,
                         mode == ModeRead ? GENERIC_READ : GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         mode == ModeRead ? OPEN_EXISTING : CREATE_NEW,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  m_Mode = mode;
  return m_Handle != INVALID_HANDLE_VALUE;
#else
  // UWP/Xbox and non-Windows: Use standard C++ streams
  std::ios::openmode openmode = std::ios::binary;
  
  if (mode == ModeRead) {
    openmode |= std::ios::in;
  } else {
    openmode |= std::ios::out;
  }
  
  m_Handle = new std::fstream();
  FSTR(m_Handle)->open(path, openmode);
  if (FSTR(m_Handle)->good() && FSTR(m_Handle)->is_open()) {
    m_Mode = mode;
    return true;
  }
  
  // Failed to open, clean up
  delete FSTR(m_Handle);
  m_Handle = NULL;
  return false;
#endif
}

#if defined(_WIN32)
bool FileHandle::Open(const wchar_t *path, Mode mode)
{
#if SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use Win32 CreateFileW
  m_Handle = CreateFileW(path,
                         mode == ModeRead ? GENERIC_READ : GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         mode == ModeRead ? OPEN_EXISTING : CREATE_NEW,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  m_Mode = mode;
  return m_Handle != INVALID_HANDLE_VALUE;
#else
  // UWP/Xbox: Use standard C++ streams with wide char
  std::ios::openmode openmode = std::ios::binary;
  
  if (mode == ModeRead) {
    openmode |= std::ios::in;
  } else {
    openmode |= std::ios::out;
  }
  
  m_Handle = new std::fstream();
  FSTR(m_Handle)->open(path, openmode);
  if (FSTR(m_Handle)->good() && FSTR(m_Handle)->is_open()) {
    m_Mode = mode;
    return true;
  }
  
  // Failed to open, clean up
  delete FSTR(m_Handle);
  m_Handle = NULL;
  return false;
#endif
}
#endif

uint64_t FileHandle::GetPosition() const
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use SetFilePointer
  LONG high = 0;
  DWORD low = SetFilePointer((HANDLE)m_Handle, 0, &high, FILE_CURRENT);
  return (uint64_t(high) << 32) | low;
#else
  // UWP/Xbox and non-Windows: Use stream tell
  if (m_Mode == ModeRead) {
    return FSTR(m_Handle)->tellg();
  } else {
    return FSTR(m_Handle)->tellp();
  }
#endif
}

uint64_t FileHandle::GetSize() const
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use GetFileSize
  DWORD high;
  DWORD low = GetFileSize((HANDLE)m_Handle, &high);
  return (uint64_t(high) << 32) | low;
#else
  // UWP/Xbox and non-Windows: Calculate size using seek
  // Save current position
  std::streampos currentPos;
  if (m_Mode == ModeRead) {
    currentPos = FSTR(m_Handle)->tellg();
    FSTR(m_Handle)->seekg(0, std::ios::end);
    std::streampos size = FSTR(m_Handle)->tellg();
    FSTR(m_Handle)->seekg(currentPos);
    return static_cast<uint64_t>(size);
  } else {
    currentPos = FSTR(m_Handle)->tellp();
    FSTR(m_Handle)->seekp(0, std::ios::end);
    std::streampos size = FSTR(m_Handle)->tellp();
    FSTR(m_Handle)->seekp(currentPos);
    return static_cast<uint64_t>(size);
  }
#endif
}

void FileHandle::Seek(uint64_t position, SeekMode seekMode)
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use SetFilePointer
  LONG high = position >> 32;
  DWORD low = (DWORD)position;
  
  DWORD moveMethod;
  switch (seekMode) {
  case SeekBegin:
    moveMethod = FILE_BEGIN;
    break;
  case SeekCurrent:
    moveMethod = FILE_CURRENT;
    break;
  case SeekEnd:
    moveMethod = FILE_END;
    break;
  default:
    moveMethod = FILE_BEGIN;
    break;
  }
  
  SetFilePointer((HANDLE)m_Handle, low, &high, moveMethod);
#else
  // UWP/Xbox and non-Windows: Use stream seek
  std::ios::seekdir dir;
  switch (seekMode) {
  case SeekBegin:
    dir = std::ios::beg;
    break;
  case SeekCurrent:
    dir = std::ios::cur;
    break;
  case SeekEnd:
    dir = std::ios::end;
    break;
  default:
    dir = std::ios::beg;
    break;
  }
  
  if (m_Mode == ModeRead) {
    FSTR(m_Handle)->seekg(position, dir);
  } else {
    FSTR(m_Handle)->seekp(position, dir);
  }
#endif
}

uint64_t FileHandle::Read(void *buffer, uint64_t size)
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use ReadFile
  DWORD bytesRead;
  ReadFile((HANDLE)m_Handle, buffer, (DWORD)size, &bytesRead, NULL);
  return bytesRead;
#else
  // UWP/Xbox and non-Windows: Use stream read
  std::streampos before = FSTR(m_Handle)->tellg();
  FSTR(m_Handle)->read((char*)buffer, size);
  return static_cast<uint64_t>(FSTR(m_Handle)->tellg() - before);
#endif
}

uint64_t FileHandle::Write(const void *buffer, uint64_t size)
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use WriteFile
  DWORD bytesWritten;
  WriteFile((HANDLE)m_Handle, buffer, (DWORD)size, &bytesWritten, NULL);
  return bytesWritten;
#else
  // UWP/Xbox and non-Windows: Use stream write
  std::streampos before = FSTR(m_Handle)->tellp();
  FSTR(m_Handle)->write((const char*)buffer, size);
  return static_cast<uint64_t>(FSTR(m_Handle)->tellp() - before);
#endif
}

void FileHandle::Close()
{
  if (!m_Handle) {
    return;
  }

#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  // Desktop Windows: Use CloseHandle
  CloseHandle((HANDLE)m_Handle);
  m_Handle = NULL;
#else
  // UWP/Xbox and non-Windows: Close and delete stream
  FSTR(m_Handle)->close();
  delete FSTR(m_Handle);
  m_Handle = NULL;
#endif
}

bool FileHandle::IsOpen() const
{
#if defined(_WIN32) && SIEDIT_WIN_DESKTOP
  return m_Handle != NULL && m_Handle != INVALID_HANDLE_VALUE;
#else
  return m_Handle != NULL && FSTR(m_Handle)->is_open();
#endif
}

}
