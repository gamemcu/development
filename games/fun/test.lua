gpio.config({
	gpio={12,14},
	dir=gpio.OUT,
	opendrain=0,
	pull=gpio.PULL_UP   
})

gpio.config({
	gpio={25,26,27,32,33},
	dir=gpio.IN,
	opendrain=0,
	pull=gpio.INTR_UP   
})

gpio.write(12,0)
gpio.write(14,1)

adc.setup(1,6,adc.ATTEN_11db)
adc.setup(1,7,adc.ATTEN_11db)

local function adcloop()
	print('x',adc.read(1,6))
	print('y',adc.read(1,7))
	print('k1',gpio.read(25))
	print('k2',gpio.read(26))
	print('k3',gpio.read(27))
	print('k4',gpio.read(32))
	print('k5',gpio.read(33))
	print()
	loop_tmr1:start()
end

loop_tmr1 = tmr.create()
loop_tmr1:register(1000, tmr.ALARM_SEMI, adcloop)
loop_tmr1:start()
