#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include <iostream>       // std::cerr
#include <typeinfo>       // operator typeid
#include <exception>      // std::exception

#include "main.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *rp_app_desc(void)
{
    return (const char *)"Template application.\n";
}


int rp_app_init(void)
{
    fprintf(stderr, "Loading template application\n");
    return 0;
}


int rp_app_exit(void)
{
    fprintf(stderr, "Unloading template application\n");
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

CBooleanParameter ledState("LED_STATE", CBaseParameter::RW, false, 0);

void print_debug(const char *sz)
{
	FILE *file = fopen ("/root/omer.debug", "a+");
    fprintf(file, "---------------------------------\n");
	fprintf (file, "new string: %s\n", sz);
	fclose (file);
}

void OnNewParams(void) {
	print_debug("before led state update\n");
	ledState.Update();
	print_debug("led state updated\n");
//	printf("led state updated"\n");
    try {
		print_debug("checking led status value\n");
	    if (ledState.Value()) {
    	    print_debug("led is now ON\n");
			try {
	    	    print_debug("calling rp_DpinSetState\n");
	    		rp_DpinSetState(RP_LED0, RP_HIGH);
	    	    print_debug("rp_DpinSetState called\n");
			}
			catch (...) {
				print_debug("didn't work");
			}
    	    print_debug("led set ON\n");
	    }
    	else {
    	    print_debug("led is now Off\n");
	    	rp_DpinSetState(RP_LED0, RP_LOW);
	        print_debug("led set OFF\n");
	    }
    }
	catch (...) {
		print_debug ("Som Ting Wong\n");
	}
    //catch (exception &e) {
    //catch (std::exception const& e) {
/*
	catch (std::exception& e) {
    	//printf("Runtime error: %s\n", e.what());
		print_debug("Runtime error");
        char sz[1024];
        sprintf(sz, "%s\n", e.what());
        print_debug("there's some exception");
  	}
*/
	print_debug("completed");
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}
