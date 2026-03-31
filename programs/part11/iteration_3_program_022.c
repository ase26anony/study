#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* True dependencies (RAW) across i, j, and k dimensions */
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j-1];
                arr2[i][j] = arr1[i][j-1] * 2 - arr2[i-2][j];
                
                /* Loop-carried dependency with varying distance */
                if (i % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i-1][j+1];
                } else if (i % 5 == 0) {
                    arr1[i][j] = arr1[i-3][j] * arr2[i-2][j-1];
                } else {
                    arr1[i][j] = arr1[i-1][j] + arr2[i][j];
                }
            }
        }
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
    barrier = 1;
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    volatile int barrier = 0;
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[2];
    
    /* Anti-dependencies through aliasing pointers */
    for (int i = 10; i < size - 10; i++) {
        int temp = *p;           /* Read from p */
        *q = temp + i;           /* Write to q (may alias with p) */
        temp = *r;               /* Read from r */
        *p = temp * 2;           /* Write to p (anti-dependency) */
        
        /* Pointer arithmetic to create complex aliasing patterns */
        if (i % 7 == 0) {
            p = &base_arr[i % 32];
            q = &base_arr[(i + 1) % 32];
        } else if (i % 11 == 0) {
            r = &base_arr[(i + 2) % 32];
            p = &base_arr[(i + 3) % 32];
        }
        
        /* Output dependency (WAW) */
        *q = *p + *r;
        *p = *q - *r;
    }
    
    asm volatile("" ::: "memory");
    barrier = 1;
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(int* restrict r1, int* restrict r2, 
                                int* restrict r3, int size) {
    volatile int barrier = 0;
    
    /* Output dependencies with restrict qualifier */
    for (int i = 2; i < size - 2; i++) {
        /* Multiple writes to same location through different expressions */
        r1[i] = r2[i-1] + r3[i+1];
        r1[i] = r1[i] * 3 - r2[i];  /* WAW on r1[i] */
        
        /* Chain of dependencies with restrict */
        r2[i] = r1[i-1] + r1[i-2];
        r3[i] = r2[i] * r2[i-1];
        r2[i] = r3[i] / 2;          /* Another WAW on r2[i] */
        
        /* Conditional WAW with varying distances */
        if (i % 4 == 0) {
            r1[i] = r1[i-4] + r2[i-2];
        } else {
            r1[i] = r1[i-1] * r3[i-3];
        }
    }
    
    asm volatile("" ::: "memory");
    barrier = 1;
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(double darr[], float farr[], 
                                char carr[], int iarr[], int size) {
    volatile int barrier = 0;
    
    /* Mixed data type dependencies */
    for (int i = 4; i < size - 4; i++) {
        /* Float/double/int dependencies with casting */
        float fval = farr[i-1];
        double dval = darr[i-2];
        int ival = iarr[i-3];
        
        /* Type conversions creating dependencies */
        darr[i] = (double)fval + dval * 2.0;
        farr[i] = (float)dval + (float)ival / 3.0f;
        
        /* Integer/char dependencies with bitwise ops */
        char cval = carr[i-1];
        iarr[i] = (ival & 0xFF) | ((int)cval << 8);
        carr[i] = (char)((iarr[i-2] >> 4) & 0xFF);
        
        /* Memory operations creating dependencies */
        if (i % 8 == 0) {
            memcpy(&darr[i], &darr[i-4], sizeof(double));
            memset(&carr[i-2], iarr[i-1] & 0xFF, 2);
        }
        
        /* Inline assembly memory barrier */
        asm volatile("" ::: "memory");
        
        /* Volatile access to force dependency */
        volatile int* volatile_ptr = &iarr[i];
        *volatile_ptr = *volatile_ptr + 1;
    }
    
    asm volatile("" ::: "memory");
    barrier = 1;
}

/* Helper to initialize arrays with pseudo-random values */
static void initialize_arrays(int arr1[N][M], int arr2[N][M], 
                             double darr[], float farr[], 
                             char carr[], int iarr[], int size) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < size; i++) {
        darr[i] = (double)(lcg_rand() % 1000) / 3.0;
        farr[i] = (float)(lcg_rand() % 1000) / 5.0f;
        carr[i] = (char)(lcg_rand() % 256);
        iarr[i] = lcg_rand() % 10000;
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[N][M], int arr2[N][M],
                                 double darr[], float farr[],
                                 char carr[], int iarr[], int size) {
    long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
        }
    }
    
    for (int i = 0; i < size; i++) {
        checksum += (long long)darr[i];
        checksum += (long long)farr[i];
        checksum += (long long)carr[i];
        checksum += (long long)iarr[i];
    }
    
    return checksum;
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    
    /* Allocate 1D arrays for other kernels */
    int size = 1024;
    double* darr = (double*)malloc(size * sizeof(double));
    float* farr = (float*)malloc(size * sizeof(float));
    char* carr = (char*)malloc(size * sizeof(char));
    int* iarr = (int*)malloc(size * sizeof(int));
    
    /* Initialize with pseudo-random values */
    initialize_arrays(arr1, arr2, darr, farr, carr, iarr, size);
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Modify array contents between kernels */
    volatile int mod = 0;
    for (int i = 0; i < 100; i++) {
        arr1[i % N][i % M] += mod;
        mod = arr1[i % N][i % M];
    }
    
    kernel2_anti_deps(&arr1[0][0], N * M);
    
    /* More modifications between kernels */
    asm volatile("" ::: "memory");
    for (int i = 0; i < size; i++) {
        darr[i] += 1.0;
        farr[i] += 2.0f;
    }
    
    kernel3_output_deps(iarr, &arr1[0][0], &arr2[0][0], size);
    
    /* Final modifications */
    volatile char vc = 0;
    for (int i = 0; i < size; i++) {
        carr[i] ^= vc;
        vc = carr[i];
    }
    
    kernel4_mixed_types(darr, farr, carr, iarr, size);
    
    /* Compute and print checksum */
    long long checksum = compute_checksum(arr1, arr2, darr, farr, carr, iarr, size);
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(darr);
    free(farr);
    free(carr);
    free(iarr);
    
    return 0;
}
