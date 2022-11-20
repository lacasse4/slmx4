# importing the required module
import matplotlib.pyplot as plt
# import numpy as np

figure, axes = plt.subplots(figsize=(16,8))
figure.show()

fd = open(r"DATA", "r")  # DATA is as fifo

while True:
    n = int(fd.readline())
    y = [];
    count = 0

    while count < n:
        count += 1
        y.append(float(fd.readline())) 

    axes.clear()
    axes.set(title="Radar normalized frame", xlabel="n=%s" % n)
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=240, top=270)
    axes.set_xlim(left=0, right=n-1)

    axes.plot(range(n), y, 'b-')
    figure.canvas.draw()
    figure.canvas.flush_events()

fd.close()
plt.pause(2)
plt.close(figure)
