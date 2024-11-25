#include <algorithm>

float* calloc1d_float(int m) {
    float* data = (float*)calloc(m, sizeof(float));
    return data;
}

unsigned int* calloc1d_uint(int m) {
    unsigned int* data = (unsigned int*)calloc(m, sizeof(unsigned int));
    return data;
}

float** malloc2d(int m, int n) {
    float** data = (float**)malloc(m * sizeof(float*));
    for (int i = 0; i < m; i++) {
        data[i] = (float*)calloc(n, sizeof(float));
    }
    return data;
}

float*** malloc3d(int m, int n, int o) {
    int i, j;
    float*** data = (float***)malloc(m * sizeof(float**));
    for (i = 0; i < m; i++) {
        data[i] = (float**)malloc(n * sizeof(float*));
        for (j = 0; j < n; j++) {
            data[i][j] = (float*)calloc(o, sizeof(float));
        }
    }
    return data;
}

float** read2d_sample(char* fname, int m, int n, int mm, int nn) {
    float** data = malloc2d(mm, nn);
    int rm = m / mm, rn = n / nn;

    FILE* fp;
    errno_t err = fopen_s(&fp, fname, "rb");
    for (int i = 0; i < mm; i++) {
        for (int j = 0; j < nn; j++) {
            fread(&data[i][j], sizeof(float), 1, fp);
            if (j == nn - 1 && n > nn) {
                fseek(fp, 4 * (n - nn), SEEK_CUR);
            }
            else if (rn > 1) {
                fseek(fp, 4 * (rn - 1), SEEK_CUR);
            }
        }
        if (i != mm - 1 && rm > 1) {
            fseek(fp, 4 * (rm - 1) * n, SEEK_CUR);
        }
    }
    return data;
}

float*** read3d(char* fname, int m, int n, int o) {
    int i, j;
    FILE* fp;
    float*** data = malloc3d(m, n, o);
    errno_t err = fopen_s(&fp, fname, "rb");
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            fread(data[i][j], sizeof(float), o, fp);
        }
    }
    return data;
}

void min_max_2d(float** data2d, int nx, int ny, float zoom) {
    float maxv = 0.0, minv = 0.0;
    int i, j;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            maxv = std::max(maxv, data2d[i][j]);
            minv = std::min(minv, data2d[i][j]);
        }
    }
    std::cout << "min " << minv << " max " << maxv << std::endl;
    if (maxv * minv < 0) {
        if (abs(maxv) > abs(minv)) {
            maxv *= zoom;
            minv = -maxv;
            for (i = 0; i < nx; i++) {
                for (j = 0; j < ny; j++) {
                    if (data2d[i][j] > maxv)
                        data2d[i][j] = maxv;
                    data2d[i][j] = (data2d[i][j] + maxv) / (2 * maxv) * 255;
                }
            }
        }
        else {
            maxv *= zoom;
            minv = -maxv;
            for (i = 0; i < nx; i++) {
                for (j = 0; j < ny; j++) {
                    if (data2d[i][j] < minv)
                        data2d[i][j] = minv;
                    if (data2d[i][j] > maxv)
                        data2d[i][j] = maxv;
                    data2d[i][j] = (data2d[i][j] + maxv) / (2 * maxv) * 255;
                }
            }
        }
    }
    else {
        for (i = 0; i < nx; i++) {
            for (j = 0; j < ny; j++) {
                data2d[i][j] = (data2d[i][j] - minv) / (maxv - minv) * 255;
            }
        }
    }
    std::cout << "min " << minv << " max " << maxv << std::endl;
}

void min_max_3d(float*** data3d, int nx, int ny, int nz, float zoom) {
    float maxv = 0.0, minv = 0.0;
    int i, j, k;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            for (k = 0; k < nz; k++) {
                maxv = std::max(maxv, data3d[i][j][k]);
                minv = std::min(minv, data3d[i][j][k]);
            }
        }
    }
    std::cout << "min " << minv << " max " << maxv << std::endl;
    if (maxv * minv < 0) {
        if (abs(maxv) > abs(minv)) {
            maxv *= zoom;
            minv = -maxv;
            for (i = 0; i < nx; i++) {
                for (j = 0; j < ny; j++) {
                    for (k = 0; k < nz; k++) {
                        if (data3d[i][j][k] > maxv)
                            data3d[i][j][k] = maxv;
                        data3d[i][j][k] = (data3d[i][j][k] + maxv) / (2 * maxv) * 255;
                    }
                }
            }
        }
        else {
            maxv *= zoom;
            minv = -maxv;
            for (i = 0; i < nx; i++) {
                for (j = 0; j < ny; j++) {
                    for (k = 0; k < nz; k++) {
                        if (data3d[i][j][k] < minv)
                            data3d[i][j][k] = minv;
                        if (data3d[i][j][k] > maxv)
                            data3d[i][j][k] = maxv;
                        data3d[i][j][k] = (data3d[i][j][k] + maxv) / (2 * maxv) * 255;
                    }
                }
            }
        }
    }
    else {
        for (i = 0; i < nx; i++) {
            for (j = 0; j < ny; j++) {
                for (k = 0; k < nz; k++) {
                    data3d[i][j][k] = (data3d[i][j][k] - minv) / (maxv - minv) * 255;
                }
            }
        }
    }
    std::cout << "min " << minv << " max " << maxv << std::endl;
}