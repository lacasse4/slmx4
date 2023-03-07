# importing the required module
import matplotlib.pyplot as plt
import sys
# import numpy as np

def read_data(filename):
    fd = open(filename, "r") 
    z = []

    while True:
        try:
            n = int(fd.readline())
        except:
            fd.close()
            return z
    
        frame_start = float(fd.readline())
        frame_end = float(fd.readline())
        y = [];
        count = 0

        while count < n:
            count += 1
            y.append(float(fd.readline()))
            
        z.append(y)

period = 0.141
distance = 9.619342

if len(sys.argv) != 3:
    print("usage: imshow.py file1 file2")
    quit()
    
data1 = read_data(sys.argv[1])
data2 = read_data(sys.argv[2])

plt.style.use('_mpl-gallery-nogrid')
figure, axes = plt.subplots(1, 2, figsize=(16,8), layout='constrained')
axes[0].set_title("Input "  + "(" + sys.argv[1] + ")")
axes[0].set_xlabel("Distance (m)")
axes[0].set_ylabel("Time (s)")
axes[0].imshow(data1, extent=(0, distance, 200*period, 0), aspect = 'auto')

axes[1].set_title("Output " + "(" + sys.argv[2] + ")")
axes[1].set_xlabel("Distance (m)")
axes[1].set_ylabel("Time(s)")
axes[1].imshow(data2, extent=(0, distance, 200*period, 0), aspect = 'auto')

plt.show()
plt.close(figure)

