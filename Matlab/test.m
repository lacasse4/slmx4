fs = 1000;
fresp = 15;
frameSize = 50;
zsize = 50;

windowSize = 10;

moyenne = zeros(1,zsize-windowSize);

resp = zeros(1,zsize);

tresh = 0.003

x = 0:10/(frameSize-1):10;
y = zeros(1,frameSize);

yfilt = zeros(1,zsize);
yfilt2 = zeros(1,zsize);

yfilt3 = zeros(1,zsize);

xz = 0:10/(zsize-1):10;
z = zeros(zsize,zsize);
t = 0:10/(zsize-1):10;

subplot(3,1,1);
h1 = plot( 1:frameSize, frameSize);
subplot(3,1,2);
h = surf(xz,t,z);
subplot(3,1,3);
h2 = plot( 1:zsize, zsize);
axis([0 10,-.01 .01]);

gain = 0.000001;
gain2 = 0.1;
gain3 = 0.005;

timevar = 0;
count = 0;

y = normpdf(x,0,0.5) + 0.3.*normpdf(x,2,.8) + 0.1.*normpdf(x,7,.4) +0.3.*normpdf(x,4,.4) ;


while true
    timevar = timevar + 1/fs;
    y = normpdf(x,0,0.5) + 0.3.*normpdf(x,2,.8) + 0.1.*normpdf(x,7,.4) + (sin(timevar*fresp*2*pi)+1) .*0.07.*normpdf(x,7+0.2*sin(timevar*fresp*2*pi),.4)+0.3.*normpdf(x,4,.4) ;

    if true
        for i=zsize:-1:2
            z(i,:) = z(i-1,:);
        end
        for i=1:zsize
            z(1,i) = y(i*(frameSize/zsize));
        end
    end

    %[b,a] = butter(10, [300 300]/(fs/2), 'bandpass');

    for i=1:zsize
        yfilt(i) = (1-gain)*(z(1,i)-z(2,i)) + gain*yfilt(i);%highpass
        yfilt2(i) = (gain2)*(yfilt(i) - yfilt2(i)) + yfilt2(i); %lowpass
        %temp = (filter(b,a,z(:,i)));
        %yfilt(i) = temp(1);
    end
    
    for i=1:zsize
        yfilt3(i) = (gain3)*(abs(yfilt2(i)) - yfilt3(i)) + yfilt3(i);
        %yfilt3 = (gain2)*(yfilt(i) - yfilt2(i)) + yfilt2(i); %lowpass
    end

    for i=windowSize/2:1:zsize-windowSize/2
        moyenne(i) = sum(yfilt3((i-windowSize/2)+1:(i+windowSize/2)));
    end

    [maxy maxx] = max(moyenne);

    %line([maxx+windowSize/2;2],[maxx+windowSize/2;0],'linestyle','--');
    
    count = count+1;
    if(count > 5)
        count = 0;
        if maxy > tresh 
            for i=zsize:-1:2
                resp(i) = resp(i-1);
            end
            resp(1) = y(maxx);
        else
            resp = zeros(1,zsize);
        end
    end

    xz(maxx)
 
    set(h1, 'xdata', x, 'ydata', yfilt);
    set(h2, 'xdata', xz, 'ydata', yfilt3);



    set(h, 'xdata', xz, 'ydata', t,'zdata',z);

    drawnow;
    pause(1/fs);
end

%h = surf(x,t,y)

%set(h,'LineStyle','none')

