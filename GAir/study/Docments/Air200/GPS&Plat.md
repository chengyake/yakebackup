# ���
>��Ȼlib�е�gps��δֱ����������gps��ܣ�������uartΪ����\
>����gpsģ��Ϊ��������Ϥһ������ƽ̨��ܻ��ǿ��Ե�
---

### ���ù�ϵ

1. core/platform/OpenAT_inc/am_openat_drv.h line��2017
>gpsģ�����ṹ�嶨�壬����linux��DTS
```
typedef struct
{
    T_AMOPENAT_RDAGPS_I2C_CFG    i2c;
    T_AMOPENAT_RDAGPS_GPS_CFG   gps;
}T_AMOPENAT_RDAGPS_PARAM;
```
2. core/cust_src/elua/platform/coolsand/src/platform_gps.c
>����linux֮ǰ��platform����Щ��Դ����
```
int platform_gps_open(void){    
    T_AMOPENAT_RDAGPS_PARAM cfg;

    cfg.gps.pinPowerOnPort = OPENAT_GPIO_7;
    cfg.gps.pinResetPort = OPENAT_GPIO_UNKNOWN;
    cfg.gps.pinBpWakeupGpsPort = OPENAT_GPO_0;
    cfg.gps.pinBpWakeupGpsPolarity = FALSE;
    cfg.gps.pinGpsWakeupBpPort = OPENAT_GPIO_1;
    cfg.gps.pinGpsWakeupBpPolarity = FALSE;

    cfg.i2c.port = OPENAT_I2C_2;

    PUB_TRACE("[platform_gps_open]");
    IVTBL(poweron_ldo)(OPENAT_LDO_POWER_ASW, 1);
    IVTBL(rdaGps_open)(&cfg);
    return PLATFORM_OK;
}

int platform_gps_close(void){
    ...
```
3. core/cust_src/elua/modules/src/gps.c
>׼����luaע��

```
static int l_gps_open(lua_State *L) {
    lua_pushinteger(L, platform_gps_open());    
    return 1; 
}
static int l_gps_close(lua_State *L) {
    lua_pushinteger(L, platform_gps_close());
    return 1; 
}
// Module function map
const LUA_REG_TYPE gpscore_map[] =
{ 
  { LSTRKEY( "open" ),  LFUNCVAL( l_gps_open ) },
  { LSTRKEY( "close" ),  LFUNCVAL( l_gps_close ) },
 
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_gpscore( lua_State *L )
{
    luaL_register( L, AUXLIB_GPSCORE, gpscore_map );
    return 1;
}  
```


4. core/cust_src/elua/platform/coolsand/include/platform_conf.h
>ͨ��LUA_PLATFORM_LIBS_ROM�꣬����holdס���ÿ�
```
#define LUA_PLATFORM_LIBS_ROM \
    _ROM( AUXLIB_BIT, luaopen_bit, bit_map ) \
    _ROM( AUXLIB_BITARRAY, luaopen_bitarray, bitarray_map ) \
    _ROM( AUXLIB_PACK, luaopen_pack, pack_map ) \
    _ROM( AUXLIB_PIO, luaopen_pio, pio_map ) \
    _ROM( AUXLIB_UART, luaopen_uart, uart_map ) \
    _ROM( AUXLIB_I2C, luaopen_i2c, i2c_map ) \
    _ROM( AUXLIB_RTOS, luaopen_rtos, rtos_map ) \
    DISP_LIB_LINE \
    _ROM( AUXLIB_PMD, luaopen_pmd, pmd_map ) \
    _ROM( AUXLIB_ADC, luaopen_adc, adc_map ) \
    ICONV_LINE \
    _ROM( AUXLIB_AUDIOCORE, luaopen_audiocore, audiocore_map ) \
    ZLIB_LINE \
    JSON_LIB_LINE \
    _ROM( AUXLIB_WATCHDOG, luaopen_watchdog, watchdog_map ) \
    _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map) \
    APN_LINE \
    _ROM( AUXLIB_GPSCORE, luaopen_gpscore, gpscore_map) 
```

5. core/cust_src/elua/lua/src/linit.c
>����ע������Ŀ�ӿڣ��ֲ�����һЩ

```
static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
#if defined(LUA_IO_LIB)    
  {LUA_IOLIBNAME, luaopen_io},
#endif
#if defined(LUA_OS_LIB)
  {LUA_OSLIBNAME, luaopen_os},
#endif
  {LUA_STRLIBNAME, luaopen_string},
#if defined(LUA_MATH_LIB)    
  {LUA_MATHLIBNAME, luaopen_math},
#endif
#if defined(LUA_DEBUG_LIB)
  {LUA_DBLIBNAME, luaopen_debug},
#endif
#if defined(LUA_PLATFORM_LIBS_ROM)
#define _ROM( name, openf, table ) { name, openf },
  LUA_PLATFORM_LIBS_ROM
#endif
  {NULL, NULL}
};
LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}
```

6. core/cust_src/elua/lua/src/lua.c line��365
>����luaL_openlibs

```
static int pmain (lua_State *L) {
 ...
 luaL_openlibs(L);  /* open libraries */
 ...
```
6. core/cust_src/elua/lua/src/lua.c line��394
>����pmain
```
int lua_main (int argc, char **argv) {
  int status;
  struct Smain s;
  lua_State *L = lua_open();  /* create state */
  if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
  s.argc = argc;
  s.argv = argv;
  status = lua_cpcall(L, &pmain, &s);
```

7. core/cust_src/elua/shell/src/shell.c line��198
>����lua_main

```
int LuaAppTask(void)
{    
    ...
    return lua_main(argc, argv);
}
```

8. core/cust_src/elua/platform/coolsand/src/platform_main.c line:497
>����LuaAppTask

```
static VOID lua_shell_main(PVOID pParameter)
{
    ...
    luaExitStatus = LuaAppTask();
```


9. core/cust_src/elua/platform/coolsand/src/platform_main.c line:148
>ͨ���ײ�rtos����lua shell��task
```
/* Main function call by OpenAT platform */
VOID cust_main(VOID) {
    ...
    g_LuaShellTaskHandle = IVTBL(create_task)((PTASK_MAIN)lua_shell_main, 
                                                NULL, 
                                                NULL, 
                                                LUA_SHELL_TASK_STACK_SIZE, 
                                                LUA_SHELL_TASK_PRIO, 
                                                OPENAT_OS_CREATE_DEFAULT, 
                                                0, 
                                                "lua shell task");
```
10. core/platform/compliation/cust.ld:line10
>ͨ��ld�ļ�ָ���������
```
ENTRY(cust_main)
```

