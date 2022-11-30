# importing the required module
import matplotlib.pyplot as plt
import signal
# import numpy as np

def handler(signum, frame):
    pass

signal.signal(signal.SIGINT, handler)

first = True

figure, axes = plt.subplots(figsize=(16,8))
figure.show()

fd = open(r"DATA", "r")  # DATA is as fifo

while True:
    try:
        n = int(fd.readline())
    except:
        break
    
    frame_start = float(fd.readline())
    frame_end = float(fd.readline())
    y = [];
    count = 0

    while count < n:
        count += 1
        y.append(float(fd.readline())) 

    axes.clear()
    axes.set(title="Power spectrum density", xlabel="n=%s" % n)
    axes.set_xlabel("Distance in metres");
    axes.set_ylabel("PSD in db");
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=0, top=+20)
    axes.set_xlim(left=frame_start, right=frame_end)
    
    if first:
        first = False
        x = [frame_start + a * frame_end / n for a in range(n)]
        
    axes.plot(x, y, 'b-')
    figure.canvas.draw()
    figure.canvas.flush_events()

fd.close()
plt.close(figure)
