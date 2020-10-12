/**
 * @file version.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Macro definitions for version information.
 * 
 * Versions of library, but also build information,
 * like info what compilers, compiler versions
 * or platforms were used.
 * 
 * <b>Formats for output:</b>
 * Library:  "<major>.<minor>.<patch>"
 * Compiler: "<name> <major>.<minor>.<patch>-<note>"
 * Platform: "<name> <architecture>"
 * 
 * 
 * @date 2020-09-30
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JAYC_VERSION_H
#define INCLUDE_JAYC_VERSION_H

#define S_(x) #x
#define S(x) S_(x)

//==============================================================================
// JAYC Library version.
//

// #define JAYC_VERSION_ALPHA
// #define JAYC_VERSION_BETA
// #define JAYC_VERSION_STABLE

#define JAYC_VERSION_MAJOR  1
#define JAYC_VERSION_MINOR  0
#define JAYC_VERSION_PATCH  0

#if defined(JAYC_VERSION_ALPHA)
  #define JAYC_VERSION_NOTE "alpha"
#elif defined(JAYC_VERSION_BETA)
  #define JAYC_VERSION_NOTE "beta"
#elif defined(JAYC_VERSION_STABLE)
  #define JAYC_VERSION_NOTE "stable"
#else
  #define JAYC_VERSION_NOTE "dev"
#endif

#define JAYC_VERSION_STRING     S(JAYC_VERSION_MAJOR) "." S(JAYC_VERSION_MINOR) "." S(JAYC_VERSION_PATCH) "-" JAYC_VERSION_NOTE

#define JAYC_BUILDTIME __DATE__ " " __TIME__

//==============================================================================
// Compilers
//

/* MS Visual Studio */
#if defined(__MSC_VER)
  #define JAYC_COMPILER_NAME     "MS Visual Studio"
  #define JAYC_COMPILER_VERSION  S(_MSC_VER)

/* clang */
#elif defined(__clang__)
  #define JAYC_COMPILER_NAME     "clang"
  #define JAYC_COMPILER_VERSION  S(__clang_major__) "." S(__clang_minor__) "." S(__clang_patchlevel__)

/* gcc */
#elif defined(__GNUC__)
  #define JAYC_COMPILER_NAME     "gcc"
  #define JAYC_COMPILER_VERSION  S(__GNUC__) "." S(__GNUC_MINOR__) "." S(__GNUC_PATCHLEVEL__)

/* mingw32 */
#elif defined(__MINGW32__)
  #define JAYC_COMPILER_NAME     "MinGW 32bit"
  #define JAYC_COMPILER_VERSION  S(__MINGW32_VERSION_MAJOR) "." S(__MINGW32_VERSION_MINOR)
  
/* mingw64 */
#elif defined(__MINGW64__)
  #define JAYC_COMPILER_NAME     "MinGW 64bit"
  #define JAYC_COMPILER_VERSION  S(__MINGW64_VERSION_MAJOR) "." S(__MINGW64_VERSION_MINOR)

/* unknown */
#else
  #define JAYC_COMPILER_NAME     "Unknown"
  #define JAYC_COMPILER_VERSION  ""

#endif

//==============================================================================
// Platform
//

/* Android */
#if defined(__ANDROID__)
  #define JAYC_PLATFORM_NAME      "Android"

/* Linux */
#elif defined(__linux__)
  #define JAYC_PLATFORM_NAME      "Linux"

/* Darwin */
#elif defined(__APPLE__)
  #define JAYC_PLATFORM_NAME      "Darwin"

/* Windows 64bit */
#elif defined(_WIN64)
  #define JAYC_PLATFORM_NAME      "Win64"

/* Windows 32bit */
#elif defined(_WIN32)
  #define JAYC_PLATFORM_NAME      "Win32"

/* FreeBSD */
#elif defined(__FreeBSD__)
  #define JAYC_PLATFORM_NAME      "FreeBSD"

/* OpenBSD */
#elif defined(__OpenBSD__)
  #define JAYC_PLATFORM_NAME      "OpenBSD"

/* default */
#else
  #define JAYC_PLATFORM_NAME      "Unknown"

#endif

//==============================================================================
// Architecture
//

/* x86 */
#if defined(__i386__)
  #define JAYC_ARCH_NAME          "x86"

/* x86_64 */
#elif defined(__x86_64__)
  #define JAYC_ARCH_NAME          "x86_64"

/* ARM */
#elif defined(__arm__)
  #define JAYC_ARCH_NAME          "ARM"

/* default */
#else
  #define JAYC_ARCH_NAME          "Unknown"

#endif

#endif /* INCLUDE_JAYC_VERSION_H */