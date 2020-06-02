/**
 * $Id: acquire.c 1246 2014-02-22 19:05:19Z ales.bardorfer $
 *
 * @brief Red Pitaya simple signal acquisition utility.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *         Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
/*
	Updates by Omer Eisenberg
	omereis@yahoo.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/param.h>

#include "main_osc.h"
#include "fpga_osc.h"
#include "redpitaya/version.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The code below acquires up to 16k samples on both Red Pitaya input
 * channels labeled ADC1 & ADC2.
 * 
 * It utilizes the routines of the Oscilloscope module for:
 *   - Triggered ADC signal acqusition to the FPGA buffers.
 *   - Parameter defined averaging & decimation.
 *   - Data transfer to SW buffers.
 *
 * Although the Oscilloscope routines export many functionalities, this 
 * simple signal acquisition utility only exploits a few of them:
 *   - Synchronization between triggering & data readout.
 *   - Only AUTO triggering is used by default.
 *   - Only decimation is parsed to t_params[8].
 *
 * Please feel free to exploit any other scope functionalities via 
 * parameters defined in t_params.
 *
 */

/*
    Update Noets
    may 24, OE: adding read_params
*/

struct rp_params {
    int nEqual;
    int nShaping;
    int nGainCh1;
    int nGainCh2;
    int nVersion;
    int nHelp;
    int idxDecimation;
    int nSize;
    char *szFileName;
    int sizeStart;
    int sizeEnd;
    int sizeDelta;
};
/** Program name */
const char *g_argv0 = NULL;
double dClocksPerSec;

/** Minimal number of command line arguments */
#define MINARGS 2

/** Oscilloscope module parameters as defined in main module
 * @see rp_main_params
 */
float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/** Max decimation index */
#define DEC_MAX 6

/** Decimation translation table */
static int g_dec[DEC_MAX] = { 1,  8,  64,  1024,  8192,  65536 };

/*****************************************************************************/
/*****************************************************************************/
//#define	SWAP (X, Y)	(X = X + Y; Y = X - Y ; X = X - Y;)
void swap (double *pd1, double *pd2)
{
	double d = *pd1;
	*pd1 = *pd2;
	*pd2 = d;
}
/*****************************************************************************/
int partition (double adValues[], int nLow, int nHigh)
// source: https://www.geeksforgeeks.org/quick-sort/
{
	double dPivot = adValues[nHigh];
	int j, i = nLow - 1;

	for (j=nLow ; j < nHigh ; j++) {
		if (adValues[j] < dPivot) {
			i++;
			swap (&adValues[j], &adValues[i]);
/*
			dSwap = adValues[j];
			adValues[j] = adValues[i];
			adValues[i] = dSwap;
*/
		}
	}
	swap (&adValues[i+1], &adValues[nHigh]);
	return (i + 1);
}
/*****************************************************************************/
void quicksort (double adValues[], int nLow, int nHigh)
{
	int idxPartition;

 	if (nLow < nHigh) {
		idxPartition = partition (adValues, nLow, nHigh);

		quicksort(adValues, nLow, idxPartition - 1);
		quicksort(adValues, idxPartition, nHigh);

	}
}
/*****************************************************************************/
char *strlwr (char *sz) {
	int n;

	if (sz != NULL) {
		for (n=0 ; n < strlen(sz) ; n++) {
			if ((sz[n] >= 'A') && (sz[n] <= 'Z'))
				sz[n] = sz[n] - 'A';
		}
	}
	return sz;
}
/*****************************************************************************/
int strcmpi (const char *sz1, const char *sz2) {
	char *szTmp1, *szTmp2;
	int iRetValue = 0;

	if ((sz1 != NULL) && (sz2 != NULL)) {
		szTmp1 = calloc(sizeof(sz1[0]), strlen(sz1) + 1);
		szTmp2 = calloc(sizeof(sz2[0]), strlen(sz2) + 1);
		strcpy (szTmp1, sz1);
		strcpy (szTmp2, sz2);
		szTmp1 = strlwr (szTmp1);
		szTmp2 = strlwr (szTmp2);
		iRetValue = strcmp (szTmp1, szTmp2);
		free (szTmp2);
		free (szTmp1);
	}
	return (iRetValue);
}
/*****************************************************************************/
/** Print usage information */
void usage() {

    const char *format =
            "\n"
            //"Usage: %s [OPTION]... SIZE <DEC>\n"
            "Usage: %s [OPTION]... <start-size> <end-size> <delta>\n"
            "\n"
            //"  --equalization  -e    Use equalization filter in FPGA (default: disabled).\n"
            //"  --shaping       -s    Use shaping filter in FPGA (default: disabled).\n"
            "  --gain1=g       -1 g  Use Channel 1 gain setting g [lv, hv] (default: lv).\n"
            "  --gain2=g       -2 g  Use Channel 2 gain setting g [lv, hv] (default: lv).\n"
            "  --file=<name>   -f    Output file name (by O.E.)\n"
            "  --version       -v    Print version info.\n"
            "  --help          -h    Print this message.\n"
            "\n"
            "    start-size - initial buffer length (# of floats)\n"
            "    end-size   - final buffer length\n"
            "                 default: start-size\n"
            "    delta      - increase size\n"
            "                 default: 50\n"
            //"    SIZE                Number of samples to acquire [0 - %u].\n"
            //"    DEC                 Decimation [%u,%u,%u,%u,%u,%u] (default: 1).\n"
            "\n";

    fprintf( stderr, format, g_argv0, SIGNAL_LENGTH,
             g_dec[0],
             g_dec[1],
             g_dec[2],
             g_dec[3],
             g_dec[4],
             g_dec[5]);
}
/*****************************************************************************/
/** Gain string (lv/hv) to number (0/1) transformation */
int get_gain(int *gain, const char *str)
{
	int nResult = 1;

    if (strcmpi (str, "lv") == 0) {
        *gain = 0;
    }
    else if (strcmpi (str, "hv") == 0) {
        *gain = 1;
    }
	else {
		nResult = 0;
	    fprintf(stderr, "Unknown gain: %s\n", str);
	}
    return (nResult);
}

/*****************************************************************************/
/* GetCommandLineParam - assign rp_params srtuct by command line arguments   */
/*****************************************************************************/
static struct option long_options[] = {
    /* These options set a flag. */
    /*{"equalization", no_argument,       0, 'e'},
    {"shaping",      no_argument,       0, 's'},*/
    {"gain1",        required_argument, 0, '1'},
    {"gain2",        required_argument, 0, '2'},
    {"file",         required_argument, 0, 'f'},
    {"version",      no_argument,       0, 'v'},
    {"help",         no_argument,       0, 'h'},
    {0, 0, 0, 0}
};
//const char *optstring = "es1:2:f:vh";
const char *optstring = "1:2:f:vh";
/*****************************************************************************/
int gain_error(int nChannel, char *szArgument)
{
    fprintf (stderr, "Wrong gain to channel %d: %s\n", nChannel, szArgument);
    usage();
    return -1;
}
/*****************************************************************************/
int get_acquisition_size(int argc, char *argv[], int iStart, struct rp_params *prp_params) {
	int nSize;

    if (iStart < argc) {
		nSize = atoi(argv[iStart]);
        if (nSize > SIGNAL_LENGTH) {
            fprintf(stderr, "Invalid SIZE: %s\n", argv[iStart]);
            usage();
            exit( EXIT_FAILURE );
        }
    }
    else {
        fprintf(stderr, "SIZE parameter missing\n");
        usage();
        exit( EXIT_FAILURE );
    }
	return (nSize);
}
/*****************************************************************************/
uint32_t get_decimation_index (int argc, char *argv[], int iStart) {
    int idxDec = -1;
	int n, dec;

    if (iStart < argc) {
        dec = atoi(argv[iStart]);
        for (n=0 ; (n < sizeof(g_dec) / sizeof(g_dec[0])) && (idxDec < 0) ; n++) {
            if (g_dec[n] == dec)
                idxDec = n;

        }

        if (idxDec < 0) {
            fprintf(stderr, "Invalid decimation DEC: %s\n", argv[iStart]);
            usage();
            return -1;
        }
    }
	else // decimation not stated, therefore returning the default
		idxDec = 0;
	return (idxDec);
}
/*****************************************************************************/
int GetCommandLineParam (int argc, char* argv[], struct rp_params *prp_params) {
    int option_index = 0, nGain;

    int ch = -1;
    if (argc < MINARGS) {
        usage();
        exit (EXIT_FAILURE);
    }
    memset(prp_params, 0, sizeof(*prp_params));
    while ( (ch = getopt_long( argc, argv, optstring, long_options, &option_index )) != -1 ) {
        switch ( ch ) {
            case 'e':
                prp_params->nEqual = 1;
                break;
            case 's':
                prp_params->nShaping = 1;
                break;
        /* Gain Channel 1 */
        case '1':
            if (get_gain(&nGain, optarg) != 0)
                prp_params->nGainCh1 = nGain;
            else
                return (gain_error(1, optarg));
            break;
        /* Gain Channel 2 */
        case '2':
            if (get_gain(&nGain, optarg) != 0)
                prp_params->nGainCh2 = nGain;
            else
                return (gain_error(2, optarg));
            break;
        case 'f':
            prp_params->szFileName = calloc (sizeof(char), 1 + strlen(optarg));
            strcpy (prp_params->szFileName, optarg);
            printf("Option 'f', optarg=%s\n", optarg);
            break;
        case 'v':
            fprintf(stdout, "%s version %s-%s\n", g_argv0, VERSION_STR, REVISION_STR);
            exit(EXIT_SUCCESS);
            break;
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }
    prp_params->sizeStart = get_acquisition_size(argc, argv, optind, prp_params);
    if (argc > optind)
        prp_params->sizeStart = atoi(argv[optind]);
    else
        prp_params->sizeStart = 100;
    if (argc > optind + 1)
        prp_params->sizeEnd = atoi(argv[optind + 1]);
    else
        prp_params->sizeEnd = prp_params->sizeStart;
    if (argc > optind + 2)
        prp_params->sizeDelta = atoi(argv[optind + 2]);
    else
        prp_params->sizeDelta = 50;
    return (0);
}
/*****************************************************************************/
void print_params(struct rp_params *prp_params) {
    printf ("nGainCh1 = %d\n", prp_params->nGainCh1);
    printf ("nGainCh2 = %d\n", prp_params->nGainCh2);
    printf ("nVersion = %d\n", prp_params->nVersion);
    printf ("nHelp = %d\n", prp_params->nHelp);
    printf("Start size: %d\n", prp_params->sizeStart);
    printf("End size: %d\n", prp_params->sizeEnd);
    printf("Delta size: %d\n", prp_params->sizeDelta);
	if (prp_params->szFileName != NULL)
		printf ("File Name = '%s'\n", prp_params->szFileName);
	else
		printf ("File Name is NULL\n");
}
/*****************************************************************************/
void set_params (struct rp_params *pParams, float t_params[])
{
//	memset (pParams, 0, sizeof (*pParams));
	t_params[EQUAL_FILT_PARAM] = (pParams->nEqual > 0 ? 1 : 0);
	t_params[SHAPE_FILT_PARAM] = (pParams->nShaping > 0 ? 1 : 0);
	t_params[GAIN1_PARAM] = pParams->nGainCh1;
	t_params[GAIN2_PARAM] = pParams->nGainCh2;
	t_params[TIME_RANGE_PARAM] = pParams->idxDecimation;
}
/*****************************************************************************/
float **create_buffer(int nBufferLength, int nSignals)
{
    float **pf;
    int n;

    pf = (float **)malloc(nSignals * sizeof(float *));
    for(n=0 ; n < nSignals ; n++)
        pf[n] = (float *) malloc(nBufferLength * sizeof(float));
    return (pf);
}
/*****************************************************************************/
void free_buffer(float **pfBuffer, int nSignals)
{
    int n;

    for (n=nSignals ; n > 0 ; n--)
        free (pfBuffer[n - 1]);
    free (pfBuffer);
}
/*****************************************************************************/
FILE *set_out_file (const char *szFileName, FILE **pfOut)
{
    FILE *filePrint;

	if (szFileName != NULL) {
		*pfOut = filePrint = fopen (szFileName, "w+");
        printf("Printing to outpu file %s\n", szFileName);
    }
    else {
        filePrint = stdout;
        *pfOut = NULL;
        printf ("Printing to stdout\n");
    }
    return (filePrint);
}
/*****************************************************************************/
int read_signals (float **pfBuffer, FILE *filePrint, struct rp_params *pParams)
{
    int sig_num, sig_len, i, ret_val;
    int retries = 150000;

    ret_val = i = 0;
    while(retries >= 0) {
        if((ret_val = rp_get_signals(&pfBuffer, &sig_num, &sig_len)) >= 0) {
                /* Signals acquired in s[][]:
                 * s[0][i] - TODO
                 * s[1][i] - Channel ADC1 raw signal
                 * s[2][i] - Channel ADC2 raw signal
                 */
		
/*
            for(i = 0; i < MIN(pParams->nSize, sig_len); i++) {
                //fprintf(filePrint, "%7d, %7d\n", (int)pfBuffer[1][i], (int)pfBuffer[2][i]);
                //fprintf(filePrint, "%g, %g\n", pfBuffer[1][i], pfBuffer[2][i]);
                fprintf(filePrint, "%g\n", pfBuffer[1][i]);
                //printf("%7d %7d\n", (int)s[1][i], (int)s[2][i]);
            }
*/
            break;
        }

        if(retries-- == 0) {
            fprintf(stderr, "Signal scquisition was not triggered!\n");
            return(0);
        }
        usleep(1000);
    }
    return (1);
}
/*****************************************************************************/
void print_vector (double ad[], int nSize)
{
	int n;

	for (n=0 ; n < nSize ; n++)
		printf("%g\n", ad[n]);
}
/*****************************************************************************/
double clock_to_time (clock_t clkTicks)
{
    double dTime;

    dTime = ((double) clkTicks) / dClocksPerSec;
    return (dTime);
}
/*****************************************************************************/
void shuffle (double *arr, int nSize)
/*  modern version of the Fisher–Yates shuffle algorithm
    pseudo code source: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
*/
{
    int n, j;
    double d;
    float rTmp;

	for (n=nSize-1 ; n > 1 ; n--) {
		d = (((double) rand()) / ((double) RAND_MAX)) * ((double) n);
		j = (int) (d + 0.5);
		rTmp = arr[n];
		arr[n] = arr[j];
		arr[j] = rTmp;
	}
}
/*****************************************************************************/
double vector_sum (double *adVector, int nSize)
{
    int n;
    double fSum = 0;

    for (n=0, fSum=0 ; n < nSize ; n++)
        fSum += adVector[n];
    return (fSum);
}
/*****************************************************************************/
double vector_mean (double *pSignal, int nSize)
{
    double dMean;

    if (nSize > 0)
        dMean = vector_sum (pSignal, nSize) / ((double) nSize);
    else
        dMean = 0;
    return (dMean);
}
/*****************************************************************************/
void perform_n (double *pSignal, int nSize)
{
    double dVariance, dMean;
    int n;

    if (nSize > 0) {
        dMean = vector_mean (pSignal, nSize);
        dVariance = 0;
        for (n=0 ; n < nSize ; n++)
            dVariance += pow(pSignal[n] - dMean, 2.0);
        dVariance /= ((double) nSize);
    }
}
/*****************************************************************************/
void bubble_sort (double *pSignal, int nBufSize)
/*
	This procedure uses bubble technique to sort an array pSignal, containing
	nBufSize elements.
	It uis used to measure times of O(n^2) algorithm.
	Source: https://www.geeksforgeeks.org/bubble-sort/
*/
{
	int i, j; 

	for (i = 0; i < nBufSize-1; i++)
       // Last i elements are already in place
       for (j = 0; j < nBufSize-i-1; j++)
           if (pSignal[j] > pSignal[j+1])
              swap(&pSignal[j], &pSignal[j+1]);

}
/*****************************************************************************/
double **matrix (int nRows, int nColumns)
{
	double **mtx;
	int n;

	mtx = (double**) calloc (nRows, sizeof(mtx[0]));
	for (n=0 ; n < nColumns ; n++)
		mtx[n] = (double*) calloc (nColumns, sizeof(mtx[0][0]));
	return (mtx);
}
/*****************************************************************************/
void free_matrix (double **mtx, int nRows, int nColumns)
{
	int n;

	for (n=nColumns - 1 ; n >= 0 ; n--)
		free (mtx[n]);
	free(mtx);
}
/*****************************************************************************/
double **matrix_multiply (double **mtxLeft, double **mtxRight, int nLeftRows, int nLeftCols, int nRightCols)
{
	double **mtxResult;
	int r, c, n;
	double d;

	mtxResult = matrix (nLeftRows, nRightCols);
	for (r=0 ; r < nLeftRows ; r++) {
		for (c=0 ; c < nRightCols ; c++) {
			d = 0;
			for (n=0 ; n < nLeftCols ; n++)
				d += mtxLeft[r][n] * mtxRight[n][c];
			mtxResult[r][c] = d;
		}
	}
	return (mtxResult);
}
/*****************************************************************************/
double **matrix_addition (double **mtxLeft, double **mtxRight, int nRows, int nCols)
{
	double **mtxResult;
	int r, c;

	mtxResult = matrix (nRows, nCols);
	for (r=0 ; r < nRows ; r++)
		for (c=0 ; c < nCols ; c++)
			mtxResult[r][c] = mtxLeft[r][c] - mtxRight[r][c];
	return (mtxResult);
}
/*****************************************************************************/
void slow_proc (double *pSignal, int nLength)
{
	double **mtxLeft, **mtxRight, **mtxResult;
	int n;

	mtxLeft = matrix (nLength, nLength);
	mtxRight = matrix (nLength, nLength);
	for (n=0 ; n < nLength ; n++)
		for (n=0 ; n < nLength ; n++)
			mtxLeft[n][n] = mtxRight[n][n] = pSignal[n];

	mtxResult = matrix_addition (mtxLeft, mtxRight, nLength, nLength);
	free_matrix (mtxResult, nLength, nLength);
	free_matrix (mtxLeft, nLength, nLength);
	free_matrix (mtxRight, nLength, nLength);
}
/*****************************************************************************/
void slow_sort (double *pSignal, int nLow, int nHigh)
/*
	Source: https://coderscat.com/3-most-slow-sorting-algorithms
*/
{
	int m;

	if (nLow >= nHigh)
		return;
	m = (int) (nLow + nHigh) / 2;
	slow_sort (pSignal, nLow, m);
	slow_sort (pSignal, m + 1, nHigh);
	if (pSignal[nHigh] < pSignal[m])
		swap (&pSignal[m], &pSignal[nHigh]);
	slow_sort (pSignal, nLow, nHigh - 1);
}	
/*****************************************************************************/
/** Acquire utility main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];
    FILE *fOut = NULL, *filePrint;
    struct rp_params params;
	int n;

	double ad[10];

	for (n=0 ; n < 10 ; n++)
		ad[n] = 100 + n;

	shuffle (ad, 10);
	printf("after shuffle\n");
	print_vector (ad, 10);
	quicksort (ad, 0, 9);
	printf("\nafter quicksort\n");
	print_vector (ad, 10);
	//exit(0);
    dClocksPerSec = (double) CLOCKS_PER_SEC;
    if (GetCommandLineParam (argc, argv, &params) < 0)
        exit(EXIT_FAILURE);
    else
        print_params(&params);

	set_params (&params, t_params);

    /* Initialization of Oscilloscope application */
    if(rp_app_init() < 0) {
        fprintf(stderr, "rp_app_init() failed!\n");
        return -1;
    }

    /* Setting of parameters in Oscilloscope main module */
    if(rp_set_params((float *)&t_params, PARAMS_NUM) < 0) {
        fprintf(stderr, "rp_set_params() failed!\n");
        return -1;
    }

    float **pfBuffer;
    int nBufSize;
    double *pSignal;
    clock_t cStart, cRead, cN, cNlogN, cn2, c0;

    pfBuffer = create_buffer(SIGNAL_LENGTH, SIGNALS_NUM);
    filePrint = set_out_file (params.szFileName, &fOut);

    FILE* fileDebug = fopen ("times.csv", "w+");
    fprintf(fileDebug, "n, Reading,O(n),O(n*log*n),O(n^2),Total\n");
    printf ("Times file created\n");

    for (nBufSize=params.sizeStart ; nBufSize <= params.sizeEnd ; nBufSize += params.sizeDelta) {
        cStart = clock ();
        pSignal = calloc (nBufSize, sizeof (pSignal[0]));
        read_signals (pfBuffer, filePrint, &params);
        for (n=0 ; n < nBufSize ; n++)
            pSignal[n] = (double) pfBuffer[0][n];
		//printf("Buffer read size: %d\n", nBufSize);
        cRead = clock() - cStart ;
		c0 = clock();
        perform_n (pSignal, nBufSize);
		//printf("calculation O(n) performed\n");
        cN = clock() - c0;
		quicksort (pSignal, 0, nBufSize - 1);
		cNlogN = clock() - c0;
		c0 = clock();
		bubble_sort (pSignal, nBufSize);
		//slow_proc (pSignal, nBufSize);
		cn2 = clock() - c0;
        free(pSignal);
		printf ("Completed %4d buffer size\n", nBufSize);
        fprintf(fileDebug, "%d,%g,%g,%g,%g,%g\n", nBufSize, clock_to_time (cRead),
				clock_to_time (cN), clock_to_time (cNlogN),
				clock_to_time (cn2), clock_to_time (clock() - cStart));
    }
	printf("\n");
    fclose (fileDebug);
    printf ("Sample loop eneded\n");
    //exit(0);
    if (fOut != 0)
        fclose(fOut);

    if(rp_app_exit() < 0) {
        fprintf(stderr, "rp_app_exit() failed!\n");
        return -1;
    }

    free_buffer(pfBuffer, SIGNALS_NUM);
	if (params.szFileName != NULL)
		free (params.szFileName);
    return 0;
}
