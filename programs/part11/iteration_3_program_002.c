/* test_ddg_coverage.c
 * Complex loop nests with various dependencies to trigger DDG edge creation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64
#define ITER 10

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* True dependencies (RAW) across i, j, k dimensions */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < M - 1; j++) {
            for (int k = 1; k < P - 1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j] * k;
                arr2[i][j] = arr1[i][j-1] - arr2[i-2][j] / (k + 1);
                
                /* Cross-dimensional dependency */
                if (k % 3 == 0) {
                    arr1[i][j] += arr1[i][j-2] * arr2[i-1][j+1];
                }
            }
            
            /* Loop-carried dependency with varying distance */
            if (i % 4 == 0) {
                arr1[i][j] = arr1[i-3][j] + arr2[i][j];
            } else if (i % 3 == 0) {
                arr1[i][j] = arr1[i-2][j] * arr2[i][j];
            } else {
                arr1[i][j] = arr1[i-1][j] - arr2[i][j];
            }
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
        barrier = arr1[i][0];
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    volatile int* volatile vptr = &base_arr[0];
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[2];
    
    /* Anti-dependencies through aliasing pointers */
    for (int i = 3; i < size - 3; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (aliases p+1) */
        *r = *q * 2;            /* Read from q, write to r */
        
        /* WAR: Write after read with pointer arithmetic */
        *p = *r - temp;         /* Write to p after reading from it earlier */
        
        /* Rotate pointers to create complex aliasing pattern */
        if (i % 5 == 0) {
            int* tmp_ptr = p;
            p = q;
            q = r;
            r = tmp_ptr;
        }
        
        /* Strided access with anti-dependency */
        base_arr[i * 2] = base_arr[i] + base_arr[i-1];
        base_arr[i] = base_arr[i * 2] - base_arr[i+1];  /* WAR */
    }
    
    /* Force dependency through volatile */
    *vptr = base_arr[size-1];
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict d1, 
                                double* restrict d2, 
                                double* restrict d3, 
                                int len) {
    /* Output dependencies (WAW) with restrict qualifiers */
    for (int i = 2; i < len - 2; i++) {
        /* Multiple writes to same location */
        d1[i] = d2[i-1] + d3[i+1];
        d1[i] = d1[i] * 1.5;            /* WAW on d1[i] */
        
        /* Chain of WAW dependencies */
        d2[i] = d1[i-1] * d3[i];
        d2[i] = d2[i] + 0.5;            /* WAW on d2[i] */
        d2[i] = d2[i] * 2.0;            /* Another WAW */
        
        /* Conditional WAW with varying distance */
        if (i % 7 == 0) {
            d3[i] = d1[i-3] + d2[i-4];
            d3[i] = d3[i] / 3.0;        /* WAW */
        } else {
            d3[i] = d1[i-1] * d2[i-2];
        }
    }
    
    /* Inline assembly barrier */
    asm volatile("" ::: "memory");
}

/* Kernel 4: Mixed data types and complex dependencies */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, 
                                float* f_arr, double* d_arr, 
                                int size) {
    union mixed_data {
        int i;
        float f;
        char bytes[4];
    } u;
    
    volatile union mixed_data vdata;
    
    /* Mixed type dependency chain */
    for (int i = 4; i < size - 4; i++) {
        /* Type casting creating dependencies */
        u.i = i_arr[i-1];
        f_arr[i] = (float)u.i * 0.5f;
        
        /* Bitwise operations with type punning */
        u.f = f_arr[i-2];
        i_arr[i] = u.i ^ 0xAAAAAAAA;
        
        /* Char/int mixed access */
        c_arr[i] = (char)(i_arr[i-3] & 0xFF);
        i_arr[i+1] = (int)c_arr[i-1] * 2;
        
        /* Double/float mixed computations */
        d_arr[i] = (double)f_arr[i-1] * 1.234567;
        f_arr[i+1] = (float)d_arr[i-2] / 2.0f;
        
        /* Memory function creating dependencies */
        if (i % 11 == 0) {
            memcpy(&c_arr[i], &i_arr[i-2], sizeof(int));
            memset(&c_arr[i-4], u.bytes[0], 3);
        }
        
        /* Volatile access to prevent optimization */
        vdata.i = i_arr[i];
        asm volatile("" ::: "memory");
    }
}

/* Kernel 5: Complex loop-carried dependencies with varying distances */
__attribute__((noinline))
static void kernel5_variable_distances(int arr[N][M], int pattern) {
    /* Loop-carried dependencies with conditional distances */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            /* Varying dependency distances based on conditions */
            if ((i + j) % 8 == 0) {
                arr[i][j] = arr[i-4][j-4] + pattern;
            } else if ((i + j) % 5 == 0) {
                arr[i][j] = arr[i-2][j+2] * pattern;
            } else if ((i + j) % 3 == 0) {
                arr[i][j] = arr[i-1][j-1] - arr[i-3][j+1];
            } else {
                arr[i][j] = arr[i][j] + 1;  /* Self-dependency */
            }
            
            /* Nested conditional with pointer-like access */
            int* ptr = &arr[i][j];
            if (j % 7 == 0) {
                *ptr = *(ptr - M) + *(ptr - M*2);  /* Distance = 1 and 2 rows */
            }
        }
        
        /* Memory barrier every few iterations */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int (*arr1)[M] = malloc(N * sizeof(*arr1));
    int (*arr2)[M] = malloc(N * sizeof(*arr2));
    double* darr1 = malloc(N * M * sizeof(double));
    double* darr2 = malloc(N * M * sizeof(double));
    double* darr3 = malloc(N * M * sizeof(double));
    char* carr = malloc(N * M * sizeof(char));
    float* farr = malloc(N * M * sizeof(float));
    int* iarr = malloc(N * M * sizeof(int));
    
    if (!arr1 || !arr2 || !darr1 || !darr2 || !darr3 || !carr || !farr || !iarr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            int idx = i * M + j;
            darr1[idx] = (double)(lcg_rand() % 1000) / 3.0;
            darr2[idx] = (double)(lcg_rand() % 1000) / 5.0;
            darr3[idx] = (double)(lcg_rand() % 1000) / 7.0;
            carr[idx] = (char)(lcg_rand() % 256);
            farr[idx] = (float)(lcg_rand() % 1000) / 2.0f;
            iarr[idx] = lcg_rand() % 1000;
        }
    }
    
    volatile int iter_counter = 0;
    
    /* Execute kernels multiple times to ensure DDG construction */
    for (int iter = 0; iter < ITER; iter++) {
        iter_counter = iter;
        
        /* Kernel 1: Flow dependencies */
        kernel1_flow_deps(arr1, arr2);
        
        /* Modify data between kernels using volatile */
        volatile int* vmod = (volatile int*)&arr1[0][0];
        *vmod = iter;
        
        /* Kernel 2: Anti-dependencies with pointer aliasing */
        kernel2_anti_deps(&arr1[0][0], N * M);
        
        /* Kernel 3: Output dependencies with restrict */
        kernel3_output_deps(darr1, darr2, darr3, N * M);
        
        /* Kernel 4: Mixed data types */
        kernel4_mixed_types(carr, iarr, farr, darr1, N * M);
        
        /* Kernel 5: Variable distance dependencies */
        kernel5_variable_distances(arr1, iter);
        
        /* Cross-kernel dependency through volatile */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
            int idx = i * M + j;
            checksum += (unsigned long long)darr1[idx];
            checksum += (unsigned long long)darr2[idx];
            checksum += (unsigned long long)darr3[idx];
            checksum += carr[idx];
            checksum += (unsigned long long)farr[idx];
            checksum += iarr[idx];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Iterations completed: %d\n", iter_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(darr1);
    free(darr2);
    free(darr3);
    free(carr);
    free(farr);
    free(iarr);
    
    return 0;
}
