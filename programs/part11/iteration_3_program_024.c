/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define ITER 10

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], float arr2[N][M], double arr3[P]) {
    volatile int barrier = 0; /* Prevent cross-iteration optimization */
    
    /* Multi-dimensional flow dependencies with varying strides */
    for (int i = 2; i < N-2; i++) {
        for (int j = 3; j < M-3; j += 2) { /* Non-unit stride */
            /* Flow (RAW) dependency with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr1[i-2][j+1] * 2;
            
            /* Cross-dimensional flow dependency */
            if (j > 5) {
                arr1[i][j-1] = arr1[i][j-3] + arr1[i-1][j+2];
            }
            
            /* Mixed array type dependency with casting */
            arr2[i][j] = (float)arr1[i][j] * 0.5f + arr2[i-1][j+1];
            
            /* Memory barrier to ensure dependencies are visible */
            asm volatile("" ::: "memory");
            barrier = arr1[i][j]; /* Use volatile-like behavior */
        }
        
        /* Loop-carried dependency to arr3 with varying distance */
        int idx = i % P;
        arr3[idx] = arr3[(idx + P - 2) % P] * 1.01 + (double)arr1[i][M/2];
    }
    
    /* Additional dependency chain with conditional distances */
    for (int i = 4; i < N; i++) {
        for (int j = 4; j < M; j++) {
            if (i % 3 == 0) {
                /* Distance 2 dependency */
                arr1[i][j] = arr1[i-2][j] + arr1[i][j-2];
            } else if (i % 5 == 0) {
                /* Distance 3 dependency */
                arr1[i][j] = arr1[i-3][j+1] - arr1[i][j-3];
            } else {
                /* Distance 1 dependency */
                arr1[i][j] = arr1[i-1][j-1] * arr1[i][j-1];
            }
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(int* base_arr, int* alias_arr1, int* alias_arr2) {
    /* Create aliasing pointers that overlap */
    int* p = &base_arr[0];
    int* q = &base_arr[1];  /* q aliases p+1 */
    int* r = &base_arr[N/2]; /* Partial overlap possible */
    
    volatile int sink = 0;
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (int i = 1; i < N*M - 10; i += 3) {
        int temp = p[i];      /* Read from p[i] */
        q[i-1] = temp + i;    /* Write to q[i-1] which aliases p[i] when i=1 */
        
        /* Additional anti-dependency chain */
        temp = r[i % (N*M/2)];
        p[(i+2) % N] = temp * 2;
        
        /* Memory clobber to prevent reordering */
        asm volatile("" ::: "memory");
        sink += temp;
    }
    
    /* Nested loop with output dependencies (WAW) */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            /* Multiple writes to same location through different pointers */
            p[idx] = i + j;
            if (j % 4 == 0) {
                q[idx] = p[idx] * 3;  /* WAW when idx overlaps */
            }
            r[idx % (N*M/2)] = p[idx] - j;
        }
    }
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, int* restrict r2, 
                                      double* restrict d1, double* restrict d2) {
    /* restrict qualifier allows more aggressive optimization but still creates DDG edges */
    
    /* Output dependencies (WAW) within restrict pointers */
    for (int i = 2; i < P; i++) {
        d1[i] = d1[i-1] + d2[i-2];
        d2[i] = d1[i] * 0.5;
        
        /* Chain of dependencies */
        r1[i] = (int)d1[i] + r2[i-1];
        r2[i] = r1[i-2] * r2[i-1];
        
        /* Conditional output dependency */
        if (i % 7 == 0) {
            d1[i] = d2[i-3] * 2.0;  /* WAW on d1[i] */
        }
    }
    
    /* Loop with pointer arithmetic creating dependencies */
    int* pr1 = r1;
    int* pr2 = r2;
    for (int i = 0; i < P*2; i++) {
        *(pr1++) = *(pr2) + i;
        *(pr2++) = *(pr1 - 1) * 2;
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, double* dbuf) {
    union mixed_data {
        int i;
        float f;
        char bytes[4];
    } md;
    
    volatile union mixed_data vmd;
    
    /* Type-punning dependencies through union */
    for (int i = 1; i < P*4; i++) {
        md.i = ibuf[i-1];
        fbuf[i] = md.f * 1.5f;  /* Flow dependency through type punning */
        
        /* Bitwise operations creating dependencies */
        md.i = md.i ^ 0x00FF00FF;
        cbuf[i] = md.bytes[0] + cbuf[i-1];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        vmd = md;  /* Volatile use */
    }
    
    /* memcpy creating dependencies */
    for (int i = 0; i < P/2; i++) {
        memcpy(&dbuf[i], &fbuf[i*2], sizeof(float));
        /* Dependency through memcpy */
        dbuf[i] = dbuf[i] + (double)ibuf[i];
        
        /* memset with dependency */
        memset(&cbuf[i*4], ibuf[i] & 0xFF, 4);
        ibuf[i] = cbuf[i*4] + i;
    }
    
    /* Mixed integer/floating-point dependency chain */
    for (int i = 2; i < P; i++) {
        float ftemp = fbuf[i-1] + (float)ibuf[i-2];
        double dtemp = (double)ftemp * dbuf[i-1];
        ibuf[i] = (int)dtemp + ibuf[i-1];
        fbuf[i] = (float)ibuf[i] / 3.0f;
        dbuf[i] = dtemp * 0.99;
    }
}

/* Initialize arrays with pseudo-random data */
static void initialize_arrays(int arr1[N][M], float arr2[N][M], double arr3[P],
                              int* base_arr, int* alias1, int* alias2,
                              int* r1, int* r2, double* dr1, double* dr2,
                              char* cbuf, int* ibuf, float* fbuf, double* dbuf) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (float)(lcg_rand() % 1000) * 0.001f;
        }
    }
    
    for (int i = 0; i < P; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    size_t total_ints = N * M;
    for (size_t i = 0; i < total_ints; i++) {
        base_arr[i] = (int)lcg_rand() % 500;
        alias1[i] = base_arr[i] + 1;
        alias2[i] = base_arr[i] * 2;
    }
    
    for (int i = 0; i < P*2; i++) {
        r1[i] = (int)lcg_rand() % 300;
        r2[i] = (int)lcg_rand() % 300;
        dr1[i] = (double)(lcg_rand() % 1000) * 0.001;
        dr2[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    for (int i = 0; i < P*8; i++) {
        cbuf[i] = (char)(lcg_rand() % 256);
        ibuf[i] = (int)lcg_rand() % 1000;
        fbuf[i] = (float)(lcg_rand() % 1000) * 0.001f;
        dbuf[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[N][M], float arr2[N][M], double arr3[P],
                                  int* base_arr, int* r1, int* r2) {
    long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += (long long)(arr2[i][j] * 1000);
        }
    }
    
    for (int i = 0; i < P; i++) {
        checksum += (long long)(arr3[i] * 1000);
    }
    
    for (int i = 0; i < N*M; i += 7) {
        checksum += base_arr[i];
    }
    
    for (int i = 0; i < P*2; i += 3) {
        checksum += r1[i] + r2[i];
    }
    
    return checksum;
}

int main(void) {
    /* Allocate multi-dimensional arrays on heap to avoid stack overflow */
    int (*arr1)[M] = malloc(N * M * sizeof(int));
    float (*arr2)[M] = malloc(N * M * sizeof(float));
    double *arr3 = malloc(P * sizeof(double));
    
    /* Base array and aliases */
    int *base_arr = malloc(N * M * sizeof(int));
    int *alias_arr1 = malloc(N * M * sizeof(int));
    int *alias_arr2 = malloc(N * M * sizeof(int));
    
    /* Restrict-qualified arrays */
    int *restrict_arr1 = malloc(P * 2 * sizeof(int));
    int *restrict_arr2 = malloc(P * 2 * sizeof(int));
    double *restrict_darr1 = malloc(P * 2 * sizeof(double));
    double *restrict_darr2 = malloc(P * 2 * sizeof(double));
    
    /* Mixed type arrays */
    char *char_buf = malloc(P * 8 * sizeof(char));
    int *int_buf = malloc(P * 8 * sizeof(int));
    float *float_buf = malloc(P * 8 * sizeof(float));
    double *double_buf = malloc(P * 8 * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !base_arr || !alias_arr1 || !alias_arr2 ||
        !restrict_arr1 || !restrict_arr2 || !restrict_darr1 || !restrict_darr2 ||
        !char_buf || !int_buf || !float_buf || !double_buf) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    initialize_arrays(arr1, arr2, arr3, base_arr, alias_arr1, alias_arr2,
                     restrict_arr1, restrict_arr2, restrict_darr1, restrict_darr2,
                     char_buf, int_buf, float_buf, double_buf);
    
    /* Execute kernels multiple times to increase coverage probability */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify array contents between kernels using volatile ops */
        volatile int mod = iter;
        for (int i = 0; i < 10; i++) {
            arr1[mod % N][i] ^= 0x1;
        }
        
        kernel1_flow_dependencies(arr1, arr2, arr3);
        kernel2_anti_dependencies(base_arr, alias_arr1, alias_arr2);
        kernel3_restrict_pointers(restrict_arr1, restrict_arr2, 
                                 restrict_darr1, restrict_darr2);
        kernel4_mixed_types(char_buf, int_buf, float_buf, double_buf);
    }
    
    /* Compute and print checksum to prevent dead code elimination */
    long long checksum = compute_checksum(arr1, arr2, arr3, base_arr, 
                                         restrict_arr1, restrict_arr2);
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(base_arr);
    free(alias_arr1);
    free(alias_arr2);
    free(restrict_arr1);
    free(restrict_arr2);
    free(restrict_darr1);
    free(restrict_darr2);
    free(char_buf);
    free(int_buf);
    free(float_buf);
    free(double_buf);
    
    return 0;
}
