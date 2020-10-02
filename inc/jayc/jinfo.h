/**
 * @file jinfo.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Functions for version and build info.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JINFO_H
#define INCLUDE_JINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns version string with build time.
 * 
 * Format example: "libJayC v0.7.4-alpha (Oct  1 2020 12:56:13)".
 * 
 * @return String with build info.
 */
const char *jinfo_build_version();

/**
 * @brief Returns info about platform on which it was built.
 * 
 * Exmaple: "Linux(x86_64)".
 * 
 * @return String with platform info.
 */
const char *jinfo_build_platform();

/**
 * @brief Returns info about compiler with which it was built.
 * 
 * Example: "gcc 7.5.0"
 * 
 * @return String with compiler info. 
 */
const char *jinfo_build_compiler();

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JINFO_H */