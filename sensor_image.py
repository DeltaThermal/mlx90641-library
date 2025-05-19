# sensor_image.py
import time
import math
import colorsys
import numpy as np
from PIL import Image
import mlx90641

def grab_frame_with_retry(attempts=5, delay=0.05):
    """Try mlx90641.get_frame() up to `attempts` times, pausing `delay` seconds between."""
    for i in range(attempts):
        try:
            return mlx90641.get_frame()
        except RuntimeError as e:
            print(f"Frame read error (attempt {i+1}/{attempts}): {e}")
            time.sleep(delay)
    raise RuntimeError(f"Failed to read frame after {attempts} attempts")

def main():
    # Initialize the MLX90641 at 16 Hz
    mlx90641.setup(16)
    # Wait for the first valid frame (Tvalid_data ≈ 80ms + 2*(1000/fps))
    time.sleep(0.25)

    # Grab one 12×16 temperature frame, retrying on error
    frame = grab_frame_with_retry(attempts=5, delay=0.05)
    mlx90641.cleanup()

    # Diagnostics
    print(f"Frame shape: {frame.shape}")
    print(f"Min: {np.nanmin(frame):.2f} °C, Max: {np.nanmax(frame):.2f} °C, NaNs: {int(np.isnan(frame).sum())}/{frame.size}")

    # Create a raw 16×12 RGB image
    h, w = frame.shape
    img = Image.new("RGB", (w, h))

    def temp_to_col(val):
        if math.isnan(val):
            return (255, 0, 255)  # magenta for invalid
        hue = (180 - (val * 6)) / 360.0
        r, g, b = colorsys.hsv_to_rgb(hue % 1, 1.0, 1.0)
        return (int(r * 255), int(g * 255), int(b * 255))

    for y in range(h):
        for x in range(w):
            img.putpixel((x, y), temp_to_col(frame[y, x]))

    # Save raw image only
    raw_name = "raw_16x12.png"
    img.save(raw_name)
    print(f"Saved {raw_name}")

if __name__ == "__main__":
    main()
