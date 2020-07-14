#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fftw3.h>
#include <string.h>
#include <xcorr.h>

#define WFLEN       20000
#define AVGNUM      16                   // 128 max

void csvread(char *filename, double *waveform);
void average(double *wf, double **nWf, int wflen, int nAvg);
void upsample(double *in, double *out, int inlen);
void crossCorr(double *wf1, double *wf2, double *out, int len);
int findMaxIndex(double *corr, int len);



int main()
{
/* *** read waveform files ************************************************* */
    fputs("read waveforms ... ", stdout);
    fflush(stdout);

    char filename[100] = {0};
    double **ch1_waveforms = malloc(AVGNUM * sizeof(double *));
    double **ch2_waveforms = malloc(AVGNUM * sizeof(double *));
    double **ch3_waveforms = malloc(AVGNUM * sizeof(double *));

    for (int i = 0; i < AVGNUM; i++) {
        ch1_waveforms[i] = malloc(WFLEN * sizeof(double));
        ch2_waveforms[i] = malloc(WFLEN * sizeof(double));
        ch3_waveforms[i] = malloc(WFLEN * sizeof(double));

        sprintf(filename, "ch1/wf%d.csv", i+1);
        csvread(filename, ch1_waveforms[i]);

        sprintf(filename, "ch2/wf%d.csv", i+1);
        csvread(filename, ch2_waveforms[i]);

        sprintf(filename, "ch3/wf%d.csv", i+1);
        csvread(filename, ch3_waveforms[i]);
    }

    puts("done!");


/* *** start runtime measurement ******************************************** */
    struct timespec start;
    struct timespec stop;

    fputs("start calculation ... ", stdout);
    fflush(stdout);

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);


/* *** averaging of <avg_num> waveforms ************************************* */
    double *ch1_waveform_avg = calloc(WFLEN, sizeof(double));
    double *ch2_waveform_avg = calloc(WFLEN, sizeof(double));
    double *ch3_waveform_avg = calloc(WFLEN, sizeof(double));

    average(ch1_waveform_avg, ch1_waveforms, WFLEN, AVGNUM);
    average(ch2_waveform_avg, ch2_waveforms, WFLEN, AVGNUM);
    average(ch3_waveform_avg, ch3_waveforms, WFLEN, AVGNUM);


/* *** upsampling *********************************************************** */
    int newWfLen = WFLEN * 2;

    double *ch1_waveform_upsamp = malloc(newWfLen * sizeof(double));
    double *ch2_waveform_upsamp = malloc(newWfLen * sizeof(double));
    double *ch3_waveform_upsamp = malloc(newWfLen * sizeof(double));

    upsample(ch1_waveform_avg, ch1_waveform_upsamp, WFLEN);
    upsample(ch2_waveform_avg, ch2_waveform_upsamp, WFLEN);
    upsample(ch3_waveform_avg, ch3_waveform_upsamp, WFLEN);


/* *** runlength correction of all channels + combining ********************* */
    int xCorrLen = 2 * newWfLen - 1;

    double *corr2 = malloc(xCorrLen * sizeof(double));
    double *corr3 = malloc(xCorrLen * sizeof(double));
    double *comb = malloc(newWfLen * sizeof(double));

    crossCorr(ch1_waveform_upsamp, ch2_waveform_upsamp, corr2, newWfLen);
    crossCorr(ch1_waveform_upsamp, ch3_waveform_upsamp, corr3, newWfLen);

    int shift2 = findMaxIndex(corr2, xCorrLen) - newWfLen + 1;
    int shift3 = findMaxIndex(corr3, xCorrLen) - newWfLen + 1;
    int trunc = newWfLen - shift3;

    for (int i = 0; i < trunc; i++) {
        comb[i] = ch1_waveform_upsamp[i+shift3] + ch2_waveform_upsamp[i+shift2] + ch3_waveform_upsamp[i];
        //printf("%lf\n", comb[i]);
    }


/* *** Get program runtime ************************************************** */
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    puts("done!");
    long runtime_us = (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000;
    printf("compute time: %d us\n", runtime_us);


/* *** free memory ********************************************************** */
    for (int i = 0; i < AVGNUM; i++) {
        free(ch1_waveforms[i]);
        free(ch2_waveforms[i]);
        free(ch3_waveforms[i]);
    }
    free(ch1_waveforms);
    free(ch2_waveforms);
    free(ch3_waveforms);

    free(ch1_waveform_avg);
    free(ch2_waveform_avg);
    free(ch3_waveform_avg);

    free(ch1_waveform_upsamp);
    free(ch2_waveform_upsamp);
    free(ch3_waveform_upsamp);

    free(corr2);
    free(corr3);
    free(comb);

    return EXIT_SUCCESS;
}




////////////////////////////////////////////////////////////////////////////////
// Funktionen
////////////////////////////////////////////////////////////////////////////////

void csvread(char *filename, double *waveform)
{
    FILE *fp = fopen(filename, "r");
    char buff[WFLEN*10] = {0};                      // 10 chars per value
    fgets(buff, WFLEN*10, fp);
    fclose(fp);

    char *val = buff;
    for (int i = 0; i < WFLEN; i++) {
        waveform[i] = atof(val);

        if (i != WFLEN-1) {
            while(*val != ',') {
                val++;
            }
            val++;
        }
    }
}


void average(double *wf, double **nWf, int wflen, int nAvg)
{
    for (int i = 0; i < nAvg; i++) {
        for (int j = 0; j < wflen; j++) {
            wf[j] += nWf[i][j];
        }
    }
    for (int i = 0; i < wflen; i++) {
        wf[i] /= nAvg;
    }
}


void upsample(double *inWf, double *outWf, int inlen)
{
    /*   FFTW must be installed.
     *   gcc -lfftw3 -lm is needed for compilation
     *
     * see: http://www.fftw.org/fftw3_doc/Complex-One_002dDimensional-DFTs.html
     *      http://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html
     *      https://stackoverflow.com/questions/5818558/computing-fft-and-ifft-with-fftw-h-in-c
     *      https://stackoverflow.com/questions/5685765/computing-fft-and-ifft-with-fftw-library-c
     */

    fftw_complex *fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * inlen);               // fft prepare data arrays
    fftw_complex *fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * inlen);
    for (int i = 0; i < inlen; i++) {
        fft_in[i][0] = inWf[i];
        fft_in[i][1] = 0;
    }

    fftw_plan fft = fftw_plan_dft_1d(inlen, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);          // fft calculate
    fftw_execute(fft);

    int outlen = inlen * 2;                                                                         // ifft prepare data arrays
    int idx;
    fftw_complex *ifft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * outlen);
    fftw_complex *ifft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * outlen);
    for (int i = 0; i < inlen/2; i++) {
        ifft_in[i][0] = fft_out[i][0];
        ifft_in[i][1] = fft_out[i][1];
    }
    for (int i = inlen/2; i < outlen-(inlen/2); i++) {
        ifft_in[i][0] = 0;
        ifft_in[i][1] = 0;
    }
    for (int i = outlen-(inlen/2); i < outlen; i++) {
        idx = i-inlen;
        ifft_in[i][0] = fft_out[idx][0];
        ifft_in[i][1] = fft_out[idx][1];
    }

    fftw_plan ifft = fftw_plan_dft_1d(outlen, ifft_in, ifft_out, FFTW_BACKWARD, FFTW_ESTIMATE);     // ifft calculate
    fftw_execute(ifft);

    double norm = 2.0/outlen;
    for (int i = 0; i < outlen; i++) {
        outWf[i] = ifft_out[i][0] * norm;
    }

    fftw_destroy_plan(fft);
    fftw_destroy_plan(ifft);
    fftw_free(fft_in);
    fftw_free(fft_out);
    fftw_free(ifft_in);
    fftw_free(ifft_out);
}


void crossCorr(double *wf1, double *wf2, double *out, int len)
{
    /*   xcorr must be installed.
     *   gcc -lxcorr is needed for compilation
     *
     * see: https://github.com/dMaggot/libxcorr
     */

    int xCorrLen = 2 * len - 1;

    fftw_complex *wf1_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
    fftw_complex *wf2_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
    fftw_complex *out_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * xCorrLen);

    for (int i = 0; i < len; i++) {                     // prepare complex signals
        wf1_cpx[i][0] = wf1[i];
        wf1_cpx[i][1] = 0;

        wf2_cpx[i][0] = wf2[i];
        wf2_cpx[i][1] = 0;
    }

    xcorr(wf1_cpx, wf2_cpx, out_cpx, len);              // calculate xcorr

    for (int i = 0; i < xCorrLen; i++) {                // norm. result and write back
        if (i < len) {
            out[i] = out_cpx[i][0] / (i+1);
        }
        else {
            out[i] = out_cpx[i][0] / (xCorrLen-i);
        }
    }

    fftw_free(wf1_cpx);
    fftw_free(wf2_cpx);
    fftw_free(out_cpx);
}


int findMaxIndex(double *corr, int len)
{
    double max = 0;
    int maxIdx;
    int trunc = len / 100 * 5;

    for (int i = trunc; i < len-trunc; i++) {
        if (corr[i] > max) {
            max = corr[i];
            maxIdx = i;
        }
    }

    return maxIdx;
}
