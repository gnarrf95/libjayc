/**
 * @file jutil_crypto.h
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Interface provides cryptography and hash functionality.
 * 
 * @date 2020-09-29
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#ifndef INCLUDE_JUTIL_CRYPTO_H
#define INCLUDE_JUTIL_CRYPTO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generates md5 hash of input and returns it as binary.
 * 
 * @param input       Input string.
 * @param size_input  Size of input string.
 * @param output      Buffer to save output.
 *                    Must be at least buffer size <tt>char buf[16]</tt>.
 * 
 * @return            @c true , if successful.
 * @return            @c false , if error occured.
 */
int jutil_crypto_md5_raw(const char *input, size_t size_input, unsigned char *output);

/**
 * @brief Generates md5 hash of input and returns it as hexstring.
 * 
 * @param input       Input string.
 * @param size_input  Size of input string.
 * @param output      Buffer to save output.
 *                    Must be at least buffer size <tt>char buf[33]</tt>.
 * 
 * @return            @c true , if successful.
 * @return            @c false , if error occured.
 */
int jutil_crypto_md5_str(const char *input, size_t size_input, char *output);

/**
 * @brief Generates 256bit sha hash of input and returns it as binary.
 * 
 * @param input       Input string.
 * @param size_input  Size of input string.
 * @param output      Buffer to save output.
 *                    Must be at least buffer size <tt>char buf[32]</tt>
 * 
 * @return            @c true , if successful.
 * @return            @c false , if error occured.
 */
int jutil_crypto_sha256_raw(const char *input, size_t size_input, unsigned char *output);

/**
 * @brief Generates 256bit sha hash of input and returns it as hexstring.
 * 
 * @param input       Input string.
 * @param size_input  Size of input string.
 * @param output      Buffer to save output.
 *                    Must be at least buffer size <tt>char buf[65]</tt>
 * 
 * @return            @c true , if successful.
 * @return            @c false , if error occured.
 */
int jutil_crypto_sha256_str(const char *input, size_t size_input, char *output);

/**
 * @brief Generates 512bit sha hash of input and returns it as binary.
 * 
 * @param input       Input string.
 * @param size_input  Size of input string.
 * @param output      Buffer to save output.
 *                    Must be at least buffer size <tt>char buf[64]</tt>
 * 
 * @return            @c true , if successful.
 * @return            @c false , if error occured.
 */
int jutil_crypto_sha512_raw(const char *input, size_t size_input, unsigned char *output);

/**
 * @brief Generates 512bit sha hash of input and returns it as hexstring.
 * 
 * @param input       Input string.
 * @param size_input  Size of input string.
 * @param output      Buffer to save output.
 *                    Must be at least buffer size <tt>char buf[129]</tt>
 * 
 * @return            @c true , if successful.
 * @return            @c false , if error occured.
 */
int jutil_crypto_sha512_str(const char *input, size_t size_input, char *output);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_JUTIL_CRYPTO_H */