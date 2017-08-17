
local function init_spi_display()
  -- pins can be assigned freely to available GPIOs
  local sclk = 18
  local mosi = 23
  local cs   = 5
  local dc   = 12--19
  local res  = 14--21
  local bus = spi.master(spi.VSPI, {sclk=sclk, mosi=mosi})
  disp = u8g2.ssd1306_128x64_noname(bus, cs, dc, res)
  -- disp = u8g2.sh1106_128x64_noname(bus, cs, dc, res)
  disp:clearBuffer()
  disp:setBitmapMode(0)
end

init_spi_display()
pcall(require,'assets')
pcall(require,'main')
