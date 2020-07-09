/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "redpitaya/rp.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//-----------------------------------------------------------------------------
struct InputParams {
	short Help;
	int Samples;
	float Trigger;
	int Delay;
	char *FileName;
	int Iterations;
};
//-----------------------------------------------------------------------------
void read_input (float *buff, uint32_t buff_size, int *pnWaits);
void set_params_defaults (struct InputParams *in_params);
//void set_params_defaults (float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
//void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
void get_options (int argc, char **argv, struct InputParams *in_params);
void print_usage();
void print_params (struct InputParams *in_params);
//void print_params (float rTrigger, int nSamples, int nDelay, char *szFile);

void normalize_buff (float *buff, uint32_t buff_size);
void print_buffer (float *buff, uint32_t buff_size, char *szFile);
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct InputParams in_params;

	get_options (argc, argv, &in_params);
	if (in_params.Help) {
		print_usage();
		exit(0);
	}

//	print_params (rTrigger, nSamples, nDelay, szFile);
	print_params (&in_params);
        /* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

        /*LOOB BACK FROM OUTPUT 2 - ONLY FOR TESTING*/
	uint32_t buff_size = in_params.Samples;//6250;//12500;//16384;//8192;//16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));

	rp_AcqReset();
	if (rp_AcqSetDecimation(RP_DEC_1/*1*/) != RP_OK)
		printf("Error setting decimation\n");;
	if (rp_AcqSetSamplingRate(RP_SMP_125M) != RP_OK)
		printf ("Setting sampleing rate error\n");
//	rp_AcqSetTriggerLevel(RP_CH_1, 10e-3); //Trig level is set in Volts while in SCPI
	rp_AcqSetTriggerLevel(RP_CH_1, in_params.Trigger); //Trig level is set in Volts while in SCPI
//	rp_AcqSetTriggerDelay(5000);

	rp_AcqSetTriggerDelay(in_params.Delay);

        // there is an option to select coupling when using SIGNALlab 250-12
        // rp_AcqSetAC_DC(RP_CH_1, RP_AC); // enables AC coupling on channel 1

        // by default LV level gain is selected
        // rp_AcqSetGain(RP_CH_1, RP_LOW); // user can switch gain using this command

	rp_AcqStart();

        /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
        /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
        /*length and smaling rate*/

	sleep(1);
	int nWaits;
	double d = ((double) in_params.Delay * -1.0) + 8188.0;
	int nIterations = in_params.Iterations;//5;
	int j, k, nStart = (int) d;//(nDelay * -124.9) + 8188; // from measurements
	float dSum=0, *adResults;
	char sz[1024];

	adResults = calloc (nIterations, sizeof (adResults[0]));
	printf("d=%g, nStart=%d, buf_size=%d\n", d, nStart, buff_size);
	for (k=0 ; k < nIterations ; k++) {
		read_input (buff, buff_size, &nWaits);
		normalize_buff (buff, buff_size);
		for (j=nStart, dSum=0 ; j < buff_size ; j++) {
			dSum += buff[j];
		}
		adResults[k]= dSum;
		sprintf (sz, "out%d.csv", k+1);
//		printf ("%d: %f\n", k, dSum);
		if (k < 0)
			print_buffer (buff, buff_size, sz);
//		printf ("%d times read. Sum: %g\n", k+1, dSum);
		if (k % 10 == 0)
			printf ("\r%d iterations...", k);
		memset (buff, 0, buff_size * sizeof (buff[0]));
	}
	printf ("\n");

//	memcpy (big_buff, buff, sizeof(buff[0]) * buff_size);
//	printf ("Read once\n");
	print_buffer (adResults, nIterations, "sums.csv");
	print_buffer (buff, buff_size, in_params.FileName);

/* end of 1st read */

	free (adResults);
	free(buff);
	rp_Release();

	return 0;
}
//-----------------------------------------------------------------------------
void read_input (float *buff, uint32_t buff_size, int *pnWaits)
{
	*pnWaits=1;
	time_t tStart, tNow;
	bool fTrigger, fTimeLimit;

	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
	time(&tStart);
	fTrigger = fTimeLimit = false;
	while((!fTrigger) && (!fTimeLimit)){
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			fTrigger = true;
		}
		usleep(1);
		time(&tNow);
		if (difftime (tNow, tStart) >= 15)
			fTimeLimit = true;
		(*pnWaits)++;
	}
//	if (fTrigger)
//		printf("Trigger Occurred\n");
//	if (fTimeLimit)
//		printf ("Time Limit Reached\n");
	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
}
//-----------------------------------------------------------------------------
//void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile)
//-----------------------------------------------------------------------------
void get_options (int argc, char **argv, struct InputParams *in_params)
{
	int c;

//	set_params_defaults (prTrigger, pnSamples, pnDelay, pfHelp, pszFile);
	set_params_defaults (in_params);
//	memset (in_params, 0, sizeof(*in_params));

	while ((c = getopt (argc, argv, "ht:n:f:d:i:")) != -1)
		switch (c) {
			default:
			case 'H':
			case 'h':
//				*pfHelp = 1;
				in_params->Help = 1;
				return;
			case 't':
			case 'T':
//				*prTrigger = atof (optarg);
				in_params->Trigger = atof (optarg);
				break;
			case 'n':
			case 'N':
//				*pnSamples = atoi (optarg);
				in_params->Samples = atoi (optarg);
				break;
			case 'f':
			case 'F':
				in_params->FileName = optarg;
//				*pszFile = optarg;
				break;
			case 'd':
			case 'D':
				in_params->Delay = atoi(optarg);
//				*pnDelay = atoi(optarg);
				break;
			case 'i':
			case 'I':
				in_params->Iterations = atoi(optarg);
				break;
		}
} 
//-----------------------------------------------------------------------------
void set_params_defaults (struct InputParams *in_params)
//void set_params_defaults (float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile)
{
	in_params->Trigger = 10e-3;
	in_params->Samples = 12500;
	in_params->Delay = 1250;
	in_params->Help = 0;
	in_params->FileName = "out.csv";
	in_params->Iterations = 10;
}
//-----------------------------------------------------------------------------
void print_usage()
{
	char *szMessage = "Red Pitaya RF input\n"
					"Synopsis:\n"
					"./rp_read -t <trigger [volts]> -n <# of samples> -f <output file name> -d <delay items> -i <# of iterations>\n"
					"  Defaults:\n"
					"    Trigger: 10mV:\n"
					"    Samples: 10,000\n"
					"    Delay  : 1250 data points\n"
					"    File   : out.csv\n"
					"    Iterations: 10\n";
	printf ("%s\n", szMessage);
}
//-----------------------------------------------------------------------------
//void print_params (float rTrigger, int nSamples, int nDelay, char *szFile)
void print_params (struct InputParams *in_params)
{
	char *szMessage = "Red Pitaya RF input\n"
					"    Trigger: %gmV:\n"
					"    Samples: %d points\n"
					"    Delay  : %d data points\n"
					"    File   : %s\n"
					"    Iterations: %d\n";
	printf (szMessage, in_params->Trigger, in_params->Samples, in_params->Delay, in_params->FileName, in_params->Iterations);
}
//-----------------------------------------------------------------------------
void normalize_buff (float *buff, uint32_t buff_size)
{
	int n;
	float fMin = 1e300;

	for (n=0 ; n < buff_size ; n++)
		fMin = min (fMin, buff[n]);
//	printf ("Buffer's minimum: %f\n", fMin);
	for (int n=0 ; n < buff_size ; n++)
		buff[n] = buff[n] - fMin;
}
//-----------------------------------------------------------------------------
void print_buffer (float *buff, uint32_t buff_size, char *szFile)
{
	FILE *fout;
	int n;

	fout = fopen (szFile, "w+");
	for(n = 0; n < buff_size; n++){
		fprintf(fout, "%f\n", buff[n]);
	}
	fclose (fout);
}
//-----------------------------------------------------------------------------
