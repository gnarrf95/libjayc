#include <jayc/jinfo.h>
#include <stdio.h>

int main()
{
  // Print Library version and build datetime.
  printf("%s\n", jinfo_build_version());

  // Print Compiler and Platform info.
  printf("Built with %s on %s\n",
    jinfo_build_compiler(),
    jinfo_build_platform()
  );

  return 0;
}