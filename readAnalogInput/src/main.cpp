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

//Signal
CFloatSignal VOLTAGE("VOLTAGE", SIGNAL_SIZE_DEFAULT, 0.0f);

//Parameter
CBooleanParameter READ_VALUE("READ_VALUE", CBaseParameter::RW, false, 0);
CIntParameter ipInput("INPUT", CBaseParameter::RW, false, 0, 0, 4);
CStringParameter spInputs("STR_INPUT", CBaseParameter::RW, "", 0);

void print_debug (const char *sz)
{
	FILE *file = fopen ("/opt/redpitaya/www/apps/readAnalogInput/omer.debug", "a+");
	fprintf (file, "-----------------------------------------------\n");
	fprintf (file, "%s\n", sz);
	fclose (file);
}

const char *rp_app_desc(void)
{
    return (const char *)"Red Pitaya read voltage.\n";
}

int rp_app_init(void)
{
    fprintf(stderr, "Loading read voltage application\n");

    // Initialization of API
    if (rpApp_Init() != RP_OK) {
        fprintf(stderr, "Red Pitaya API init failed!\n");
        return EXIT_FAILURE;
    }
    else fprintf(stderr, "Red Pitaya API init success!\n");

    return 0;
}

int rp_app_exit(void)
{
    fprintf(stderr, "Unloading read voltage application\n");

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

void Tokenize (const std::string &strLine, TStrVec &vstr, const char cDelimiter=',')
{
	size_t sPos, sStart = 0;
	std::string strDelimiter, str;
//	char szBuf[1024];

//	print_debug("in Tokenize");
	strDelimiter.push_back(cDelimiter);
	sPos = strLine.find(strDelimiter, 0);

/*
	sprintf(szBuf, "strLine='%s'\n", strLine.c_str());
	print_debug(szBuf);
	sprintf(szBuf, "spos=%d\n", sPos);
	print_debug(szBuf);
*/
	while (sPos != std::string::npos) {
		if (sPos == sStart)
			str = std::string("");
		else
			str = strLine.substr(sStart, (sPos - sStart));
		vstr.push_back (str);
		sStart = sPos + 1;
		sPos = strLine.find(strDelimiter, sStart);
	}
	if (strLine.length() > sStart) {
		str = strLine.substr(sStart, (strLine.length() - sStart));
		vstr.push_back (str);
	}
}

void OnNewParams(void)
{
	char sz[1024];
    READ_VALUE.Update();
	ipInput.Update();
	print_debug ("ipInput updated");
	sprintf (sz, "ipInput value: %d\n", ipInput.Value());
	print_debug (sz);

	TIntVec v;
	v.push_back (12);

	TStrVec vstrTokens;

	spInputs.Update();
	sprintf (sz, "spInput ls value: '%s'\n", spInputs.Value().c_str());
	print_debug (sz);

	Tokenize (spInputs.Value(), vstrTokens);
	for (int n=0 ; n < vstrTokens.size() ; n++) {
		sprintf (sz, "Token #%d: %s", n, vstrTokens[n].c_str());
		print_debug (sz);
	}

//	sprintf (sz, "spInput length: '%d' bytes\n", spInputs.Value().length());
//	print_debug (sz);

    if (READ_VALUE.Value() == true)
    {
        float val;
		//static int n = 1;
    
        //Read data from pin
        rp_AIpinGetValue(0, &val);

        //Write data to signal
		//val = (float) n++;
        VOLTAGE[0] = val;

        //Reset READ value
        READ_VALUE.Set(false);
    }
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}
