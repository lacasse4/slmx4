# importing the required module
import matplotlib.pyplot as plt
import sys
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
            if count < 8:
                value = 0.0
            y.append(value)
            count += 1
            
        z.append(y)
        n_frames += 1

    fd.close()
    return z

if len(sys.argv) != 2:
    print("usage: imshow1.py file")
    quit()
    
data = read_data(sys.argv[1])

plt.style.use('_mpl-gallery-nogrid')
figure, axes = plt.subplots(1, 1, figsize=(8,8), layout='constrained')
axes.set_title("File: " + sys.argv[1])
axes.set_xlabel("Distance")
axes.set_ylabel("Time")
axes.imshow(data, aspect = 'auto')

plt.show()
plt.close(figure)

