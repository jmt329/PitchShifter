% ringbuffer size
B = 1024;

% darth vader: 0.6, simple average
% chipmunk: 2.5, single
% robot: 1.6, simple average
% changing: 1.6, bufgain + modulate
% chorus: 1.2, +s_in 

% test pitch shift
[s_in,fs] = audioread('../../emma.wav');

% sampling frequency
% fs=44100;
% x = linspace(0,1,fs);
% s_in = sin(2*pi*440*x);

% define variables
ringbuffer1 = zeros(B,1);
ringbuffer2 = zeros(B,1);
ringbuffer3 = zeros(B,1);
ringbuffer4 = zeros(B,1);

% initialize output signal
s_avg      = zeros(length(s_in), 1);
s_no_cross = zeros(length(s_in), 1);

s_chord1 = zeros(length(s_in), 1);
s_chord2 = zeros(length(s_in), 1);

delta1 = 2^(-3/12); % changes pitch (by changing read-out increments)
delta2 = 2^(-6/12);

% start pitch shifting
idx1 = 0;
idx2 = 0;
idx3 = 0;
idx4 = 0;

for tt=1:length(s_in)
    
    % write to ringbuffer1 
    w_addr1 = mod(tt,B)+1;
    ringbuffer1(w_addr1) = s_in(tt);
    
    % write to ringbuffer2
    w_addr2 = mod(tt+round(B/2),B)+1;
    ringbuffer2(w_addr2) = s_in(tt);   
    
    % read from ringbuffer1
    idx1 = idx1 + delta1;
    r_addr1 = mod(round(idx1)-1,B)+1;
   
    % read from ringbuffer2
    idx2 = idx2 + delta1;
    r_addr2 = mod(round(idx2)-1,B)+1;  
    
    
    % write to ringbuffer3 
    w_addr3 = mod(tt,B)+1;
    ringbuffer3(w_addr3) = s_in(tt);
    
    % write to ringbuffer4
    w_addr4 = mod(tt+round(B/2),B)+1;
    ringbuffer4(w_addr4) = s_in(tt);   
    
    % read from ringbuffer3
    idx3 = idx3 + delta2;
    r_addr3 = mod(round(idx3)-1,B)+1;
   
    % read from ringbuffer4
    idx4 = idx4 + delta2;
    r_addr4 = mod(round(idx4)-1,B)+1;  
    
    % modulate stuff
    % delta = 1 + 0.5*sin(1*pi*tt/fs); 
    
    % Options for reading out:
    
    % avg cross faiding
    s_avg(tt,1) = 0.5*(ringbuffer1(r_addr1,1) + ringbuffer2(r_addr2,1));
    
    s_chord1(tt,1) = 0.25*(ringbuffer1(r_addr1,1) + ringbuffer2(r_addr2,1));
    
    s_chord2(tt,1) = 0.25*(ringbuffer3(r_addr3,1) + ringbuffer4(r_addr4,1));
    
    
        
    % no cross faiding
    s_no_cross(tt,1) = ringbuffer1(r_addr1,1);
    
end

soundsc(0.5*s_in, fs)
pause(0.005)
soundsc(s_chord1, fs)
pause(0.005)
soundsc(s_chord2, fs)

pause 

soundsc(0.5*s_in(:,1) + s_chord1 + s_chord2, fs)


% figure
% plot(s_in, 'r')
% hold on
% plot(s_avg, 'b')
% set(gca, 'FontSize', 16);
% title('avg cross fading')
% legend('input sine wave', 'shifted', 'Location', 'Best')
% 
% s_med = medfilt1(s_avg, 5);
% 
% figure
% plot(s_avg, 'r')
% hold on
% plot(s_med, 'b')
% set(gca, 'FontSize', 16);
% title('Adding a median filter')
% legend('unfiltered', 'median filtered', 'Location', 'Best')
% 
% fc = 3500;
% [b,a] = butter(6, fc/(fs/2));
% s_med_butter = filter(b, a, s_med);
% s_butter = filter(b, a, s_avg);
% 
% figure
% plot(s_med, 'r')
% hold on
% plot(s_med_butter, 'b')
% hold on
% plot(s_butter, 'g')
% set(gca, 'FontSize', 16);
% title('Testing different filters')
% legend('median', 'median and butter', 'butter', 'Location', 'Best')
