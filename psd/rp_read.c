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
	uint32_t buff_size = 16384;//8192;//16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));
	float *big_buff = (float*) calloc(2 * buff_size, sizeof(big_buff[0]));

	rp_AcqReset();
	if (rp_AcqSetDecimation(RP_DEC_1/*1*/) != RP_OK)
		printf("Error setting dec9imation\n");;
	if (rp_AcqSetSamplingRate(RP_SMP_125M) != RP_OK)
		printf ("Setting sampleing rate error\n");
	rp_AcqSetTriggerLevel(RP_CH_1, 10e-3); //Trig level is set in Volts while in SCPI
	rp_AcqSetTriggerDelay(1024);

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
	fout = fopen ("out.csv", "w+");
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
