-- gpio.config({
--   gpio={12,14},
--   dir=gpio.OUT,
--   opendrain=0,
--   pull=gpio.PULL_UP   
-- })

gpio.config({
    gpio={32,33,25,26},
    dir=gpio.IN,
    opendrain=0,
    pull=gpio.INTR_UP   
})

gpio.write(12,0)
gpio.write(14,1)

adc.setup(1,6,adc.ATTEN_11db)
adc.setup(1,7,adc.ATTEN_11db)

key={ pressedstats={} }

function key:check()
  local x=adc.read(1,6)
  local y=adc.read(1,7)
  local k1=gpio.read(32)
  local k2=gpio.read(33)
  local k3=gpio.read(25)
  local k4=gpio.read(26)
  if x>3500 then
    self.pressedstats['right']=true
  end
  if x<3400 then
    self.pressedstats['right']=false
  end
  if x<2300 then
    self.pressedstats['left']=true
  end
  if x>2400 then
    self.pressedstats['left']=false
  end
  if y>3500 then
    self.pressedstats['down']=true
  end
  if y<3400 then
    self.pressedstats['down']=false
  end
  if y<2000 then
    self.pressedstats['up']=true
  end
  if y>2100 then
    self.pressedstats['up']=false
  end
  if k1==0 then
    self.pressedstats['a']=true
  else
    self.pressedstats['a']=false
  end
  if k2==0 then
    self.pressedstats['b']=true
  else
    self.pressedstats['b']=false
  end
end

function key:press(k)
    self.pressedstats[k]=true
end

function key:pressed(k)
  --return self.pressedstats[k]
  if self.pressedstats[k] then
    self.pressedstats[k]=false
    return true
  end
  return false
end
