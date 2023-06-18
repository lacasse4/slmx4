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

