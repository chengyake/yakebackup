/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    rtos.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/7
 *
 * Description:
 *          lua.rtos��
 **************************************************************************/

#include <ctype.h>
#include <string.h>
#include <malloc.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "auxmods.h"
#include "lrotable.h"
#include "platform.h"
/*begin\NEW\zhutianhua\2017.2.28 14:38\����rtos.set_trace�ӿڣ��ɿ����Ƿ����Lua��trace*/
#include "platform_conf.h"
/*end\NEW\zhutianhua\2017.2.28 14:38\����rtos.set_trace�ӿڣ��ɿ����Ƿ����Lua��trace*/
#include "platform_rtos.h"
#include "platform_malloc.h"

static void setfieldInt(lua_State *L, const char *key, int value)
{
    lua_pushstring(L, key);
    lua_pushinteger(L, value);
    lua_rawset(L, -3);// ����key,value ���õ�table��
}

static void setfieldBool(lua_State *L, const char *key, int value)
{
    if(value < 0) // invalid value
        return;

    lua_pushstring(L, key);
    lua_pushboolean(L, value);
    lua_rawset(L, -3);// ����key,value ���õ�table��
}

static int handle_msg(lua_State *L, PlatformMessage *pMsg)
{    
    int ret = 1;
    
    switch(pMsg->id)
    {
    case RTOS_MSG_WAIT_MSG_TIMEOUT:
        lua_pushinteger(L, pMsg->id);
        // no error msg data.
        break;
        
    case RTOS_MSG_TIMER:
        lua_pushinteger(L, pMsg->id);
        lua_pushinteger(L, pMsg->data.timer_id);
        ret = 2;
        break;

    case RTOS_MSG_UART_RX_DATA:
        lua_pushinteger(L, pMsg->id);
        lua_pushinteger(L, pMsg->data.uart_id);
        ret = 2;
        break;

    case RTOS_MSG_KEYPAD:
        /* ��table��ʽ������Ϣ���� */
        lua_newtable(L);    
        setfieldInt(L, "id", pMsg->id);
        setfieldBool(L, "pressed", pMsg->data.keypadMsgData.bPressed);
        setfieldInt(L, "key_matrix_row", pMsg->data.keypadMsgData.data.matrix.row);
        setfieldInt(L, "key_matrix_col", pMsg->data.keypadMsgData.data.matrix.col);
        break;

/*+\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/
    case RTOS_MSG_INT:
        /* ��table��ʽ������Ϣ���� */
        lua_newtable(L);    
        setfieldInt(L, "id", pMsg->id);
        setfieldInt(L, "int_id", pMsg->data.interruptData.id);
        setfieldInt(L, "int_resnum", pMsg->data.interruptData.resnum);
        break;
/*-\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/

/*+\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
    case RTOS_MSG_PMD:
        /* ��table��ʽ������Ϣ���� */
        lua_newtable(L);    
        setfieldInt(L, "id", pMsg->id);
        setfieldBool(L, "present", pMsg->data.pmdData.battStatus);
        setfieldInt(L, "voltage", pMsg->data.pmdData.battVolt);
        setfieldInt(L, "level", pMsg->data.pmdData.battLevel);
        setfieldBool(L, "charger", pMsg->data.pmdData.chargerStatus);
        setfieldInt(L, "state", pMsg->data.pmdData.chargeState);
        break;
/*-\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/

/*+\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
    case RTOS_MSG_AUDIO:
        /* ��table��ʽ������Ϣ���� */
        lua_newtable(L);    
        setfieldInt(L, "id", pMsg->id);
        if(pMsg->data.audioData.playEndInd == TRUE)
            setfieldBool(L,"play_end_ind",TRUE);
        else if(pMsg->data.audioData.playErrorInd == TRUE)
            setfieldBool(L,"play_error_ind",TRUE);
        break;
/*-\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */

    default:
        ret = 0;
        break;
    }
    
    return ret;
}

static int l_rtos_receive(lua_State *L) 		/* rtos.receive() */
{
    u32 timeout = luaL_checkinteger( L, 1 );
    PlatformMessage *pMsg = NULL;
/*+\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
    static BOOL firstRecv = TRUE;
    int ret = 0;

    if(firstRecv)
    {
        // ��һ�ν�����Ϣʱ�����Ƿ���Ҫ����ϵͳ
        firstRecv = FALSE;
        platform_poweron_try();
    }
/*-\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */

    if(platform_rtos_receive((void**)&pMsg, timeout) != PLATFORM_OK)
    {
        return luaL_error( L, "rtos.receive error!" );
    }
    
    ret = handle_msg(L, pMsg);

    if(pMsg)
    {
    /*+\NEW\liweiqiang\2013.12.6\libc malloc��dlmallocͨ�� */
        // ����Ϣ�ڴ���ƽ̨�������߳������,����platform_free���ͷ�
        platform_free(pMsg);
    /*-\NEW\liweiqiang\2013.12.6\libc malloc��dlmallocͨ�� */
    }

    return ret;
}

static int l_rtos_sleep(lua_State *L)   /* rtos.sleep()*/
{
    int ms = luaL_checkinteger( L, 1 );

    platform_os_sleep(ms);
    
    return 0;
}

static int l_rtos_timer_start(lua_State *L)
{
    int timer_id = luaL_checkinteger(L,1);
    int ms = luaL_checkinteger(L,2);
    int ret;

    ret = platform_rtos_start_timer(timer_id, ms);

    lua_pushinteger(L, ret);

    return 1;
}

static int l_rtos_timer_stop(lua_State *L)
{
    int timer_id = luaL_checkinteger(L,1);
    int ret;

    ret = platform_rtos_stop_timer(timer_id);

    lua_pushinteger(L, ret);

    return 1;
}

static int l_rtos_init_module(lua_State *L)
{
    int module_id = luaL_checkinteger(L, 1);
    int ret;

    switch(module_id)
    {
    case RTOS_MODULE_ID_KEYPAD:
        {
            PlatformKeypadInitParam param;

            int type = luaL_checkinteger(L, 2);
            int inmask = luaL_checkinteger(L, 3);
            int outmask = luaL_checkinteger(L, 4);

            param.type = type;
            param.matrix.inMask = inmask;
            param.matrix.outMask = outmask;

            ret = platform_rtos_init_module(RTOS_MODULE_ID_KEYPAD, &param);
        }
        break;

    default:
        return luaL_error(L, "rtos.init_module: module id must < %d", NumOfRTOSModules);
        break;
    }

    lua_pushinteger(L, ret);

    return 1;
}

/*+\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
static int l_rtos_poweron_reason(lua_State *L)
{
    lua_pushinteger(L, platform_get_poweron_reason());
    return 1;
}

static int l_rtos_poweron(lua_State *L)
{
    int flag = luaL_checkinteger(L, 1);
    platform_rtos_poweron(flag);
    return 0;
}
/*-\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */

static int l_rtos_poweroff(lua_State *L)
{
	platform_rtos_poweroff();	
	return 0;
}

/*-\NEW\zhuwangbin\2017.2.12\��Ӱ汾��ѯ�ӿ� */
static int l_get_version(lua_State *L)
{
	char *ver;

	ver = platform_rtos_get_version();
	lua_pushlstring(L, ver, strlen(ver));
	
	return 1;
}
/*-\NEW\zhuwangbin\2017.2.12\��Ӱ汾��ѯ�ӿ� */

static int l_get_env_usage(lua_State *L)
{
	lua_pushinteger(L,platform_get_env_usage());	
	return 1;
}

/*+\NEW\liweiqiang\2013.9.7\����rtos.restart�ӿ�*/
static int l_rtos_restart(lua_State *L)
{
	platform_rtos_restart();	
	return 0;
}
/*-\NEW\liweiqiang\2013.9.7\����rtos.restart�ӿ�*/

/*+\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/
static int l_rtos_tick(lua_State *L)
{
    lua_pushinteger(L, platform_rtos_tick());
    return 1;
}
/*-\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/

/*begin\NEW\zhutianhua\2017.2.28 14:12\����rtos.set_trace�ӿڣ��ɿ����Ƿ����Lua��trace*/
static int l_set_trace(lua_State *L)
{
    u32 flag = luaL_optinteger(L, 1, 0);
    if(flag==1)
    {
        platform_set_console_port(luaL_optinteger(L, 2, PLATFORM_PORT_ID_DEBUG));
    }
    else
    {
        platform_set_console_port(NUM_UART);
    }
    lua_pushboolean(L,1);
    return 1;
}
/*end\NEW\zhutianhua\2017.2.28 14:12\����rtos.set_trace�ӿڣ��ɿ����Ƿ����Lua��trace*/

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE rtos_map[] =
{
    { LSTRKEY( "init_module" ),  LFUNCVAL( l_rtos_init_module ) },
/*+\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
    { LSTRKEY( "poweron_reason" ),  LFUNCVAL( l_rtos_poweron_reason ) },
    { LSTRKEY( "poweron" ),  LFUNCVAL( l_rtos_poweron ) },
/*-\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
    { LSTRKEY( "poweroff" ),  LFUNCVAL( l_rtos_poweroff ) },
/*+\NEW\liweiqiang\2013.9.7\����rtos.restart�ӿ�*/
    { LSTRKEY( "restart" ),  LFUNCVAL( l_rtos_restart ) },
/*-\NEW\liweiqiang\2013.9.7\����rtos.restart�ӿ�*/
    { LSTRKEY( "receive" ),  LFUNCVAL( l_rtos_receive ) },
    //{ LSTRKEY( "send" ), LFUNCVAL( l_rtos_send ) }, //�ݲ��ṩsend�ӿ�
    { LSTRKEY( "sleep" ), LFUNCVAL( l_rtos_sleep ) },
    { LSTRKEY( "timer_start" ), LFUNCVAL( l_rtos_timer_start ) },
    { LSTRKEY( "timer_stop" ), LFUNCVAL( l_rtos_timer_stop ) },
/*+\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/
    { LSTRKEY( "tick" ), LFUNCVAL( l_rtos_tick ) },
/*-\NEW\liweiqiang\2013.4.5\����rtos.tick�ӿ�*/
    { LSTRKEY( "get_env_usage" ), LFUNCVAL( l_get_env_usage ) },
/*-\NEW\zhuwangbin\2017.2.12\��Ӱ汾��ѯ�ӿ� */
    { LSTRKEY( "get_version" ), LFUNCVAL( l_get_version ) },
/*-\NEW\zhuwangbin\2017.2.12\��Ӱ汾��ѯ�ӿ� */
    /*begin\NEW\zhutianhua\2017.2.28 14:4\����rtos.set_trace�ӿڣ��ɿ����Ƿ����Lua��trace*/
    { LSTRKEY( "set_trace" ), LFUNCVAL( l_set_trace ) },
    /*end\NEW\zhutianhua\2017.2.28 14:4\����rtos.set_trace�ӿڣ��ɿ����Ƿ����Lua��trace*/

	{ LNILKEY, LNILVAL }
};

int luaopen_rtos( lua_State *L )
{
    luaL_register( L, AUXLIB_RTOS, rtos_map );

    // module id
    MOD_REG_NUMBER(L, "MOD_KEYPAD", RTOS_MODULE_ID_KEYPAD);

/*+\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
    // ����ԭ��
    #define REG_POWERON_RESON(rEASON) MOD_REG_NUMBER(L, #rEASON, PLATFORM_##rEASON)
    REG_POWERON_RESON(POWERON_KEY);
    REG_POWERON_RESON(POWERON_CHARGER);
    REG_POWERON_RESON(POWERON_ALARM);
    REG_POWERON_RESON(POWERON_RESTART);
    REG_POWERON_RESON(POWERON_OTHER);
    REG_POWERON_RESON(POWERON_UNKNOWN);
/*-\NEW\liweiqiang\2013.12.12\���ӳ�翪��ʱ���û����о����Ƿ�����ϵͳ */
    /*+\NewReq NEW\zhuth\2014.6.18\���ӿ���ԭ��ֵ�ӿ�*/
    REG_POWERON_RESON(POWERON_EXCEPTION);
    REG_POWERON_RESON(POWERON_HOST);
    REG_POWERON_RESON(POWERON_WATCHDOG);
    /*-\NewReq NEW\zhuth\2014.6.18\���ӿ���ԭ��ֵ�ӿ�*/

    // msg id
    MOD_REG_NUMBER(L, "WAIT_MSG_TIMEOUT", RTOS_MSG_WAIT_MSG_TIMEOUT);
    MOD_REG_NUMBER(L, "MSG_TIMER", RTOS_MSG_TIMER);
    MOD_REG_NUMBER(L, "MSG_KEYPAD", RTOS_MSG_KEYPAD);
    MOD_REG_NUMBER(L, "MSG_UART_RXDATA", RTOS_MSG_UART_RX_DATA);
/*+\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
    MOD_REG_NUMBER(L, "MSG_INT", RTOS_MSG_INT);
/*-\NEW\liweiqiang\2013.4.5\����lua gpio �ж�����*/
/*+\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
    MOD_REG_NUMBER(L, "MSG_PMD", RTOS_MSG_PMD);
/*-\NEW\liweiqiang\2013.7.8\����rtos.pmd��Ϣ*/
/*+\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
    MOD_REG_NUMBER(L, "MSG_AUDIO", RTOS_MSG_AUDIO);
/*-\NEW\liweiqiang\2013.11.4\����audio.core�ӿڿ� */
    //timeout
    MOD_REG_NUMBER(L, "INF_TIMEOUT", PLATFORM_RTOS_WAIT_MSG_INFINITE);

    // ���б�Ҫ�ĳ�ʼ��
    platform_rtos_init();

    return 1;
}

