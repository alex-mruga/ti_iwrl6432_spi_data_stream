import serial
import binascii
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import threading

# ----- Configuration Parameters -----
SERIAL_PORT = 'COM5'
BAUD_RATE = 115200

DATA_LENGTH = 128         # Number of complex samples per frame
SAMPLE_SIZE = 4           # Each complex sample: 2x int16 (2 bytes each)
HEADER = b'\xAA\xBB\xCC\xDD'   # 4-byte header marker
FOOTER = b'\xDD\xCC\xBB\xAA'   # 4-byte footer marker

# Radar Parameters
BANDWIDTH = 3328e6        # in Hz
C = 3e8                   # Speed of light (m/s)
range_resolution = C / (2 * BANDWIDTH)  # Range resolution (meters per bin)

x_axis_fft = np.arange(DATA_LENGTH) * range_resolution
x_axis_time = np.arange(DATA_LENGTH)

# ----- Serial Port Setup -----
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# Shared variable and lock to hold the latest received frame.
latest_frame = None
frame_lock = threading.Lock()

def read_frame():
    """
    Blocking call to read a frame from Serial.
    If header/footer or size doesn't match, returns None.
    """
    # Wait for header
    while True:
        b = ser.read(1)
        if not b:
            continue
        if b == HEADER[0:1]:
            rest = ser.read(3)
            if b + rest == HEADER:
                # Header received; exit loop.
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

def serial_thread():
    """
    Background thread function that continuously reads frames
    and updates the shared latest_frame variable.
    """
    global latest_frame
    while True:
        frame_data = read_frame()
        if frame_data is not None:
            with frame_lock:
                latest_frame = frame_data

# Start the background serial thread.
thread = threading.Thread(target=serial_thread, daemon=True)
thread.start()

# ----- Set Up Plot -----
fig, (ax_fft, ax_time) = plt.subplots(2, 1, figsize=(8, 8))
plt.tight_layout(pad=3.0)

# FFT Magnitude Plot
line_fft, = ax_fft.plot([], [], lw=2)
ax_fft.set_xlim(0, x_axis_fft[-1])
ax_fft.set_ylim(0, 3500)
ax_fft.set_xlabel("Range (meters)")
ax_fft.set_ylabel("FFT Magnitude")
ax_fft.set_title("Range FFT (Magnitude)")
ax_fft.set_yticklabels([])

# Real and Imaginary Plot
line_real, = ax_time.plot([], [], lw=2, color='blue', label='Real')
line_imag, = ax_time.plot([], [], lw=2, color='red', label='Imag')
ax_time.set_xlim(0, DATA_LENGTH-1)
ax_time.set_ylim(-2000, 2000)
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
    # Copy latest frame from shared variable if available.
    with frame_lock:
        current_frame = latest_frame

    if current_frame is not None:
        # Update FFT plot (absolute value of data)
        fft_magnitude = np.abs(current_frame)
        line_fft.set_data(x_axis_fft, fft_magnitude)

        # Update time domain plot (Real & Imaginary)
        line_real.set_data(x_axis_time, current_frame.real)
        line_imag.set_data(x_axis_time, current_frame.imag)
        
    return line_fft, line_real, line_imag

ani = animation.FuncAnimation(fig, update, init_func=init, blit=True, interval=5)
plt.show()
