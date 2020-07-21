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
int read_input_volts (float *buff, uint32_t buff_size, int *pnWaits, struct InputParams *in_params);
//int read_input (float *buff, uint32_t buff_size, int *pnWaits, struct InputParams *in_params);
void set_params_defaults (struct InputParams *in_params);
//void set_params_defaults (float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
//void get_options (int argc, char **argv, float *prTrigger, int *pnSamples, int *pnDelay, short *pfHelp, char **pszFile);
void get_options (int argc, char **argv, struct InputParams *in_params);
void print_usage();
void print_params (struct InputParams *in_params);
void print_mote_buffer (float *buff, uint32_t buff_size, char *szFile);
//void print_params (float rTrigger, int nSamples, int nDelay, char *szFile);

void normalize_buff_float (float *buff, uint32_t buff_size);
void print_buffer_volts (float *buff, uint32_t buff_size, char *szFile);
void calc_histogram (float *adResults, uint32_t nSize);
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
	//float *buff;// = (float *)malloc(buff_size * sizeof(float));
	uint16_t *buff;
	float *afBuff;

//	buff =  (uint16_t *)malloc(buff_size * sizeof(buff[0]));
	afBuff = (float*) calloc(buff_size, sizeof(afBuff[0]));

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
	int nWaits, nValids/*, fPrint, iBiggest*/;
	double d = ((double) in_params.Delay * -1.0) + 8188.0;
	float dSum=0;
//	int nIterations = in_params.Iterations;//5;
	int j, k, nStart = (int) d;//(nDelay * -124.9) + 8188, fPrint; // from measurements
	float *adResults, dHistMin=0, dHistMax=0, *adMax, dSamplesMax, dBiggest;
	char sz[1024];

	adResults = calloc (in_params.Iterations, sizeof (adResults[0]));
	adMax = calloc (in_params.Iterations, sizeof (adResults[0]));
	adMax[0] = 17;
	printf("d=%g, nStart=%d, buf_size=%d\n", d, nStart, buff_size);
	nStart = 0;
	for (k=0, nValids=0 ; k < in_params.Iterations ; k++) {
		if (read_input_volts (afBuff, buff_size, &nWaits, &in_params) > 0) {
			normalize_buff_float (afBuff, buff_size);
			sprintf (sz, "n%02d.csv", k+1);
//			print_buffer_volts (afBuff, buff_size, sz);

			for (j=nStart ; j < buff_size ; j++) {
				if (j == nStart)
					dBiggest = dSum = dSamplesMax = afBuff[0];
				else {
					dSum += afBuff[j];
					dSamplesMax = max (dSamplesMax, afBuff[j]);
					if (dSamplesMax > dBiggest) {
						dBiggest = dSamplesMax;
//						iBiggest = k;
					}
				}
			}
//			fPrint = false;//dSum > 10e3; .// don't print for now
			adResults[k]= dSum;
			adMax[k] = dSamplesMax;
			nValids++;
//			if (fPrint) {
//				sprintf (sz, "big%d.csv", k+1);
//				print_buffer (buff, buff_size, sz);
//			}
/**/
		}
		memset (afBuff, 0, buff_size * sizeof (buff[0]));
		if ((k % 100) == 0)
			fprintf (stderr, "Completed %d iterations, Max: %g, Min: %g\r", k, dHistMax, dHistMin);
	}
	fprintf (stderr, "===============================\n");
	fprintf (stderr, "Reading done after %d iterations\n", in_params.Iterations);
	fprintf (stderr, "===============================\n");
	calc_histogram (adResults, buff_size);
/*
	for (k=0 ; k < nValids ; k++) {
		dSum = adResults[k];
		if (k == 0)
			dHistMin = dHistMax = dSum;
		else {
			if (dSum < dHistMin) {
				dHistMin = dSum;
			}
			if (dSum > dHistMax) {
				dHistMax = dSum;
			}
		}
	}
*/
	printf ("\n");

/*
	int *anHistogram, nSize;
	int n, idx;
	fprintf (stderr, "Iterations: %d\n", in_params.Iterations);
//	fprintf (stderr, "Size: %d\n", sizeof (anHistogram[0]));
//	nSize = in_params.Iterations * sizeof (anHistogram[0]);
	printf ("Allocation size: %d\n", nSize);
//	anHistogram = (int*) malloc (nSize);
	fprintf (stderr, "Histogram Memory Allocated\n");
	fprintf (stderr, "Histogram Memory Freed\n");
	double dBin = (dHistMax - dHistMin) / 1024.0;
	printf ("===============================\n");
	printf("\nMinimum: %g,\nMaximum: %g\nBin: %g\n", dHistMin, dHistMax, dBin);
	printf("%d valid iterations out of %d\n", nValids, in_params.Iterations);
	printf ("===============================\n");

	for (n=0 ; n < nValids ; n++)
		anHistogram[n] = 0;
	for (n=0 ; n < nValids ; n++) {
		idx = (int) (adResults[n] / dBin);
		if (idx < nValids)
			anHistogram[n] = anHistogram[n] + 1;
	}

	FILE *fHist = fopen ("hist.csv", "w+");
	for (n=0 ; n < 1024 ; n++)
		fprintf (fHist, "%d\n", anHistogram[n]);
	fclose (fHist);
	free (anHistogram);
*/
/*
*/
//	memcpy (big_buff, buff, sizeof(buff[0]) * buff_size);
//	printf ("Read once\n");
	print_buffer_volts (adResults, in_params.Iterations, "sums.csv");
	print_buffer_volts (adMax, in_params.Iterations, "max.csv");
	//print_buffer (buff, buff_size, in_params.FileName);

/* end of 1st read */

//	free (adResults);
//	free (afBuff);
//	rp_Release();

	return 0;
}
int nDebug=1;
//-----------------------------------------------------------------------------
int read_input_volts (float *buff, uint32_t buff_size, int *pnWaits, struct InputParams *in_params)
{
	*pnWaits=1;
	time_t tStart, tNow;
	bool fTrigger, fTimeLimit;

//	rp_AcqReset ();


	if (rp_AcqSetDecimation(RP_DEC_1) != RP_OK)
		printf("Error setting decimation\n");;
	if (rp_AcqSetSamplingRate(RP_SMP_125M) != RP_OK)
		printf ("Setting sampleing rate error\n");

	rp_AcqSetTriggerLevel(RP_CH_1, in_params->Trigger); //Trig level is set in Volts while in SCPI

	rp_AcqSetTriggerDelay(in_params->Delay);

	rp_AcqStart ();

	usleep(100);

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
	rp_AcqStop ();
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
void normalize_buff (uint32_t *buff, uint32_t buff_size)
{
	int n;
	uint32_t fMin = 0;

	for (n=0 ; n < buff_size ; n++)
		fMin = min (fMin, buff[n]);
	for (int n=0 ; n < buff_size ; n++)
		buff[n] = buff[n] - fMin;
}
//-----------------------------------------------------------------------------
void normalize_buff_float (float *buff, uint32_t buff_size)
{
	int n;
	float fMin = 1e300;

//	for (n=0 ; n < buff_size ; n++)
//		if (buff[n] < 0)
//			buff[n] = 0;
	for (n=0 ; n < buff_size ; n++)
		fMin = min (fMin, buff[n]);
	for (int n=0 ; n < buff_size ; n++)
		buff[n] = buff[n] - fMin;
}
//-----------------------------------------------------------------------------
void print_buffer_volts (float *buff, uint32_t buff_size, char *szFile)
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
		fprintf(fout, "%g\n", buff[n]);
	}
	fclose (fout);
}
//-----------------------------------------------------------------------------
void calc_histogram (float *adResults, uint32_t nSize)
{
	int n, idx;
	int *anHistogram;
	float rMax, rMin, rBin;

	anHistogram = (int*) calloc (nSize, sizeof (anHistogram[0]));
	rMin = rMax = adResults[0];
	for (n=1 ; n < nSize ; n++) {
		rMin = min(rMin, adResults[n]);
		rMax = max(rMax, adResults[n]);
	}
	rBin = (rMax - rMin) / 1024;
	for (n=0 ; n < nSize ; n++) {
		idx = (int) (adResults[n] / rBin);
		anHistogram[idx] = anHistogram[idx] + 1;
	}
	FILE *file;
	file = fopen ("hist.csv", "w+");
	for (n=0 ; n < 1024 ; n++)
		fprintf (file, "%d\n", anHistogram[n]);
	fclose(file);
	free (anHistogram);
}
//-----------------------------------------------------------------------------
