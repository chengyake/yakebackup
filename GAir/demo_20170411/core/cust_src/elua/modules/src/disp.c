/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    disp.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *          lua.disp��
 **************************************************************************/

#if defined LUA_DISP_LIB

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_disp.h"

int checkFiledInt(lua_State *L, int index, const char *key)
{
    int d;
    lua_getfield(L, index, key);
    d = luaL_checkinteger(L, -1);
    lua_remove(L, -1);
    return d;
}

int getFiledInt(lua_State *L, int index, const char *key)
{
    int d;
    lua_getfield(L, index, key);
    d = lua_tointeger(L, -1);
    lua_remove(L, -1);
    return d;
}

static int optFiledInt(lua_State *L, int index, const char *key, int defval)
{
    int d;
    lua_getfield(L, index, key);
    d = luaL_optint(L, -1, defval);
    lua_remove(L, -1);
    return d;
}

// disp.init
static int disp_init(lua_State *L) {

    PlatformDispInitParam param;
    int cmdTableIndex;

    luaL_checktype(L, 1, LUA_TTABLE);

    memset(&param, 0, sizeof(param));

    param.width = getFiledInt(L, 1, "width");
    param.height = getFiledInt(L, 1, "height");
    
    if(param.width == 0 || param.height == 0)
    {
        return luaL_error(L, "disp.init: error param width(%d) height(%d)", 
                                param.width, param.height);
    }
    
    param.bpp = getFiledInt(L, 1, "bpp");

/*+\NEW\2013.4.10\���Ӻڰ�����ʾ֧�� */
    //16λɫ����or�ڰ���
    if(!(param.bpp == 16 || param.bpp == 1))
    {
        return luaL_error(L, "disp.init: pixel depth must be 16 or 1!"); 
    }
    
    // lcd����ӿ�
    param.bus = getFiledInt(L, 1, "bus");

    /*+\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */
    // ��ͬ����ӿڶ���
    if(param.bus == PLATFORM_LCD_BUS_I2C || param.bus == PLATFORM_LCD_BUS_SPI){
        lua_getfield(L, 1, "interface");
        luaL_checktype(L, -1, LUA_TTABLE);

        if(param.bus == PLATFORM_LCD_BUS_I2C){
            param.lcd_itf.bus_i2c.bus_id = checkFiledInt(L, -1, "bus_id");
            param.lcd_itf.bus_i2c.freq = checkFiledInt(L, -1, "freq");
            param.lcd_itf.bus_i2c.slave_addr = checkFiledInt(L, -1, "slave_addr");
            param.lcd_itf.bus_i2c.cmd_addr = checkFiledInt(L, -1, "cmd_addr");
            param.lcd_itf.bus_i2c.data_addr = checkFiledInt(L, -1, "data_addr");
        } else if(param.bus == PLATFORM_LCD_BUS_SPI){
            param.lcd_itf.bus_spi.bus_id = checkFiledInt(L, -1, "bus_id");
            param.lcd_itf.bus_spi.pin_rs = checkFiledInt(L, -1, "pin_rs");
            param.lcd_itf.bus_spi.pin_cs = optFiledInt(L, -1, "pin_cs", PLATFORM_IO_UNKNOWN_PIN);
            param.lcd_itf.bus_spi.freq = checkFiledInt(L, -1, "freq");
        }
    }
    /*-\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */    

    // lcd rst�ű��붨��
    param.pin_rst = checkFiledInt(L, 1, "pinrst");

    lua_getfield(L, 1, "pincs");

    if(lua_type(L,-1) != LUA_TNUMBER)
        param.pin_cs = PLATFORM_IO_UNKNOWN_PIN;
    else
        param.pin_cs = lua_tonumber(L,-1);

    // ����ƫ����Ĭ��0
    param.x_offset = getFiledInt(L, 1, "xoffset");
    param.y_offset = getFiledInt(L, 1, "yoffset");
    
    /*+\new\liweiqiang\2014.10.21\���Ӳ�ͬ�ڰ������ɫ���� */
    param.hwfillcolor = optFiledInt(L, 1, "hwfillcolor", -1);
    /*-\new\liweiqiang\2014.10.21\���Ӳ�ͬ�ڰ������ɫ���� */

    // .initcmd ��ʼ��ָ���
    lua_getfield(L, 1, "initcmd");
    luaL_checktype(L, -1, LUA_TTABLE);
    param.tableSize = luaL_getn(L, -1);
    param.pLcdCmdTable = malloc(sizeof(int)*param.tableSize);
    
    for(cmdTableIndex = 0; cmdTableIndex < param.tableSize; cmdTableIndex++)
    {
        lua_rawgeti(L, -1, cmdTableIndex+1);
        param.pLcdCmdTable[cmdTableIndex] = lua_tointeger(L, -1);
        lua_remove(L,-1);
    }
/*-\NEW\2013.4.10\���Ӻڰ�����ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */
    lua_getfield(L, 1, "sleepcmd");
    if(lua_type(L, -1) == LUA_TTABLE)
    {
        param.sleepCmdSize = luaL_getn(L, -1);
        param.pLcdSleepCmd = malloc(sizeof(int)*param.sleepCmdSize);

        for(cmdTableIndex = 0; cmdTableIndex < param.sleepCmdSize; cmdTableIndex++)
        {
            lua_rawgeti(L, -1, cmdTableIndex+1);
            param.pLcdSleepCmd[cmdTableIndex] = lua_tointeger(L, -1);
            lua_remove(L,-1);
        }
    }

    lua_getfield(L, 1, "wakecmd");
    if(lua_type(L, -1) == LUA_TTABLE)
    {
        param.wakeCmdSize = luaL_getn(L, -1);
        param.pLcdWakeCmd = malloc(sizeof(int)*param.wakeCmdSize);
        
        for(cmdTableIndex = 0; cmdTableIndex < param.wakeCmdSize; cmdTableIndex++)
        {
            lua_rawgeti(L, -1, cmdTableIndex+1);
            param.pLcdWakeCmd[cmdTableIndex] = lua_tointeger(L, -1);
            lua_remove(L,-1);
        }
    }
/*-\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */

    platform_disp_init(&param);

    free(param.pLcdCmdTable);

    /*+\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */
    if(param.pLcdSleepCmd)
        free(param.pLcdSleepCmd);

    if(param.pLcdWakeCmd)
        free(param.pLcdWakeCmd);
    /*-\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */

    return 0;
}

static int disp_close(lua_State *L) {
    platform_disp_close();
    return 0;
}
// disp.clear
static int disp_clear(lua_State *L) {    
  platform_disp_clear();
  return 0; 
}

static int disp_update(lua_State *L){
    platform_disp_update();
    return 0;
}
   
// disp.puttext
static int disp_puttext(lua_State *L) {
  const char *str;
  u16 x, y;
  
  str   = luaL_checkstring(L, 1);
  x     = cast(u16,luaL_checkinteger(L, 2));
  y     = cast(u16,luaL_checkinteger(L, 3));

  platform_disp_puttext(str, x, y);

  return 0; 
}

/*+\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
//disp.putimage
static int disp_putimage(lua_State *L) {
    const char *filename;
    /*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    u16 x, y, left, top, right, bottom;
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    int transcolor;

    filename   = luaL_checkstring(L, 1);
    x     = luaL_optint(L, 2, 0);
/*+\NEW\liweiqiang\2013.11.12\������ʾͼƬy�����޷����� */
    y     = luaL_optint(L, 3, 0);
/*-\NEW\liweiqiang\2013.11.12\������ʾͼƬy�����޷����� */

/*+\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */
    /*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    transcolor = luaL_optint(L, 4, -1); //Ĭ�ϲ�͸��
    left = luaL_optint(L, 5, 0);
    top = luaL_optint(L, 6, 0);
    right = luaL_optint(L, 7, 0);
    bottom = luaL_optint(L, 8, 0);

    platform_disp_putimage(filename, x, y, transcolor, left, top, right, bottom);
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
/*-\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */
    
    return 0; 
}
/*-\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */
static int disp_drawrect(lua_State *L)
{
    int left = luaL_checkinteger(L, 1);
    int top = luaL_checkinteger(L, 2);
    int right = luaL_checkinteger(L, 3);
    int bottom = luaL_checkinteger(L, 4);
    int color = luaL_optint(L, 5, -1);

    platform_disp_drawrect(left, top, right, bottom, color);

    return 0;
}
/*-\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */
static int disp_setcolor(lua_State *L)
{
    int color = luaL_checkinteger(L, 1);
    int retcolor = platform_disp_setcolor(color);
    lua_pushinteger(L, retcolor);
    return 1;
}

static int disp_setbkcolor(lua_State *L)
{
    int color = luaL_checkinteger(L, 1);
    int retcolor = platform_disp_setbkcolor(color);
    lua_pushinteger(L, retcolor);
    return 1;
}
/*-\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */

/*+\NEW\liweiqiang\2013.12.9\���ӷ������������� */
static int disp_loadfont(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    int fontid = platform_disp_loadfont(filename); 
   
    lua_pushinteger(L, fontid);
    return 1;
}

static int disp_setfont(lua_State *L)
{
    int fontid = luaL_checkinteger(L, 1);
    int oldfontid = platform_disp_setfont(fontid);

    lua_pushinteger(L, oldfontid);
    return 1;
}
/*-\NEW\liweiqiang\2013.12.9\���ӷ������������� */

/*+\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/
extern void platform_lcd_powersave(int sleep_wake);
static int disp_sleep(lua_State *L) {    
    int sleep = luaL_checkinteger(L,1);

    platform_lcd_powersave(sleep);
    return 0; 
}
/*-\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE disp_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( disp_init ) },
  { LSTRKEY( "close" ),  LFUNCVAL( disp_close ) },
  { LSTRKEY( "clear" ), LFUNCVAL( disp_clear ) },
  { LSTRKEY( "update" ), LFUNCVAL( disp_update ) },
  { LSTRKEY( "puttext" ), LFUNCVAL( disp_puttext ) },
/*+\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
  { LSTRKEY( "putimage" ), LFUNCVAL( disp_putimage ) },
/*-\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */
  { LSTRKEY( "drawrect" ), LFUNCVAL( disp_drawrect ) },
/*-\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */
  { LSTRKEY( "setcolor" ), LFUNCVAL( disp_setcolor ) },
  { LSTRKEY( "setbkcolor" ), LFUNCVAL( disp_setbkcolor ) },
/*-\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */

/*+\NEW\liweiqiang\2013.12.9\���ӷ������������� */
  { LSTRKEY( "loadfont" ), LFUNCVAL( disp_loadfont ) },
  { LSTRKEY( "setfont" ), LFUNCVAL( disp_setfont ) },
/*-\NEW\liweiqiang\2013.12.9\���ӷ������������� */

  /*+\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/
  { LSTRKEY( "sleep" ), LFUNCVAL( disp_sleep ) },
  /*-\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/

  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_disp( lua_State *L )
{
  luaL_register( L, AUXLIB_DISP, disp_map );

  MOD_REG_NUMBER(L, "BUS_SPI4LINE", PLATFORM_LCD_BUS_SPI4LINE);
  MOD_REG_NUMBER(L, "BUS_PARALLEL", PLATFORM_LCD_BUS_PARALLEL);
/*+\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */
  MOD_REG_NUMBER(L, "BUS_I2C", PLATFORM_LCD_BUS_I2C);
  MOD_REG_NUMBER(L, "BUS_SPI", PLATFORM_LCD_BUS_SPI);
/*-\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */
  
  return 1;
}  
#endif
