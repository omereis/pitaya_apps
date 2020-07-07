/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "redpitaya/rp.h"

//-----------------------------------------------------------------------------
void set_params_defaults (float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
void print_usage();
void print_params (float rTrigger, int nSamples, int nDelay, char *szFile);
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	float rTrigger;
	int nSamples, nDelay;
	short fHelp;
	char *szFile;

	get_options (argc, argv, &rTrigger, &nSamples, &nDelay, &fHelp, &szFile);
	if (fHelp) {
		print_usage();
		exit(0);
	}

	print_params (rTrigger, nSamples, nDelay, szFile);
        /* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

        /*LOOB BACK FROM OUTPUT 2 - ONLY FOR TESTING*/
	uint32_t buff_size = nSamples;//6250;//12500;//16384;//8192;//16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));
	float *big_buff = (float*) calloc(2 * buff_size, sizeof(big_buff[0]));

	rp_AcqReset();
	if (rp_AcqSetDecimation(RP_DEC_1/*1*/) != RP_OK)
		printf("Error setting dec9imation\n");;
	if (rp_AcqSetSamplingRate(RP_SMP_125M) != RP_OK)
		printf ("Setting sampleing rate error\n");
//	rp_AcqSetTriggerLevel(RP_CH_1, 10e-3); //Trig level is set in Volts while in SCPI
	rp_AcqSetTriggerLevel(RP_CH_1, rTrigger); //Trig level is set in Volts while in SCPI
//	rp_AcqSetTriggerDelay(5000);

/*
	if (argc >= 2)
		nDelay = atoi(argv[1]);
	else
		nDelay = 1000;
*/
	//rp_AcqSetTriggerDelay(1000);
	rp_AcqSetTriggerDelay(nDelay);
//	printf ("\nDelay: %d\n", nDelay);

        // there is an option to select coupling when using SIGNALlab 250-12
        // rp_AcqSetAC_DC(RP_CH_1, RP_AC); // enables AC coupling on channel 1

        // by default LV level gain is selected
        // rp_AcqSetGain(RP_CH_1, RP_LOW); // user can switch gain using this command

	rp_AcqStart();

        /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
        /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
        /*length and smaling rate*/

	sleep(1);
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
	int nWaits=1;
	time_t tStart, tNow;
	bool fTrigger, fTimeLimit;

	time(&tStart);
	fTrigger = fTimeLimit = false;
	/*while(1){*/
	while((!fTrigger) && (!fTimeLimit)){
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			fTrigger = true;
			/*break;*/
		}
		usleep(1);
		time(&tNow);
		if (difftime (tNow, tStart) >= 15)
			fTimeLimit = true;
			/*break;*/
		nWaits++;
	}

	if (fTrigger)
		printf("Trigger Occurred\n");
	if (fTimeLimit)
		printf ("Time Limit Reached\n");
	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
	memcpy (big_buff, buff, sizeof(buff[0]) * buff_size);
	FILE *fout;
	int i;
	fout = fopen (szFile, "w+");
//	fout = fopen ("out.csv", "w+");
	for(i = 0; i < buff_size; i++){
		fprintf(fout, "%f\n", buff[i]);
	}
	fclose (fout);
	printf ("Read once, after %d polls\n", nWaits);
/* end of 1st read */

	free(buff);
	rp_Release();

	return 0;
}
//-----------------------------------------------------------------------------
void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile)
{
	int c;

	set_params_defaults (prTrigger, pnSamples, pnDelay, pfHelp, pszFile);

	while ((c = getopt (argc, argv, "ht:n:f:d:")) != -1)
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
			case 'd':
			case 'D':
				*pnDelay = atoi(optarg);
				break;
			default:
				printf("Unfamiliar option: %c\n", c);
				*pfHelp = 1;
				return;
		}
} 
//-----------------------------------------------------------------------------
void set_params_defaults (float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile)
{
	*prTrigger = 10e-3;
	*pnSamples = 12500;
	*pnDelay = 1250;
	*pfHelp = 0;
	*pszFile = "out.csv";
}
//-----------------------------------------------------------------------------
void print_usage()
{
	char *szMessage = "Red Pitaya RF input\n"
					"Synopsis:\n"
					"./rp_read -t <trigger [volts]> -n <# of samples> -f <output file name> -d <delay items>\n"
					"  Defaults:\n"
					"    Trigger: 10mV:\n"
					"    Samples: 10,000\n"
					"    Delay  : 1250 data points\n"
					"    File   : out.csv\n";
	printf ("%s\n", szMessage);
}
//-----------------------------------------------------------------------------
void print_params (float rTrigger, int nSamples, int nDelay, char *szFile)
{
	char *szMessage = "Red Pitaya RF input\n"
					"    Trigger: %gmV:\n"
					"    Samples: %d points\n"
					"    Delay  : %d data points\n"
					"    File   : %s\n";
	printf (szMessage, rTrigger, nSamples, nDelay, szFile);
}
//-----------------------------------------------------------------------------
