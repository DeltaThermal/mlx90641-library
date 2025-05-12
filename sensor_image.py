# sensor_image.py
import time
import math
import colorsys
import numpy as np
from PIL import Image
import mlx90641

# Initialize the MLX90641 at 16 Hz and wait for first valid frame
def main():
    mlx90641.setup(16)
    time.sleep(0.25)  # 250 ms delay for Tvalid_data

    # Grab one 12×16 temperature frame (°C)
    frame = mlx90641.get_frame()
    mlx90641.cleanup()

    # Print basic diagnostics
    print(f"Frame shape: {frame.shape}")
    print(f"Min: {np.nanmin(frame):.2f} °C, Max: {np.nanmax(frame):.2f} °C, NaNs: {int(np.isnan(frame).sum())}/{frame.size}")

    # Create a raw 16×12 RGB image
    h, w = frame.shape
    img = Image.new('RGB', (w, h))

    def temp_to_col(val):
        if math.isnan(val):
            return (255, 0, 255)  # magenta for invalid
        hue = (180 - (val * 6)) / 360.0
        r, g, b = colorsys.hsv_to_rgb(hue % 1, 1.0, 1.0)
        return (int(r*255), int(g*255), int(b*255))

    for y in range(h):
        for x in range(w):
            img.putpixel((x, y), temp_to_col(frame[y, x]))

    # Save raw image only
    raw_name = 'raw_16x12.png'
    img.save(raw_name)
    print(f"Saved {raw_name}")

if __name__ == '__main__':
    main()
