static void cmd_thread(void)
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

        if(strcmp(&cmd[0], "help"))
        {
            printf("%s\n", cmd);
        }
        else
        {
            printf("----------------------------------------\n");
            printf("cmd\targ1\targ2\targ3\n");
            printf("----------------------------------------\n");
        }


    }
}

int add_cmd_thread()
{
    return pthread_create(&id,NULL,(void *) dbg_thread,NULL);
}



/**************************cmd func*************************************/
static inline get_user_prop()
{
    printf("\n");
    printf("USER CONFIG:\n");
    printf("\n");
    printf("----------------------------------------\n");
    printf("CTRL_FLASH_HZ\t:\t%d\nFIX_OUTPUT\t:\t0x%x\nTHRESHOLD\t:\t0x%x\n", CTRL_FLASH_HZ, OUTPUT, THRESHOLD);
    printf("----------------------------------------\n");
    //printf("name\t:\tstart\tend\tnum\n");
    printf("YUAN\t:\t%d\t%d\t%d\n", YUAN_START, YUAN_END, YUAN_NUM);
    printf("SA\t:\t%d\t%d\t%d\n", SA_START, SA_END, SA_NUM);
    printf("SN\t:\t%d\t%d\t%d\n", SN_START, SN_END, SN_NUM);
    printf("I\t:\t%d\t%d\t%d\n", I_START, I_END, I_NUM);
    printf("A\t:\t%d\t%d\t%d\n", A_START, A_END, A_NUM);
    printf("N\t:\t%d\t%d\t%d\n", N_START, N_END, N_NUM);
    printf("O\t:\t%d\t%d\t%d\n", O_START, O_END, O_NUM);
    printf("H\t:\t%d\t%d\t%d\n", H_START, H_END, H_NUM);
    printf("----------------------------------------\n");
}
