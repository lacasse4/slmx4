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

maxrms = read_data("DATA-iter64-pps128-maxrms-141ms")
# rms = read_data("DATA-iter64-pps128-rms-141ms")
# fr33 = read_data("DATA-iter64-pps128-fra33-141ms")
# fr13 = read_data("DATA-iter64-pps128-fra13-141ms")
# fr21 = read_data("DATA-iter64-pps128-fra21-141ms")
# fr63 = read_data("DATA-iter64-pps128-fra63-141ms")
# z5 = read_data("DATA-iter64-pps128-wind5-141ms")
# z3 = read_data("DATA-iter64-pps128-wind3-141ms")
# z1 = read_data("DATA-iter64-pps128-wind1-141ms")

plt.style.use('_mpl-gallery-nogrid')
figure, axes = plt.subplots(figsize=(8,8))
# axes[0].imshow(fr13)
# axes[1].imshow(fr21)
axes.imshow(maxrms)


plt.show()
plt.close(figure)

