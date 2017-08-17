local menu_main=class(view)
menu_main.menumain = {
  [1]='menu_help',
  [2]='billboard',
  [3]='menu_info',
  [4]='menu_conf'
}
 
function menu_main:init(...)
  self.menusel=2
  self.title=disp:bmpCreate(asset_titleScreen_32x64)
  self.bmb=disp:bmpCreate(asset_badgeMysticBalloon)
  self.mainMenu=disp:bmpCreate(asset_mainMenu)
  self.selector=disp:bmpCreate(asset_selector)
end

function menu_main:update()
  for i=0,3 do
    self.title:update(32*i,0,i)
  end
  self.bmb:update(85,45,0)
  self.mainMenu:update(51,9,0)
  self.selector:update(46,9*self.menusel,0)
  if scene:every_nframe(10) then
    if key:pressed('up') and (self.menusel > 1) then
      self.menusel = self.menusel-1
    end
    if key:pressed('down') and (self.menusel < 4)  then
      self.menusel = self.menusel+1
    end
  end
  if key:pressed('a') or key:pressed('b') then
    scene:setrunning(self.menumain[self.menusel])
  end
end

return menu_main
