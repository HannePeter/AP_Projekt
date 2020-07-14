clear
home
close all



ch1_foldername = "ch1/";
ch2_foldername = "ch2/";
ch3_foldername = "ch3/";
wflen = 20000;
avg_num = 16;                   # 128 max


### read waveform files into matrices ##########################################
puts("read waveforms ... ");

ch1_waveforms = zeros(avg_num, wflen);
ch2_waveforms = zeros(avg_num, wflen);
ch3_waveforms = zeros(avg_num, wflen);

for i = 1:avg_num
    filename = ["wf", num2str(i), ".csv"];
    ch1_waveforms(i, :) = csvread([ch1_foldername, filename]);
    ch2_waveforms(i, :) = csvread([ch2_foldername, filename]);
    ch3_waveforms(i, :) = csvread([ch3_foldername, filename]);
endfor

puts("done!\n");

### start runtime measurement ##################################################
puts("start calculation ... ");
start = tic;


### averaging of <avg_num> waveforms ###########################################
ch1_waveform_avg = zeros(1, wflen);
ch2_waveform_avg = zeros(1, wflen);
ch3_waveform_avg = zeros(1, wflen);

for i = 1:avg_num
    ch1_waveform_avg += ch1_waveforms(i, :);
    ch2_waveform_avg += ch2_waveforms(i, :);
    ch3_waveform_avg += ch3_waveforms(i, :);
endfor

ch1_waveform_avg /= avg_num;
ch2_waveform_avg /= avg_num;
ch3_waveform_avg /= avg_num;


### upsampling #################################################################
ch1_S = fft(ch1_waveform_avg);
ch2_S = fft(ch2_waveform_avg);
ch3_S = fft(ch3_waveform_avg);

ch1_S_up = [ch1_S(1:wflen/2), zeros(1, wflen), ch1_S(wflen/2+1:wflen)];
ch2_S_up = [ch2_S(1:wflen/2), zeros(1, wflen), ch2_S(wflen/2+1:wflen)];
ch3_S_up = [ch3_S(1:wflen/2), zeros(1, wflen), ch3_S(wflen/2+1:wflen)];

ch1_waveform_upsamp = real(ifft(ch1_S_up)) * 2;
ch2_waveform_upsamp = real(ifft(ch2_S_up)) * 2;
ch3_waveform_upsamp = real(ifft(ch3_S_up)) * 2;

wflen *= 2;


### runlength correction of all channels + combining ###########################
normVec = [(1:wflen), fliplr(1:wflen-1)];

corr2 = xcorr(ch1_waveform_upsamp, ch2_waveform_upsamp) ./ normVec;    # = cross correlation
corr3 = xcorr(ch1_waveform_upsamp, ch3_waveform_upsamp) ./ normVec;

trunc = ceil(length(corr2) / 100 * 5);          # 5% truncatination
[val2, idx2] = max(corr2(trunc:end-trunc));     # dont use 5% of waveform ends
[val3, idx3] = max(corr3(trunc:end-trunc));
idx2 += (trunc - 1);                            # we have to add trunc, to get correct index
idx3 += (trunc - 1);
shift2 = idx2 - wflen;
shift3 = idx3 - wflen;

ch1_waveform_shifted = ch1_waveform_upsamp(shift3+1:wflen);
ch2_waveform_shifted = ch2_waveform_upsamp(shift2+1:wflen-shift2);
ch3_waveform_shifted = ch3_waveform_upsamp(1:wflen-shift3);

comb = ch1_waveform_shifted .+ ch2_waveform_shifted .+ ch3_waveform_shifted;


### Get program runtime ########################################################
runtime = toc(start) * 1000
puts(" ... done!\n");



### plot #######################################################################
puts("plot waveforms ... ");

figure(1);
subplot(3, 1, 1);
plot(ch1_waveforms(1, 1:500));
title("Channel 1");
subplot(3, 1, 2);
plot(ch1_waveform_avg(1:500));
title("Channel 1 averaged");
subplot(3, 1, 3);
plot(ch1_waveform_upsamp(1:1000));
title("Channel 1 upsampled");

figure(2);
subplot(3, 1, 1);
plot(ch2_waveforms(1, 1:500));
title("Channel 2");
subplot(3, 1, 2);
plot(ch2_waveform_avg(1:500));
title("Channel 2 averaged");
subplot(3, 1, 3);
plot(ch2_waveform_upsamp(1:1000));
title("Channel 2 upsampled");

figure(3);
subplot(3, 1, 1);
plot(ch3_waveforms(1, 1:500));
title("Channel 3");
subplot(3, 1, 2);
plot(ch3_waveform_avg(1:500));
title("Channel 3 averaged");
subplot(3, 1, 3);
plot(ch3_waveform_upsamp(1:1000));
title("Channel 3 upsampled");

figure(4);
subplot(2, 1, 1);
plot(corr2);
title("corss correlation channel 1 / 2");
subplot(2, 1, 2);
plot(corr3);
title("corss correlation channel 1 / 3");

figure(5);
subplot(4, 1, 1);
plot(ch1_waveform_shifted(1:500));
title("Channel 1 shifted");
ylim([-0.5, 3.5]);
subplot(4, 1, 2);
plot(ch2_waveform_shifted(1:500));
title("Channel 2 shifted");
ylim([-0.5, 3.5]);
subplot(4, 1, 3);
plot(ch3_waveform_shifted(1:500));
title("Channel 3 shifted");
ylim([-0.5, 3.5]);
subplot(4, 1, 4);
plot(comb(1:500));
title("Channels combined");
ylim([-0.5, 3.5]);

puts("done!\n");
