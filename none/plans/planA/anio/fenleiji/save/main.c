#include <string.h>
#include <stdio.h>
#include "yuan.h"
#include "tools.h"

/*
void main()
{
    int i;
    unsigned int data[20]={5,78,13333,1489,665,46,54,654,65,4324,9,54,65465,46545,45,58,7000,2, 0, 0x7fffffff};

    get_user_prop();

    quick_sort(data, 0, 19);

    for(i=0; i<20; i++)
    {
        printf("%d\n", data[i]);
    }

}
*/
#if 0
int main(void)
{
    char input[16] = "abc,d";
    char *p;
    /**/ /* strtok places a NULL terminator
    in front of the token, if found */
    p = strtok(input, ",");
    if (p) printf("%s\n", p);
    /**/ /* A second call to strtok using a NULL
    as the first parameter returns a pointer
    to the character following the token */
    p = strtok(NULL, ",");
    if (p) printf("%s\n", p);
    return 0;
}
#endif

#if 0
void main()
{
    char *p;
    char cmdline[256];
    char cmd[64];
    char arg1[64];
    char arg2[64];
    char arg3[64];

    printf("This is chengyake's ...\n");
    while(1)
    {
        printf("\n>>");

        memset(&cmdline, 0, sizeof(cmdline));
        memset(&arg1, 0, sizeof(arg1));
        memset(&arg2, 0, sizeof(arg2));
        memset(&arg3, 0, sizeof(arg3));

        gets(&cmdline[0]);
        
        p=null;
        p = strtok(cmdline, " ");
        if (p) sprintf(&cmd[0], "%s", p);
        p = strtok(NULL, " ");
        if (p) sprintf(&arg1[0], "%s", p);
        p = strtok(NULL, " ");
        if (p) sprintf(&arg2[0], "%s", p);
        p = strtok(NULL, " ");
        if (p) sprintf(&arg3[0], "%s", p);

        if(cmd[0] != 0)
        {
            printf("%s\n", cmd);
        }
        if(arg1[0] != 0)
        {
            printf("%s\n", arg1);
        }
        if(arg2[0] != 0)
        {
            printf("%s\n", arg2);
        }
        if(arg3[0] != 0)
        {
            printf("%s\n", arg3);
        }

    }
}
#endif





