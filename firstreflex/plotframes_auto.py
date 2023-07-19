# plot all radar scan from an image file.  Keyboard driven.

# importing the required module
import matplotlib.pyplot as plt
import sys
import math
# import numpy as np

def read_data(filename):
    fd = open(filename, "r") 
    z = []
    n_frames = 0

    while n_frames < 1024:
        y = []
        count = 0

        while count < 188:
            value = float(fd.readline())
#            if count < 8:
#                value = 0.0
            y.append(value)
            count += 1
            
        z.append(y)
        n_frames += 1

    fd.close()
    return z


if len(sys.argv) < 2 or len(sys.argv) > 3:
    print("usage:")
    print("       plotframes_auto.py file")
    print("       plotframes_auto.py file start")
    quit()

start = 0
if len(sys.argv) == 3:
    start = int(sys.argv[2]) 

data = read_data(sys.argv[1])

figure, axes = plt.subplots(figsize=(16,8))
figure.show()

while start < 1024:
    axes.clear()
    axes.set(title="Frame data", xlabel="n=188")
    axes.set_xlabel("Position in 5.25 cm increments")
    axes.set_ylabel("Signal in watt")
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=0, top=4)
    axes.set_xlim(left=0, right=188)
    x = [a for a in range(188)]
    axes.plot(x, data[start], 'b-')
    print(start)
    figure.canvas.draw()
    figure.canvas.flush_events()
    start += 1

plt.close(figure)



