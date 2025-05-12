import time
import numpy as np
from smbus2 import SMBus
import mlx90641

# Configuration
I2C_ADDR = 0x33  # Default address for MLX90641
bus = SMBus(1)

# === Helper: Read word-aligned block (16-bit words) ===
def read_word_block(addr, start_reg, num_words):
    raw = bus.read_i2c_block_data(addr, start_reg, num_words * 2)
    return np.array([(raw[i] << 8) | raw[i+1] for i in range(0, len(raw), 2)], dtype=np.uint16)

# === 1. Read EEPROM and extract parameters ===
print("Reading EEPROM...")
ee_data = read_word_block(I2C_ADDR, 0x24, 832)
params = mlx90641.ParamsMLX90641()
err = mlx90641.extract_parameters(ee_data, params)
if err != 0:
    raise RuntimeError(f"Parameter extraction failed with error code {err}")

# === 2. Read frame data ===
print("Reading frame...")
frame_data = read_word_block(I2C_ADDR, 0x04, 242)

# === 3. Calculate temperature ===
print("Calculating temperature...")
temps = mlx90641.calculate_to(frame_data, params, emissivity=0.95, tr=23.0)

# === 4. Display (reshape to 12x16 grid) ===
temp_grid = temps.reshape((12, 16))
print("Temperature Grid (°C):")
print(np.round(temp_grid, 1))
