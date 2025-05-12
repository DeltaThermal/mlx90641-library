import mlx90641, numpy as np

mlx90641.setup(16)

ee = np.array(mlx90641.dump_eeprom())
print("EEPROM sample:", ee[:16])

frm = np.array(mlx90641.dump_frame())
print("Frame sample:", frm[:16])

mlx90641.cleanup()
