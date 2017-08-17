require('function')

import('conf')
view = import('view')
sprite=import('sprite')

conf.screen_center=math.floor(conf.screen_width/2)
conf.level_row_pixels=conf.row_cells*conf.tile_width
conf.level_col_pixels=conf.col_cells*conf.tile_height
conf.maxcamx=conf.level_row_pixels-conf.screen_width-1
conf.maxcamy=conf.level_col_pixels-conf.screen_height-1

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

key={ pressedstats={} }

function key:check()
  local x=adc.read(1,6)
  local y=adc.read(1,7)
  local k1=gpio.read(25)
  local k2=gpio.read(26)
  local k3=gpio.read(27)
  local k4=gpio.read(32)
  local k5=gpio.read(33)
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

-- scene = { views={},frameCount=0 }
-- setmetatable(scene.views,{__mode='v'})

scene = { frameCount=0 }

function scene:setrunning(name)
  local status,view=xpcall(function()
    return import(name)
  end,function(msg) print(msg) end)
  local t=type(view)
  if status and (t=='table') then
    self.cur=view.new()
  else
    error(string.format('not find view: %s\n',name)) 
  end
  print(node.heap())
  -- collectgarbage()
end

function scene:clear()
  disp:clearBuffer()
end

function scene:loop()
  if self.cur then
    self.cur:update()
  end
  disp:sendBuffer()
  self.frameCount = self.frameCount+1
  key:check()
  if scene:every_nframe(60) then
    -- print(self.frameCount)
  end
  loop_tmr:start()
end

function scene:every_nframe(n)
  return (self.frameCount%n == 0)
end

loop_tmr = tmr.create()
loop_tmr:register(10, tmr.ALARM_SEMI, function()
  scene:loop()
end)
loop_tmr:start()
