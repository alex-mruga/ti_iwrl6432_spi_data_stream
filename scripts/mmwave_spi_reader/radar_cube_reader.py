from .spi_ftdi_frame_reader import SpiFtdiFrameReader
import time



class RadarCubeReader:
    def __init__(self,
                num_tx_antennas: int,
                num_rx_antennas: int,
                num_range_bins: int,
                num_chirps_per_frame: int
               ):

        self.num_range_bins     = num_range_bins
        self.num_virt_antennas  = num_tx_antennas * num_rx_antennas
        self.num_doppler_chirps = num_chirps_per_frame / num_tx_antennas
        





prev_time = time.time()

reader = SpiFrameReader(frame_length=6144)

for block in reader:
    current_time = time.time()
    elapsed = current_time - prev_time
    prev_time = current_time

    print(block)

    print(f"frame read - {elapsed:.3f} s since last frame")
