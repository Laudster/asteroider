from pillow import Image

alpha = Image.open("monogram-bitmap.png").convert("RGBA").split()[-1]
alpha_pixels = alpha.load()
binary_array = [
	[1 if alpha_pixels[x, y] == 255 else 0 for x in range(alpha.width)]
	for y in range(alpha.height)]

for row in binary_array:
	print("\t{" + ",".join(map(str, row)) + "},")

	https://voxelmanip.se/2025/01/16/drawing-text-in-the-sdl-renderer-without-sdl-ttf/