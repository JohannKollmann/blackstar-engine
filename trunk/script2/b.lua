function init()
	bindlua("sharedfunction", "mybind")
	bindlua("sharedfunction")
	mybind()
	sharedfunction()
end
