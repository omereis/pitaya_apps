
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include <redpitaya/rp.h>
//#include "redpitaya/rp.h"

// XE6: HTHD-8PN8CA-LQWYSD-KCYD
/*
Parameters
Trigger level [volts]
buffer length [time]
Iterations [number. -1 equals infinity]
output type : raw, short/long sum, amplitude/length
short time
long time
note: buffer_length = short + long

*/
//-----------------------------------------------------------------------------
/*
class TSamplingParams {
public:
protected:
private:
    int m_nSamplingRate;
    int m_nDecimation;
    short m_nInput1;
    short m_nInput2;
    int m_nBuffer;
    int m_nPreTrigger; // number of points acquired before the trigger
    class TTriggerParams m_trigger;
};
//-----------------------------------------------------------------------------
class TTriggerParams {
public:
protected:
private:
    int m_nInput;
    int m_nSource;
    int m_nLevel;
    int m_nDelay;
};
*/
//-----------------------------------------------------------------------------
void InitRedPitaya();
void SetTrigger ();
void SetSampling();
void exit_error (const char *szMessage);
bool ReadInputs (float *afBuffer, uint32_t nSize);
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    float *afBuffer;
    uint32_t nSize;
    InitRedPitaya();
    SetTrigger ();
    SetSampling();
    nSize = 100000;
    printf ("Red Pitaya initiated done\n");
    afBuffer = new float[nSize];
    ReadInputs (afBuffer, nSize);
    rp_Release ();
    printf ("Red Pitaya released.\nBye.\n");
}
//-----------------------------------------------------------------------------
void InitRedPitaya()
{
    if(rp_Init() != RP_OK)
        exit_error ("Red Pitaya API init failed!");
	if (rp_AcqReset() != RP_OK)
        exit_error ("Red Pitaya Reset Failed");
}
//-----------------------------------------------------------------------------
void SetTrigger ()
{
	if (rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE) != RP_OK)
		exit_error ("Error setting trigger source\n");
	if (rp_AcqSetTriggerLevel(RP_CH_1, 10e-3) != RP_OK)
		exit_error ("Error setting trigger level\n");
	if (rp_AcqSetTriggerDelay(1250) != RP_OK)
		exit_error ("Error setting trigger delay\n");
}
//-----------------------------------------------------------------------------
void SetSampling()
{
	if (rp_AcqSetDecimation(RP_DEC_1) != RP_OK)
		exit_error ("Error setting decimation\n");
	if (rp_AcqSetSamplingRate(RP_SMP_125M) != RP_OK)
		exit_error ("Setting sampleing rate error\n");
}
//-----------------------------------------------------------------------------
void exit_error (const char *szMessage)
{
    fprintf (stderr, "%s\n", szMessage);
    exit (-1);
}
//-----------------------------------------------------------------------------
bool ReadInputs (float *afBuffer, uint32_t nSize)
{
	time_t tStart, tNow;
	bool fTrigger, fTimeLimit;

    for (uint32_t n=0 ; n < nSize ; n++)
        afBuffer[n] = 0;
	rp_acq_trig_state_t state = RP_TRIG_STATE_WAITING;
	time(&tStart);
	fTrigger = fTimeLimit = false;
	while((!fTrigger) && (!fTimeLimit)) {
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED)
			fTrigger = true;
		usleep(1);
		time(&tNow);
		if (difftime (tNow, tStart) >= 15)
			fTimeLimit = true;
    }
	if (fTrigger) {
		uint32_t nTrigPos;
		rp_AcqGetWritePointerAtTrig (&nTrigPos); // get pointer at trigger time
		rp_AcqGetDataV(RP_CH_1, nTrigPos-100, &nSize, afBuffer); // 80 nSec before trigger
	}
	rp_AcqStop ();
    return (fTrigger);
}

//-----------------------------------------------------------------------------
