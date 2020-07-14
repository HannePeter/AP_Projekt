import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack
import scipy.signal
from timeit import default_timer as timer

ch1_foldername = "ch1/"
ch2_foldername = "ch2/"
ch3_foldername = "ch3/"
wflen = 20000
avg_num = 16                        # max 128


### read waveform files into matrices ##########################################
print("read waveforms ...", end='', flush=True)

ch1_waveforms = np.empty([avg_num, wflen])
ch2_waveforms = np.empty([avg_num, wflen])
ch3_waveforms = np.empty([avg_num, wflen])

for i in range(avg_num):
    filename = "wf" + str(i+1) + ".csv"
    ch1_waveforms[i] = np.genfromtxt(ch1_foldername + filename, delimiter= ',')
    ch2_waveforms[i] = np.genfromtxt(ch2_foldername + filename, delimiter= ',')
    ch3_waveforms[i] = np.genfromtxt(ch3_foldername + filename, delimiter= ',')


print("done!")

### start runtime measurement ##################################################
print("start calculation ... ", end='', flush=True)
start = timer()


### averaging of <avg_num> waveforms ###########################################
ch1_waveform_avg = np.zeros(wflen)
ch2_waveform_avg = np.zeros(wflen)
ch3_waveform_avg = np.zeros(wflen)

for i in range(avg_num):
    ch1_waveform_avg += ch1_waveforms[i]
    ch2_waveform_avg += ch2_waveforms[i]
    ch3_waveform_avg += ch3_waveforms[i]


ch1_waveform_avg /= avg_num
ch2_waveform_avg /= avg_num
ch3_waveform_avg /= avg_num


### upsampling #################################################################
ch1_S = scipy.fftpack.fft(ch1_waveform_avg)
ch2_S = scipy.fftpack.fft(ch2_waveform_avg)
ch3_S = scipy.fftpack.fft(ch3_waveform_avg)

ch1_S_up = np.concatenate((ch1_S[0:int(wflen/2)], np.zeros(wflen), ch1_S[int(wflen/2):wflen]))
ch2_S_up = np.concatenate((ch2_S[0:int(wflen/2)], np.zeros(wflen), ch2_S[int(wflen/2):wflen]))
ch3_S_up = np.concatenate((ch3_S[0:int(wflen/2)], np.zeros(wflen), ch3_S[int(wflen/2):wflen]))

ch1_waveform_upsamp = np.real(scipy.fftpack.ifft(ch1_S_up)) * 2
ch2_waveform_upsamp = np.real(scipy.fftpack.ifft(ch2_S_up)) * 2
ch3_waveform_upsamp = np.real(scipy.fftpack.ifft(ch3_S_up)) * 2

wflen *= 2


### runlength correction of all channels + combining ###########################
normVec = np.append(np.arange(1, wflen+1), np.arange(wflen-1, 0, -1))

corr2 = scipy.signal.correlate(ch1_waveform_upsamp, ch2_waveform_upsamp) / normVec
corr3 = scipy.signal.correlate(ch1_waveform_upsamp, ch3_waveform_upsamp) / normVec

trunc = int(len(corr2)/100 * 5);
idx2 = np.argmax(corr2[trunc:len(corr2)-trunc])
idx3 = np.argmax(corr3[trunc:len(corr3)-trunc])
idx2 += (trunc + 1)
idx3 += (trunc + 1)
shift2 = idx2 - wflen
shift3 = idx3 - wflen

ch1_waveform_shifted = ch1_waveform_upsamp[shift3:wflen]
ch2_waveform_shifted = ch2_waveform_upsamp[shift2:wflen-shift2]
ch3_waveform_shifted = ch3_waveform_upsamp[0:wflen-shift3]

comb = ch1_waveform_shifted + ch2_waveform_shifted + ch3_waveform_shifted


### Get program runtime ########################################################
end = timer()
print("done!")
print("compute time: ", (end - start)*1000, "ms")    # timer output


### plot #######################################################################
print("plot waveforms ... ", end='', flush=True)

#plt.subplot(311)
#plt.title("Channel 1")
#plt.tight_layout()
#plt.plot(ch1_waveforms[0, 0:499])
#
#plt.subplot(312)
#plt.title("Channel 1 averaged")
#plt.tight_layout()
#plt.plot(ch1_waveform_avg[0:499])
#
#plt.subplot(313)
#plt.title("Channel 1 upsampled")
#plt.tight_layout()
#plt.plot(ch1_waveform_upsamp[0:999])

print("done!")
