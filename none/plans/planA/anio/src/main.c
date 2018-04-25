#include <stdio.h>
#include "sys.h"
#include "type.h"
#include "utils.h"


extern int add_cmd_thread();


void main() 
{
    add_cmd_thread();
    printf("----");
    while(1);
}
