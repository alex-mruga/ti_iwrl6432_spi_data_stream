import serial
import binascii
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# ----- Configuration Parameters -----
SERIAL_PORT = 'COM5'      # Update with your actual port (e.g., '/dev/ttyUSB0')
BAUD_RATE = 115200        # Match your MCU configuration

DATA_LENGTH = 64         # Number of complex samples per frame (one antenna's worth)
SAMPLE_SIZE = 4           # Each complex sample: 2x uint16 (2 bytes each)
HEADER = b'\xAA\xBB\xCC\xDD'   # 4-byte header marker
FOOTER = b'\xDD\xCC\xBB\xAA'   # 4-byte footer marker

# Total frame size in bytes:
# header (4) + payload (DATA_LENGTH * 4) + footer (4)
FRAME_SIZE = 4 + (DATA_LENGTH * SAMPLE_SIZE) + 4

# Radar Parameters (adjust as needed)
BANDWIDTH = 1564e6        # Example: 150 MHz bandwidth
C = 3e8                   # Speed of light (m/s)
range_resolution = C / (2 * BANDWIDTH)  # Range resolution (meters per bin)

# Create the x-axis for the FFT plot (only using the positive frequencies)
# Note: We use half the FFT length since FFT output is symmetric.
x_axis = np.arange(DATA_LENGTH // 2) * range_resolution

# ----- Set Up Serial Communication -----
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

def read_frame():
    """
    Searches for a valid frame by looking for the header marker,
    then reads the frame counter, payload, and footer.
    Returns (frame_counter, data_complex) or (None, None) if frame is invalid.
    """
    # Look for the header marker
    while True:
        # Read one byte at a time until we find the first header byte
        b = ser.read(1)
        if not b:
            continue  # no data; keep waiting
        if b == HEADER[0:1]:
            # Read the next three bytes to complete header
            rest = ser.read(3)
            if b + rest == HEADER:
                print("header received, start reading frame data\n")
                break  # header found

    # Read payload (DATA_LENGTH complex samples; each sample is 4 bytes)
    payload_bytes = ser.read(DATA_LENGTH * SAMPLE_SIZE)

    print("Received Data (Hex):\n", binascii.hexlify(payload_bytes, ' ').decode().upper())

    if len(payload_bytes) != DATA_LENGTH * SAMPLE_SIZE:
        return None, None
    

    # Read footer (4 bytes) and verify it
    footer_bytes = ser.read(4)
    if footer_bytes != FOOTER:
        return None, None  # Invalid frame; could also flush buffer or resync
    print("footer received, frame data read\n")

    # Convert payload to an array of int16 values and then to complex numbers
    raw = np.frombuffer(payload_bytes, dtype=np.int16)
    if raw.size != DATA_LENGTH * 2:
        return None, None
    raw = raw.reshape((DATA_LENGTH, 2))
    # Convert to float for FFT processing (you might want to scale or offset these values)
    data_complex = raw[:, 0] + 1j * raw[:, 1]
    return data_complex

# ----- Set Up Plot -----
fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2)
ax.set_xlim(0, x_axis[-1])
ax.set_ylim(-40, 80)  # Adjust according to expected FFT magnitude range
ax.set_xlabel("Range (meters)")
ax.set_ylabel("FFT Magnitude")
ax.set_title("Range FFT Display")

# Add a small text display for the frame counter (placed in the top left)
frame_text = ax.text(0.05, 0.95, '', transform=ax.transAxes, fontsize=10,
                     verticalalignment='top', bbox=dict(facecolor='white', alpha=0.6, edgecolor='none'))

def init():
    line.set_data([], [])
    frame_text.set_text('')
    return line, frame_text

def update(frame):
    # Try to read a full, valid frame
    data_complex = read_frame()
    if data_complex is not None:
        # Compute the FFT on the complex data and take the magnitude
        fft_result = np.abs(data_complex)
        print(np.max(fft_result))
        # Use only the first half (positive frequencies)
        fft_magnitude = fft_result[:DATA_LENGTH // 2]
        fft_mag_db = 20 * np.log10(fft_magnitude)
        line.set_data(x_axis, fft_mag_db)
    else:
        # If no valid frame was read, keep the previous data
        pass
    return line, frame_text

# Create animation with a suitable update interval (ms)
ani = animation.FuncAnimation(fig, update, init_func=init, blit=True, interval=10)

plt.show()