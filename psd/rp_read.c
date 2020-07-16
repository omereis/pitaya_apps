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
int read_input (float *buff, uint32_t buff_size, int *pnWaits, struct InputParams *in_params);
void set_params_defaults (struct InputParams *in_params);
//void set_params_defaults (float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
//void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
void get_options (int argc, char **argv, struct InputParams *in_params);
void print_usage();
void print_params (struct InputParams *in_params);
void print_mote_buffer (float *buff, uint32_t buff_size, char *szFile);
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
	int nWaits, nValids, fPrint, iBiggest;
	double d = ((double) in_params.Delay * -1.0) + 8188.0;
	float dSum=0;
//	int nIterations = in_params.Iterations;//5;
	int j, k, nStart = (int) d;//(nDelay * -124.9) + 8188, fPrint; // from measurements
	float *adResults, dHistMin=0, dHistMax=0, dSamplesMax, *adMax, dBiggest;
	char sz[1024];

	adResults = calloc (in_params.Iterations, sizeof (adResults[0]));
	adMax = calloc (in_params.Iterations, sizeof (adResults[0]));
	printf("d=%g, nStart=%d, buf_size=%d\n", d, nStart, buff_size);
	for (k=0, nValids=0 ; k < in_params.Iterations ; k++) {
		if (read_input (buff, buff_size, &nWaits, &in_params) > 0) {
			sprintf (sz, "b%02d.csv", k+1);
			print_buffer (buff, buff_size, sz);
			normalize_buff (buff, buff_size);
			sprintf (sz, "n%02d.csv", k+1);
			print_buffer (buff, buff_size, sz);
			for (j=nStart/*, dSum=0*/ ; j < buff_size ; j++) {
				if (j == 0)
					dBiggest = dSum = dSamplesMax = buff[0];
				else {
					dSum += (double) buff[j];
					dSamplesMax = max (dSamplesMax, buff[j]);
					if (dSamplesMax > dBiggest) {
						dBiggest = dSamplesMax;
						iBiggest = k;
					}
				}
			}
			fPrint = false;//dSum > 10e3; .// don't print for now
			adResults[k]= dSum;
			adMax[k] = dSamplesMax;
			nValids++;
			if (fPrint) {
				sprintf (sz, "big%d.csv", k+1);
				print_buffer (buff, buff_size, sz);
			}
		}
//		sprintf (sz, "smpl%d.csv", k+1);
//		printf ("%d: %f\n", k, dSum);
//		print_mote_buffer (buff, buff_size, "outs.csv");
//		if ((k % 500) == 0)
//			print_buffer (buff, buff_size, sz);
//		printf ("%d times read. Sum: %g\n", k+1, dSum);
//		if (k % 10 == 0)
//			printf ("\r%d iterations...", k);
		memset (buff, 0, buff_size * sizeof (buff[0]));
		if ((k % 100) == 0)
			fprintf (stderr, "Completed %d iterations, Max: %g, Min: %g\r", k, dHistMax, dHistMin);
	}
	fprintf (stderr, "===============================\n");
	printf ("Biggest in index %d\n", iBiggest);
	fprintf (stderr, "Reading done after %d iterations\n", in_params.Iterations);
	fprintf (stderr, "===============================\n");
//	getchar();
	for (k=0 ; k < nValids/*in_params.Iterations*/ ; k++) {
		dSum = adResults[k];
//		fprintf(stderr, "Iteration %d, sum: %g\n", k, dSum);
/**/
		if (k == 0)
			dHistMin = dHistMax = dSum;
		else {
			if (dSum < dHistMin) {
//				printf ("Min at %d\n", k);
				dHistMin = dSum;
			}
			if (dSum > dHistMax) {
//				printf ("Max at %d\n", k);
				dHistMax = dSum;
			}
		}
/**/
	}
	printf ("\n");
	int *anHistogram, idx, n, nSize;
	fprintf (stderr, "Iterations: %d\n", in_params.Iterations);
	fprintf (stderr, "Size: %d\n", sizeof (anHistogram[0]));
	nSize = in_params.Iterations * sizeof (anHistogram[0]);
	printf ("Allocation size: %d\n", nSize);
	anHistogram = calloc (in_params.Iterations, sizeof (anHistogram[0]));
	printf ("Histogram Memory Allocated\n");
	double dBin = (dHistMax - dHistMin) / 1024.0;
	printf ("===============================\n");
	printf("\nMinimum: %g,\nMaximum: %g\nBin: %g\n", dHistMin, dHistMax, dBin);
	printf("\%d valid iterations out of %d\n", nValids, in_params.Iterations);
	printf ("===============================\n");
	for (n=0 ; n < nValids/*in_params.Iterations*/ ; n++)
		anHistogram[n] = 0;
	for (n=0 ; n < in_params.Iterations ; n++) {
		idx = (int) (adResults[n] / dBin);
		if (idx < in_params.Iterations)
			anHistogram[n] = anHistogram[n] + 1;
	}
	FILE *fHist = fopen ("hist.csv", "w+");
	for (n=0 ; n < 1024 ; n++)
		fprintf (fHist, "%d\n", anHistogram[n]);
	fclose (fHist);
	free (anHistogram);
//	memcpy (big_buff, buff, sizeof(buff[0]) * buff_size);
//	printf ("Read once\n");
	print_buffer (adResults, in_params.Iterations, "sums.csv");
	print_buffer (adMax, in_params.Iterations, "smpmx.csv");
	print_buffer (buff, buff_size, in_params.FileName);

/* end of 1st read */

	free (adResults);
	free(buff);
	rp_Release();

	return 0;
}
int nDebug=1;
//-----------------------------------------------------------------------------
int read_input (float *buff, uint32_t buff_size, int *pnWaits, struct InputParams *in_params)
{
	*pnWaits=1;
	time_t tStart, tNow;
	bool fTrigger, fTimeLimit;

//	fprintf (stderr, "\nread_input #%d\n", nDebug);

//	rp_AcqReset ();

//	fprintf (stderr, "Reset %d times\n", nDebug);

/*
*/
	if (rp_AcqSetDecimation(RP_DEC_1) != RP_OK)
		printf("Error setting decimation\n");;
//	fprintf (stderr, "Decimationset %d times\n", nDebug);
	if (rp_AcqSetSamplingRate(RP_SMP_125M) != RP_OK)
		printf ("Setting sampleing rate error\n");

	rp_AcqSetTriggerLevel(RP_CH_1, in_params->Trigger); //Trig level is set in Volts while in SCPI

	rp_AcqSetTriggerDelay(in_params->Delay);

	rp_AcqStart ();

//	fprintf (stderr, "starting #%d\n", nDebug);

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
	if (fTimeLimit)
		printf ("Time Limit Reached\n");
	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
//	fprintf (stderr, "read data\n");
	rp_AcqStop ();
//	fprintf (stderr, "Stop #%d\n", nDebug);
	nDebug++;
	if (fTimeLimit)
		return (0);
	else
		return (1);
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
void print_mote_buffer (float *buff, uint32_t buff_size, char *szFile)
{
	FILE *fout;
	int n;

	fout = fopen (szFile, "a+");
	for(n = 0; n < buff_size; n++){
		fprintf(fout, "%f\n", buff[n]);
	}
	fclose (fout);
}
//-----------------------------------------------------------------------------
