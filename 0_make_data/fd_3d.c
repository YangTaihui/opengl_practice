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
float it, temp_float, tmax = 0.6, dt = 1e-4, delay = 0.05;
float save_dt = 0.002;
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

	float*** data = fd_3d(nx, nz, nt, isx, isz, dx, dt, save_dt, vmodel, wav);
	int save_nt = nt/(save_dt/dt)+1;
	
	time(&stop_time);
	time_t elapsed = stop_time - start_time;
	char time_used[80];
	strftime(time_used, 80, "%H:%M:%S", gmtime(&elapsed));
	printf("Total use: %s\n", time_used);

	char fname[100];
	sprintf(fname, "%s/3d_%dx%dx%d.bin", folder, nx, save_nt, nz);
	FILE *fp = fopen(fname, "wb");
	for ( j = 0; j < nx; j++) {
		for ( i = 0; i < save_nt; i++) {
			fwrite(data[i][j], sizeof(float), nz, fp);
		}
	}
	fclose(fp);
	printf("write data %s success\n", fname);

	free(wav);
	free2d(vmodel, nx);
	free3d(data, save_nt, nx);
	return 0;
}

