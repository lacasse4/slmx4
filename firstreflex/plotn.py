# importing the required module
import matplotlib.pyplot as plt
import sys
# import numpy as np

def read_data(filename):
    fd = open(filename, "r") 
    y = [];
    count = 0

    while True:
        try:
            y.append(float(fd.readline()))
        except:
            fd.close()
            return y, count
        count += 1

if len(sys.argv) < 2 and len(sys.argv) > 4:
    print("usage:")
    print("      plotn.py <file>")
    print("   or plotn.py <file> <max>")
    print("   or plotn.py <file> <max> <min>")
    quit()

y, n = read_data(sys.argv[1])
minimum = min(y)
maximum = max(y)
if len(sys.argv) >= 3:
    maximum = float(sys.argv[2])
if len(sys.argv) >= 4:
    minimum = float(sys.argv[3])
print(n)

figure, axes = plt.subplots(figsize=(16,8))
figure.show()
        
axes.clear()
axes.set(title="Respiration profile", xlabel="n=%s" % n)
axes.set_xlabel("Time in 37 ms increment")
axes.set_ylabel("PSD in watt");
axes.set_autoscale_on(False)
axes.set_ylim(bottom=minimum, top=maximum)
axes.set_xlim(left=0, right=n)
x = [a for a in range(n)]
axes.plot(x, y, 'b-')
plt.show()

plt.close(figure)

