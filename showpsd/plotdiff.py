# importing the required module
import matplotlib.pyplot as plt
import signal
# import numpy as np

def handler(signum, frame):
    pass

# signal.signal(signal.SIGINT, handler)

first = True
frame_count = 0

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
    axes.set_xlabel("Distance in metres (frame " + str(frame_count) + ")")
    axes.set_ylabel("PSD in db");
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=-0.1, top=0.1)
    axes.set_xlim(left=frame_start, right=frame_end)
    frame_count += 1
    
    if first:
        first = False
        x = [frame_start + a * frame_end / n for a in range(n)]
        
    axes.plot(x, y, 'b-')
    figure.canvas.draw()
    figure.canvas.flush_events()

input('Press enter to continue: ')
fd.close()
plt.close(figure)
