PROJECT = "SOCKET_SHORT_CONNECTION_FLYMODE"
VERSION = "1.0.0"
require"sys"
--�رսű��е�����trace��ӡ
--sys.opntrace(false)
require"dbg"
dbg.setup("udp","www.test.com",9072)
require"update"
update.setup("udp","www.test.com",2233)
require"test"

net.setled(true)
sys.init(0,0)
--��Ҫץcore�е�traceʱ������������
--ril.request("AT*TRACE=\"SXS\",1,0")
--ril.request("AT*TRACE=\"DSS\",1,0")
--ril.request("AT*TRACE=\"RDA\",1,0")
--���ù���ģʽΪ��ģʽ
sys.setworkmode(sys.SIMPLE_MODE)
sys.run()
