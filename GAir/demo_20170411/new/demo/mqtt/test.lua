require"misc"
require"mqtt"
require"common"
module(...,package.seeall)

local ssub,schar,smatch,sbyte,slen = string.sub,string.char,string.match,string.byte,string.len
--����ʱ���Լ��ķ�����
local PROT,ADDR,PORT = "TCP","lbsmqtt.airm2m.com",1884
local mqttclient


--[[
��������print
����  ����ӡ�ӿڣ����ļ��е����д�ӡ�������testǰ׺
����  ����
����ֵ����
]]
local function print(...)
	_G.print("test",...)
end

local qos0cnt,qos1cnt = 1,1

--[[
��������pubqos0testsndcb
����  ��������1��qosΪ0����Ϣ�����ͽ���Ļص�����
����  ��
		usertag������mqttclient:publishʱ�����usertag
		result��true��ʾ���ͳɹ���false����nil����ʧ��
����ֵ����
]]
local function pubqos0testsndcb(usertag,result)
	print("pubqos0testsndcb",usertag,result)
	sys.timer_start(pubqos0test,10000)
	qos0cnt = qos0cnt+1
end

--[[
��������pubqos0test
����  ������1��qosΪ0����Ϣ
����  ����
����ֵ����
]]
function pubqos0test()
	--ע�⣺�ڴ˴��Լ�ȥ����payload�����ݱ��룬mqtt���в����payload���������κα���ת��
	mqttclient:publish("/qos0topic","qos0data",0,pubqos0testsndcb,"publish0test_"..qos0cnt)
end

--[[
��������pubqos1testackcb
����  ������1��qosΪ1����Ϣ���յ�PUBACK�Ļص�����
����  ��
		usertag������mqttclient:publishʱ�����usertag
		result��true��ʾ�����ɹ���false����nil��ʾʧ��
����ֵ����
]]
local function pubqos1testackcb(usertag,result)
	print("pubqos1testackcb",usertag,result)
	sys.timer_start(pubqos1test,20000)
	qos1cnt = qos1cnt+1
end

--[[
��������pubqos1test
����  ������1��qosΪ1����Ϣ
����  ����
����ֵ����
]]
function pubqos1test()
	--ע�⣺�ڴ˴��Լ�ȥ����payload�����ݱ��룬mqtt���в����payload���������κα���ת��
	mqttclient:publish("/����qos1topic","����qos1data",1,pubqos1testackcb,"publish1test_"..qos1cnt)
end

--[[
��������subackcb
����  ��MQTT SUBSCRIBE֮���յ�SUBACK�Ļص�����
����  ��
		usertag������mqttclient:subscribeʱ�����usertag
		result��true��ʾ���ĳɹ���false����nil��ʾʧ��
����ֵ����
]]
local function subackcb(usertag,result)
	print("subackcb",usertag,result)
end

--[[
��������rcvmessage
����  ���յ�PUBLISH��Ϣʱ�Ļص�����
����  ��
		topic����Ϣ���⣨gb2312���룩
		payload����Ϣ���أ�ԭʼ���룬�յ���payload��ʲô���ݣ�����ʲô���ݣ�û�����κα���ת����
		qos����Ϣ�����ȼ�
����ֵ����
]]
local function rcvmessagecb(topic,payload,qos)
	print("rcvmessagecb",topic,payload,qos)
end

--[[
��������connectedcb
����  ��MQTT CONNECT�ɹ��ص�����
����  ����		
����ֵ����
]]
local function connectedcb()
	print("connectedcb")
	--��������
	mqttclient:subscribe({{topic="/event0",qos=0}, {topic="/����event1",qos=1}}, subackcb, "subscribetest")
	--ע���¼��Ļص�������MESSAGE�¼���ʾ�յ���PUBLISH��Ϣ
	mqttclient:regevtcb({MESSAGE=rcvmessagecb})
	--����һ��qosΪ0����Ϣ
	pubqos0test()
	--����һ��qosΪ1����Ϣ
	pubqos1test()
end

--[[
��������connecterrcb
����  ��MQTT CONNECTʧ�ܻص�����
����  ��
		r��ʧ��ԭ��ֵ
			1��Connection Refused: unacceptable protocol version
			2��Connection Refused: identifier rejected
			3��Connection Refused: server unavailable
			4��Connection Refused: bad user name or password
			5��Connection Refused: not authorized
����ֵ����
]]
local function connecterrcb(r)
	print("connecterrcb",r)
end

--[[
��������imeirdy
����  ��IMEI��ȡ�ɹ����ɹ��󣬲�ȥ����mqtt client�����ӷ���������Ϊ�õ���IMEI��
����  ����		
����ֵ����
]]
local function imeirdy()
	--����һ��mqtt client
	mqttclient = mqtt.create(PROT,ADDR,PORT)
	--������������,�������Ҫ��������һ�д��룬���Ҹ����Լ����������will����
	--mqttclient:configwill(1,0,0,"/willtopic","will payload")
	--����mqtt������
	mqttclient:connect(misc.getimei(),600,"user","password",connectedcb,connecterrcb)
end

local procer =
{
	IMEI_READY = imeirdy,
}
--ע����Ϣ�Ĵ�����
sys.regapp(procer)
