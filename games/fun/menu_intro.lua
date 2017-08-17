local menu_intro=class(view)
menu_intro.lefteye = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 1, 2, 3,
    4, 4, 4, 1,
}

menu_intro.righteye = {
    0, 0, 0, 0,
    0, 0, 0, 1,
    0, 2, 3, 3,
    2, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 1,
    0, 2, 3, 3,
    2, 0, 0, 0,
}

function menu_intro:init( ... )
	self.count=0
  self.sparkleFrames=0
  self.blinkingFrames=0
  self.titlebmp=disp:bmpCreate(asset_titleScreen_32x64)
  self.bmbbmp=disp:bmpCreate(asset_badgeMysticBalloon)
  self.startsbmp=disp:bmpCreate(asset_stars_7x16)
  self.leftGuyLeftEyebmp=disp:bmpCreate(asset_leftGuyLeftEye_5x8)
  self.leftGuyRightEyebmp=disp:bmpCreate(asset_leftGuyRightEye_6x8)
  self.rightGuyEyesbmp=disp:bmpCreate(asset_rightGuyEyes_16x8)
  for i=0,3 do
     self.titlebmp:update(32*i,0,i)
  end
  self.bmbbmp:update(85,45,0)
end

function menu_intro:update()
  if self.count < 320 then
    self.count = self.count+1
    if scene:every_nframe(10) then
      self.sparkleFrames = (self.sparkleFrames+1)%5
      self.startsbmp:update(79,43,self.sparkleFrames)
    end
    if scene:every_nframe(14) then
      self.blinkingFrames = (self.blinkingFrames+1)%32+1
      self.leftGuyLeftEyebmp:update(9,9,self.lefteye[self.blinkingFrames])
      self.leftGuyRightEyebmp:update(15,13,self.righteye[self.blinkingFrames])
     	self.rightGuyEyesbmp:update(109,34,self.righteye[self.blinkingFrames])
    end
  else
    scene:setrunning('menu_main')
  end
end

return menu_intro
