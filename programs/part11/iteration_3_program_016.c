/* test_ddg_coverage.c
 * Complex loops to trigger DDG edge creation in GCC's data dependency graph builder
 * Compile with: gcc -O2 -fmodulo-sched -fdump-ddg test_ddg_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 256
#define M 128
#define P 64

/* Simple LCG for pseudo-random initialization */
static inline unsigned int lcg(unsigned int *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Kernel 1: Triple-nested loop with flow dependencies across dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    volatile int sink = 0;
    
    /* Flow dependencies (RAW) across i, j, k dimensions */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < M - 1; j++) {
            for (int k = 1; k < P - 1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+1] + arr2[i][j-1];
                arr2[i][j] = arr1[i][j-2] * arr2[i-2][j];
                
                /* Additional dependency with varying distance */
                if (k % 3 == 0) {
                    arr1[i][j] += arr1[i][j-3];
                } else if (k % 5 == 0) {
                    arr1[i][j] += arr1[i-2][j];
                }
                
                /* Memory barrier to prevent optimization */
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* Prevent dead code elimination */
    sink = arr1[0][0] + arr2[0][0];
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int *base_arr, int size) {
    volatile int sink = 0;
    
    /* Create aliasing pointers */
    int *p = &base_arr[0];
    int *q = &base_arr[1];
    int *r = &base_arr[2];
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (int i = 3; i < size - 3; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (may alias with p) */
        
        /* Complex aliasing pattern */
        if (i % 4 == 0) {
            *r = *p + *q;       /* Read from potentially aliased p and q */
            p = &base_arr[i % 16];
        } else if (i % 7 == 0) {
            q = &base_arr[(i + 3) % 16];
        }
        
        /* Output dependency (WAW) */
        base_arr[i % 8] = base_arr[(i + 1) % 8] * 2;
        base_arr[i % 8] = base_arr[(i + 2) % 8] + 1;  /* Overwrites previous */
        
        /* Varying dependency distances */
        if (i % 11 == 0) {
            base_arr[i] = base_arr[i-5] + base_arr[i-3];
        }
    }
    
    sink = *p + *q;
}

/* Kernel 3: Loop with restrict pointers and output dependencies */
__attribute__((noinline))
static void kernel3_output_deps(double *restrict d1, 
                                 double *restrict d2, 
                                 double *restrict d3, 
                                 int len) {
    volatile double sink = 0.0;
    
    /* Output dependencies (WAW) with restrict qualifiers */
    for (int i = 2; i < len - 2; i++) {
        /* Chain of output dependencies */
        d1[i] = d2[i-1] * d3[i+1];
        d1[i] = d1[i] + 1.5;          /* WAW on d1[i] */
        
        d2[i] = d1[i-2] + d3[i-1];
        d2[i] = d2[i] * 0.5;          /* WAW on d2[i] */
        
        /* Loop-carried flow dependency with restrict */
        d3[i] = d3[i-1] + d3[i-2];
        
        /* Conditional dependency distance */
        if (i % 13 == 0) {
            d1[i] = d1[i-7] * 3.14;
        } else if (i % 17 == 0) {
            d2[i] = d2[i-4] / 2.718;
        }
    }
    
    sink = d1[0] + d2[0] + d3[0];
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(float *farr, double *darr, 
                                 char *carr, int *iarr, int len) {
    volatile float fsink = 0.0f;
    volatile int isink = 0;
    
    union mixed_data {
        int i;
        float f;
        char c[4];
    } u;
    
    /* Mixed type dependencies with casting */
    for (int i = 4; i < len - 4; i++) {
        /* Type casting creating dependencies */
        u.i = iarr[i-1];
        farr[i] = u.f + (float)darr[i-2];
        
        /* Bitwise operations */
        iarr[i] = (iarr[i-1] << 2) | (iarr[i-3] >> 1);
        
        /* Char array access with pointer arithmetic */
        carr[i] = (char)(farr[i-1] * 100);
        carr[i+1] = carr[i] + carr[i-2];
        
        /* Memory function creating dependencies */
        if (i % 19 == 0) {
            memcpy(&iarr[i], &iarr[i-8], sizeof(int));
        }
        
        /* Multiple inline assembly barriers */
        asm volatile("" ::: "memory");
        asm volatile("" ::: "memory");
        
        /* Volatile variable creating artificial dependency */
        volatile int vol_var = i;
        iarr[i] += vol_var;
        
        /* Union access with different types */
        u.f = farr[i-1];
        darr[i] = (double)u.i / 256.0;
    }
    
    fsink = farr[0];
    isink = iarr[0];
}

/* Kernel 5: Complex loop-carried dependencies with varying distances */
__attribute__((noinline))
static void kernel5_varying_distances(int arr[N][M], int brr[N][M]) {
    volatile int sink = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = 2; i < N - 2; i++) {
        for (int j = 2; j < M - 2; j++) {
            /* Dependency distance varies based on conditions */
            if ((i + j) % 3 == 0) {
                arr[i][j] = arr[i-2][j+1] + brr[i-1][j-2];  /* distance 2 in i */
            } else if ((i + j) % 5 == 0) {
                arr[i][j] = arr[i-3][j] * brr[i][j-3];      /* distance 3 in j */
            } else if ((i + j) % 7 == 0) {
                arr[i][j] = arr[i-1][j-4] - brr[i-4][j-1];  /* distance 4 in both */
            } else {
                arr[i][j] = arr[i-1][j-1] + 1;              /* distance 1 */
            }
            
            /* Anti-dependency with pointer aliasing */
            int *ptr1 = &arr[i][j];
            int *ptr2 = &arr[(i+1)%N][(j-1+M)%M];
            int temp = *ptr1;
            *ptr2 = temp + i * j;
            
            /* Output dependency chain */
            brr[i][j] = arr[i][j] * 2;
            brr[i][j] = brr[i][j] + arr[i-1][j];  /* WAW on brr[i][j] */
            
            /* Memory barrier every 8 iterations */
            if (j % 8 == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
    
    sink = arr[0][0] + brr[0][0];
}

int main(void) {
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    double darr1[512];
    double darr2[512];
    float farr[1024];
    char carr[1024];
    int iarr[1024];
    
    /* Initialize with pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg(&seed) % 1000;
            arr2[i][j] = (int)lcg(&seed) % 1000;
        }
    }
    
    for (int i = 0; i < 512; i++) {
        darr1[i] = (double)(lcg(&seed) % 1000) / 10.0;
        darr2[i] = (double)(lcg(&seed) % 1000) / 10.0;
    }
    
    for (int i = 0; i < 1024; i++) {
        farr[i] = (float)(lcg(&seed) % 1000) / 10.0f;
        carr[i] = (char)(lcg(&seed) % 256);
        iarr[i] = lcg(&seed) % 10000;
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Modify arrays between kernels using volatile operations */
    volatile int mod = 1;
    for (int i = 0; i < 10; i++) {
        arr1[i][i] += mod;
    }
    
    kernel2_anti_deps(&arr1[0][0], N * M);
    
    /* More volatile modifications */
    volatile double dmod = 3.14159;
    for (int i = 0; i < 20; i++) {
        darr1[i] += dmod;
    }
    
    kernel3_output_deps(darr1, darr2, darr1, 512);  /* darr1 aliased twice */
    
    kernel4_mixed_types(farr, darr1, carr, iarr, 1024);
    
    /* Final complex kernel */
    kernel5_varying_distances(arr1, arr2);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)darr1[i] + (long long)darr2[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (long long)farr[i] + carr[i] + iarr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
