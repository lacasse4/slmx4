%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% vcom_test.m
% This is a demonstration how to use the class *vcom_xep_radar_connector*
% Copyright: 2020 Sensor Logic
% Written by: Justin Hadella
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
clear;
clc;

%Communication avec le port USB contenant le capteur
r = vcom_xep_radar_connector('COM9'); % adjust for *your* COM port!
r.Open('X4');

% As a side-effect many settings on write will cause the numSamplers
% variable to update
fprintf('bins = %d\n', r.numSamplers);

% Actually every variable from the radar is requested in this manner.
iterations = r.Item('iterations');
fprintf('iterations = %d\n', iterations);

% Setting some variables
r.TryUpdateChip('rx_wait', 0);
r.TryUpdateChip('frame_start', 0.3);
r.TryUpdateChip('frame_end', 4.0);
%r.TryUpdateChip('ddc_en', 1);
r.TryUpdateChip('PPS', 10);

% Set up time plot signal
frameSize = r.numSamplers;   % Get # bins/samplers in a frame
frame = zeros(1, frameSize); % Preallocate frame
h_fig = figure;
ax1 = gca;

dist = 0.3:(4.0-0.3)/frameSize:4.0;

%Initialisation des variables
windowSize = 80;
moyenne = zeros(1,frameSize - windowSize);
resp_delay = 1;
resp_size = 100;
resp = zeros(1,resp_size);
tresh = 0.05;
maxx_prev = windowSize; 

Valeur_Scale = 0.1;

%Initialisation de la variable pour détecter
%Une grande hausse de perturbation
Perturb_Delta = 20;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot(4,1,1)
h1 = plot( 1:frameSize, frame);
axis([0,623,-10 10])
title('radar time waveform');
xlabel('bin');
ylabel('amplitude');
xl  = xline(100);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot(4,1,2)
h2 = plot( 1:frameSize, frame);
axis([0,623 -Valeur_Scale Valeur_Scale]);
title('Radar with Butterworth 3rd filter [0.1-5] Hz');
xlabel('bin');
ylabel('amplitude');

%legend('y(x)', sprintf('Maximum y = %0.3f', maxy))
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot(4,1,3)
h3 = plot( 1:frameSize, frame);
axis([0,623 -Valeur_Scale Valeur_Scale]);
title('Radar with Butterworth 3rd filter [0.1-5] Hz');
xlabel('bin');
ylabel('amplitude');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot(4,1,4)
h4 = plot( 1:frameSize, frame);
%axis([0,623 -2 2]);
title('Radar with Butterworth 3rd filter [0.1-5] Hz');
xlabel('bin');
ylabel('amplitude');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Bandpass Gains
gain1 = 0.8; %HighPass Gain
gain2 = 0.02; %LowPass Gain
gain3 = 0.03; %LowPass Abs Gain for position detection
gain4 = 0.008; %LowPass for maxx

gain_perturb = 0.1; %Gain pour le passe-bas suite à perturbation

%Initialisation des variables
statut = 0;
count = 0;
diff_valeur = 0;
presence = 0;

%Débute avec perturbation pour se "callibrer"
compteur = 10;

y = [.1 .1 .1 .1 .1 .1 .1 .1 .1 .1];
x = [1];

%Initialisation des variables framebuff et outframe:
framebuff = zeros(10,frameSize);
filtFrame = zeros(4,frameSize);
resp_out = zeros(1,frameSize);
        
% Plot data while window is open
while isgraphics(h_fig)
    %try
        
        %Mettre statut=1 s'il y a une perturbation
        if(resp(1,1)/mean(resp(1,1:3)) > 1.25)
            statut = 1;
            mobilite = 1;
        end
    
        %Détection de la présence de la personne:
        if(abs(mean(resp(1,1:25)-mean(resp(1,26:50))))<0.2)
            presence=0;
        else
            presence=1;
        end

        frame = abs(r.GetFrameNormalizedDouble-255);
        frame = filter(y,x,frame);
        framebuff(2,:) = framebuff(1,:);
        framebuff(1,:) = frame;

        %Filtre passe-bande
        filtFrame(1,:) = (1+gain1)/2*(framebuff(1,:)-framebuff(2,:)) + gain1*filtFrame(1,:); %highpass
        filtFrame(2,:) = (gain2)*(filtFrame(1,:) - filtFrame(2,:)) + filtFrame(2,:); %lowpass
        
        for i=1:frameSize
            filtFrame(3,i) = (gain3)*(abs(filtFrame(2,i)) - filtFrame(3,i)) + filtFrame(3,i);
        end

        %Additionner les points se retrouvant à droite de la plage
        %d'échantillonnage et le mettre dans la variable moyenne
        for i=windowSize/2:1:frameSize-windowSize/2
            moyenne(i) = sum(filtFrame(3,((i-windowSize/2)+1:(i+windowSize/2))));
        end
        
        %Trouver le maximum en x et en y des points et y ajouter la moitié
        %restante pour positionner le point au plus récent
        [maxy maxx] = max(moyenne);
        %maxx = maxx + windowSize / 2;
        
        %maxx_prev = (gain4)*(maxx - maxx_prev) + maxx_prev; %lowpass

        %Si statut=1 et que le bus resp est rempli pour les trois premières
        % valeurs 
        if(statut==1 && all(resp(1,1:4)))
            mobilite = 1;
            if(compteur>0)
                %resp(1)=0;
                %Si instable, mettre un gain très petit pour ne pas générer
                %de mauvaises valeurs
                compteur=compteur-1;
                
                %maxx_prev=0;
                %Appliquer un filtre passe-bas à partir de la valeur trouvée
                
            elseif(compteur==0)
                statut=0;
                compteur=10;
                %Si stabiliser, retourner avec le gain précédent
                
                %Appliquer un filtre passe-bas à partir de la valeur trouvée
                 %lowpass
            end

        elseif(statut==0)
            mobilite = 0;
        end
        maxx_prev = (gain4)*(maxx - maxx_prev) + maxx_prev;
        %Conversion de maxx_int en integer 8
        maxx_int = int8(maxx_prev);
        count = count+1;
        %if(count > resp_delay)
        
        for i=resp_size:-1:2
            resp(i) = resp(i-1);
        end
        %Faire la somme des valeurs de frame se trouvant *********
        resp(1) = sum(frame(maxx_int-windowSize/2:maxx_int+windowSize/2));

        %Afficher la respiration en temps réelle sur le dernier graphique
        if(true)
            count = 0;
            if ((maxy > tresh) && (mobilite==0) && (presence==1))
                resp_out = resp;
            else
                resp_out = zeros(1,resp_size);
            end
        end
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %TEST PHIL:        
        %Calcul de la différence entre la donnée précédente et la donnée
        %présente
        diff_valeur = sum(resp(1,1:10)) - sum(resp(1,11:20));
        
        %disp(resp(1,1));
        %disp(mobilite);
        %disp(presence);
        fprintf('avant : %i ; apres : %i \r\n',maxx,maxx_prev)

        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

        %Affichage des 4 graphiques en temps réel
        set(h1, 'xdata', 1:frameSize, 'ydata', frame(1,:));
        set(h2, 'xdata', 1:frameSize, 'ydata', filtFrame(2,:));
        set(h3, 'xdata', 1:frameSize, 'ydata', filtFrame(3,:));
        set(h4, 'xdata', 1:resp_size, 'ydata', resp_out);

        xl.Value = maxx_int;
        
        %Aller chercher le MAX des valeurs pour ensuite l'afficher dans le
        %display:
        %maxy = max(abs(outframe(2,:)));
        %disp(maxx);

        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        
        %set(h1, 'xdata', 1:frameSize, 'ydata', frame(1,:));
        %set(h2, 'xdata', 1:frameSize, 'ydata', outframe(1,:));
        drawnow;
    %catch ME
    %end
end

r.Close();