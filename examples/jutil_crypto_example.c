#include <jayc/jlog_stdio.h>
#include <jayc/jproc.h>
#include <jayc/jutil_crypto.h>
#include <string.h>

#define EXITVALUE_SUCCESS 0
#define EXITVALUE_FAILURE 1

int main()
{
  jlog_t *logger = jlog_stdio_session_init(JLOG_LOGTYPE_DEBUG);
  if(logger == NULL)
  {
    jproc_exit(EXITVALUE_FAILURE);
  }
  jlog_global_session_set(logger);

  const char *before = "Hello World!";
  size_t before_size = strlen(before);

  /* Get binary MD5. */
  unsigned char md5_raw[16];
  if(jutil_crypto_md5_raw(before, before_size, md5_raw) == 0)
  {
    JLOG_ERROR("jutil_crypto_md5_raw() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Get MD5 hexstring. */
  char md5_string[33] = { 0 };
  if(jutil_crypto_md5_str(before, before_size, md5_string) == 0)
  {
    JLOG_ERROR("jutil_crypto_md5_str() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("MD5 : [%s] -> [%s].", before, md5_string);

  /* Get binary SHA256. */
  unsigned char sha256_raw[32];
  if(jutil_crypto_sha256_raw(before, before_size, sha256_raw) == 0)
  {
    JLOG_ERROR("jutil_crypto_sha256_raw() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Get SHA256 hexstring. */
  char sha256_str[65] = { 0 };
  if(jutil_crypto_sha256_str(before, before_size, sha256_str) == 0)
  {
    JLOG_ERROR("jutil_crypto_sha256_str() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("SHA256 : [%s] -> [%s].", before, sha256_str);

  /* Get binary SHA512. */
  unsigned char sha512_raw[64];
  if(jutil_crypto_sha512_raw(before, before_size, sha512_raw) == 0)
  {
    JLOG_ERROR("jutil_crypto_sha512_raw() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }

  /* Get SHA512 hexstring. */
  char sha512_str[129] = { 0 };
  if(jutil_crypto_sha512_str(before, before_size, sha512_str) == 0)
  {
    JLOG_ERROR("jutil_crypto_sha512_str() failed.");
    jproc_exit(EXITVALUE_FAILURE);
  }
  JLOG_INFO("SHA512 : [%s] -> [%s].", before, sha512_str);

  jproc_exit(EXITVALUE_SUCCESS);
}