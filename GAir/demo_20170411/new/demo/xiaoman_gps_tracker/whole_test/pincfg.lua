--[[
ģ�����ƣ�GPIO
ģ�鹦�ܣ�GPIO���úͲ���
ģ������޸�ʱ�䣺2017.02.16
]]
require"pins"
module(...,package.seeall)

--��ȻGSENSOR�����֧���жϣ������жϻỽ��ϵͳ�����ӹ���
--��������Ϊ���뷽ʽ����gsensor.lua��ȥ��ѯ������״̬
GSENSOR = {pin=pio.P0_3,dir=pio.INPUT,valid=0}
WATCHDOG = {pin=pio.P0_14,init=false,valid=0}
RST_SCMWD = {pin=pio.P0_12,defval=true,valid=1}

pins.reg(GSENSOR,WATCHDOG,RST_SCMWD)

