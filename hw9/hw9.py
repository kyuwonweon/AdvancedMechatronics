import csv
import matplotlib.pyplot as plt
import numpy as np

files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

for file in files:
    t = []
    y = []
    
    with open(file) as f:
        reader = csv.reader(f)
        for row in reader:
            t.append(float(row[0]))
            y.append(float(row[1]))
            
    Fs = len(y) / t[-1]
    n = len(y)
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n / 2))]
    
    Y = np.fft.fft(y) / n
    Y = Y[range(int(n / 2))]
    
    fig, (ax1, ax2) = plt.subplots(2, 1)
    fig.suptitle(f"Signal and FFT for {file}")
    
    ax1.plot(t, y, 'b')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    
    ax2.loglog(frq, abs(Y), 'b')
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    
    plt.tight_layout()
    plt.show()