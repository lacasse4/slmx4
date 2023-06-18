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

if len(sys.argv) != 2:
    print("usage: plotsinglebreath.py file")
    quit()

y, n = read_data(sys.argv[1])
print(n)

figure, axes = plt.subplots(figsize=(16,8))
figure.show()
        
axes.clear()
axes.set(title="Respiration profile", xlabel="n=%s" % n)
axes.set_xlabel("Time in 37 ms increment")
axes.set_ylabel("PSD in db");
axes.set_autoscale_on(False)
axes.set_ylim(bottom=-30, top=-15)
axes.set_xlim(left=0, right=n)
x = [a for a in range(n)]
axes.plot(x, y, 'b-')
plt.show()

plt.close(figure)

