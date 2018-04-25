
import pandas as pd

def initialize(context):
    set_benchmark('000300.XSHG')

def handle_data(context, data):
    
    stocks=['399001.XSHE', '399006.XSHE', '000300.XSHG', 
            '000908.XSHG', '000909.XSHG', '000910.XSHG', '000911.XSHG', 
            '000912.XSHG', '000913.XSHG', '000914.XSHG', '000915.XSHG', 
            '000916.XSHG', '000917.XSHG', '000918.XSHG', '000919.XSHG', 
            '000920.XSHG', '000921.XSHG', '000950.XSHG', '000951.XSHG', 
            '000952.XSHG', '000957.XSHG', '000968.XSHG', '000969.XSHG', 
            '399007.XSHE', '399008.XSHE', '002230.XSHE']
    
    sdata = get_price('000001.XSHG', start_date='2010-12-02', end_date='2016-12-31 23:00:00', frequency='1d', fields=['open','avg','close','volume'])
    
    for i in stocks:
        ziji = get_price(i, start_date='2010-12-02', end_date='2016-12-31 23:00:00', frequency='1d', fields=['open','avg','close','volume'])
        sdata=pd.concat([sdata, ziji],axis=1)
        
    write_file('chengyk', sdata.to_csv(), append=False) #写到文件中
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    



