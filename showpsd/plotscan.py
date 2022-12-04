# importing the required module
import matplotlib.pyplot as plt
# import numpy as np
n = 0
frame_start = 0
frame_end = 0

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


fr, n, frame_start, frame_end = read_data("DATA-iter64-pps128-frame-141ms")
print("n = " + str(n))
print("frame_start = " + str(frame_start))
print("frame_end = " + str(frame_end))
print("len(fr[0]) = " + str(len(fr[0])))
print("len(fr) = " + str(len(fr)))

figure, axes = plt.subplots(figsize=(16,8))
figure.show()
first = True

for i in range(len(fr[0])):
    y = []
    for j in range(len(fr)):
        y.append(fr[j][i])
        
    axes.clear()
    axes.set(title="Power spectrum density", xlabel="n=%s" % n)
    axes.set_xlabel("Time in 141 ms increment")
    axes.set_ylabel("PSD in db");
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=-0.1, top=0.1)
    axes.set_xlim(left=0, right=len(fr))
    if first:
        first = False
        x = [a for a in range(len(fr))]
    axes.plot(x, y, 'b-')
    figure.canvas.draw()
    figure.canvas.flush_events()

plt.close(figure)


