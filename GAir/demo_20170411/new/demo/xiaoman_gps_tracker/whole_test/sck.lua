--[[
ģ�����ƣ�socket����
ģ�鹦�ܣ�socket���ӣ�Ӧ�����ݴ���
ģ������޸�ʱ�䣺2017.02.16
]]
--����ʱ���Լ��ķ������������޸������PROT��ADDR��PORT 
require"socket"

module(...,package.seeall)

--[[
��������
1����������׼�����������Ӻ�̨
2��ÿ��HEART_RPT_FREQ�뷢��һ��������"heart data\r\n"����̨��
3��ÿ��LOC_RPT_FREQ�룬���һ�´��������Ƿ����𶯣���������𶯣���GPS���GPS_OPEN_MAX�룩��GPS��λ�ɹ����߶�λ��ʱ������һ��λ�ð�����̨
3�����̨���ֳ����ӣ��Ͽ���������ȥ���������ӳɹ���Ȼ���յ�2����������
4���յ���̨������ʱ����rcv�����д�ӡ����


������Ϊ�����ӣ�ֻҪ��������ܹ���⵽�������쳣�������Զ�ȥ�������ӣ�
��ʱ����ּ�ⲻ�����쳣�������������������һ�㰴�����·�ʽ��������һ����������ÿ��Aʱ�䷢��һ�ε���̨����̨�ظ�Ӧ���������n����Aʱ�䶼û���յ���̨���κ����ݣ�����Ϊ������δ֪�������쳣����ʱ����link.shut�����Ͽ���Ȼ���Զ�����
]]

local ssub,schar,smatch,sbyte,slen = string.sub,string.char,string.match,string.byte,string.len
--����ʱ���Լ��ķ�����
local SCK_IDX,PROT,ADDR,PORT = 1,"TCP","120.26.196.195",9999
--λ�ð����������ϱ��������λ��
local LOC_RPT_FREQ,HEART_RPT_FREQ = 20,300
--ÿ�ο���GPS���ʱ��
local GPS_OPEN_MAX = 120
--��һ��λ�ð��ϱ�����ڣ��豸�Ƿ�������
local shkflg
--linksta:���̨��socket����״̬
local linksta
--һ�����������ڵĶ�����������Ӻ�̨ʧ�ܣ��᳢���������������ΪRECONN_PERIOD�룬�������RECONN_MAX_CNT��
--���һ�����������ڶ�û�����ӳɹ�����ȴ�RECONN_CYCLE_PERIOD������·���һ����������
--�������RECONN_CYCLE_MAX_CNT�ε��������ڶ�û�����ӳɹ������������
local RECONN_MAX_CNT,RECONN_PERIOD,RECONN_CYCLE_MAX_CNT,RECONN_CYCLE_PERIOD = 3,5,3,20
--reconncnt:��ǰ���������ڣ��Ѿ������Ĵ���
--reconncyclecnt:�������ٸ��������ڣ���û�����ӳɹ�
--һ�����ӳɹ������Ḵλ���������
--conning:�Ƿ��ڳ�������
local reconncnt,reconncyclecnt,conning = 0,0

--[[
��������print
����  ����ӡ�ӿڣ����ļ��е����д�ӡ�������sckǰ׺
����  ����
����ֵ����
]]
local function print(...)
	_G.print("sck",...)
end

--[[
��������snd
����  �����÷��ͽӿڷ�������
����  ��
        data�����͵����ݣ��ڷ��ͽ���¼�������ntfy�У��ḳֵ��item.data��
		para�����͵Ĳ������ڷ��ͽ���¼�������ntfy�У��ḳֵ��item.para�� 
����ֵ�����÷��ͽӿڵĽ�������������ݷ����Ƿ�ɹ��Ľ�������ݷ����Ƿ�ɹ��Ľ����ntfy�е�SEND�¼���֪ͨ����trueΪ�ɹ�������Ϊʧ��
]]
function snd(data,para)
	return socket.send(SCK_IDX,data,para)
end


--[[
��������opngpscb
����  ������gps���ܡ��Ļص�������
����  ��
		cause����gps����ʱ����ı�ǣ��ڴ˻ص������лش�����
����ֵ����
]]
function opngpscb(cause)
	print("opngpscb",cause)
	local data = "gps fix error\r\n"
	--��λ�ɹ�
	if gps.isfix() then
		--��γ����Ϣ
		data = gps.getgpslocation().."\r\n"
	end
	if not snd(data,"LOCRPT") then
		locrptcb()
	end
end

--[[
��������locrpt
����  ������λ�ð�Ԥ����
����  ����
����ֵ����
]]
function locrpt()
	print("locrpt",linksta,shkflg)
	--�����Ϻ�̨�����豸��������
	if linksta then
		--snd("loc data\r\n","LOCRPT")
		if shkflg then
			--��GPS���ʱ��GPS_OPEN_MAX��
			--��GPS_OPEN_MAX�ڶ�λ�ɹ����߳�ʱ��λʧ�ܣ��������opngpscb�ص�����������Ĳ���Ϊcause��ֵ"LOCRPT"
			gps.open(gps.TIMERORSUC,{cause="LOCRPT",val=GPS_OPEN_MAX,cb=opngpscb})
		else
			locrptcb()
		end
	end
end


--[[
��������locrptcb
����  ��λ�ð����ͻص���������ʱ����LOC_RPT_FREQ���Ӻ��ٴη���λ�ð�
����  ��		
		item��table���ͣ�{data=,para=}����Ϣ�ش��Ĳ��������ݣ��������socket.sendʱ����ĵ�2���͵�3�������ֱ�Ϊdat��par����item={data=dat,para=par}
		result�� bool���ͣ����ͽ����trueΪ�ɹ�������Ϊʧ��
����ֵ����
]]
function locrptcb(item,result)
	print("locrptcb",linksta)
	--����𶯱�־
	shkflg = false
	if linksta then
		sys.timer_start(locrpt,LOC_RPT_FREQ*1000)
	end
end


--[[
��������heartrpt
����  ���������������ݵ���̨
����  ����
����ֵ����
]]
function heartrpt()
	print("heartrpt",linksta)
	if linksta then
		if not snd("heart data\r\n","HEARTRPT")	then
			heartrptcb()
		end
	end
end

--[[
��������heartrptcb
����  �����������ͻص���������ʱ����HEART_RPT_FREQ���Ӻ��ٴη���������
����  ��		
		item��table���ͣ�{data=,para=}����Ϣ�ش��Ĳ��������ݣ��������socket.sendʱ����ĵ�2���͵�3�������ֱ�Ϊdat��par����item={data=dat,para=par}
		result�� bool���ͣ����ͽ����trueΪ�ɹ�������Ϊʧ��
����ֵ����
]]
function heartrptcb(item,result)
	print("heartrptcb",linksta)
	if linksta then
		sys.timer_start(heartrpt,HEART_RPT_FREQ*1000)
	end
end


--[[
��������sndcb
����  �����ݷ��ͽ������
����  ��          
		item��table���ͣ�{data=,para=}����Ϣ�ش��Ĳ��������ݣ��������socket.sendʱ����ĵ�2���͵�3�������ֱ�Ϊdat��par����item={data=dat,para=par}
		result�� bool���ͣ����ͽ����trueΪ�ɹ�������Ϊʧ��
����ֵ����
]]
local function sndcb(item,result)
	print("sndcb",item.para,result)
	if not item.para then return end
	if item.para=="LOCRPT" then
		locrptcb(item,result)
	elseif item.para=="HEARTRPT" then
		heartrptcb(item,result)
	end
end


--[[
��������reconn
����  ��������̨����
        һ�����������ڵĶ�����������Ӻ�̨ʧ�ܣ��᳢���������������ΪRECONN_PERIOD�룬�������RECONN_MAX_CNT��
        ���һ�����������ڶ�û�����ӳɹ�����ȴ�RECONN_CYCLE_PERIOD������·���һ����������
        �������RECONN_CYCLE_MAX_CNT�ε��������ڶ�û�����ӳɹ������������
����  ����
����ֵ����
]]
local function reconn()
	print("reconn",reconncnt,conning,reconncyclecnt)
	--conning��ʾ���ڳ������Ӻ�̨��һ��Ҫ�жϴ˱����������п��ܷ��𲻱�Ҫ������������reconncnt���ӣ�ʵ�ʵ�������������
	if conning then return end
	--һ�����������ڵ�����
	if reconncnt < RECONN_MAX_CNT then		
		reconncnt = reconncnt+1
		link.shut()
		connect()
	--һ���������ڵ�������ʧ��
	else
		reconncnt,reconncyclecnt = 0,reconncyclecnt+1
		if reconncyclecnt >= RECONN_CYCLE_MAX_CNT then
			sys.restart("connect fail")
		end
		sys.timer_start(reconn,RECONN_CYCLE_PERIOD*1000)
	end
end

--[[
��������ntfy
����  ��socket״̬�Ĵ�����
����  ��
        idx��number���ͣ�socket.lua��ά����socket idx��������socket.connectʱ����ĵ�һ��������ͬ��������Ժ��Բ�����
        evt��string���ͣ���Ϣ�¼�����
		result�� bool���ͣ���Ϣ�¼������trueΪ�ɹ�������Ϊʧ��
		item��table���ͣ�{data=,para=}����Ϣ�ش��Ĳ��������ݣ�Ŀǰֻ����SEND���͵��¼����õ��˴˲������������socket.sendʱ����ĵ�2���͵�3�������ֱ�Ϊdat��par����item={data=dat,para=par}
����ֵ����
]]
function ntfy(idx,evt,result,item)
	print("ntfy",evt,result,item)
	--���ӽ��������socket.connect����첽�¼���
	if evt == "CONNECT" then
		conning = false
		--���ӳɹ�
		if result then
			reconncnt,reconncyclecnt,linksta = 0,0,true
			--ֹͣ������ʱ��
			sys.timer_stop(reconn)
			--��������������̨
			heartrpt()
			--����λ�ð�����̨
			locrpt()
		--����ʧ��
		else
			--RECONN_PERIOD�������
			sys.timer_start(reconn,RECONN_PERIOD*1000)
		end	
	--���ݷ��ͽ��������socket.send����첽�¼���
	elseif evt == "SEND" then
		if item then
			sndcb(item,result)
		end
		--����ʧ�ܣ�RECONN_PERIOD���������̨����Ҫ����reconn����ʱsocket״̬��Ȼ��CONNECTED���ᵼ��һֱ�����Ϸ�����
		--if not result then sys.timer_start(reconn,RECONN_PERIOD*1000) end
		if not result then link.shut() end
	--���ӱ����Ͽ�
	elseif evt == "STATE" and result == "CLOSED" then
		linksta = false
		sys.timer_stop(heartrpt)
		sys.timer_stop(locrpt)
		reconn()
	--���������Ͽ�������link.shut����첽�¼���
	elseif evt == "STATE" and result == "SHUTED" then
		linksta = false
		sys.timer_stop(heartrpt)
		sys.timer_stop(locrpt)
		reconn()
	--���������Ͽ�������socket.disconnect����첽�¼���
	elseif evt == "DISCONNECT" then
		linksta = false
		sys.timer_stop(heartrpt)
		sys.timer_stop(locrpt)
		reconn()		
	end
	--�����������Ͽ�������·����������
	if smatch((type(result)=="string") and result or "","ERROR") then
		--RECONN_PERIOD�����������Ҫ����reconn����ʱsocket״̬��Ȼ��CONNECTED���ᵼ��һֱ�����Ϸ�����
		--sys.timer_start(reconn,RECONN_PERIOD*1000)
		link.shut()
	end
end

--[[
��������rcv
����  ��socket�������ݵĴ�����
����  ��
        id ��socket.lua��ά����socket idx��������socket.connectʱ����ĵ�һ��������ͬ��������Ժ��Բ�����
        data�����յ�������
����ֵ����
]]
function rcv(id,data)
	print("rcv",data)
end

--[[
��������connect
����  ����������̨�����������ӣ�
        ������������Ѿ�׼���ã���������Ӻ�̨��������������ᱻ���𣬵���������׼���������Զ�ȥ���Ӻ�̨
		ntfy��socket״̬�Ĵ�����
		rcv��socket�������ݵĴ�����
����  ����
����ֵ����
]]
function connect()
	socket.connect(SCK_IDX,PROT,ADDR,PORT,ntfy,rcv)
	conning = true
end

--[[
��������shkind
����  ��GSENSOR_SHK_IND��Ϣ����������shkflg��־���´�λ�ð��ϱ�ʱ�䵽��ʱ�����shkflg��־
����  ����
����ֵ��true����ʾ����Ӧ�ÿ��Խ��Ŵ���GSENSOR_SHK_IND��Ϣ
]]
local function shkind()
	shkflg = true
	return true
end

--��ģ���ע���ڲ���Ϣ��������
local procer =
{
	GSENSOR_SHK_IND = shkind,
}
--ע����Ϣ��������
sys.regapp(procer)

connect()
