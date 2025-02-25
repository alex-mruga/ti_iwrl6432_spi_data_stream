import serial
import binascii
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# ----- Configuration Parameters -----
SERIAL_PORT = 'COM5'      
BAUD_RATE = 115200        

DATA_LENGTH = 128         # Number of complex samples per frame (one antenna's worth)
SAMPLE_SIZE = 4           # Each complex sample: 2x uint16 (2 bytes each)
HEADER = b'\xAA\xBB\xCC\xDD'   # 4-byte header marker
FOOTER = b'\xDD\xCC\xBB\xAA'   # 4-byte footer marker

# Total frame size in bytes:
# header (4) + payload (DATA_LENGTH * 4) + footer (4)
FRAME_SIZE = 4 + (DATA_LENGTH * SAMPLE_SIZE) + 4

# Radar Parameters
BANDWIDTH = 3328e6        # in Hz
C = 3e8                   # Speed of light (m/s)
range_resolution = C / (2 * BANDWIDTH)  # Range resolution (meters per bin)

x_axis_fft = np.arange(DATA_LENGTH) * range_resolution
x_axis_time = np.arange(DATA_LENGTH)

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

def read_frame():
    """
    Reads frame from Serial. If Header/Footer or size doesn't match requirements discard frame. 
    """
    while True:
        b = ser.read(1)
        if not b:
            continue
        if b == HEADER[0:1]:
            rest = ser.read(3)
            if b + rest == HEADER:
                print("Received Header.Start reading frame data.")
                break

    payload_bytes = ser.read(DATA_LENGTH * SAMPLE_SIZE)
    if len(payload_bytes) != DATA_LENGTH * SAMPLE_SIZE:
        return None
    
    footer_bytes = ser.read(4)
    if footer_bytes != FOOTER:
        return None

    raw = np.frombuffer(payload_bytes, dtype=np.int16)
    if raw.size != DATA_LENGTH * 2:
        return None
    raw = raw.reshape((DATA_LENGTH, 2))
    data_complex = raw[:, 0] + 1j * raw[:, 1]
    return data_complex

# ----- Set Up Plot -----
fig, (ax_fft, ax_time) = plt.subplots(2, 1, figsize=(8, 8))
plt.tight_layout(pad=3.0)

# Plot FFT-Magnitude
line_fft, = ax_fft.plot([], [], lw=2)
ax_fft.set_xlim(0, x_axis_fft[-1])
ax_fft.set_ylim(0, 5000)
ax_fft.set_xlabel("Range (meters)")
ax_fft.set_ylabel("FFT Magnitude")
ax_fft.set_title("Range FFT (Magnitude)")
ax_fft.set_yticklabels([])

# Plot für Real und Imaginary
line_real, = ax_time.plot([], [], lw=2, color='blue', label='Real')
line_imag, = ax_time.plot([], [], lw=2, color='red', label='Imag')
ax_time.set_xlim(0, DATA_LENGTH-1)
ax_time.set_ylim(-5000, 5000)
ax_time.set_xlabel("Range bin index")
ax_time.set_ylabel("Amplitude")
ax_time.set_title("Range FFT (Real & Imaginary)")
ax_time.legend(loc='upper right')
ax_time.set_yticklabels([])


def init():
    line_fft.set_data([], [])
    line_real.set_data([], [])
    line_imag.set_data([], [])
    return line_fft, line_real, line_imag

def update(frame):
    data_complex = read_frame()
    if data_complex is not None:
        fft_result = np.abs(data_complex)
        fft_magnitude = fft_result[:DATA_LENGTH]
        line_fft.set_data(x_axis_fft, fft_magnitude)

        line_real.set_data(x_axis_time, data_complex.real)
        line_imag.set_data(x_axis_time, data_complex.imag)

    return line_fft, line_real, line_imag

ani = animation.FuncAnimation(fig, update, init_func=init, blit=True, interval=10)
plt.show()
