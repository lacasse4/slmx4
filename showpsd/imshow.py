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

if len(sys.argv) != 3:
    print("usage: imshow.py file1 file2")
    quit()
    
data1 = read_data(sys.argv[1])
data2 = read_data(sys.argv[2])

plt.style.use('_mpl-gallery-nogrid')
figure, axes = plt.subplots(1, 2, figsize=(16,8))
axes[0].imshow(data1)
axes[0].set(title=sys.argv[1])
axes[1].imshow(data2)
axes[1].set(title=sys.argv[2])

plt.show()
plt.close(figure)

