require"pins"
module(...,package.seeall)

--���������˿�Դģ�������п�����GPIO�����ţ�ÿ������ֻ����ʾ��Ҫ
--�û�����������Լ������������޸�

--pinֵ�������£�
--pio.P0_XX����ʾGPIOXX������pio.P0_15����ʾGPIO15
--pio.P1_XX����ʾGPOXX������pio.P1_8����ʾGPO8

--dirֵ�������£�Ĭ��ֵΪpio.OUTPUT����
--pio.OUTPUT����ʾ�������ʼ��������͵�ƽ
--pio.OUTPUT1����ʾ�������ʼ��������ߵ�ƽ
--pio.INPUT����ʾ���룬��Ҫ��ѯ����ĵ�ƽ״̬
--pio.INT����ʾ�жϣ���ƽ״̬�����仯ʱ���ϱ���Ϣ�����뱾ģ���intmsg����

--validֵ�������£�Ĭ��ֵΪ1����
--valid��ֵ����ģ���е�set��get�ӿ����ʹ��
--dirΪ���ʱ�����set�ӿ�ʹ�ã�set�ĵ�һ���������Ϊtrue��������validֵ��ʾ�ĵ�ƽ��0��ʾ�͵�ƽ��1��ʾ�ߵ�ƽ
--dirΪ������ж�ʱ�����get�ӿ�ʹ�ã�������ŵĵ�ƽ��valid��ֵһ�£�get�ӿڷ���true�����򷵻�false
--dirΪ�ж�ʱ����ϱ�ģ��intmsg�����е�sys.dispatch(string.format("PIN_%s_IND",v.name),v.val)ʹ�ã�������ŵĵ�ƽ��valid��ֵһ�£�v.valΪtrue������v.valΪfalse
--0
--1

--cbΪ�ж����ŵĻص����������жϲ���ʱ�����������cb�������cb����������жϵĵ�ƽ��valid��ֵ��ͬ����cb(true)������cb(false)

--�ȼ���PIN22 = {pin=pio.P1_8,dir=pio.OUTPUT,valid=1}
--��22�����ţ�GPO8������Ϊ�������ʼ������͵�ƽ��valid=1������set(true,PIN22),������ߵ�ƽ������set(false,PIN22),������͵�ƽ
PIN22 = {pin=pio.P1_8}

--��23�����ţ�GPO6������Ϊ�������ʼ������ߵ�ƽ��valid=0������set(true,PIN23),������͵�ƽ������set(false,PIN23),������ߵ�ƽ
PIN23 = {pin=pio.P1_6,dir=pio.OUTPUT1,valid=0}

--�����������ú����PIN22����
PIN25 = {pin=pio.P0_14}
PIN26 = {pin=pio.P0_3}
PIN27 = {pin=pio.P0_1}


local function pin5cb(v)
	print("pin5cb",v)
end
--��5�����ţ�GPIO6������Ϊ�жϣ�valid=1
--intcb��ʾ�жϹܽŵ��жϴ������������ж�ʱ�����Ϊ�ߵ�ƽ����ص�intcb(true)�����Ϊ�͵�ƽ����ص�intcb(false)
--����get(PIN5)ʱ�����Ϊ�ߵ�ƽ���򷵻�true�����Ϊ�͵�ƽ���򷵻�false
PIN5 = {name="PIN5",pin=pio.P0_6,dir=pio.INT,valid=1,intcb=pin5cb}

--��PIN22����
--PIN6 = {pin=pio.P0_15}

--��20�����ţ�GPIO13������Ϊ���룻valid=0
--����get(PIN20)ʱ�����Ϊ�ߵ�ƽ���򷵻�false�����Ϊ�͵�ƽ���򷵻�true
PIN20 = {pin=pio.P0_13,dir=pio.INPUT,valid=0}

--�����������ú����PIN22����
PIN21 = {pin=pio.P0_8}
PIN16 = {pin=pio.P0_24}
PIN17 = {pin=pio.P0_25}

pins.reg(PIN22,PIN23,PIN25,PIN26,PIN27,PIN5,--[[PIN6,]]PIN20,PIN21,PIN16,PIN17)
