#!/usr/bin/env python3
"""
Create a test animated GIF (16x64 pixels) for iPixel display testing.
"""

from PIL import Image, ImageDraw
import os

# Create output directory if it doesn't exist
os.makedirs("test_images", exist_ok=True)

# GIF parameters
WIDTH = 64
HEIGHT = 16
FRAMES = 8
DURATION = 200  # milliseconds per frame

# Create frames
frames = []

for frame_idx in range(FRAMES):
    # Create a new image with black background
    img = Image.new('RGB', (WIDTH, HEIGHT), color='black')
    draw = ImageDraw.Draw(img)
    
    # Draw a moving colored bar
    bar_position = (frame_idx * HEIGHT) // FRAMES
    
    # Draw a vertical bar that moves down
    for y in range(HEIGHT):
        if bar_position <= y < bar_position + 8:
            # Rainbow colors
            hue = (frame_idx * 360 // FRAMES) % 360
            # Simple color cycling: Red -> Green -> Blue -> Red
            if hue < 120:
                r = 255
                g = int(255 * hue / 120)
                b = 0
            elif hue < 240:
                r = int(255 * (240 - hue) / 120)
                g = 255
                b = int(255 * (hue - 120) / 120)
            else:
                r = 0
                g = int(255 * (360 - hue) / 120)
                b = 255
            
            # Draw horizontal line
            draw.line([(0, y), (WIDTH - 1, y)], fill=(r, g, b))
    
    frames.append(img)

# Save as animated GIF
output_path = "test_images/test_animation_16x64.gif"
frames[0].save(
    output_path,
    save_all=True,
    append_images=frames[1:],
    duration=DURATION,
    loop=0,  # Loop forever
    optimize=False
)

print(f"✅ Created animated GIF: {output_path}")
print(f"   Size: {WIDTH}x{HEIGHT} pixels")
print(f"   Frames: {FRAMES}")
print(f"   Duration per frame: {DURATION}ms")
print(f"   File size: {os.path.getsize(output_path)} bytes")

# Also create a simple static PNG for testing
png_img = Image.new('RGB', (WIDTH, HEIGHT), color='black')
png_draw = ImageDraw.Draw(png_img)

# Draw a simple pattern
for x in range(WIDTH):
    for y in range(HEIGHT):
        if (x + y) % 2 == 0:
            png_draw.point((x, y), fill=(255, 0, 0))  # Red
        else:
            png_draw.point((x, y), fill=(0, 255, 0))  # Green

png_path = "test_images/test_image_16x64.png"
png_img.save(png_path)

print(f"✅ Created static PNG: {png_path}")
print(f"   Size: {WIDTH}x{HEIGHT} pixels")
print(f"   File size: {os.path.getsize(png_path)} bytes")

