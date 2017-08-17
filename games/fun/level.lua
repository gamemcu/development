local kid=import('kid')
local elements=import('elements')
local wind,fan,walker,coin,spike,key,door=elements.wind,elements.fan,elements.walker,elements.coin,elements.spike,elements.key,elements.door
local level=class()

local function collide(a,b)
	return not ((a.x>=b.x+b.width) or 
							(a.x+a.width<=b.x) or 
							(a.y>=b.y+b.height)or			
							(a.y+a.height<=b.y))
end

function level:init(...)
	local args={...}
	local level=dofile('leveldata.lua')[args[1]]
	-- local level=require('leveldata')[args[1]]
	self.tiles=level.tiles
	self.map=disp:mapCreate(asset_tileSet_16x16,level.tiles,{
		row_cells=conf.row_cells,
		col_cells=conf.col_cells
	})
	self.kid=kid.new(level.start[1]*16,level.start[2]*16,u8g2.DRAW_REVERSE)
	self.door=door.new(level.finish[1]*16,level.finish[2]*16)
	self.doorbmp=disp:bmpCreate(asset_door_16x16)
	self.elementsbmp=disp:bmpCreate(asset_elements_10x16)
	if level.walkers then
		self.walkers={}
		self.walkerbmp=disp:bmpCreate(asset_walkerSprite_8x8)
		for _,v in ipairs(level.walkers) do
			self.walkers[#self.walkers+1]=walker.new(v[1]*16,v[2]*16+8)
		end
	end
	if level.fans then
		self.fans={}
		self.fanbmp=disp:bmpCreate(asset_fan_16x16)
		for _,v in ipairs(level.fans) do
			self.fans[#self.fans+1]=fan.new(v[1]*16,v[2]*16,v[3]*16)
		end
	end
	if level.coins then
		self.coins={}
		for _,v in ipairs(level.coins) do
			self.coins[#self.coins+1]=coin.new(v[1]*16,v[2]*16)
		end
	end
	if level.spikes then
		self.spikes={}
		self.spikebmp=disp:bmpCreate(asset_spikes_8x8)
		for _,v in ipairs(level.spikes) do
			self.spikes[#self.spikes+1]=spike.new(v[1]*16,v[2]*16,v[3],self.map)
		end
	end
	if level.key then
		self.key=key.new(level.key[1]*16,level.key[2]*16)
	end
end

function level:update()
	if self.door then
		self.doorbmp:update(self.door.x-camera.x,self.door.y-camera.y,self.kid.key)
		if collide(self.door,self.kid) and self.kid.key>0 then
			return true
		end
	end
	if self.key then
		self.elementsbmp:update(self.key.x-camera.x,self.key.y-camera.y,4)
		if collide(self.key,self.kid) then
			self.key=nil
			self.kid.key=1
		end
	end
	if self.walkers then
		for k,walker in pairs(self.walkers) do
			walker:update(self.map,self.kid)
			self.walkerbmp:update(walker.x-camera.x,walker.y-camera.y,walker.frame)
			if self.kid:is(self.kid.STATE_SUCK) and collide(walker,self.kid.suck) then
				walker.ishurt=true
			else
				walker.ishurt=false
			end
			if walker.hp<=0 then
				self.walkers[k]=nil
				if self.kid.bollons<3 then
					self.kid.bollons=self.kid.bollons+1
				else
					self.kid.score=self.kid.score+100
				end
				self.kid.score=self.kid.score+50
			end
		end
	end
	if self.fans then
		local iscollided=false
		for _,fan in pairs(self.fans) do
			fan:update()
			self.fanbmp:update(fan.x-camera.x,fan.y-camera.y+8,fan.frame)
			if collide(fan.wind,self.kid) then
				self.kid.yacc=env.gravity-0.008
				iscollided=true
			end
		end
		if not iscollided then
			self.kid.yacc=0
		end
	end
	if self.coins then
		for k,coin in pairs(self.coins) do
			coin:update()
			self.elementsbmp:update(coin.x-camera.x,coin.y-camera.y,coin.frame)
			if collide(coin,self.kid) then
				self.coins[k]=nil
				self.kid.score=self.kid.score+50
				self.kid.coins=self.kid.coins+1
				-- self.kid.yacc=self.kid.yacc+0.002
			end
		end
	end
	if self.spikes then
		for _,spike in pairs(self.spikes) do
			for i=0,spike.n-1 do
				if spike.frame<2 then
					self.spikebmp:update(spike.x-camera.x,spike.y+8-camera.y,spike.frame)
				else
					self.spikebmp:update(spike.x+8-camera.x,spike.y-camera.y,spike.frame)
				end
			end
		end
	end
	return false
end

return level

