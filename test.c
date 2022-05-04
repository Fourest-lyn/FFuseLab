#include <stdio.h>
#include <string.h>

static const int maxN=1000;
static int debugTime=0;


int main()
{
    char s[maxN];
    scanf("%s",s);
    printf("%s, len=%ld\n",s,strlen(s));

    int len=strlen(s),count=0,index[2];
    for(int i=len-1;i>=0;--i)
    {
        if (s[i]=='/') index[count++]=i;
        if (count==2) break;
    }
    char newPath[maxN]="";
    strncat(newPath,s,index[1]);
    strncat(newPath,s+index[0],len-index[0]);
    strncat(newPath,s+index[1],index[0]-index[1]);
    printf("new path=%s, len(newPath)=%ld, old path=%s\n",newPath,strlen(newPath),s);
    return 0;
}
