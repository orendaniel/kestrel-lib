kestrel = require "kestrel"


cam = kestrel.opendevice("/dev/video0")

img = cam:readframe()

gray = kestrel.grayscale(img)
sobel = kestrel.sobel(gray)

bin = sobel:inrange({70}, {255})

kestrel.write_pixelmap(img, "img.ppm")

kestrel.write_pixelmap(sobel, "sobel.ppm")

kestrel.write_pixelmap(bin, "bin.ppm")
cam:close()
