--[[
ģ�����ƣ�gsensor����
ģ�鹦�ܣ�Ŀǰ����������Ƿ�������
ģ������޸�ʱ�䣺2017.02.16
]]

module(...,package.seeall)

--i2c id
--gsensor�����жϵļĴ�����ַ
local i2cid,intregaddr = 1,0x1A

--[[
��������print
����  ����ӡ�ӿڣ����ļ��е����д�ӡ�������gsensorǰ׺
����  ����
����ֵ����
]]
local function print(...)
	_G.print("gsensor",...)
end

--[[
��������clrint
����  �����gsensorоƬ�ڲ��������жϱ�־������gsensor���ܿ�ʼ����´���
����  ����
����ֵ����
]]
local function clrint() 
	if pins.get(pincfg.GSENSOR) then
		i2c.read(i2cid,intregaddr,1)
	end
end

--[[
��������init2
����  ��gsensor�ڶ�����ʼ��
����  ����
����ֵ����
]]
local function init2()
	local cmd,i = {0x1B,0x00,0x6A,0x01,0x1E,0x20,0x21,0x04,0x1B,0x00,0x1B,0xDA,0x1B,0xDA}
	for i=1,#cmd,2 do
		i2c.write(i2cid,cmd[i],cmd[i+1])
		print("init2",string.format("%02X",cmd[i]),string.format("%02X",string.byte(i2c.read(i2cid,cmd[i],1))))
	end
	clrint()
end

--[[
��������checkready
����  ����顰gsensor��һ����ʼ�����Ƿ�ɹ�
����  ����
����ֵ����
]]
local function checkready()
	local s = i2c.read(i2cid,0x1D,1)
	print("checkready",s,(s and s~="") and string.byte(s) or "nil")
	if s and s~="" then
		if bit.band(string.byte(s),0x80)==0 then
			init2()
			return
		end
	end
	sys.timer_start(checkready,1000)
end

--[[
��������init
����  ��gsensor��һ����ʼ��
����  ����
����ֵ����
]]
local function init()
	--gsensor��i2c��ַ
	local i2cslaveaddr = 0x0E
	--��i2c����
	if i2c.setup(i2cid,i2c.SLOW,i2cslaveaddr) ~= i2c.SLOW then
		print("init fail")
		return
	end
	i2c.write(i2cid,0x1D,0x80)
	sys.timer_start(checkready,1000)
end

--[[
��������qryshk
����  ����ѯgsensor�Ƿ�����
����  ����
����ֵ����
]]
local function qryshk()
	--��������
	if pins.get(pincfg.GSENSOR) then
		--������𶯱�־���Ա��ܹ�����´���
		clrint()
		print("GSENSOR_SHK_IND")
		--����һ��GSENSOR_SHK_IND���ڲ���Ϣ����ʾ�豸��������
		sys.dispatch("GSENSOR_SHK_IND")
	end
end

--����һ��10���ѭ����ʱ������ѯ�Ƿ�����
--֮���Բ���ȡ�жϷ�ʽ������ΪƵ����ʱ���ж�̫�ĵ�
sys.timer_loop_start(qryshk,10000)
init()

--��ʱ�ᷢ���쳣����ѯ����û���𶯣�����gsensor�ڲ��ļĴ����Ѿ����������𶯵ı�־
--30�����һ�����𶯱�־��������������쳣
sys.timer_loop_start(clrint,30000)
