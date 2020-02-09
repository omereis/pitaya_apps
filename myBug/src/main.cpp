#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include "main.h"
//-----------------------------------------------------------------------------
void print_debug(const char *sz)
{
    FILE *file = fopen ("/tmp/omer.debug", "s+");
    fprintf (file, "---------------------------------------------------\n");
    fprintf (file, "%s\n", sz);
    fclose(file);
}
//-----------------------------------------------------------------------------
const char *rp_app_desc(void)
{
    return (const char *)"Template application.\n";
}
//-----------------------------------------------------------------------------
int rp_app_init(void)
{
	if (rpApp_Init() == RP_OK)
		print_debug("init OK");
	else
		print_debug("init sucks");
    //fprintf(stderr, "Loading template application\n");
    return 0;
}
//-----------------------------------------------------------------------------
int rp_app_exit(void)
{
	if (rpApp_Release() == RP_OK)
		print_debug("Release OK");
	else
		print_debug("Release sucks");
    return 0;
}
//-----------------------------------------------------------------------------
int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}
//-----------------------------------------------------------------------------
int rp_get_params(rp_app_params_t **p)
{
    return 0;
}
//-----------------------------------------------------------------------------
int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}
//-----------------------------------------------------------------------------
void UpdateSignals(void){}
//-----------------------------------------------------------------------------
void UpdateParams(void){}
//-----------------------------------------------------------------------------
void OnNewParams(void) {
    print_debug("new param here\n");
}
//-----------------------------------------------------------------------------
void OnNewSignals(void){
    print_debug("new param here\n");
}
//-----------------------------------------------------------------------------
void PostUpdateSignals(void){}
//-----------------------------------------------------------------------------


