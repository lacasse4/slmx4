# importing the required module
import matplotlib.pyplot as plt
import sys
# import numpy as np

def read_data(filename):
    fd = open(filename, "r") 
    y = []
    count = 0

    while True:
        try:
            y.append(float(fd.readline()))
        except:
            fd.close()
            return y, count
        count += 1

if len(sys.argv) != 3:
    print("usage:")
    print("      plotn2.py <file1> <file2>")
    quit()

y1, n1 = read_data(sys.argv[1])
y2, n2 = read_data(sys.argv[2])
minimum = min(y1)
maximum = max(y1)
print(n1)

figure, axes = plt.subplots(2, 1,figsize=(16,8))
figure.show()
        
axes[0].clear()
# axes[0].set(title="Respiration profile", xlabel="n=%s" % n1)
# axes[0].set_xlabel("Time in 37 ms increment")
# axes[0].set_ylabel("PSD in watt");
axes[0].set_autoscale_on(False)
axes[0].set_ylim(bottom=0, top=1)
axes[0].set_xlim(left=0, right=n1)

axes[1].clear()
# axes[0].set(title="Respiration profile", xlabel="n=%s" % n1)
# axes[0].set_xlabel("Time in 37 ms increment")
# axes[0].set_ylabel("PSD in watt");
axes[1].set_autoscale_on(False)
axes[1].set_ylim(bottom=0, top=1)
axes[1].set_xlim(left=0, right=n1)

x = [a for a in range(n1)]
axes[0].plot(x, y1, 'b-')
axes[1].plot(x, y2, 'b-')
plt.show()

plt.close(figure)

