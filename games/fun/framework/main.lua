require('function')

import('conf')
import('board')
view = import('view')
sprite=import('sprite')

conf.screen_center=math.floor(conf.screen_width/2)
conf.level_row_pixels=conf.row_cells*conf.tile_width
conf.level_col_pixels=conf.col_cells*conf.tile_height
conf.maxcamx=conf.level_row_pixels-conf.screen_width-1
conf.maxcamy=conf.level_col_pixels-conf.screen_height-1

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
