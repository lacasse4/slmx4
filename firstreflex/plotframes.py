# plot all radar scan from an image file.  Keyboard driven.

# importing the required module
import matplotlib.pyplot as plt
import sys
import math
# import numpy as np

def read_data(filename):
    fd = open(filename, "r") 
    z = []

    while True:
        try:
            n = int(fd.readline())
        except:
            fd.close()
            return z, n, frame_start, frame_end
    
        frame_start = float(fd.readline())
        frame_end = float(fd.readline())
        y = [];
        count = 0

        while count < n:
            count += 1
            y.append(float(fd.readline()))
            
        z.append(y)

if len(sys.argv) != 2:
    print("usage: plotradar.py file")
    quit()

fr, n, frame_start, frame_end = read_data(sys.argv[1])
print(n)
print(frame_start)
print(frame_end)
print(len(fr))
print(n)

figure, axes = plt.subplots(figsize=(16,8))
figure.show()
        
for i in range(200):
    axes.clear()
    axes.set(title="Power spectrum density", xlabel="n=%s" % n)
    axes.set_xlabel("Time in 141 ms increment")
    axes.set_ylabel("PSD in db");
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=-40.0, top=0.0)
    axes.set_xlim(left=0, right=n)
    x = [a for a in range(n)]
    axes.plot(x, fr[i], 'b-')
    figure.canvas.draw()
    figure.canvas.flush_events()
    input(str(i))

plt.close(figure)



