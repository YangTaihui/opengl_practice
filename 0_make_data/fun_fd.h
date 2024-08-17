#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MAX(x, y)   \
({                  \
typeof(x) _x = (x); \
typeof(y) _y = (y); \
_x > _y ? _x : _y;  \
})

#define MIN(x, y)   \
({                  \
typeof(x) _x = (x); \
typeof(y) _y = (y); \
_x < _y ? _x : _y;  \
})

void fd(unsigned int nx, unsigned int nz, float ** iu, float ** iv, float ** iw, float** r) {
	float temp_float;
	unsigned int j, k;
	for (j = 0; j < nx; j++) {
		for (k = 0; k < nz; k++) {
			temp_float = -4 * iv[j][k];
			if (j > 0)
				temp_float += iv[j - 1][k];
			if (j < nx - 1)
				temp_float += iv[j + 1][k];
			if (k > 0)
				temp_float += iv[j][k - 1];
			if (k < nz - 1)
				temp_float += iv[j][k + 1];
			iu[j][k] = 2 * iv[j][k] - iw[j][k] + r[j][k] * r[j][k] * temp_float;
		}
	}
	for (j = 0; j < nz; j++) {
		iu[0][j] = iv[1][j] + (r[0][j] - 1) / (r[0][j] + 1) * (iu[1][j] - iv[0][j]);
		iu[nx - 1][j] = iv[nx - 2][j] + (r[nx - 1][j] - 1) / (r[nx - 1][j] + 1) * (iu[nx - 2][j] - iv[nx - 1][j]);
	}
	for (j = 0; j < nx; j++)
		iu[j][nz - 1] = iv[j][nz - 2] + (r[j][nz - 1] - 1) / (r[j][nz - 1] + 1) * (iu[j][nz - 2] - iv[j][nz - 1]);
}

float** malloc2d(unsigned int m, unsigned int n) {
	float** data = (float**)malloc(m * sizeof(float*));
	for ( unsigned int i = 0; i < m; i++) {
		data[i] = (float*)calloc(n, sizeof(float));
	}
	return data;
}

float*** malloc3d(unsigned int m, unsigned int n, unsigned int o) {
	float*** data = (float***)malloc(m * sizeof(float**));
	for ( unsigned int i = 0; i < m; i++) {
		data[i] = (float**)malloc(n * sizeof(float*));
		for ( unsigned int j = 0; j < n; j++) {
			data[i][j] = (float*)calloc(o, sizeof(float));
		}
	}
	return data;
}

void free2d(float **data, unsigned int m){
	for ( unsigned int i = 0; i < m; i++) {
		free(data[i]);
	}
	free(data);
}

void free3d(float ***data, unsigned int m, unsigned int n){
	for ( unsigned int i = 0; i < m; i++) {
		for ( unsigned int j = 0; j < n; j++) {
			free(data[i][j]);
		}
		free(data[i]);
	}
	free(data);
}


float** m2r(unsigned int nx, unsigned int nz, unsigned int dx, float dt, float** vmodel) {
	float** r = malloc2d(nx, nz);
	for (unsigned int i = 0; i < nx; i++) {
		for (unsigned int j = 0; j < nz; j++) {
			r[i][j] =  vmodel[i][j] * dt / dx;
		}
	}
	return r;
}


float** mute(unsigned int nx, unsigned int nz, unsigned int nt, unsigned int isx, unsigned int isz, unsigned int dx, float dt, float** vmodel, float* wav) {
	unsigned int i, j;
	float** vm_homo = malloc2d(nx, nz);
	float v0 = vmodel[0][0];
	for (i = 0; i < nx; i++)
		for (j = 0; j < nz; j++)
			vm_homo[i][j] = v0;
	float** r = m2r(nx, nz, dx, dt, vmodel);
	float** r_homo = m2r(nx, nz, dx, dt, vm_homo);
	float** muted_gather = malloc2d(nt, nx);
	float** u = malloc2d(nx, nz), **v = malloc2d(nx, nz), **w = malloc2d(nx, nz);
	float** u_homo = malloc2d(nx, nz), **v_homo = malloc2d(nx, nz), **w_homo = malloc2d(nx, nz);
	w[isx][isz] = wav[0];
	v[isx][isz] = wav[1];
	w_homo[isx][isz] = wav[0];
	v_homo[isx][isz] = wav[1];
	for (i = 2; i < nt; i++) {
		fd(nx, nz, u, v, w, r);
		fd(nx, nz, u_homo, v_homo, w_homo, r_homo);
		u[isx][isz] += wav[i];
		u_homo[isx][isz] += wav[i];
		for (j = 0; j < nx; j++) {
			muted_gather[i][j] = u[j][0] - u_homo[j][0];
		}
		w = v;
		v = u;
		u = w;
		w_homo = v_homo;
		v_homo = u_homo;
		u_homo = w_homo;
	}
	free2d(vm_homo, nx);
	free2d(r, nx);
	free2d(r_homo, nx);
	free2d(u, nx);
	free2d(v, nx);
	free2d(u_homo, nx);
	free2d(v_homo, nx);
	return muted_gather;
}

float*** fd_3d(unsigned int nx, unsigned int nz, unsigned int nt, unsigned int isx, unsigned int isz, unsigned int dx, float dt, float save_dt, float** vmodel, float* wav) {
	unsigned int i, j, k, ii=0;
	int save_interval = save_dt/dt;
	int save_nt = nt/save_interval+1;

	float** r = m2r(nx, nz, dx, dt, vmodel);

	float*** gather = malloc3d(save_nt, nx, nz);
	float** u = malloc2d(nx, nz), **v = malloc2d(nx, nz), **w = malloc2d(nx, nz);

	w[isx][isz] = wav[0];
	v[isx][isz] = wav[1];

	for (i = 2; i < nt; i++) {
		fd(nx, nz, u, v, w, r);

		u[isx][isz] += wav[i];

		if (i%save_interval==0) {
			for (j = 0; j < nx; j++){
				for (k = 0; k < nz; k++){
					gather[ii][j][k] = u[j][k];
				}
			}
			ii += 1;
		}
		w = v;
		v = u;
		u = w;

	}

	free2d(r, nx);
	free2d(u, nx);
	free2d(v, nx);
	return gather;
}
