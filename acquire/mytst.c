#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

/*****************************************************************************/
void print_vector (double ad[], int nSize)
{
	int n;

	for (n=0 ; n < nSize ; n++)
		printf("%g\n", ad[n]);
}
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
	int j, i = nLow;

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
int main (int argc, char *argv[]) {
	double ad[10];
	int n;

	for (n=0 ; n < 10 ; n++)
		ad[n] = 100 + n;
	ad[0] = 100;
	ad[1] = 109;
	ad[2] = 102;
	ad[3] = 104;
	ad[4] = 101;
	ad[5] = 106;
	ad[6] = 107;
	ad[7] = 105;
	ad[8] = 103;
	ad[9] = 108;
	print_vector (ad, 10);

}
/*****************************************************************************/
int main1 (int argc, char *argv[]) {
	int n, nItems, j, nTmp;
	int *aItems;
	double d;

	printf("Maximum random number is %d\n", RAND_MAX);
	if (argc > 1)
		nItems = atoi(argv[1]);
	else
		nItems = 10;
	if (nItems > 0) {
		aItems = (int*) malloc (nItems * sizeof(aItems[0]));
		for (n=0 ; n < nItems ; n++)
			aItems[n] = n + 1;
		printf("Before shuffle\n");
		printf("--------------\n");
		for (n=0 ; n < nItems ; n++)
			printf ("Item %3d: %3d\n", n, aItems[n]);
		printf("==============\n");
		printf ("Index(n)\trandom(d)\tinteger(j)\taItems[n]\taItems[j]\n");
		for (n=nItems-1 ; n > 1 ; n--) {
			d = (((double) rand()) / ((double) RAND_MAX)) * ((double) n);
			j = (int) (d + 0.5);
			nTmp = aItems[n];
			aItems[n] = aItems[j];
			aItems[j] = nTmp;
			printf ("%d\t%g\t%d\t%d\t%d\n", n, d, j, aItems[n], aItems[j]);
		}
		printf("After shuffle\n");
		printf("--------------\n");
		for (n=0 ; n < nItems ; n++)
			printf ("Item %3d: %3d\n", n, aItems[n]);

	}

}
