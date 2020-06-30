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
	/*getc(stdin);*/

	int nBuffSize;
	if (argc > 1)
		nBuffSize = atoi(argv[1]);
	else
		nBuffSize = 10000;
	uint32_t buff_size = nBuffSize;// 2000;//50;//16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));

	rp_AcqReset();
	rp_AcqSetDecimation(1);
	float rTrig;
	if (argc > 2)
		rTrig = atof(argv[2]);
	else
		rTrig = 1e-3;
	printf ("Trigger level set to %g\n", rTrig);
	rp_AcqSetTriggerLevel(RP_CH_1, rTrig); //Trig level is set in Volts while in SCPIcqSetTriggerLevel:e!

	//rp_AcqSetTriggerLevel(RP_CH_2,0.005); //Trig level is set in Volts while in SCPI
	rp_AcqSetTriggerDelay(0);
	//rp_AcqSetTriggerDelay(buff_size);

	//sleep(1);
	usleep(1);

	uint32_t nPreCounter;
	rp_AcqGetPreTriggerCounter (&nPreCounter);
	printf ("pre-counter size: %d\n", nPreCounter);

	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	while(1){
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			break;
		}
	}

	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
	char *szFileName;
	if (argc > 3)
		szFileName = argv[3];
	else
		szFileName = "adc.csv";
	int i;
	//FILE *file = fopen ("adc.csv", "w+");
	FILE *file = fopen (szFileName, "w+");
	for(i = 0; i < buff_size; i++) {
		fprintf(file, "%f\n", buff[i]);
		/*printf("%f\n", buff[i]);*/
	}
	fclose (file);
	printf("'%s' file written\n", szFileName);
        /* Releasing resources */

    rp_Release();
	free(buff);
	printf("So long\n");
}
