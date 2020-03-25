kestrel = require "kestrel"


img = kestrel.read_pixelmap("test_image.ppm")

gray = kestrel.grayscale(img)
sobel = kestrel.sobel(gray)

bin = sobel:inrange({70}, {255})

kestrel.write_pixelmap(gray, "gray.ppm")

kestrel.write_pixelmap(sobel, "sobel.ppm")

kestrel.write_pixelmap(bin, "bin.ppm")

