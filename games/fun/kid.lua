local band,bor,bnot,lshift,rshift= bit.band,bit.bor,bit.bnot,bit.lshift,bit.rshift

local kid=class(sprite)
kid.width=12
kid.height=16

kid.FACE_LEFT=1
kid.FACE_RIGHT=0
kid.STATE_NONE=0
kid.STATE_WALK=1
kid.STATE_JUMP=2
kid.STATE_LAND=4
kid.STATE_BALLOON=8
kid.STATE_SUCK=16
kid.STATE_CHANGE=0x80

local suck=class(sprite)
suck.width=16
suck.height=14
function suck:init(...)
	self.bmp=disp:bmpCreate(asset_kidSpriteSuck_16x16)
end

function suck:update(x,y,face)
	if face==kid.FACE_RIGHT then
		self.x=x+16
	else
		self.x=x-16
	end
	self.y=y+2
	if scene:every_nframe(8) then
		self.frame=(self.frame+1)%2
	end
	self.bmp:update(x-camera.x,y-camera.y,2*face+self.frame)
end

function kid:init(...)
	local args={...}
	local mode=args[3]
	self.state=kid.STATE_NONE
	self.face=kid.FACE_RIGHT
	self.xspeed=0
	self.yspeed=0
	self.xacc=0
	self.yacc=0
	self.bollons=1
	self.coins=0
	self.score=0
	self.key=0
	self.bodybmp=disp:bmpCreate(asset_kidSprite_12x16,mode)
	self.ballonbmp=disp:bmpCreate(asset_balloon)
	self.suck=suck.new()
	self.suck:update(self.x,self.y,self.face)
	self.particle_system=disp:particleSystemCreate({
		bmpdata=asset_particle,
		n=3,
		pattern=u8g2.PARTICLE_PATTERN1,
	})
end

function kid:getframe()
	local index=6*self.face+self.frame
	if self:is(kid.STATE_JUMP) then
		index=6*self.face+4
	end
	if self:is(kid.STATE_LAND) then
		index=6*self.face+5
	end
	return index
end

function kid:is(...)
	if band(self.state,bor(...))>0 then
		return true
	end
	return false
end

function kid:addstate(...)
	self.state=bor(self.state,...)
end

function kid:removestate(sta)
	self.state=band(self.state,bnot(sta))
end

function kid:update()
	if scene:every_nframe(15) then
		if self:is(kid.STATE_WALK) and not self:is(kid.STATE_JUMP) then
	    self.frame=(self.frame+1)%4
		end
	end
	local x=self.x-camera.x
	local y=self.y-(camera.y+camera.yoffset)
	if self:is(kid.STATE_BALLOON) then
		self.ballonbmp:update(x+4,y-9)
		if self.bollons>1 then
			self.ballonbmp:update(x+1,y-11)
		end
		if self.bollons>2 then
			self.ballonbmp:update(x+7,y-12)
		end
	end
	self.bodybmp:update(x,y,self:getframe())
	if self:is(kid.STATE_SUCK) then
		self.suck:update(self.x,self.y,self.face)
		self.sucktime=self.sucktime-1
		if self.sucktime<=0 then
			self.state=kid.STATE_NONE
		end
		if scene:every_nframe(4) then
			if self.face==kid.FACE_LEFT then
				self.particle_system:update(x,y+8,u8g2.PARTICLE_DIR_LEFT)
			else
				self.particle_system:update(x+16,y+8,u8g2.PARTICLE_DIR_RIGHT)
			end
		end
	end
end

return kid
