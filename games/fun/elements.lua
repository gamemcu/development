local particle=class(sprite)

local wind=class(sprite)
wind.width=16
function wind:init(...)
	local args={...}
	self.height=args[3]
	self.particle_system=disp:particleSystemCreate({
		bmpdata=asset_particle,
		n=4,
		pattern=u8g2.PARTICLE_PATTERN0,
		height=self.height,
	})
end

function wind:update(x,y)
	self.particle_system:update(x,y)
end

local fan=class(sprite)
fan.width=16
fan.height=16
function fan:init(...)
	local args={...}
	local windheight=args[3]
	self.wind=wind.new(self.x,self.y-windheight,windheight)
end

function fan:update()
	local x=self.x-camera.x
	local y=self.y-camera.y
	if scene:every_nframe(2) then
		self.wind:update(x,y)
	end
	if scene:every_nframe(5) then
		self.frame=(self.frame+1)%3
	end
end

local walker=class(sprite)
walker.width=8
walker.height=8
function walker:init(...)
	self.hp=160
	self.ishurt=false
	self.xspeed=1
	self.dir=1
end

function walker:update(map,kid)
	local hor=map:check_obstacle(self.x+4+self.dir*5,self.y+4)
	local bottom=map:check_obstacle(self.x+4+self.dir*5,self.y+8)
	if not self.ishurt then
		if not hor and bottom then
			self.x=self.x+self.dir*self.xspeed
		else
			self.dir=-self.dir
		end
	else
		self.hp=self.hp-1
	end
	if self.hp<=0 then
		if kid.dir==kid.FACE_LEFT then
			self.x=self.x+self.xspeed
			if self.x>=kid.x-walker.width then
				return
			end
		else
			self.x=self.x-self.xspeed
			if self.x>=kid.x+kid.width then
				return
			end
		end
	end
	if scene:every_nframe(8) then
		self.frame=(self.frame+1)%2
	end
end

local coin=class(sprite)
coin.width=10
coin.height=16
function coin:update()
	if scene:every_nframe(8) then
		self.frame=(self.frame+1)%4
	end
end

local spike=class(sprite)
spike.width=8
spike.height=8
function spike:init( ... )
	local args={...}
	self.n=args[3]
	local map=args[4]
	if map:check_obstacle(self.x,self.y-1) then
		self.frame=3
	elseif map:check_obstacle(self.x,self.y+1) then
		self.frame=2
	elseif map:check_obstacle(self.x-1,self.y) then
		self.frame=0
	else
		self.frame=1
	end
end

local key=class(sprite)
key.width=10
key.height=16

local door=class(sprite)
door.width=16
door.height=16

return {
	wind=wind,
	fan=fan,
	walker=walker,
	coin=coin,
	spike=spike,
	key=key,
	door=door
}
