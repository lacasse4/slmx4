%data = readtable('FILE');
data = csvread('../SW_Repartisseur/FILE');
h = plot(data(:))
ylim([-0.2,0.2])
while(1)
  data = csvread('../SW_Repartisseur/FILE');
  set(h,'ydata', data(:));
  drawnow;
end


