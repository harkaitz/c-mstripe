#include "../mstripe-language.h"
#include <stdio.h>

int main () {

    setlocale(LC_ALL, "");

    fputs(mstripe_language(), stdout);
    fputc('\n', stdout);

    return 0;
}
