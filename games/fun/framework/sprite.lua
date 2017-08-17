local sprite=class()

function sprite:init(...)
	local args={...}
	self.x=args[1]
	self.y=args[2]
	self.frame=0
end

return sprite
