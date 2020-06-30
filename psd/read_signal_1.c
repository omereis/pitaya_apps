/*
Source: https://redpitaya.readthedocs.io/en/latest/appsFeatures/examples/acqRF-exm1.html
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*#include "rrpp.h"*/
#include "redpitaya/rp.h"

int main(int argc, char **argv)
{
	printf("Welcome to Red Pitaya reader\n");
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
    }
        /*LOOP BACK FROM OUTPUT 2 - ONLY FOR TESTING*/
/*
    rp_GenReset();
    rp_GenFreq(RP_CH_1, 10e3);
    rp_GenAmp(RP_CH_1, 1.0);
    rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
    rp_GenOutEnable(RP_CH_1);
*/
	printf ("Hit <Enter> key to terminate\n");
	getc(stdin);
	rp_AcqReset();
	rp_AcqSetDecimation(1);
	rp_AcqSetTriggerLevel(0.1); //Trig level is set in Volts while in SCPI
	rp_AcqSetTriggerDelay(0);

    rp_Release();
	printf("So long\n");
}
