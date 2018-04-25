from yake_utils import *

BEFORE_DAYS=30
VOLUME_DETECT_STANDARD=8.0
VOLUME_HOLD_STANDARD=6.50
VOLUME_SELL_STANDARD=6.50

PRICE_BUY_LINE=0.09  #放量最高价之下N点买入
PRICE_STOP_LOSS=1.0/2  #未盈利前止损
PRICE_STOP_LOSS_1=1.0/2 # 10个点，低盈利后，对最大利益损失止损
PRICE_STOP_LOSS_2=1.00/4 # 20个点，高盈利后，对最大利益损失止损
PRICE_STOP_LOSS_3=1.0/4 # 30个点，高盈利后，对最大利益损失止损
PRICE_STOP_EARNINGS=0.25

#{"code" (current_volume_sum, top_volume_sum, press_price, top_price, five_price, top_returns, stage, limit_days, volume_now, assessment)}

def initialize(context):
    
    
    g.debug = 0
    
    if g.debug == 1:
        enable_profile()
        stocks=['600702.XSHG',]
        #stocks=['600667.XSHG',]
        #stocks=['600111.XSHG',]
        #stocks=['600169.XSHG',]
        #stocks=['600104.XSHG',]
        
    elif g.debug == 2:
        enable_profile()
        s = get_index_stocks('000002.XSHG')
        num=len(s)
        j=random.randint(1,num)
        stocks=[s[j],]
    elif g.debug == 3:
        enable_profile()
        s = get_index_stocks('000002.XSHG')
        j=0
        stocks=s[j:j+100]
        
    elif g.debug == 0:
        #stocks=get_index_stocks('000002.XSHG', '000001.XSHG')
        stocks_list=get_index_stocks('000002.XSHG')
        stocks_list.extend(get_index_stocks('399001.XSHE'))
        
        stocks=[]
        for i in stocks_list:
            if i not in stocks:
                stocks.append(i)
        
        
        
        
    for i in stocks:
        all_before_today[i]=[0,0,0,0,0,0,0,0,0,0]

    set_universe(stocks)
    set_commission(PerTrade(buy_cost=0.0003, sell_cost=0.0013, min_cost=5))
    hold=0
    log.info("All stocks %s number is %d" % (stocks, len(stocks)))
    
    run_daily(update_before_open_func, time='before_open')



def update_before_open_func(context):
    for stock in context.universe:
        volume_price_by_day(context, stock)
        pass
    
def volume_price_by_day(context, stock):
    global all_before_today
    global BEFORE_DAYS, VOLUME_DETECT_STANDARD, VOLUME_HOLD_STANDARD, VOLUME_SELL_STANDARD
    global PRICE_BUY_LINE, PRICE_STOP_LOSS, PRICE_STOP_EARNINGS
    
    
    current_volume_sum=all_before_today[stock][0]
    top_volume_sum=all_before_today[stock][1]
    press_price=all_before_today[stock][2]
    top_price=all_before_today[stock][3]
    five_price=all_before_today[stock][4]
    top_returns=all_before_today[stock][5]
    stage=all_before_today[stock][6]
    limit_days=all_before_today[stock][7]
    volume_now=all_before_today[stock][8]
    assessment=all_before_today[stock][9]
    
    volume_list=history(50, unit='1d', field='volume', security_list=[stock,], df=False)
    open_list=history(5, unit='1d', field='open', security_list=[stock,], df=False)
    close_list=history(6, unit='1d', field='close', security_list=[stock,], df=False)
    price_list=history(5, unit='1d', field='price', security_list=[stock,], df=False)
    
    volume_now=0
    
    avg=get_average(volume_list[stock])
    if avg == 0:
        return
    

    #log.info("--stage %d--" % stage)
    if stage == 0:
        current_volume_sum=1.0000*get_sum(volume_list[stock], open_list[stock], close_list[stock])/avg
        if current_volume_sum > VOLUME_DETECT_STANDARD:
            top_volume_sum=current_volume_sum
            press_price=price_list[stock][-1]
            top_price=press_price
            stage=1

        
    else: #stage > 0:
        if close_list[stock][-1] > open_list[stock][-1]:
            current_volume_sum+=volume_list[stock][-1]/avg
            if current_volume_sum > top_volume_sum:
                top_volume_sum=current_volume_sum
        else:
            current_volume_sum-=volume_list[stock][-1]/avg
        
        if price_list[stock][-1] > top_price:
            top_price=price_list[stock][-1]
        
        #record(tp=top_price, price=price_list[stock][-1])
        #log.info("tp=%f, pp=%f" % (top_price, price_list[stock][-1]))
        
            
        if 1.0000*(top_price - press_price)/press_price > 0.2100: #时间线拉得太长
            set_args(stock, 0)
            return    
        
        if stage == 1:
            limit_days+=1
            if limit_days > 30:
                set_args(stock, 0)
                return
            
            five_price=0
            five_near=get_average(close_list[stock][1:5])
            five_far=get_average(close_list[stock][0:4])
            if(five_far < five_near):
                five_price=1
            #log.info("--%d--" % five_price)    
            
            
        if stage == 2:
            if context.portfolio.positions[stock].amount > 0:
                stage = 3
            else:
                stage = 1
            

        
    if g.debug == 1:
        record(top=top_volume_sum, cur=current_volume_sum)
        pass

    set_args(stock, current_volume_sum, top_volume_sum, press_price, top_price, five_price, top_returns, stage, limit_days, volume_now, assessment)


def get_sum(vlist, olist, clist):
    sum=0
    if clist[-1] > olist[-1]:
        sum+=vlist[-1]
    else:
        sum-=vlist[-1]
    if clist[-2] > olist[-2]:
        sum+=vlist[-2]
    else:
        sum-=vlist[-2]    
    if clist[-3] > olist[-3]:
        sum+=vlist[-3]
    else:
        sum-=vlist[-3]
    if clist[-4] > olist[-4]:
        sum+=vlist[-4]
    else:
        sum-=vlist[-4]
    if clist[-5] > olist[-5]:
        sum+=vlist[-5]
    else:
        sum-=vlist[-5]
    return sum
    
def get_average(list):
    return 1.0000*sum(list)/len(list)


def handle_data(context, data):
    global all_before_today

    for stock in context.universe:
        volume_price_by_min(context, data, stock)
        pass



def volume_price_by_min(context, data, stock):
    global all_before_today, hold
    global BEFORE_DAYS, VOLUME_DETECT_STANDARD, VOLUME_HOLD_STANDARD, VOLUME_SELL_STANDARD
    global PRICE_BUY_LINE, PRICE_STOP_LOSS, PRICE_STOP_LOSS_1, PRICE_STOP_LOSS_2, PRICE_STOP_LOSS_2, PRICE_STOP_EARNINGS
    
    gp=data[stock]
    current_volume_sum=all_before_today[stock][0]
    top_volume_sum=all_before_today[stock][1]
    press_price=all_before_today[stock][2]
    top_price=all_before_today[stock][3]
    five_price=all_before_today[stock][4]
    top_returns=all_before_today[stock][5]
    stage=all_before_today[stock][6]
    limit_days=all_before_today[stock][7]
    volume_now=all_before_today[stock][8]
    assessment=all_before_today[stock][9]

    #avg=get_average(history(50, unit='1d', field='volume', security_list=[stock,], df=False)[stock])
    avg=gp.mavg(50, field='volume')
    avg_3=gp.mavg(3, field='volume')
    

    
    if avg == 0 or stage == 0 or stage == 2:
        return
    
    
    if gp.price > gp.open:
        flag=1
    else:
        flag=-1
    
    if stage == 1:
        if (top_volume_sum - current_volume_sum) > VOLUME_HOLD_STANDARD:  #价格低到目标线前放量跑，则弃
            set_args(stock, 0)
            return
        
        if  five_price > 0 and avg > avg_3 and (top_price - gp.price)/top_price > PRICE_BUY_LINE:
            #if None != order(stock, context.portfolio.cash/gp.price):
            if None != order(stock, 100):
                log.info("Buying_1 %s %d at price:%f" % (stock, context.portfolio.positions[stock].amount, gp.price))
                hold=1
                stage = 2
                
    elif stage == 3 and hold == 1:
        returns=(gp.price - context.portfolio.positions[stock].avg_cost)/context.portfolio.positions[stock].avg_cost
        aclc_returns=(top_price - context.portfolio.positions[stock].avg_cost)/context.portfolio.positions[stock].avg_cost
        #log.info("%f-%f" %(returns, aclc_returns))
        if top_returns < returns:
            top_returns = returns

        
        if (top_volume_sum - current_volume_sum) > VOLUME_SELL_STANDARD:
            if None != order_target(stock, 0):
                log.info("Selling_1 %s at price:%f" % (stock, gp.price))
                set_args(stock, 0)
                return
        if returns <  -PRICE_STOP_LOSS*aclc_returns:
            if None != order_target(stock, 0):
                log.info("Selling_2 %s at price:%f" % (stock, gp.price))
                set_args(stock, 0)
                return
        if top_returns > aclc_returns and (top_returns - returns) >  PRICE_STOP_LOSS_3*aclc_returns:
            if None != order_target(stock, 0):
                log.info("Selling_3 %s at price:%f" % (stock, gp.price))
                set_args(stock, 0)
                return
        if top_returns > aclc_returns/2 and (top_returns - returns) >  PRICE_STOP_LOSS_2*aclc_returns:
            if None != order_target(stock, 0):
                log.info("Selling_4 %s at price:%f" % (stock, gp.price))
                set_args(stock, 0)
                return
        if top_returns > aclc_returns/3 and (top_returns - returns) > PRICE_STOP_LOSS_1*aclc_returns:
            if None != order_target(stock, 0):
                log.info("Selling_5 %s at price:%f" % (stock, gp.price))
                set_args(stock, 0)
                return
    set_args(stock, current_volume_sum, top_volume_sum, press_price, top_price, five_price, top_returns, stage, limit_days, volume_now, assessment)
    


def set_args(stock, current_volume_sum=0, top_volume_sum=0, press_price=0, top_price=0, five_price=0, top_returns=0, stage=0, limit_days=0, volume_now=0, assessment=0):
    all_before_today[stock][0]=current_volume_sum
    all_before_today[stock][1]=top_volume_sum
    all_before_today[stock][2]=press_price
    all_before_today[stock][3]=top_price
    all_before_today[stock][4]=five_price
    all_before_today[stock][5]=top_returns
    all_before_today[stock][6]=stage
    all_before_today[stock][7]=limit_days
    all_before_today[stock][8]=volume_now
    all_before_today[stock][9]=assessment

   




    
    
   
