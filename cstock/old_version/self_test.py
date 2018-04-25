from yake_utils import *

BEFORE_DAYS=30
VOLUME_DETECT_STANDARD=6.00
VOLUME_HOLD_STANDARD=4.00
VOLUME_SELL_STANDARD=2.00

PRICE_BUY_LINE=0.10
PRICE_STOP_LOSS=0.05
PRICE_STOP_EARNINGS=0.16

#{"code" (datetime, price, press_price)}
notiy_list={}
hold_list={}



def initialize(context):
    global all_before_today
    stocks=['600702.XSHG',]
    for i in stocks:
        all_before_today[i]=[0,0,0,0,0,0,0]

    set_universe(stocks)



def handle_data(context, data):
    global all_before_today

    for stock in context.universe:
        volume_price_by_min(context, data, stock)
        pass
















    
def volume_price_by_day(context, stock):
    global all_before_today
    global BEFORE_DAYS, VOLUME_DETECT_STANDARD, VOLUME_HOLD_STANDARD, VOLUME_SELL_STANDARD
    global PRICE_BUY_LINE, PRICE_STOP_LOSS, PRICE_STOP_EARNINGS
    
    current_volume_sum=all_before_today[stock][0]
    top_volume_sum=all_before_today[stock][1]
    press_price=all_before_today[stock][2]
    top_price=all_before_today[stock][3]
    stage=all_before_today[stock][4]
    assessment=all_before_today[stock][5]
    
    volume_list=history(50, unit='1d', field='volume', security_list=[stock,], df=False)
    open_list=history(5, unit='1d', field='open', security_list=[stock,], df=False)
    close_list=history(5, unit='1d', field='close', security_list=[stock,], df=False)
    price_list=history(5, unit='1d', field='price', security_list=[stock,], df=False)
    avg=get_average(volume_list[stock])
    
    log.info("%d" % stage)
    #record(v=volume_list[stock][-1])
    if stage == 0:
        current_volume_sum=1.0000*get_sum(volume_list[stock], open_list[stock], close_list[stock])/avg
        #log.info("%f %f %f" % (open_list[stock][-1], avg, current_volume_sum))
        #log.info("%f %f" % (volume_list[stock][-1], get_sum(volume_list[stock], open_list[stock], close_list[stock])))
        if current_volume_sum > VOLUME_DETECT_STANDARD:
            stage=1
            top_volume_sum=current_volume_sum
            press_price=price_list[stock][-1]
            top_price=press_price
            log.info("----enter stage 1----")
    
    
        #record(avg=avg)
    else: #stage > 0:
        if stage == 2:
            stage = 3
        if close_list[stock][-1] > open_list[stock][-1]:
            current_volume_sum+=volume_list[stock][-1]/avg
            top_volume_sum=current_volume_sum
        else:
            current_volume_sum-=volume_list[stock][-1]/avg
        
        if price_list[stock][-1] > top_price:
            top_price=price_list[stock][-1]
        
        
        #if (top_volume_sum - current_volume_sum) > VOLUME_HOLD_STANDARD:
        #    all_before_today[stock]=[0,0,0,0,0,0]
        #    return
    
    record(top=top_volume_sum, cur=current_volume_sum)
    all_before_today[stock][0]=current_volume_sum
    all_before_today[stock][1]=top_volume_sum
    all_before_today[stock][2]=press_price
    all_before_today[stock][3]=top_price
    all_before_today[stock][4]=stage
    all_before_today[stock][5]=assessment
    all_before_today[stock][6]=0


def get_sum(vlist, olist, clist):
    v1,v2,v3,v4,v5=0,0,0,0,0
    if clist[-1] > olist[-1]:
        v1=1
    else:
        v1=-1
    if clist[-2] > olist[-2]:
        v2=1
    else:
        v2=-1    
    if clist[-3] > olist[-3]:
        v3=1
    else:
        v3=-1 
    if clist[-4] > olist[-4]:
        v4=1
    else:
        v3=-1 
    if clist[-5] > olist[-5]:
        v5=1
    else:
        v5=-1
    return vlist[-1]*v1 + vlist[-2]*v2 + vlist[-3]*v3 + vlist[-4]*v4 + vlist[-5]*v5
    
def get_average(list):
    sum=0
    for i in list:
        sum+=i
    
    return 1.0000*sum/len(list)





    
#------------------------min--------------------
def handle_data(context, data):
    global all_before_today

    for stock in context.universe:
        volume_price_by_min(context, data, stock)
        pass



def volume_price_by_min(context, data, stock):
    global all_before_today
    global BEFORE_DAYS, VOLUME_DETECT_STANDARD, VOLUME_HOLD_STANDARD, VOLUME_SELL_STANDARD
    global PRICE_BUY_LINE, PRICE_STOP_LOSS, PRICE_STOP_EARNINGS
    gp=data[stock]
    
    current_volume_sum=all_before_today[stock][0]
    top_volume_sum=all_before_today[stock][1]
    press_price=all_before_today[stock][2]
    top_price=all_before_today[stock][3]
    stage=all_before_today[stock][4]
    assessment=all_before_today[stock][5]
    volume_now=all_before_today[stock][6]
    
    volume_list=history(50, unit='1d', field='volume', security_list=[stock,], df=False)
    avg=get_average(volume_list[stock])
    

    if stage == 0:
        return
    
    volume_now +=gp.volume
    if gp.price > gp.open:
        flag=1
    else:
        flag=-1
    
    
    if stage == 1:
        
        if (top_volume_sum - current_volume_sum + volume_now/avg*flag) > VOLUME_HOLD_STANDARD:
            all_before_today[stock]=[0,0,0,0,0,0]
            return
        if press_price - gp.price > PRICE_BUY_LINE:
            if None == order(stock, context.portfolio.cash/gp.price):
                stage = 2
    
    elif stage == 3:
        if (top_volume_sum - current_volume_sum + volume_now/avg*flag) > VOLUME_SELL_STANDARD:
            if None == order_target(stock, 0):
                log.info("Selling_1 %s at price:%d" % (stock, gp.price))
                all_before_today[stock]={0,0,0,0,0,0,0}
                return
        if (top_price - gp.price)/top_price >  PRICE_STOP_LOSS:
            if None == order_target(stock, 0):
                log.info("Selling_1 %s at price:%d" % (stock, gp.price))
                all_before_today[stock]={0,0,0,0,0,0,0}
                return
        
        
        
    #all_before_today[stock][0]=current_volume_sum
    #all_before_today[stock][1]=top_volume_sum
    #all_before_today[stock][2]=press_price
    #all_before_today[stock][3]=top_price
    all_before_today[stock][4]=stage
    all_before_today[stock][5]=assessment
    all_before_today[stock][6]=volume_now
    



   




    
    
   
