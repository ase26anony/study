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
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* Flow dependencies (RAW) across i, j, and k dimensions */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < M - 1; j++) {
            for (int k = 1; k < P - 1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j] * k;
                arr2[i][j] = arr1[i][j-1] + arr2[i-2][j] - k;
                
                /* Additional dependency with varying distance */
                if (k % 3 == 0) {
                    arr1[i][j] = arr1[i][j] + arr1[i-2][j+1];
                } else {
                    arr1[i][j] = arr1[i][j] + arr1[i-1][j-1];
                }
            }
        }
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(int* arr, int size) {
    int *p = &arr[0];
    int *q = &arr[1];  /* q aliases with p+1 */
    int *r = &arr[2];  /* r aliases with p+2 */
    
    volatile int v = 0;
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (int i = 3; i < size - 3; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (aliases p+1) - WAR with next iteration */
        *r = *q * 2;            /* Read from q, write to r */
        
        /* Shift pointers to create overlapping accesses */
        p = &arr[i];
        q = &arr[i+1];
        r = &arr[i+2];
        
        /* Conditional dependency with varying distance */
        if (i % 4 == 0) {
            arr[i] = arr[i-3] + arr[i-1];
        } else if (i % 4 == 1) {
            arr[i] = arr[i-2] * arr[i-1];
        } else {
            arr[i] = arr[i-1] + 1;
        }
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_dependencies(double* restrict d1, 
                                        double* restrict d2, 
                                        float* farr, int size) {
    /* Output dependencies (WAW) with restrict qualifiers */
    for (int i = 2; i < size - 2; i++) {
        /* WAW on d1[i] */
        d1[i] = d2[i-1] * 3.14;
        d1[i] = d2[i-2] * 2.71;  /* Overwrites previous write - WAW */
        
        /* Mixed data type dependencies */
        farr[i] = (float)d1[i] + (float)d2[i-1];
        
        /* Loop-carried dependency with varying distance */
        if (i % 5 == 0) {
            d2[i] = d1[i-4] + d2[i-2];
        } else {
            d2[i] = d1[i-1] + d2[i-1];
        }
        
        /* Additional output dependency */
        farr[i] = farr[i] * 2.0f;  /* WAW on farr[i] */
    }
}

/* Kernel 4: Mixed data types, volatile, and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* carr, int* iarr, 
                                float* farr, double* darr, int size) {
    union mixed_data {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile union mixed_data vu;
    
    /* Complex dependency chain with mixed types */
    for (int i = 4; i < size - 4; i++) {
        /* Type casting creating dependencies */
        u.i = iarr[i-1];
        farr[i] = (float)u.i * 1.5f;
        
        /* Bitwise operations creating dependencies */
        iarr[i] = (iarr[i-2] << 2) | (iarr[i-1] >> 1);
        
        /* Char array access with dependencies */
        carr[i] = (char)(iarr[i] & 0xFF);
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&carr[i], &carr[i-4], 4);
        }
        
        /* Volatile access forcing dependency */
        vu.i = iarr[i];
        darr[i] = (double)vu.i + darr[i-1];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Additional mixed-type dependency */
        u.f = farr[i-1];
        iarr[i+1] = u.i + iarr[i-3];
    }
}

/* Kernel 5: Complex loop-carried dependencies with varying distances */
__attribute__((noinline))
static void kernel5_varying_distances(int arr[N][M], int arr2[N][M]) {
    /* Complex loop-carried dependencies with varying distances */
    for (int i = 2; i < N - 2; i++) {
        for (int j = 2; j < M - 2; j++) {
            /* Multiple dependencies with different distances */
            if ((i + j) % 3 == 0) {
                arr[i][j] = arr[i-2][j+1] + arr[i-1][j-2];  /* distance 2 and 1 */
            } else if ((i + j) % 3 == 1) {
                arr[i][j] = arr[i-1][j+2] * arr[i-2][j-1];  /* distance 1 and 2 */
            } else {
                arr[i][j] = arr[i-1][j-1] + arr[i-1][j+1];  /* distance 1 */
            }
            
            /* Cross-array dependency */
            arr2[i][j] = arr[i][j] + arr2[i-1][j] + arr2[i][j-1];
            
            /* Additional strided access */
            if (j % 4 == 0) {
                arr[i][j] = arr[i][j] + arr[i][j-3];
            }
        }
        
        /* Volatile write to prevent optimization */
        volatile int* vptr = &arr[i][0];
        *vptr = *vptr + 1;
    }
}

int main(void) {
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N * M];
    double darr1[N * M / 2];
    double darr2[N * M / 2];
    float farr[N * M];
    char carr[N * M];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            linear_arr[i * M + j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < N * M / 2; i++) {
        darr1[i] = (double)(lcg_rand() % 1000) / 10.0;
        darr2[i] = (double)(lcg_rand() % 1000) / 10.0;
        farr[i] = (float)(lcg_rand() % 1000) / 10.0f;
        carr[i] = (char)(lcg_rand() % 256);
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Modify array contents between kernels using volatile */
    volatile int mod = 0;
    for (int i = 0; i < 10; i++) {
        arr1[i][i] += mod;
        mod++;
    }
    
    kernel2_anti_dependencies(linear_arr, N * M);
    
    /* More volatile modifications */
    asm volatile("" ::: "memory");
    for (int i = 0; i < 10; i++) {
        darr1[i] += (double)mod;
    }
    
    kernel3_output_dependencies(darr1, darr2, farr, N * M / 2);
    
    /* Additional barrier */
    volatile double vd = 0.0;
    for (int i = 0; i < 10; i++) {
        vd += darr1[i];
    }
    
    kernel4_mixed_types(carr, linear_arr, farr, darr1, N * M / 2);
    
    /* More modifications */
    asm volatile("" ::: "memory");
    for (int i = 0; i < 10; i++) {
        arr2[i][i] = mod;
    }
    
    kernel5_varying_distances(arr1, arr2);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < N * M; i++) {
        checksum += linear_arr[i] + (int)carr[i];
    }
    
    for (int i = 0; i < N * M / 2; i++) {
        checksum += (long long)darr1[i] + (long long)darr2[i] + (long long)farr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
