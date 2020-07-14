#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WFLEN   32000
#define WFNUM   128
#define CHANNUM 3
#define CHANLEN 20000

void genPrbs(double *wf, int wflen);
void addNoise(double *wf, int wflen);
void saveAsCSV(char *filename, double *wf, int wflen);
void createDelay(double *wf, double *chwf, int chlen, int shift);



int main() {
    time_t t;
    srand((unsigned) time(&t));
    double waveform[WFLEN] = {0.0};
    double chanWF[CHANLEN] = {0.0};
    char foldername[100];
    char filename[100];
    char pathname[100];
    int shift = 0;

    for (int j = 0; j < CHANNUM; j++) {
        printf("channel %d ... ", j+1);
        fflush(stdout);

        sprintf(foldername, "ch%d/", j+1);
        shift = ((WFLEN - CHANLEN) / (CHANNUM-1)) * j;

        for (int i = 0; i < WFNUM; i++) {
            sprintf(filename, "wf%d.csv", i+1);
            strcpy(pathname, foldername);
            strcat(pathname, filename);

            genPrbs(waveform, WFLEN);
            addNoise(waveform, WFLEN);
            createDelay(waveform, chanWF, CHANLEN, shift);
            saveAsCSV(pathname, chanWF, CHANLEN);
        }
        puts("done!");
    }

    puts("waveforms created successfully!");
    return EXIT_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////
void genPrbs(double *wf, int wflen) {
    /*
     * PRBS 15 generator, hard coded
     */

	unsigned long vecSiz = 32767; // = 2^15 -1
	double *vecFloat = malloc(vecSiz * sizeof(double));
	
	unsigned long a = 1; 								// 4 bytes, 32 bits
    const unsigned long trunc = 65535; //2^16 -1       	// 4 bytes, 32 bits
	int newbit;

    for (int i = 0; i < vecSiz; i++) {
        newbit = (((a >> 13) ^ (a >> 14)) & 1);
       	vecFloat[i] = (double) newbit;
		a = ((a << 1) | newbit) & trunc;
    }

   	int j = -1;
    for (int i = 0; i < wflen; i++) {                   // create waveform with 4 samples / bit
        if (i % 4 == 0)
            j++;

        wf[i] = vecFloat[j];
    }

    free(vecFloat);
}


void addNoise(double *wf, int wflen) {
    int r;

    for (int i = 0 ; i < wflen; i++) {
        r = rand() % 100;
        wf[i] += (double)r * 0.001;     // noise from 0 to 0.1
        wf[i] -= 0.05;                  // noise from -0.05 to 0.05
    }
}


void saveAsCSV(char *filename, double *wf, int wflen) {
    FILE *fp = fopen(filename, "w");

    for (int i = 0; i < wflen; i++) {
        if (i == wflen-1)
            fprintf(fp, "%lf", wf[i]);
        else
            fprintf(fp, "%lf,", wf[i]);
    }

    fclose(fp);
}


void createDelay(double *wf, double *chwf, int chlen, int shift) {
    for (int i = 0; i < chlen; i++) {
        chwf[i] = wf[i+shift];
    }
}
