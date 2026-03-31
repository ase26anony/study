/* test_ddg_coverage.c
 * Complex dependency patterns to exercise GCC's Data Dependency Graph edge creation
 * Compile with: gcc -O2 -fmodulo-sched -fdump-ddg -fdump-rtl-sched1 test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 256
#define M 256
#define P 128
#define ITER 10

/* Simple LCG for pseudo-random initialization */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Volatile sink to prevent optimization */
static volatile int volatile_sink;

/* Kernel 1: Triple-nested loop with multi-dimensional flow dependencies */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) across all three dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Flow dependency with varying distances */
                arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1] * 2;
                
                /* Cross-array dependency */
                arr2[i][j] = arr1[i][j] + arr2[i-2][j+1];
                
                /* Strided access pattern */
                if ((i + j) % 3 == 0) {
                    arr1[i][j] += arr1[i-3][j] / 2;
                } else {
                    arr1[i][j] += arr1[i-1][j+1] * 3;
                }
            }
        }
    }
    
    /* Additional loop-carried dependency with distance > 1 */
    for (i = 4; i < N; i++) {
        /* Distance 2 and 4 dependencies */
        if (i % 3 == 0) {
            arr1[i][0] = arr1[i-2][0] + arr1[i-4][0];
        } else {
            arr1[i][0] = arr1[i-1][0] * 2;
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* restrict base_arr, int* alias_arr) {
    int i;
    int* p = &base_arr[0];
    int* q = &base_arr[1];  /* q aliases p+1 */
    int* r = alias_arr;
    
    /* Force aliasing - r may overlap with p/q */
    if (lcg_rand() % 2) {
        r = &base_arr[N/2];
    }
    
    /* Anti-dependencies through aliased pointers */
    for (i = 1; i < N-1; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (aliases p+1) - WAR */
        p = &base_arr[i];
        q = &base_arr[i+1];
        
        /* Additional WAR with r */
        temp = *r;
        r = &base_arr[(i * 7) % N];
        *r = temp * 2;
    }
    
    /* Complex pointer chasing */
    int* ptrs[4];
    for (i = 0; i < 4; i++) {
        ptrs[i] = &base_arr[(i * N/4)];
    }
    
    for (i = 0; i < N/4; i++) {
        /* Multiple reads before writes causing WAR */
        int sum = *ptrs[0] + *ptrs[1];
        *ptrs[2] = sum;
        *ptrs[3] = *ptrs[1] * 2;
        
        /* Rotate pointers */
        ptrs[0]++; ptrs[1]++; ptrs[2]++; ptrs[3]++;
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict arr3, double* restrict arr4, 
                                float* restrict arr5) {
    int i, j;
    
    /* Output dependencies (WAW) on same array */
    for (i = 0; i < N; i++) {
        double val = (double)i * 1.5;
        
        /* Multiple writes to same location */
        arr3[i] = val;
        arr3[i] = val * 2.0;  /* WAW */
        
        /* Conditional WAW */
        if (i % 4 == 0) {
            arr3[i] = val * 3.0;
        }
    }
    
    /* WAW across arrays with type conversion */
    for (i = 0; i < N/2; i++) {
        float fval = (float)arr3[i];
        arr5[i] = fval;
        arr5[i] = fval * 1.5f;  /* WAW on float array */
        
        /* Mixed type dependencies */
        arr4[i] = (double)arr5[i] * 2.0;
        arr5[i] = (float)arr4[i] / 3.0f;
    }
    
    /* Nested loops with WAW */
    for (i = 1; i < N/4; i++) {
        for (j = 1; j < M/4; j++) {
            int idx = i * (M/4) + j;
            arr3[idx] = arr3[idx-1] + 1.0;
            arr3[idx] = arr3[idx] * 1.1;  /* WAW */
            
            /* Distance-2 WAW */
            if (j % 3 == 0) {
                arr4[idx] = arr3[idx] * 2.0;
                arr4[idx] = arr4[idx] + 1.0;  /* Another WAW */
            }
        }
    }
}

/* Kernel 4: Mixed data types with inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, 
                                double* dbuf) {
    int i;
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    /* Type-punning through union creates dependencies */
    for (i = 0; i < N-4; i++) {
        u.i = ibuf[i];
        fbuf[i] = u.f * 2.0f;
        
        /* Memory barrier to force dependency */
        asm volatile("" ::: "memory");
        
        u.f = fbuf[i] + 1.0f;
        ibuf[i+1] = u.i;
        
        /* Bitwise operations with dependencies */
        ibuf[i] = (ibuf[i] << 3) | (ibuf[i] >> 29);
    }
    
    /* Volatile variables creating artificial dependencies */
    volatile int vol_var = 0;
    volatile float vol_float = 0.0f;
    
    for (i = 0; i < N; i++) {
        vol_var = i;
        ibuf[i] = vol_var * 2;
        
        vol_float = (float)ibuf[i] / 3.0f;
        fbuf[i] = vol_float;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* memcpy creating dependencies */
        if (i < N-8) {
            memcpy(&cbuf[i], &cbuf[i+4], 4);  /* Overlapping copy */
        }
    }
    
    /* Mixed-type dependency chain */
    for (i = 1; i < N-1; i++) {
        char c = cbuf[i];
        int ival = (int)c * 3;
        float fval = (float)ival / 7.0f;
        double dval = (double)fval * 1.234;
        
        dbuf[i] = dval;
        ibuf[i] = (int)dbuf[i];
        fbuf[i] = (float)ibuf[i];
        cbuf[i+1] = (char)fbuf[i];
    }
}

/* Main function orchestrating all kernels */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[N*M/2];
    double arr4[N*M/2];
    float arr5[N*M/2];
    char cbuf[N*M];
    int ibuf[N*M];
    float fbuf[N*M];
    double dbuf[N*M];
    
    int i, j;
    
    /* Initialize with LCG to avoid compile-time computation */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < N*M/2; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) / 3.0;
        arr4[i] = (double)(lcg_rand() % 1000) / 7.0;
        arr5[i] = (float)(lcg_rand() % 1000) / 5.0f;
    }
    
    for (i = 0; i < N*M; i++) {
        cbuf[i] = (char)(lcg_rand() % 256);
        ibuf[i] = lcg_rand() % 10000;
        fbuf[i] = (float)(lcg_rand() % 1000) / 11.0f;
        dbuf[i] = (double)(lcg_rand() % 1000) / 13.0;
    }
    
    /* Execute kernels multiple times to increase coverage chances */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify arrays between kernels using volatile ops */
        volatile_sink = iter;
        
        kernel1_flow_deps(arr1, arr2);
        
        /* Force dependency between kernels */
        asm volatile("" ::: "memory");
        
        kernel2_anti_deps(&arr1[0][0], &arr2[0][0]);
        
        /* Volatile store/load creating cross-kernel edges */
        volatile_sink = arr1[0][0];
        
        kernel3_output_deps(arr3, arr4, arr5);
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        kernel4_mixed_types(cbuf, ibuf, fbuf, dbuf);
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += (uint64_t)arr1[i][j];
            checksum += (uint64_t)arr2[i][j];
        }
    }
    
    for (i = 0; i < N*M/2; i++) {
        checksum += (uint64_t)arr3[i];
        checksum += (uint64_t)arr4[i];
        checksum += (uint64_t)arr5[i];
    }
    
    for (i = 0; i < N*M; i += 97) {  /* Strided access for checksum */
        checksum += (uint64_t)cbuf[i];
        checksum += (uint64_t)ibuf[i];
        checksum += (uint64_t)fbuf[i];
        checksum += (uint64_t)dbuf[i];
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
