module(...,package.seeall)

local i2cid = 1

--[[
��������print
����  ����ӡ�ӿڣ����ļ��е����д�ӡ�������testǰ׺
����  ����
����ֵ����
]]
local function print(...)
	_G.print("test",...)
end

--[[
��������init
����  ����i2c��д��ʼ����������豸�Ĵ��������Ӵ��豸�Ĵ�����ȡֵ
����  ����
����ֵ����
]]
local function init()
	local i2cslaveaddr = 0x0E
	if i2c.setup(i2cid,i2c.SLOW,i2cslaveaddr) ~= i2c.SLOW then
		print("init fail")
		return
	end
	local cmd,i = {0x1B,0x00,0x6A,0x01,0x1E,0x20,0x21,0x04,0x1B,0x00,0x1B,0xDA,0x1B,0xDA}
	for i=1,#cmd,2 do
		i2c.write(i2cid,cmd[i],cmd[i+1])
		print("init",string.format("%02X",cmd[i]),string.format("%02X",string.byte(i2c.read(i2cid,cmd[i],1))))
	end
end

init()
--5���ر�i2c
sys.timer_start(i2c.close,5000,i2cid)
