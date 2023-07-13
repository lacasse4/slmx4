# importing the required module
import matplotlib.pyplot as plt
import sys
# import numpy as np

def read_data(filename):
    fd = open(filename, "r") 

    n = int(fd.readline())
    y = []
    count = 0

    while count < n:
        count += 1
        y.append(float(fd.readline()))
            
    fd.close()
    return y, n

if len(sys.argv) != 2:
    print("usage: plotsinglebreath.py file")
    quit()

while True:
    y, n = read_data(sys.argv[1])

    figure, axes = plt.subplots(figsize=(16,8))
    figure.show()
        
    axes.clear()
    axes.set(title="Power spectrum density", xlabel="n=%s" % n)
    axes.set_xlabel("Time in 141 ms increment")
    axes.set_ylabel("PSD in watt");
    axes.set_autoscale_on(False)
    axes.set_ylim(bottom=0.0, top=20)
    axes.set_xlim(left=0, right=n)
    x = [a for a in range(n)]
    axes.plot(x, y, 'b-')
    plt.show()

plt.close(figure)



