

from mylib import *

def harden_times(security, days):
    ret=0
    #unit='%dd' %days
    security_list=[security,]
    high_price=history(days, '1d', 'high', security_list, False)
    limit_price=history(days, '1d', 'high_limit', security_list, False)
    paused_b=history(days, '1d', 'paused', security_list, False)
    for i in range(days):
        if (high_price[security][i] == limit_price[security][i] and not paused_b[security][i]):
            ret+=1
        
    #log.info("%d", ret)
    
