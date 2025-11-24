# Platform Compatibility Guide

## Overview

libweaver has been designed to support multiple Windows platforms:
- **Desktop Windows**: Full Win32 API support with HANDLE-based file I/O
- **UWP (Universal Windows Platform)**: Windows Store apps with restricted API access
- **Xbox**: Xbox platform with restricted API access
- **Other platforms**: Linux, macOS, etc. using standard C++ streams

## Platform Detection

The platform detection is handled automatically through `platform_win.h`:

```cpp
#include "platform_win.h"

#if SIEDIT_WIN_DESKTOP
  // Desktop Windows code using Win32 APIs
#elif SIEDIT_WIN_UWP
  // UWP/Xbox code using standard C++ APIs
#endif
```

### Macros

- `SIEDIT_WIN_DESKTOP`: Set to 1 when building for desktop Windows with full Win32 API access
- `SIEDIT_WIN_UWP`: Set to 1 when building for UWP/Xbox with restricted API access

## File I/O Abstraction

All file I/O operations go through the `FileHandle` class in `file_io.h/cpp`. This provides a uniform interface across all platforms:

### Desktop Windows
- Uses Win32 APIs: `CreateFile`, `ReadFile`, `WriteFile`, `SetFilePointer`, `GetFileSize`, `CloseHandle`
- Optimal performance with direct HANDLE operations
- Supports both narrow (`char*`) and wide (`wchar_t*`) character paths

### UWP/Xbox
- Uses standard C++ `<fstream>` APIs
- Compatible with Windows Store and Xbox app restrictions
- Supports both narrow and wide character paths through stream APIs

### Other Platforms
- Uses standard C++ `<fstream>` APIs
- Platform-independent implementation
- Maximum portability

## Building for Different Platforms

### Desktop Windows
No special configuration needed. The code automatically detects desktop Windows and uses Win32 APIs.

```bash
cmake -B build
cmake --build build
```

### UWP/Xbox
The build system automatically detects UWP/Xbox through the Windows SDK's `winapifamily.h` header. When `WINAPI_PARTITION_DESKTOP` is not available, the code falls back to standard C++ APIs.

For explicit UWP builds, use the appropriate CMake toolchain:

```bash
cmake -B build -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0
cmake --build build
```

### Linux/macOS
Standard build process:

```bash
cmake -B build
cmake --build build
```

## API Changes

### Public API
No changes to the public API. All existing code using `si::File` continues to work without modification.

### Internal Changes
- `File` class now uses `FileHandle` internally instead of raw `void*` handles
- Removed `m_Mode` member from `File` class (now tracked by `FileHandle`)
- All platform-specific code moved to `file_io.cpp`

## Error Handling

The abstraction layer includes comprehensive error handling:

### Win32 APIs
- Checks return values from all Win32 functions
- Validates `INVALID_HANDLE_VALUE`, `INVALID_SET_FILE_POINTER`, `INVALID_FILE_SIZE`
- Uses `GetLastError()` to distinguish between valid and error conditions

### Stream APIs
- Checks stream state using `good()`, `fail()`, `is_open()`
- Validates `tellg()` and `tellp()` for `-1` error returns
- Uses `gcount()` for accurate read byte counts

### Return Values
- Failed operations return 0 for byte counts or sizes
- Boolean operations (open, close) return appropriate success/failure indicators
- Errors are handled gracefully without crashing

## Testing

A test program is available that validates:
- File creation and writing
- File reading and position tracking
- Data integrity (U32 values, strings)
- Size calculation

Run the test:
```bash
cd build
./test_file_io  # If built with tests
```

## Migration Notes

### Upgrading from Previous Versions
No code changes required. The abstraction is fully backward compatible.

### Platform-Specific Considerations

**Desktop Windows:**
- File write operations now use `CREATE_ALWAYS` instead of `CREATE_NEW`
- This allows overwriting existing files, which is more intuitive behavior
- Previous code using `CREATE_NEW` would fail if the file already existed

**UWP/Xbox:**
- File system access may be restricted to specific folders (app data, temp)
- Some path formats may not be supported
- Test file operations thoroughly in the target environment

**All Platforms:**
- The library maintains C++98 compatibility
- No external dependencies beyond standard C++ library
- Thread-safety depends on individual `File` object usage (not thread-safe per instance)

## Future Enhancements

Potential future improvements:
- Async I/O support for better performance on modern platforms
- Memory-mapped file I/O for large files
- Extended error reporting (error codes, messages)
- Platform-specific optimizations (e.g., mmap on Unix)
