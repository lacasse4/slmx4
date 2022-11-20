from matplotlib import pyplot as plt

figure = plt.figure()
axes = figure.gca()
figure.show()

for i in range(5,10):
    x = [];
    y = [];
    for j in range(i):
        x.append(j)
        y.append(j*j)
    axes.plot(x, y, 'go')
    figure.canvas.draw()
    plt.pause(0.1)
plt.close(figure)