local billboard=class(view)

local info=archive('r',{
	nlevel=1,
	bollons=1,
	coins=0,
	score=0,
})

local badgeNextLevel=disp:bmpCreate(asset_badgeNextLevel)
local badgeLevel=disp:bmpCreate(asset_badgeLevel)
local bigfont=disp:bmpCreate(asset_numbersBig_7x8,u8g2.DRAW_TRANSPARENCY)
local fontwidth=7
local bigfontmask=disp:bmpCreate(asset_numbersBigMask_2x16)
local bigfontmask1=disp:bmpCreate(asset_numbersBigMask01)
local badgeElements=disp:bmpCreate(asset_badgeElements_12x16)
local badgePressKey=disp:bmpCreate(asset_badgePressKey)
local pk_visible=false

function billboard:update()
	scene:clear()
  badgeNextLevel:update(35,4,0)
  badgeLevel:update(76, 4)
  showdigit(bigfont,fontwidth,78,13,info.nlevel,2)
  if scene:every_nframe(30) then
  	if info.coins>0 then
  		info.coins=info.coins-1
  	end
  	if info.bollons>0 then
  		info.bollons=info.bollons-1
  	end
  	pk_visible=not pk_visible
  end	
  local total=info.bollons+info.coins
  if total>0 then
	  for i=0,total-1 do
	  	if i<info.coins then
				badgeElements:update(65-7*total+i*14,27)
			else
				badgeElements:update(65-7*total+i*14,27,1)
			end
		end
	else
		if pk_visible then
			badgePressKey:update(38,29)
		end
	end
	bigfontmask:update(42,47,0)
	for i=0,5 do
		bigfontmask1:update(43+i*7,47)
	end
	bigfontmask:update(42+42,47,1)
	showdigit(bigfont,fontwidth,43,49,info.score,6)
  
  if key:pressed('b') then
    scene:setrunning('game_continue')
  end
end

return billboard
