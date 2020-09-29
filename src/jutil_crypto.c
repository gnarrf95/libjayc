/**
 * @file jutil_crypto.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Implements jutil_crypto functions.
 * 
 * @date 2020-09-2
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jutil_crypto.h>
#include <stdio.h>
#include <stdbool.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

//------------------------------------------------------------------------------
//
int jutil_crypto_md5_raw(const char *input, size_t size_input, unsigned char *output)
{
  MD5_CTX ctx;

  MD5_Init(&ctx);

  /*
   * Solution, to fix string sizes over 512 Bytes.
   * Found here: https://stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c
   */
  while(size_input > 0)
  {
    if(size_input > 512)
    {
      MD5_Update(&ctx, input, 512);
    }
    else
    {
      MD5_Update(&ctx, input, size_input);
      break;
    }

    size_input -= 512;
    input += 512;
  }

  MD5_Final(output, &ctx);

  return true;
}

//------------------------------------------------------------------------------
//
int jutil_crypto_md5_str(const char *input, size_t size_input, char *output)
{
  unsigned char hash_buf[16];
  if(jutil_crypto_md5_raw(input, size_input, hash_buf) == false)
  {
    return false;
  }

  int ctr;
  for(ctr = 0; ctr < 16; ctr++)
  {
    snprintf(&(output[ctr*2]), 32, "%02x", (unsigned int)hash_buf[ctr]);
  }
  output[32] = 0;

  return true;
}

//------------------------------------------------------------------------------
//
int jutil_crypto_sha256_raw(const char *input, size_t size_input, unsigned char *output)
{
  SHA256((unsigned char *)input, size_input, output);
  return true;
}

//------------------------------------------------------------------------------
//
int jutil_crypto_sha256_str(const char *input, size_t size_input, char *output)
{
  unsigned char hash_buf[32];
  if(jutil_crypto_sha256_raw(input, size_input, hash_buf) == false)
  {
    return false;
  }

  int ctr;
  for(ctr = 0; ctr < 32; ctr++)
  {
    snprintf(&(output[ctr*2]), 64, "%02x", (unsigned int)hash_buf[ctr]);
  }
  output[64] = 0;

  return true;
}

//------------------------------------------------------------------------------
//
int jutil_crypto_sha512_raw(const char *input, size_t size_input, unsigned char *output)
{
  SHA512((unsigned char *)input, size_input, output);
  return true;
}

//------------------------------------------------------------------------------
//
int jutil_crypto_sha512_str(const char *input, size_t size_input, char *output)
{
  unsigned char hash_buf[64];
  if(jutil_crypto_sha512_raw(input, size_input, hash_buf) == false)
  {
    return false;
  }

  int ctr;
  for(ctr = 0; ctr < 64; ctr++)
  {
    snprintf(&(output[ctr*2]), 128, "%02x", (unsigned int)hash_buf[ctr]);
  }
  output[128] = 0;

  return true;
}