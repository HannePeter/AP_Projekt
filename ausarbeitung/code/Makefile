dsp_c: dsp_c.c
	gcc -O3 -ftree-vectorize -msse2 -lfftw3 -lxcorr -lm dsp_c.c -o dsp_c

waveforms:
	gcc genWaveforms.c -o genWaveforms 
	mkdir ch1
	mkdir ch2
	mkdir ch3
	./genWaveforms
	rm genWaveforms

run:
	./dsp_c

clean:
	rm -rf ch1 ch2 ch3 genWaveforms dsp_c
