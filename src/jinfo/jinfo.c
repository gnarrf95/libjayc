/**
 * @file jinfo.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements functions for version and build info.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jinfo.h>
#include <version.h>

//------------------------------------------------------------------------------
//
const char *jinfo_build_version()
{
  return "libJayC v" JAYC_VERSION_STRING " (" JAYC_BUILDTIME ")";
}

//------------------------------------------------------------------------------
//
const char *jinfo_build_platform()
{
  return JAYC_PLATFORM_NAME "(" JAYC_ARCH_NAME ")";
}

//------------------------------------------------------------------------------
//
const char *jinfo_build_compiler()
{
  return JAYC_COMPILER_NAME " " JAYC_COMPILER_VERSION;
}