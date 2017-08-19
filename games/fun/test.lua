gpio.config({
    gpio={32,33,26,27},
    dir=gpio.IN,
    opendrain=0,
    pull=gpio.INTR_UP   
})

adc.setup(1,6,adc.ATTEN_11db)
adc.setup(1,7,adc.ATTEN_11db)
ws2812.write({pin = 4, data = string.char(255, 0, 0, 255, 255, 255)})
-- ledc.setup(25,0,0,0,0,1000)
yaw, pitch, roll = 0, 0, 0
local function mpu6050_init()
	i2c.setup(i2c.HW0, 17, 16, i2c.FAST)
	gpio.trig(21, gpio.INTR_UP, function(gpio, level)
		yaw, pitch, roll = mpu6050dmp.trans()
	end)
	mpu6050dmp.setup(i2c.HW0)
	mpu6050dmp.set_xgyro_offset(220)
	mpu6050dmp.set_ygyro_offset(76)
	mpu6050dmp.set_zgyro_offset(-85)
	mpu6050dmp.set_zaccel_offset(1788)
	mpu6050dmp.enable(1)
end

local function testloop()
	print('x',adc.read(1,6))
	print('y',adc.read(1,7))
	print('k1',gpio.read(32))
	print('k2',gpio.read(33))
	print('k3',gpio.read(26))
	print('k4',gpio.read(27))
	print('yaw',yaw,'pitch',pitch,'roll',roll)
	print()
	test_loop_tmr1:start()
end
-- pcall(mpu6050_init)
mpu6050_init()
test_loop_tmr1 = tmr.create()
test_loop_tmr1:register(1000, tmr.ALARM_SEMI, testloop)
test_loop_tmr1:start()
