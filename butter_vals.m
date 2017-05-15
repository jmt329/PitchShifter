%18-bit, 16-bit fraction,  2's comp conversion
clear all
figure(1);clf;
clc

%The scale has to be adjusted so that filter coefficients are
% -1.0<coeff<1.0
%Scaling performed is 2^(-scale)
scale = 3;
%For lowpass, set equal to normalized Freq (cutoff/(Fs/2))
%For bandpass, set equal to normalized Freq vector ([low high]/(Fs/2))
freq = [3500/(48000/2)] ;
%Filter order:
%    use 2,4 for lowpass 
%        1,2 for bandpass
%NOTE that for a bandpass filter (order) poles are generated for the high
%and low cutoffs, so the total order is (order)*2
order = 6;

%could also use butter, or cheby1 or cheby2 or besself
% but note that besself is lowpass only!
[b, a] = butter(order, freq) ;
b_orig = b;
a_orig = a;
%[b, a] = cheby1(order, 0.1, freq) ;
%[b, a] = besself(order, freq) ;
b = b * (2^-scale)
a = -a * (2^-scale)


disp(' ')
fprintf('//Filter: cutoff=%f \n',freq)
sorder = order*length(freq);
if sorder==2
    scstr = 'IIR2 filter(';
elseif sorder==4
    scstr = 'IIR4 filter('; 
elseif sorder==6
    scstr = 'IIR6_32bit_fixed filter('; 
else
    error('order*length(freq) must equal 2 4 or 6')
end
fprintf('%s \n',scstr); 
fprintf('     .audio_out (your_out), \n')
fprintf('     .audio_in (your_in), \n')
fprintf('     .scale (3''d%1d), \n', scale)
for i=1:length(b)
    if b(i)>=0
        fprintf('     .b%1d (32''h%s), \n', i, dec2hex(fix(2^30*b(i)))) ;
    else
        fprintf('     .b%1d (32''h%s), \n', i, dec2hex(bitcmp(fix(2^30*-b(i)),'uint32')+1));
    end
end

for i=2:length(a)
    if a(i)>=0
        fprintf('     .a%1d (32''h%s), \n', i, dec2hex(fix(2^30*a(i))))
    else
        fprintf('     .a%1d (32''h%s), \n', i,dec2hex(bitcmp(fix(2^30*-a(i)),'uint32')+1))
    end
end
fprintf('     .clk(YOUR_CLOCK), \n');
fprintf('     .data_val(YOUR_VAL_SIGNAL), \n');
fprintf('     .rst(rst), \n');
fprintf('     .audio_out_val(audio_out_val) \n');
fprintf(') ; //end filter \n');

disp(' ')
disp('CHECK scaling! all b''s and a''s <1 absolute value?') 
disp('BUT as big as possible?')
b
a

%sampling rate on DE2 board
Fs = 48000;
[b,a] = butter(order, freq) ;
[fresponse, ffreq] = freqz(b,a,300);
plot(ffreq/pi*Fs/2,abs(fresponse), 'b', 'linewidth',2);
xlabel('frequency'); ylabel('filter amplitude');
hold on
b = fix((b*(2^-scale))*2^30) ;
a = fix((a*(2^-scale))*2^30) ;
[fresponse, ffreq] = freqz(b,a,300);
plot(ffreq/pi*Fs/2,abs(fresponse), 'r', 'linewidth',2);
legend('exact','scaled 16-bit')
