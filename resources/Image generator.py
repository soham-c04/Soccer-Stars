# Re-import required libraries
from PIL import Image

# File paths
draw_img_path = "draw.jpg"
new_green_bg_path = "bk_green.png"

# Load images
draw_img = Image.open(draw_img_path).convert("RGBA")
new_green_bg = Image.open(new_green_bg_path).convert("RGB")

# Resize green background to match draw image
new_green_resized = new_green_bg.resize(draw_img.size)

# Remove existing white/bright background from draw image
datas = draw_img.getdata()
newData = []
for item in datas:
    if item[0] > 200 and item[1] > 200 and item[2] > 200:
        newData.append((255, 255, 255, 0))  # Transparent
    else:
        newData.append(item)
draw_img.putdata(newData)

# Composite draw image over new green background
result_draw_custom_green = Image.alpha_composite(new_green_resized.convert("RGBA"), draw_img)
result_path = "draw_custom.jpg"
result_draw_custom_green.save(result_path)

result_draw_custom_green.show()
