gpio.config({
    gpio={32,33,25,26},
    dir=gpio.IN,
    opendrain=0,
    pull=gpio.INTR_UP   
})

adc.setup(1,6,adc.ATTEN_11db)
adc.setup(1,7,adc.ATTEN_11db)
ws2812.write({pin = 4, data = string.char(255, 0, 0, 255, 255, 255)})

local function testloop()
	print('x',adc.read(1,6))
	print('y',adc.read(1,7))
	print('k1',gpio.read(32))
	print('k2',gpio.read(33))
	print('k3',gpio.read(25))
	print('k4',gpio.read(26))
	print()
	test_loop_tmr1:start()
end

test_loop_tmr1 = tmr.create()
test_loop_tmr1:register(1000, tmr.ALARM_SEMI, testloop)
test_loop_tmr1:start()
