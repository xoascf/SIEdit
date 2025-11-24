#ifndef PLATFORM_WIN_H
#define PLATFORM_WIN_H

/**
 * Platform detection header for Windows-specific builds.
 * 
 * This header detects the Windows API family partition and defines macros
 * to enable/disable desktop-only Win32 APIs for UWP/Xbox compatibility.
 * 
 * Defines:
 * - SIEDIT_WIN_DESKTOP: Set to 1 if building for desktop Windows with full Win32 API
 * - SIEDIT_WIN_UWP: Set to 1 if building for UWP/Xbox with restricted API
 */

#if defined(_WIN32)
  // Include Windows API family detection header
  #include <winapifamily.h>
  
  // Check if we're building for desktop Windows with full Win32 API access
  #if !defined(WINAPI_FAMILY) || WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    #define SIEDIT_WIN_DESKTOP 1
  #else
    // UWP, Xbox, or other restricted Windows partition
    #define SIEDIT_WIN_UWP 1
  #endif
#endif

#endif // PLATFORM_WIN_H
