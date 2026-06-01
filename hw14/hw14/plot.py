import sys
import serial
import time
import matplotlib.pyplot as plt
import numpy as np

if len(sys.argv) < 2:
    print("Usage: hx_plot.py <serial port> [baudrate]")
    sys.exit(1)

port = sys.argv[1]
baud = int(sys.argv[2]) if len(sys.argv)>2 else 115200
NUM_LINES = 5*80

def read_lines(ser,n):
    lines = []
    start = time.time()
    while len(lines) < n and (time.time() - start) < 50.0:
        line = ser.readline()
        if not line:
            continue
        try:
            s = line.decode('utf-8', errors='replace').strip()  # was: error=
        except Exception:
            continue
        if s:
            lines.append(s)
    return lines                    # was: indented inside while (only returned 1 line)

with serial.Serial(port, baud, timeout = 10) as ser:
    time.sleep(2)                   # wait for Pico to reboot after DTR reset
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    print("Collection requested...")
    ser.write(f"{NUM_LINES}\n".encode('utf-8'))
    ser.flush()

    raw_lines = read_lines(ser,NUM_LINES)
    print("Collected "+ str(len(raw_lines))+" lines")
    if len(raw_lines) < NUM_LINES:
        print(f"Warning: expected {NUM_LINES} lines, got{len(raw_lines)}")

indices = []
times = []
raw_vals = []   # added
values = []
for ln in raw_lines:
    parts = ln.split()
    if len(parts) < 4:              # was: < 3
        continue
    try:
        idx = int(parts[0])
        t   = float(parts[1])
        rv  = float(parts[2])       # added: raw
        v   = float(parts[3])       # was: parts[2]
    except ValueError:
        continue

    indices.append(idx)
    times.append(t)
    raw_vals.append(rv)             # added
    values.append(v)

if not times:
    print("No valid data parsed.")
    sys.exit(1)

if times:
    base = times[0]
    for i in range(len(times)):
        times[i] -= base
        times[i] = times[i]/1000

plt.figure(figsize=(8,4))
plt.plot(times, raw_vals, label='raw')      # added raw
plt.plot(times, values,   label='filtered')  # added label
plt.xlabel('Time (s)')
plt.ylabel('Value')
plt.title('HX711 values vs time')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

Fs = len(times)/(times[-1]-times[0])
Ts = 1.0/Fs
print("Sample Rate = "+str(Fs))
ts = np.arange(0, times[-1], Ts)    # was: np.arrange
y_raw  = raw_vals
y_filt = values
n = len(y_raw)
k = np.arange(n)
T = n/Fs
frq = k/T
frq = frq[range(int(n/2))]
Y_raw  = np.fft.fft(y_raw) /n      # added raw FFT
Y_filt = np.fft.fft(y_filt)/n
Y_raw  = Y_raw [range(int(n/2))]
Y_filt = Y_filt[range(int(n/2))]

fig, (ax1, ax2) = plt.subplots(2,1)
ax1.plot(times, y_raw,  'r', label='raw')       # added raw
ax1.plot(times, y_filt, 'b', label='filtered')  # added filtered
ax1.legend()
ax1.set_xlabel('Time (s)')                      # was: ax2.set_x_label
ax1.set_ylabel('Amplitude')                     # was: ax1.set_y_label
ax2.loglog(frq, abs(Y_raw),  'r', label='raw')  # added raw FFT
ax2.loglog(frq, abs(Y_filt), 'b', label='filtered')
ax2.legend()
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')                     # was: ax2.set_y_label
plt.show()
