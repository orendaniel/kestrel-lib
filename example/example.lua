--[[ 
A basic example
]]

-- import kestrel with
kestrel = require "kestrel"

-- open camera set resolution to 160x120
-- you can set other properties such as FPS or brightness using v4l2-ctl
cam = kestrel.opendevice("/dev/video0", 160, 120)

for n=1,100 do
	local st = os.clock()

	-- read frame from camera
	local img = cam:readframe()

	-- convert frame to hsv
	local hsv = kestrel.rgb_to_hsv(img)

	-- make a binary thershold image
	local bin = hsv:inrange({0, 0, 40}, {255, 255, 255})

	-- get image contours
	local contours = kestrel.findcontours(bin)


	print("found " .. tostring(#contours) .. " contours")

	for i, cnt in pairs(contours) do
		-- calculate the center and area of each contour
		center = cnt:center()
		area = cnt:area()
		print("#" .. tostring(i), cnt:area(), center.x, center.y)
	end

	print("process time: ", os.clock() - st)
	collectgarbage("collect")
	print("---------------")
end

-- close the camera
cam:close()
