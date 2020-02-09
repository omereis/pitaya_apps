#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include "main.h"

void print_debug (const char *sz)
{
	FILE *file = fopen("/tmp/omer.debug", "a+");
	fprintf(file, "---------------------------------------------------\n");
	fprintf (file, "String length: %d\n", strlen(sz));
	fprintf (file, "%s\n", sz);
	fclose (file);
}



//Parameters
CBooleanParameter ledState("LED_STATE", CBaseParameter::RW, false, 0);








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

	print_debug ("OnNewParams, ledState updated\n");
    if (ledState.Value() == false)
    {
		print_debug("before calling rp_DpinSetState LOW");
        rp_DpinSetState(RP_LED0, RP_LOW); 
		print_debug("called rp_DpinSetState");
    }
    else
    {
		print_debug("before calling rp_DpinSetState HIGH");
        rp_DpinSetState(RP_LED0, RP_HIGH); 
		print_debug("called rp_DpinSetState HIGH");
    }
}


void OnNewSignals(void){}

void PostUpdateSignals(void){}