#include "file.h"
#include "file_io.h"

#include <algorithm>

namespace si {

File::File()
{
  m_Handle = NULL;
}

bool File::Open(const char *c, Mode mode)
{
  // Create FileHandle if needed
  if (!m_Handle) {
    m_Handle = new FileHandle();
  }
  
  FileHandle::Mode fhMode = (mode == Read) ? FileHandle::ModeRead : FileHandle::ModeWrite;
  return static_cast<FileHandle*>(m_Handle)->Open(c, fhMode);
}

#ifdef _WIN32
bool File::Open(const wchar_t *c, Mode mode)
{
  // Create FileHandle if needed
  if (!m_Handle) {
    m_Handle = new FileHandle();
  }
  
  FileHandle::Mode fhMode = (mode == Read) ? FileHandle::ModeRead : FileHandle::ModeWrite;
  return static_cast<FileHandle*>(m_Handle)->Open(c, fhMode);
}
#endif

File::pos_t File::pos()
{
  if (!m_Handle) {
    return 0;
  }
  return static_cast<FileHandle*>(m_Handle)->GetPosition();
}

File::pos_t File::size()
{
  if (!m_Handle) {
    return 0;
  }
  return static_cast<FileHandle*>(m_Handle)->GetSize();
}

void File::seek(File::pos_t p, SeekMode s)
{
  if (!m_Handle) {
    return;
  }
  
  FileHandle::SeekMode seekMode;
  switch (s) {
  case SeekStart:
    seekMode = FileHandle::SeekBegin;
    break;
  case SeekCurrent:
    seekMode = FileHandle::SeekCurrent;
    break;
  case SeekEnd:
    seekMode = FileHandle::SeekEnd;
    break;
  default:
    seekMode = FileHandle::SeekBegin;
    break;
  }
  
  static_cast<FileHandle*>(m_Handle)->Seek(p, seekMode);
}

void File::Close()
{
  if (m_Handle) {
    static_cast<FileHandle*>(m_Handle)->Close();
    delete static_cast<FileHandle*>(m_Handle);
    m_Handle = NULL;
  }
}

File::pos_t File::ReadData(void *data, File::pos_t size)
{
  if (!m_Handle) {
    return 0;
  }
  return static_cast<FileHandle*>(m_Handle)->Read(data, size);
}

File::pos_t File::WriteData(const void *data, File::pos_t size)
{
  if (!m_Handle) {
    return 0;
  }
  return static_cast<FileHandle*>(m_Handle)->Write(data, size);
}

uint8_t FileBase::ReadU8()
{
  uint8_t u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteU8(uint8_t u)
{
  WriteData(&u, sizeof(u));
}

uint16_t FileBase::ReadU16()
{
  uint16_t u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteU16(uint16_t u)
{
  WriteData(&u, sizeof(u));
}

uint32_t FileBase::ReadU32()
{
  uint32_t u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteU32(uint32_t u)
{
  WriteData(&u, sizeof(u));
}

Vector3 FileBase::ReadVector3()
{
  Vector3 u;
  ReadData(&u, sizeof(u));
  return u;
}

void FileBase::WriteVector3(const Vector3 &v)
{
  WriteData(&v, sizeof(v));
}

std::string FileBase::ReadString()
{
  std::string d;

  while (true) {
    char c;
    ReadData(&c, 1);
    if (c == 0) {
      break;
    }
    d.push_back(c);
  }

  return d;
}

void FileBase::WriteString(const std::string &d)
{
  WriteData(d.c_str(), d.size());

  // Ensure null terminator
  WriteU8(0);
}

bytearray FileBase::ReadBytes(File::pos_t size)
{
  bytearray d;

  d.resize(size);
  ReadData(d.data(), size);

  return d;
}

void FileBase::WriteBytes(const bytearray &ba)
{
  WriteData(ba.data(), ba.size());
}

MemoryBuffer::MemoryBuffer()
{
  m_Position = 0;
}

MemoryBuffer::MemoryBuffer(const bytearray &data)
{
  m_Internal = data;
  m_Position = 0;
}

File::pos_t MemoryBuffer::pos()
{
  return m_Position;
}

File::pos_t MemoryBuffer::size()
{
  return m_Internal.size();
}

void MemoryBuffer::seek(File::pos_t p, SeekMode s)
{
  switch (s) {
  case SeekStart:
    m_Position = std::min(p, size());
    break;
  case SeekCurrent:
    m_Position = std::min(m_Position + p, size());
    break;
  case SeekEnd:
    if (p > size()) {
      m_Position = 0;
    } else {
      m_Position = size() - p;
    }
    break;
  }
}

File::pos_t MemoryBuffer::ReadData(void *data, File::pos_t size)
{
  pos_t remaining = m_Internal.size() - m_Position;
  size = std::min(size, remaining);
  memcpy(data, m_Internal.data() + m_Position, size);
  m_Position += size;
  return size;
}

File::pos_t MemoryBuffer::WriteData(const void *data, File::pos_t size)
{
  pos_t end = m_Position + size;
  if (end > m_Internal.size()) {
    m_Internal.resize(end);
  }
  memcpy(m_Internal.data() + m_Position, data, size);
  m_Position += size;
  return size;
}

}
