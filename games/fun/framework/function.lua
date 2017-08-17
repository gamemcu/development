function class(...)
	local cls={}
	local supers={...}
	cls.__index=cls
	if #supers==1 then
		setmetatable(cls,{__index=supers[1]})
	elseif #supers>1 then
		setmetatable(cls,{__index=function(t,k)
			for _,super in ipairs(supers) do
				if super[k] then
					t[k]=super[k]
					return super[k]
				end
			end
		end})
	end
	if not cls.init then
		cls.init=function() end
	end
	cls.new=function(...)
		local o={}
		setmetatable(o,cls)
		for _,super in ipairs(supers) do
			super.init(o,...)
		end
		o:init(...)
		return o
	end
	return cls
end

function import(name)
	return dofile(name..'.lua')
end

function serialize(o)
	local s=''
	local tp=type(o)
	if tp == "number" then
		s=s..o
	elseif tp == "string" then
		s=s..string.format("%q", o)
	elseif tp == "table" then
		s=s.."{\n"
	for k,v in pairs(o) do
		s=s.." "..k.." = "
		s=s..serialize(v)
		s=s..",\n"
	end
		s=s.."}\n"
	else
		error("cannot serialize a " .. type(o))
	end
	return s
end

function archive(op,info)
	local info=info or {}
	if op=='r' then
		local r=file.open('archive','r')
		if r then
			local s=file.read()
			local t=loadstring('return'..s)()
			for k,_ in pairs(info) do
				if t[k]==nil then
					op='w'
					break;
				end
			end
			if op ~= 'w' then
				info=t
			end
		else
			op='w'
		end
		file.close()
	end
	if op=='w' then
		file.open('archive','w')
		file.write(serialize(info))
		file.close()
	end
	return info
end

function showdigit(bmp,w,x,y,n,limit)
	x=x+limit*w
	local i=1
	repeat
		bmp:update(x-i*w,y,n%10)
		i=i+1
		n=n/10
	until i>limit
end
