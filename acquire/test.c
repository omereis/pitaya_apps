#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int main (int argc, char *argv[]) {
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
