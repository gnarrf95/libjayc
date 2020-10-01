/**
 * @file jinfo.c
 * @author Manuel Nadji (https://github.com/gnarrf95)
 * 
 * @brief Simple tool to print library build info.
 * 
 * @date 2020-10-01
 * @copyright Copyright (c) 2020 by Manuel Nadji
 * 
 */

#include <jayc/jinfo.h>
#include <jayc/jproc.h>
#include <stdio.h>

//------------------------------------------------------------------------------
//
int main()
{
  printf("%s\n", jinfo_build_version());
  printf("Built with %s on %s\n", jinfo_build_compiler(), jinfo_build_platform());

  jproc_exit(0);
}