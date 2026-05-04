import csv
import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import firwin

files_params = {
    'sigA.csv': {'cutoff': 150, 'numtaps': 51, 'window': 'hamming'},
    'sigB.csv': {'cutoff': 30, 'numtaps': 51, 'window': 'hamming'},
    'sigC.csv': {'cutoff': 100, 'numtaps': 21, 'window': 'hamming'},
    'sigD.csv': {'cutoff': 5, 'numtaps': 31, 'window': 'hamming'}
}

fig, axes = plt.subplots(4, 2, figsize=(15, 12))
fig.suptitle("FIR")

for idx, (file, params) in enumerate(files_params.items()):
    t = []
    y = []
    
    with open(file) as f:
        reader = csv.reader(f)
        for row in reader:
            t.append(float(row[0]))
            y.append(float(row[1]))
            
    Fs = len(y) / t[-1]
    
    cutoff = params['cutoff']
    numtaps = params['numtaps']
    window = params['window']
    
    weights = firwin(numtaps, cutoff, window=window, fs=Fs)
    
    y_filtered = []
    for i in range(len(y)):
        val = 0
        for j in range(numtaps):
            if i - j >= 0:
                val += weights[j] * y[i - j]
        y_filtered.append(val)
        
    n = len(y)
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n / 2))]
    
    Y_unfilt = np.fft.fft(y) / n
    Y_unfilt = abs(Y_unfilt[range(int(n / 2))])
    
    Y_filt = np.fft.fft(y_filtered) / n
    Y_filt = abs(Y_filt[range(int(n / 2))])
    
    ax1 = axes[idx, 0]
    ax2 = axes[idx, 1]
    
    ax1.plot(t, y, 'k', label='Unfiltered')
    ax1.plot(t, y_filtered, 'r', label='Filtered')
    ax1.set_title(f"{file}")
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.legend(loc='upper right')
    
    ax2.loglog(frq, Y_unfilt, 'k', label='Unfiltered FFT')
    ax2.loglog(frq, Y_filt, 'r', label='Filtered FFT')
    ax2.set_title(f"{file} FFT")
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.legend(loc='lower left')

plt.tight_layout()
plt.show()