#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    void *s;
    s=realloc(s,7);
    sprintf((char*)s,"12345678");
    printf("%s\n",(char*)s);
    return 0;
}
