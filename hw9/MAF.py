import csv
import matplotlib.pyplot as plt
import numpy as np

files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']
x_vals = {'sigA.csv': 5, 'sigB.csv': 10, 'sigC.csv': 5, 'sigD.csv': 10}

fig, axes = plt.subplots(4, 2, figsize=(15, 12))
fig.suptitle("MAF")

for idx, file in enumerate(files):
    x = x_vals[file]
    t = []
    y = []
    
    with open(file) as f:
        reader = csv.reader(f)
        for row in reader:
            t.append(float(row[0]))
            y.append(float(row[1]))
            
    y_filtered = []
    for i in range(len(y)):
        if i < x:
            y_filtered.append(sum(y[0:i+1]) / x)
        else:
            y_filtered.append(sum(y[i-x+1:i+1]) / x)
            
    Fs = len(y) / t[-1]
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
    ax1.set_title(f"{file} -- (Averaged {x} points)")
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