# importing the required module
import matplotlib.pyplot as plt
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

fr = read_data("DATA-iter64-pps128-frame-141ms")
z5 = read_data("DATA-iter64-pps128-wind5-141ms")
z1 = read_data("DATA-iter64-pps128-wind1-141ms")

plt.style.use('_mpl-gallery-nogrid')
figure, axes = plt.subplots(1, 3, figsize=(20,8))
axes[0].imshow(z1)
axes[1].imshow(z5)
axes[2].imshow(fr)

plt.show()
plt.close(figure)

