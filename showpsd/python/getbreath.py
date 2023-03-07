# importing the required module
# import matplotlib.pyplot as plt
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
            return z, n, frame_start, frame_end
    
        frame_start = float(fd.readline())
        frame_end = float(fd.readline())
        y = [];
        count = 0
        while count < n:
            count += 1
            y.append(float(fd.readline()))
        z.append(y)

def write_data(filename, x):
    fd = open(filename, "w") 
    fd.write(str(len(x)) + "\n")
    for i in range(len(x)):
        fd.write(str(x[i]) + "\n")
    fd.close()

if len(sys.argv) != 3:
    print("usage: getbreath.py file <d>")
    print("       where d is the distance (0 meter - 9 meters)");
    quit()

fr, num_samples, frame_start, frame_end = read_data(sys.argv[1])
print("num_samples = " + str(num_samples))
print("frame_start = " + str(frame_start))
print("frame_end   = " + str(frame_end))
num_scan = len(fr)
print("num_scan    = " + str(num_scan))

distance = float(sys.argv[2])
print("distance    = " + str(distance))
index = round(num_samples * (distance - frame_start) / (frame_end - frame_start))

x = []
for i in range(num_scan):
    x.append(fr[i][index])

filename = "BREATH" + str(index)
write_data(filename, x)