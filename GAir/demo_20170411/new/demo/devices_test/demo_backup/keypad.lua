require"pins"
module(...,package.seeall)




local key0_flag = 0

local function check_key0()
    if pins.get(key0) then
        print("----key 0 Event send----")
    end
    key0_flag = 0
end

local function key0cb(v)
    if v and key0_flag == 0 then
        key0_flag = 1
        sys.timer_start(check_key0, 8, nil)
	end
end




key0 = {name="key0",pin=pio.P0_1, dir=pio.INT,valid=1,intcb=key0cb}




pins.reg(key0)
print("----------chengyake test int key----------")





