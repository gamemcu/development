local lshift,rshift=bit.lshift,bit.rshift

local game_continue=class(view)

local function state2walk(kid)
	if (kid.state<kid.STATE_WALK) or kid:is(kid.STATE_CHANGE) then
		kid:removestate(kid.STATE_CHANGE)
		kid.state=kid.STATE_WALK
		if not kid.blocked then
			kid.xspeed=1
		end
	end
end

local function checkinput(kid)
	if key:pressed('right') then
		state2walk(kid)
		kid.face=kid.FACE_RIGHT
		if kid.x<conf.level_row_pixels-12 then
			kid.x=kid.x+kid.xspeed
		end
	elseif key:pressed('left') then
		state2walk(kid)
		kid.face=kid.FACE_LEFT
		if kid.x>0 then
				kid.x=kid.x-kid.xspeed
		end
	else
		kid.frame=0
		kid:removestate(kid.STATE_WALK)
	end
	-- if key:pressed('up') then
	-- 	if camera.y+camera.yoffset-camera.yspeed>0 then
	-- 		camera.yoffset=camera.yoffset-camera.yspeed
	-- 	end
	-- elseif key:pressed('down') then
	-- 	if camera.y+camera.yoffset+camera.yspeed<conf.maxcamy then
	-- 		camera.yoffset=camera.yoffset+camera.yspeed
	-- 	end
	-- else
		--camera.yoffset=0
	-- end
	if key:pressed('a') and (kid.yspeed==0) then
		kid.state=kid.STATE_SUCK
		kid.sucktime=120
	end
	if key:pressed('b') then
		if keyb_released then
			keyb_released=false
			if kid.state<kid.STATE_JUMP then
					kid.state=kid.STATE_JUMP
					kid.xspeed=1
					kid.yspeed=env.kid_max_yspeed--1.067--2
			else
				if kid:is(kid.STATE_BALLOON) then
					kid.yspeed=env.kid_max_yspeed/4
				else
					kid:addstate(kid.STATE_LAND,kid.STATE_BALLOON)
					kid.yspeed=0
					kid.xacc=env.airf
				end
			end
		end
	else
		keyb_released=true
	end
end

local function checkstate(kid,level)
	kid.y=kid.y-kid.yspeed
	kid.yspeed=kid.yspeed+kid.yacc-env.gravity
	if kid:is(kid.STATE_JUMP) then
	 	if kid.yspeed<=0 then 
			kid.state=kid.STATE_LAND
		end
	end
	if kid:is(kid.STATE_JUMP,kid.STATE_LAND) then
		if kid.xspeed>0 then
			kid.xspeed=kid.xspeed+kid.xacc-env.airf
		else
			kid.xspeed=0
		end
	end
  local xbody=kid.x+6
 	local ybody=kid.y+8
	local left=level.map:check_obstacle(kid.x-1,ybody,level.tiles,conf)
	local right=level.map:check_obstacle(kid.x+12,ybody,level.tiles,conf)
	local top=level.map:check_obstacle(xbody,kid.y-1,level.tiles,conf)
	local bottom=level.map:check_obstacle(xbody,kid.y+16,level.tiles,conf)
	if (left and (kid.face==kid.FACE_LEFT)) or (right and (kid.face==kid.FACE_RIGHT)) then
		kid.xspeed=0
		kid.blocked=true
	else
		if kid.blocked then
			kid.blocked=false
			kid:addstate(kid.STATE_CHANGE)
		end
	end
	if top and kid:is(kid.STATE_JUMP) then
		kid.state=kid.STATE_LAND
		kid.yspeed=0
	end 
	if bottom then
		kid.xacc=0
		kid.yspeed=0
		if kid:is(kid.STATE_LAND) then
			kid.y=lshift(rshift(kid.y+8,4),4)
			kid.state=kid.STATE_NONE
		end
	else
		if kid:is(kid.STATE_NONE,kid.STATE_WALK) then
			kid.state=kid.STATE_LAND
		end
	end
end

local function update_camera(kid)
	local camx=kid.x-58
	local camy=kid.y-24
	camera.x=camx
	camera.y=camy
end

function game_continue:init()
	self.hud=disp:bmpCreate(asset_elementsHUD_5x8)
	local info=archive('r',{
		nlevel=1,
		bollons=1,
		coins=0,
		score=0,
	})
	self:levelload(info)
end

function game_continue:levelload(info)
	self.level=nil
	collectgarbage()
	local level=import('level').new(info.nlevel)
	self.level=level
	self.map=level.map
	self.kid=level.kid
	self.kid.score=info.score
	self.nlevel=info.nlevel
end

function game_continue:nextlevel()
	self.nlevel=self.nlevel+1
	local info=archive('w',{
		nlevel=self.nlevel,
		bollons=self.kid.bollons,
		coins=self.kid.coins,
		score=self.kid.score,
	})
	scene:setrunning('billboard')
end

function game_continue:showinfo()
	for i=0,self.kid.bollons-1 do
		self.hud:update(i*7,0,10)
	end
	if self.kid.key>0 then
		self.hud:update(28,0,13)
	end
	showdigit(self.hud,7,85,0,self.kid.score,6)
end

function game_continue:update()
	self.map:update(camera.x,camera.y+camera.yoffset)
	self:showinfo()
	if self.level:update() then
		self:nextlevel()
		return
	end
	self.kid:update()
	checkinput(self.kid)
	checkstate(self.kid,self.level)
	update_camera(self.kid)
end

return game_continue
