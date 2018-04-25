module(...,package.seeall)

local bit = require "bit"

local i2cid = 1
local BMP280_ADDRESS = 0x77 --Connect SDO to VDDIO
local BMP280_RESET_VALUE = 0xB6

--calibration parameters
local BMP280_DIG_T1_LSB_REG = 0x88
local BMP280_DIG_T1_MSB_REG = 0x89
local BMP280_DIG_T2_LSB_REG = 0x8A
local BMP280_DIG_T2_MSB_REG = 0x8B
local BMP280_DIG_T3_LSB_REG = 0x8C
local BMP280_DIG_T3_MSB_REG = 0x8D
local BMP280_DIG_P1_LSB_REG = 0x8E
local BMP280_DIG_P1_MSB_REG = 0x8F
local BMP280_DIG_P2_LSB_REG = 0x90
local BMP280_DIG_P2_MSB_REG = 0x91
local BMP280_DIG_P3_LSB_REG = 0x92
local BMP280_DIG_P3_MSB_REG = 0x93
local BMP280_DIG_P4_LSB_REG = 0x94
local BMP280_DIG_P4_MSB_REG = 0x95
local BMP280_DIG_P5_LSB_REG = 0x96
local BMP280_DIG_P5_MSB_REG = 0x97
local BMP280_DIG_P6_LSB_REG = 0x98
local BMP280_DIG_P6_MSB_REG = 0x99
local BMP280_DIG_P7_LSB_REG = 0x9A
local BMP280_DIG_P7_MSB_REG = 0x9B
local BMP280_DIG_P8_LSB_REG = 0x9C
local BMP280_DIG_P8_MSB_REG = 0x9D
local BMP280_DIG_P9_LSB_REG = 0x9E
local BMP280_DIG_P9_MSB_REG = 0x9F

local BMP280_CHIPID_REG     = 0xD0  --Chip ID Register 
local BMP280_RESET_REG      = 0xE0  --Softreset Register 
local BMP280_STATUS_REG     = 0xF3  --Status Register 
local BMP280_CTRLMEAS_REG   = 0xF4  --Ctrl Measure Register 
local BMP280_CONFIG_REG     = 0xF5  --Configuration Register 
local BMP280_PRESSURE_MSB_REG             = 0xF7  --Pressure MSB Register 
local BMP280_PRESSURE_LSB_REG             = 0xF8  --Pressure LSB Register 
local BMP280_PRESSURE_XLSB_REG            = 0xF9  --Pressure XLSB Register 
local BMP280_TEMPERATURE_MSB_REG          = 0xFA  --Temperature MSB Reg 
local BMP280_TEMPERATURE_LSB_REG          = 0xFB  --Temperature LSB Reg 
local BMP280_TEMPERATURE_XLSB_REG         = 0xFC  --Temperature XLSB Reg 

local t1,t2,t3,p1,p2,p3,p4,p5,p6,p7,p8,p9



local function print(...)
	_G.print("BMP280: ",...)
end



local function bmp280_reset()
    i2c.write(i2cid, BMP280_RESET_REG, BMP280_RESET_VALUE)
end

local function bmp280_init()
    local lsb = 0
    local msb = 0

    --unsigned short
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_T1_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_T1_MSB_REG , 1))
    t1 =  bit.bor(bit.lshift(msb, 8), lsb)

    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_T2_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_T2_MSB_REG , 1))
    t2 =  bit.bor(bit.lshift(msb, 8), lsb)
    if t2 >= 0x8000 then 
        t2 =  -(0xFFFF-t2)
    end

    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_T3_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_T3_MSB_REG , 1))
    t3 =  bit.bor(bit.lshift(msb, 8), lsb)
    if t3 >= 0x8000 then 
        t3 =  -(0xFFFF-t3)
    end

    --unsigned short
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P1_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P1_MSB_REG , 1))
    p1 =  bit.bor(bit.lshift(msb, 8), lsb)

    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P2_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P2_MSB_REG , 1))
    p2 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p2 >= 0x8000 then 
        p2 =  -(0xFFFF-p2)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P3_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P3_MSB_REG , 1))
    p3 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p3 >= 0x8000 then 
        p3 =  -(0xFFFF-p3)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P4_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P4_MSB_REG , 1))
    p4 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p4 >= 0x8000 then 
        p4 =  -(0xFFFF-p4)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P5_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P5_MSB_REG , 1))
    p5 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p5 >= 0x8000 then 
        p5 =  -(0xFFFF-p5)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P6_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P6_MSB_REG , 1))
    p6 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p6 >= 0x8000 then 
        p6 =  -(0xFFFF-p6)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P7_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P7_MSB_REG , 1))
    p7 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p7 >= 0x8000 then 
        p7 =  -(0xFFFF-p7)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P8_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P8_MSB_REG , 1))
    p8 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p8 >= 0x8000 then 
        p8 =  -(0xFFFF-p8)
    end
    lsb = string.byte(i2c.read(i2cid, BMP280_DIG_P9_LSB_REG , 1))
    msb = string.byte(i2c.read(i2cid, BMP280_DIG_P9_MSB_REG , 1))
    p9 =  bit.bor(bit.lshift(msb, 8), lsb)
    if p9 >= 0x8000 then 
        p9 =  -(0xFFFF-p9)
    end

    bmp280_reset()



    --i2c.write(i2cid, BMP280_CTRLMEAS_REG, 0x24)
    --i2c.write(i2cid, BMP280_CONFIG_REG, 0x04)
    i2c.write(i2cid, BMP280_CTRLMEAS_REG, 0xFF)
    i2c.write(i2cid, BMP280_CONFIG_REG, 0x14)


end

local function bmp280_compensate_temperature()

end


local savedata = 0
local function bmp280_get_temperature()

    local xlsb = string.byte(i2c.read(i2cid, BMP280_TEMPERATURE_XLSB_REG , 1))
    local lsb = string.byte(i2c.read(i2cid, BMP280_TEMPERATURE_LSB_REG , 1))
    local msb = string.byte(i2c.read(i2cid, BMP280_TEMPERATURE_MSB_REG , 1))

    --local adc = bit.bor(bit.bor(bit.lshift(msb, 12), bit.lshift(lsb,4)), bit.rshift(xlsb,4))
    local adc = msb*4096+lsb*16+xlsb/16
    --[[
    local ttt = 14/3
    print("get data xlsb:"..ttt.." adc:"..adc);
    local var1 = (adc/16384-t1/1024)*t2
    local var2 = (adc/131072-t1/8192)*(adc/131072-t1/8192)*t3
    local temp = (var1+var2)/5120
    savedata = var1 + var2
    return temp
    ]]--
    local var1 = (adc*1000/16384-t1*1000/1024)*t2
    local var2 = (adc*1000/131072-t1*1000/8192)*(adc*1000/131072-t1*1000/8192)/1000*t3
    local temp = (var1+var2)/5120
    savedata = var1 + var2
    return temp
end

local function bmp280_get_pressure()

    local xlsb =string.byte(i2c.read(i2cid, BMP280_PRESSURE_XLSB_REG , 1))
    local lsb = string.byte(i2c.read(i2cid, BMP280_PRESSURE_LSB_REG , 1))
    local msb = string.byte(i2c.read(i2cid, BMP280_PRESSURE_MSB_REG , 1))

    --local adc = bit.bor(bit.bor(bit.lshift(msb, 12), bit.lshift(lsb,4)), bit.rshift(xlsb,4))
    local adc = msb*4096+lsb*16+xlsb/16
    --[[
    local var1 = savedata/2 - 64000
    local var2 = var1*var1*p6/32768
    var2 = var2*var1*p5*2
    var2 = var2/4 + p4*65536
    var1 = (p3*var1*var1/524288 + p2*var1)/524288
    
    if var1 == 0 then
        return 0
    end

    press = 1048576 - adc
    press = (press - var2/4096)*6250/var1
    var1 = p9*press*press/2147473648
    var2 = press*p8/32768
    press = press + (var1 + var2 + p7)/16

    return press
    ]]--
    local var1 = savedata/2 - 64000*1000
    local var2 = var1*var1/1000*p6/32768
    var2 = var2*var1/1000*p5*2
    var2 = var2/4 + p4*65536*1000
    var1 = (p3*var1*var1/1000/524288 + p2*var1)/524288
    
    if var1 == 0 then
        return 0
    end

    press = 1048576 - adc
    press = (press*1000*1000 - var2*1000/4096)*6250/var1
    var1 = p9*press*press*1000/2147473648
    var2 = press*p8*1000/32768
    press = press + (var1 + var2 + p7)/16

    return press
end

local function get_pt_data()

    print("get temp  data "..bmp280_get_temperature())
    print("get press data "..bmp280_get_pressure()/256)

    sys.timer_start(get_pt_data,1000,nil)
end

local function init()

	if i2c.setup(i2cid,i2c.SLOW,BMP280_ADDRESS ) ~= i2c.SLOW then
		print("init fail")
		return
	end
    print("get id "..string.format("%02X",string.byte(i2c.read(i2cid, BMP280_CHIPID_REG, 1) or 0xff)))
    bmp280_init()
    print("init success")
end

init()
get_pt_data()


