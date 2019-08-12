/**
 * @file teonet_platform.h
 * @brief Defines to make platform dependent and compiler dependent compilation easier.
 */

#ifndef TEONET_PLATFORM_H
#define TEONET_PLATFORM_H

// Determine target operating system.
#if defined(__ANDROID__)
// Defined if target OS is android.
#define TEONET_OS_ANDROID
#elif defined(__linux__)
// Defined if target OS is linux.
#define TEONET_OS_LINUX
#elif defined(_WIN32)
// Defined if target OS is Windows.
#define TEONET_OS_WINDOWS
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define TEONET_OS_IOS
#else
#define TEONET_OS_MACOS
#endif
#else
#error Unsupported target OS.
#endif

// Determine current compiler type.
#if defined(__MINGW32__)
// Defined if current compiler is MINGW. Use this for compiler-dependent code.
#define TEONET_COMPILER_MINGW
#elif defined(__GNUC__)
// Defined if current compiler is GCC. Use this for compiler-dependent code.
#define TEONET_COMPILER_GCC
#elif defined(_MSC_VER)
// Defined if current compiler is MSVC. Use this for compiler-dependent code.
#define TEONET_COMPILER_MSVC
#else
#error Unsupported compiler.
#endif

// This section is for doxygen. Keep it in sync with macroses above.
#if defined(FORCE_DOXYGEN)
// Defined if target OS is android.
#define TEONET_OS_ANDROID
#undef TEONET_OS_ANDROID
/// Defined if target OS is linux.
#define TEONET_OS_LINUX
#undef TEONET_OS_LINUX  // We have to undefine all macroses to not screw up preprocessing.
/// Defined if target OS is Windows.
#define TEONET_OS_WINDOWS
#undef TEONET_OS_WINDOWS

// Defined if current compiler is MINGW. Use this for compiler-dependent code.
#define TEONET_COMPILER_MINGW
#undef TEONET_COMPILER_MINGW
/// Defined if current compiler is GCC. Use this for compiler-dependent code.
#define TEONET_COMPILER_GCC
#undef TEONET_COMPILER_GCC
/// Defined if current compiler is MSVC. Use this for compiler-dependent code.
#define TEONET_COMPILER_MSVC
#undef TEONET_COMPILER_MSVC
#endif

#endif
