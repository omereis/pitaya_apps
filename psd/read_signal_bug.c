/*
Source: https://redpitaya.readthedocs.io/en/latest/appsFeatures/examples/acqRF-exm1.html
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "redpitaya/rp.h"

//-----------------------------------------------------------------------------
void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, short *pfHelp, char **pszFile)
{
	int c;
	*prTrigger = 0, *pnSamples = 0, *pfHelp = 0, *pszFile = NULL;

	while ((c = getopt (argc, argv, "ht:n:f:")) != -1)
		switch (c) {
			case 'H':
			case 'h':
				*pfHelp = 1;
				return;
			case 't':
			case 'T':
				*prTrigger = atof (optarg);
				break;
			case 'n':
			case 'N':
				*pnSamples = atoi (optarg);
				break;
			case 'f':
			case 'F':
				*pszFile = optarg;
				break;
			default:
				printf("Unfamiliar option: %c\n", c);
				*pfHelp = 1;
				return;
		}
}
//-----------------------------------------------------------------------------
void print_usage()
{
	char *szMessage = "Red Pitaya RF input\n"
					"Synopsis:\n"
					"./read_signal -t <trigger [volts]> -n <# of samples> -f <output file name>\n"
					"  Defaults:\n"
					"    Trigger: 10mV:\n"
					"    Samples: 10,000\n"
					"    File: out.csv\n";
	printf ("%s\n", szMessage);
}
//-----------------------------------------------------------------------------
void print_options (float rTrigger, int nSamples, char *szFile)
{
	char *szMessageFormat = "Reader Options:\n"
							" Trigger: %g milli Volts\n"
							" Buffer : %d\n"
							" File   : %s\n";
	printf (szMessageFormat, rTrigger * 1e3, nSamples, szFile);
}
//-----------------------------------------------------------------------------
double clock_time (clock_t tStart, clock_t tEnd)
{
	double d = ((double) (tEnd - tStart)) / ((double) (CLOCKS_PER_SEC));
	return (d);
}
//-----------------------------------------------------------------------------
clock_t g_cTriggerSet = 0, g_cTriggered = 0;
void read_adc_samples (float buff[], float rTrigger, uint32_t buff_size)
{
	for (int n=0 ; n < buff_size ; n++)
		buff[n] = 0;
	rp_AcqStart();

	//sleep(1);
	usleep(1);
	float rHyst;
	int nErr;
	rp_AcqSetTriggerHyst(rTrigger / 2);
	if ((nErr = rp_AcqGetTriggerHyst (&rHyst)) == 0)
		printf ("Trigger hysteresis is %g\n", rHyst);
	else
		printf ("Trigger hysteresis reading error %d\n", nErr);

	g_cTriggerSet = clock ();
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	while(1) {
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			break;
		}
	}
	g_cTriggered = clock();
	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
}
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	float rTrigger;
	int nSamples;
	short fHelp;
	char *szFile;

	get_options (argc, argv, &rTrigger, &nSamples, &fHelp, &szFile);

	if (fHelp) {
		print_usage();
		exit (0);
	}
	else {
		print_options (rTrigger, nSamples, szFile);
	}

	printf("Welcome to Red Pitaya reader\n");
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
    }
        /*LOOP BACK FROM OUTPUT 2 - ONLY FOR TESTING*/

	uint32_t buff_size = nSamples;//16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));

	rp_AcqReset();
	rp_AcqSetDecimation(1);
	rp_AcqSetTriggerLevel(RP_CH_1, rTrigger); //Trig level is set in Volts while in SCPI
	//rp_AcqSetTriggerLevel(RP_CH_1, 0.01); //Trig level is set in Volts while in SCPI
	//rp_AcqSetTriggerDelay(nSamples);
	rp_AcqSetTriggerDelay(0);

        // there is an option to select coupling when using SIGNALlab 250-12
        // rp_AcqSetAC_DC(RP_CH_1, RP_AC); // enables AC coupling on channel 1

        // by default LV level gain is selected
        // rp_AcqSetGain(RP_CH_1, RP_LOW); // user can switch gain using this command
/*
	float *big_buff = (float*) calloc(2 * buff_size, sizeof (big_buff[0]));
	double dReadTime, dOverhead;
	clock_t c, cCopy;
	clock_t cStart=clock();
	printf ("Reading, #1\n");
	read_adc_samples (buff, rTrigger, buff_size);
	dReadTime = clock_time (clock(), cStart);
	c = clock();
	memcpy (big_buff, buff, sizeof(buff[0]) * buff_size);
	cCopy = clock() - c;
	c = clock();
	rp_AcqSetTriggerLevel(RP_CH_1, rTrigger); //Trig level is set in Volts while in SCPI
	printf ("Reading, #2\n");
	read_adc_samples (buff, rTrigger, buff_size);
	dOverhead = clock_time (g_cTriggerSet, g_cTriggered);
	memcpy (&big_buff[buff_size], buff, sizeof(buff[0]) * buff_size);
	printf ("Total time: %g\n", clock_time(clock(), cStart));
	printf ("Overhead time: %g\n", dOverhead);
	printf ("Copy time: %g\n", clock_time (cCopy, 0));
	printf ("A/D Read time: %g\n", dReadTime);
*/
/**/
	rp_AcqStart();

	** After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer
		Here we have used time delay of one second but you can calculate exact value taking in to account buffer
		length and smaling rate**

	//sleep(1);
	usleep(1);
	float rHyst;
	int nErr;
	rp_AcqSetTriggerHyst(rTrigger / 2);
	if ((nErr = rp_AcqGetTriggerHyst (&rHyst)) == 0)
		printf ("Trigger hysteresis is %g\n", rHyst);
	else
		printf ("Trigger hysteresis reading error %d\n", nErr);

	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	while(1) {
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			break;
		}
	}

	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
/**/
	int i;
	//FILE *file = fopen ("adc.csv", "w+");
	FILE *file = fopen (szFile, "w+");

	for(i = 0; i < buff_size; i++){
		//printf("%f\n", buff[i]);
		fprintf(file, "%f\n", buff[i]);
	}
	fclose (file);
	printf ("File %s written\n", szFile);
        /* Releasing resources */
	rp_AcqStop();
	free(buff);
	rp_Release();
	printf("So long\n");
}
