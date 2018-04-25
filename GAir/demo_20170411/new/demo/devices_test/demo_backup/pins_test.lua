require"pins"
module(...,package.seeall)

--GPIO
PIN27 = {pin=pio.P0_1,dir=pio.INPUT,valid=1}
PIN26 = {pin=pio.P0_3,dir=pio.OUTPUT1,valid=1}
PIN34 = {pin=pio.P0_4,dir=pio.OUTPUT1,valid=1}
--PIN24 = {pin=pio.P0_5,dir=pio.OUTPUT1,valid=1} wdi used
--PIN5  = {pin=pio.P0_6,dir=pio.OUTPUT1,valid=1} wd reset used
--PIN21 = {pin=pio.P0_8,dir=pio.OUTPUT1,valid=1} failed
PIN20 = {pin=pio.P0_13,dir=pio.OUTPUT1,valid=1}
PIN25 = {pin=pio.P0_14,dir=pio.OUTPUT1,valid=1}
--PIN6  = {pin=pio.P0_15,dir=pio.OUTPUT1,valid=1} net repeat used
PIN16 = {pin=pio.P0_24,dir=pio.OUTPUT1,valid=1} --i2c scl
PIN17 = {pin=pio.P0_25,dir=pio.OUTPUT1,valid=1} --i2c sda

--GPO
PIN23 = {pin=pio.P1_6,dir=pio.OUTPUT1,valid=1}
PIN22 = {pin=pio.P1_8,dir=pio.OUTPUT1,valid=1}
PIN33 = {pin=pio.P1_9,dir=pio.OUTPUT1,valid=1}


--connect pin27 to pin33
pins.reg(PIN27,PIN33)

local flag = 0;
local function switch_level()
    if flag == 0 then
        pins.set(true,PIN33)
        flag = 1
        print("-------1")
    else 
        pins.set(false,PIN33)
        flag = 0
        print("-------0")
    end
    local level = pins.get(PIN27)
    if level then
        print("---high")
    else
        print("---low")
    end

    sys.timer_start(switch_level, 1000, nil)
end


switch_level()
print("-----------------chengyake test success-------------a")
