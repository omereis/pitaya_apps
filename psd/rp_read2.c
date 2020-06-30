/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "redpitaya/rp.h"

int main(int argc, char **argv){

        /* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

        /*LOOB BACK FROM OUTPUT 2 - ONLY FOR TESTING*/
	uint32_t buff_size = 16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));
	float *big_buff = (float*) calloc(2 * buff_size, sizeof(big_buff[0]));

	rp_AcqReset();
	rp_AcqSetDecimation(1);
	rp_AcqSetTriggerLevel(RP_CH_1, 1.0); //Trig level is set in Volts while in SCPI
	rp_AcqSetTriggerDelay(0);

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

	while(1){
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			break;
		}
		usleep(1);
		nWaits++;
	}
	clock_t cStart;
	struct timespec tStart, tEnd;

	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
	memcpy (big_buff, buff, sizeof(buff[0]) * buff_size);
	FILE *fout;
/*
	int i;
	FILE *fout = fopen ("out.csv", "w+");
	for(i = 0; i < buff_size; i++){
		fprintf(fout, "%f\n", buff[i]);
	}
	fclose (fout);
*/
	printf ("Read once, after %d polls\n", nWaits);
/* end of 1st read */
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
	nWaits = 1;
	cStart = clock();
	clock_gettime(CLOCK_REALTIME, &tStart);
        /* Releasing resources */
	while(1){
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			break;
		}
		/*usleep(1);*/
		nWaits++;
	}
	clock_t cTrig = clock() - cStart;
	clock_gettime(CLOCK_REALTIME, &tEnd);

	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
	clock_t cRead = clock() - cStart;
	fout = fopen ("out.csv", "w+");
	for(int i = 0; i < buff_size; i++){
		fprintf(fout, "%f,%f\n", big_buff[i], buff[i]);
	}
	fclose (fout);
	printf ("Read twice, this time %d polls\n", nWaits);
	free(buff);
	rp_Release();
	printf("Trigger waiting time: %g milliseconds\n", 1e3 * ((double) cTrig) / ((double) CLOCKS_PER_SEC));
	printf("Trigger and Reading time: %g milliseconds\n", 1e3 * ((double) cRead) / ((double) CLOCKS_PER_SEC));
	printf ("Real time: %g\n", (double) ((tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_nsec - tStart.tv_nsec)) / 1e9);
	return 0;
}
