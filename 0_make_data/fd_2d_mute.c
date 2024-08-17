#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "fun_fd.h"
#define PI 3.14159265


char folder[] = "D:/RunData";
unsigned int nx = 101, nz = 200;
unsigned int dx = 5, dz = 5;
unsigned int isx = 50, isz = 0;
unsigned int i, j, k, nt, f = 20;
float it, temp_float, tmax = 3, dt = 1e-4, delay = 0.05;
time_t start_time, stop_time;

int main() {
	time(&start_time);
	if (access(folder, 0) == -1)
		mkdir(folder);
	
	nt = (tmax / dt) + 1;
	printf("nx%d nz%d nt%d isx%d isz%d\n", nx, nz, nt, isx, isz);

	float* wav = (float*)malloc(nt * sizeof(float));
	for (i = 0, it = 0; i < nt; i++, it += dt) {
		temp_float = pow(PI * f * (it - delay), 2);
		wav[i] = (1 - 2 * temp_float) * exp(-temp_float);
	}

	float **vmodel = malloc2d(nx, nz);

	for (i = 0; i < nx; i++) {
		for (j = 0; j < nz; j++) {
			if (j < 30)
				vmodel[i][j] = 1500;
			else if (j < 60)
				vmodel[i][j] = 3500;
			else
				vmodel[i][j] = 4000;
		}
	}

	float** data = mute(nx, nz, nt, isx, isz, dx, dt, vmodel, wav);

	time(&stop_time);
	time_t elapsed = stop_time - start_time;
	char time_used[80];
	strftime(time_used, 80, "%H:%M:%S", gmtime(&elapsed));
	printf("Total use: %s\n", time_used);

	char fname[100];
	sprintf(fname, "%s/2d_%dx%d.bin", folder, nt, nx);
	FILE *fp = fopen(fname, "wb");
	for ( i = 0; i < nt; i++) {
		fwrite(data[i], sizeof(float), nx, fp);
	}
	fclose(fp);
	printf("write data %s success\n", fname);

	free(wav);
	free2d(vmodel, nx);
	free2d(data, nx);
	return 0;
}

