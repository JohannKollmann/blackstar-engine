function init()
	sharelua("sharedfunction")
	bindc("CShare", "PrintWithC")
	PrintWithC("lua says hi!\n")
	load("b.lua")
	
	bindc("ralloc")
	bindc("rset")
	bindc("rget")
	ralloc("myResi", 13)
end

function sharedfunction()
	PrintWithC(1, 2.3, "bla\n");
end

function increase_resi()
	PrintWithC("increasing the resi from " .. rget("myResi"))
	rset("myResi", rget("myResi")+1.0)
	PrintWithC(" to " .. rget("myResi") .. "!")
end
