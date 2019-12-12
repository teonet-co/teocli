#pragma once

#ifndef TEOCLI_API_H
#define TEOCLI_API_H

#include "teobase/platform.h"

// Default behavior is static library.
// No defines are needed to build or use static library.
// Define TEOCLI_DYNAMIC and TEOCLI_EXPORTS to build dynamic library.
// Define TEOCLI_DYNAMIC to import dynamic library.
// Use TEOCLI_API macro on all public API functions.
// Use TEOCLI_INTERNAL macro on functions used only in this library.
#if defined(TEOCLI_DYNAMIC)
#if defined(TEONET_OS_WINDOWS)
#if defined(TEOCLI_EXPORTS)
#define TEOCLI_API __declspec(dllexport)
#else
#define TEOCLI_API __declspec(dllimport)
#endif
#define TEOCLI_INTERNAL
#else
#define TEOCLI_API __attribute__((visibility("default")))
#define TEOCLI_INTERNAL __attribute__((visibility("hidden")))
#endif
#else
#define TEOCLI_API
#define TEOCLI_INTERNAL
#endif

#endif // TEOCLI_API_H
