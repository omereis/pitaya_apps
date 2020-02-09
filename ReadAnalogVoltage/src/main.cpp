#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include "main.h"



//Signal size
#define SIGNAL_SIZE_DEFAULT      1


//Parameters
CBooleanParameter ledState("LED_STATE", CBaseParameter::RW, false, 0);

CFloatSignal VOLTAGE("VOLTAGE", SIGNAL_SIZE_DEFAULT, 0.0f);

CBooleanParameter READ_VALUE("READ_VALUE", CBaseParameter::RW, false, 0);




const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya LED control.\n";
}


int rp_app_init(void)
{
    fprintf(stderr, "Loading LED control\n");

    // Initialization of API
    if (rpApp_Init() != RP_OK) 
    {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }
    else fprintf(stderr, "Red Pitaya API init success!\n");


    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading LED control\n");

    rpApp_Release();

    return 0;
}


int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}


int rp_get_params(rp_app_params_t **p)
{
    return 0;
}


int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}








void UpdateSignals(void){}


void UpdateParams(void){}


void OnNewParams(void) 
{

ledState.Update();

    if (ledState.Value() == false)
    {
        rp_DpinSetState(RP_LED0, RP_LOW); 
    }
    else
    {
        rp_DpinSetState(RP_LED0, RP_HIGH); 
    }

	READ_VALUE.Update();
	if (READ_VALUE.Value() == true)
	{
		float val;

		//Read data from pin
		rp_AIpinGetValue(0, &val);

		//Write data to signal
		VOLTAGE[0] = val;

		//Reset READ value
		READ_VALUE.Set(false);
	}

}


void OnNewSignals(void){}

void PostUpdateSignals(void){}
