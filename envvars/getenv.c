#include <stdlib.h>
#include <stdio.h>

//cat /proc/$$/environ | tr \\0 \\n
//ps -aef --forest | ag bash
// There is stuff in /etc/environment for system-wide use.

int main(int argc, char **argv) {
    printf("%s -> %p\n", argv[1], getenv(argv[1]));
    return 0;
}

