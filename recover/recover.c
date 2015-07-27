#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_N 100

int X[MAX_N], mX[MAX_N];
double Y[MAX_N], mY[MAX_N];
int kmap[MAX_N];

int main(int argc, char **argv)
{
	int in = 0, out = 0, known = 0, unknown = 0, i, j, z;
	char linebuf[80];
	for(i = 0; i < argc; ++i)
		if (0 == strncmp(argv[i], "--in=", 5))
			in = atoi(argv[i] + 5);
		else if (0 == strncmp(argv[i], "--out=", 6))
			out = atoi(argv[i] + 6);
	if (in == 0 || out <= in || out > MAX_N) {
		fprintf(stderr, "invalid in / out arguments\n");
		return 1;
	}
	for(i = 0; i < out; ++i) {
		if (!fgets(linebuf, 80, stdin)) {
			fprintf(stderr, "too few input lines\n");
			return 1;
		}
		if (strstr(linebuf, "MISSING") > 0) {
			kmap[i] = MAX_N + unknown;
			mX[unknown++] = i;
		} else {
			double num;
			char *token = strtok(linebuf, " /");
			num = atof(token);
			if (NULL != (token = strtok(NULL, " /")))
				num /= atof(token);
			kmap[i] = known;
			X[known] = i;
			Y[known] = num;
			++known;
		}
	}
	if (known < in) {
		fprintf(stderr, "too many missing fragments\n");
		return 1;
	}
	/*
	int k;
	for(k = 0; k < known; ++k)
	{
		printf("X[%d]=%d Y[%d]=%f\n", k, X[k], k, Y[k]);
	}
	for(k = 0; k < unknown; ++k)
	{
		printf("mX[%d]=%d\n", k, mX[k]);
	}
	*/
	for(z = 0; z < unknown; ++z) {
		mY[z] = 0;
		for(i = 0; i < in; ++i) {
			int s = 1, t = 1;
			for(j = 0; j < in; ++j) {
				if (j != i) {
					s *= (mX[z] - X[j]);
					t *= (X[i] - X[j]);
				}
			}
			mY[z] += (Y[i] * s) / t;
		}
	}
	for(i = 0; i < in; ++i) {
		int val, k = kmap[i];
		if (k >= MAX_N)
			val = (int)round(mY[k - MAX_N]);
		else
			val = (int)round(Y[k]);
		/* why binary, jt? why? */
		for(j = 7; j >= 0; j--)
			putchar(val & (1 << j) ? '1' : '0');
		putchar('\n');
	}
	return 0;
}
