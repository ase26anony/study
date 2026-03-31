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
static void kernel1_flow_dependencies(int arr1[N][M], float arr2[N][M]) {
    volatile int barrier = 0;
    
    /* Flow dependencies (RAW) across i, j, and k dimensions */
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < M - 1; j++) {
            for (int k = 1; k < P - 1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1] * 2 - arr1[i-2][j];
                
                /* Cross-dimensional dependency with varying distance */
                if (i % 3 == 0) {
                    arr2[i][j] = arr2[i-2][j+1] * 1.5f + arr2[i][j-2];
                } else {
                    arr2[i][j] = arr2[i-1][j] * 2.0f - arr2[i][j-1];
                }
                
                /* Memory barrier to prevent optimization */
                if (barrier == 0) {
                    asm volatile("" ::: "memory");
                }
            }
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(double* arr3, int size) {
    /* Create aliasing pointers */
    double* p = &arr3[0];
    double* q = &arr3[1];
    double* r = &arr3[2];
    volatile double* vp = &arr3[size/2];
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (int i = 3; i < size - 3; i++) {
        double temp = *p + *q;  /* Read from p and q */
        
        /* Write to locations that may alias with reads */
        *p = temp * 1.1;
        *q = *r * 0.9;
        
        /* Pointer arithmetic that creates aliasing */
        if (i % 5 == 0) {
            p = &arr3[i];
            q = &arr3[i-2];
            r = &arr3[i+1];
        } else if (i % 7 == 0) {
            p = &arr3[i-1];
            q = &arr3[i+2];
            r = &arr3[i-3];
        }
        
        /* Volatile write to create memory dependency */
        *vp = temp;
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_dependencies(int* restrict res1, int* restrict res2, 
                                        const int* restrict src1, const int* restrict src2, int len) {
    /* Output dependencies (WAW) with restrict qualifiers */
    for (int i = 2; i < len - 2; i++) {
        /* Multiple writes to same location with restrict */
        int val1 = src1[i] + src1[i-1];
        int val2 = src2[i] * 2 - src2[i-2];
        
        /* WAW dependency - multiple assignments */
        res1[i] = val1 + val2;
        res1[i] = res1[i] * 3;  /* Overwrites previous value */
        
        /* Conditional WAW with varying distance */
        if (i % 4 == 0) {
            res2[i] = res1[i-2] + res1[i+1];
            res2[i] = res2[i] / 2;  /* Another WAW */
        } else {
            res2[i] = res1[i-1] * res1[i+2];
            res2[i] = res2[i] % 100;  /* Another WAW */
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed data types and complex dependencies */
__attribute__((noinline))
static void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, double* dbuf, int size) {
    union mixed_data {
        int i;
        float f;
        char c[4];
    } data;
    
    volatile union mixed_data vdata;
    
    /* Complex dependency chain with mixed types */
    for (int i = 4; i < size - 4; i++) {
        /* Type casting creating dependencies */
        data.i = ibuf[i] + ibuf[i-1];
        fbuf[i] = (float)data.i * 0.5f;
        
        /* Bitwise operations with type punning */
        data.f = fbuf[i-2] + fbuf[i+1];
        ibuf[i] = data.i ^ 0xAAAAAAAA;
        
        /* Char array access with dependencies */
        cbuf[i] = (char)(ibuf[i] & 0xFF);
        cbuf[i+1] = cbuf[i] + cbuf[i-1];
        
        /* Double precision with memory function dependency */
        dbuf[i] = (double)fbuf[i] * 2.0;
        
        /* memcpy creating dependencies */
        if (i % 8 == 0) {
            memcpy(&cbuf[i-2], &cbuf[i+1], 3);
        }
        
        /* Volatile union access */
        vdata.i = i;
        fbuf[i] += vdata.f;
        
        /* Multiple barriers */
        asm volatile("" ::: "memory");
        asm volatile("" ::: "memory");
    }
}

/* Kernel 5: Loop-carried dependencies with varying distances */
__attribute__((noinline))
static void kernel5_varying_distances(int* arr, int* brr, int size) {
    /* Loop-carried dependencies with different distances */
    for (int i = 4; i < size - 4; i++) {
        /* Varying dependency distances based on conditions */
        if (i % 3 == 0) {
            /* Distance 2 dependency */
            arr[i] = arr[i-2] + brr[i+1];
            brr[i] = arr[i] * 2;
        } else if (i % 5 == 0) {
            /* Distance 3 dependency */
            arr[i] = arr[i-3] - brr[i-1];
            brr[i] = arr[i] / 2;
        } else {
            /* Distance 1 dependency */
            arr[i] = arr[i-1] + brr[i];
            brr[i] = arr[i] % 100;
        }
        
        /* Additional cross-iteration dependency */
        if (i > 10 && i % 7 == 0) {
            brr[i] = brr[i-5] + brr[i-3] + brr[i-1];
        }
        
        /* Volatile variable for dependency */
        volatile int dep = arr[i];
        brr[i] += dep;
    }
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    float arr2[N][M];
    double arr3[N * 2];
    int res1[N], res2[N];
    char cbuf[N * 4];
    int ibuf[N * 2];
    float fbuf[N * 2];
    double dbuf[N * 2];
    int larr[N * 3], lbrr[N * 3];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
            arr2[i][j] = (float)(lcg_rand() % 1000) * 0.1f;
        }
    }
    
    for (int i = 0; i < N * 2; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) * 0.01;
        ibuf[i] = lcg_rand() % 1000;
        fbuf[i] = (float)(lcg_rand() % 1000) * 0.1f;
        dbuf[i] = (double)(lcg_rand() % 1000) * 0.01;
    }
    
    for (int i = 0; i < N * 4; i++) {
        cbuf[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < N * 3; i++) {
        larr[i] = lcg_rand() % 1000;
        lbrr[i] = lcg_rand() % 1000;
    }
    
    for (int i = 0; i < N; i++) {
        res1[i] = lcg_rand() % 1000;
        res2[i] = lcg_rand() % 1000;
    }
    
    /* Execute kernels with volatile operations between them */
    volatile int sync = 0;
    
    kernel1_flow_dependencies(arr1, arr2);
    sync = 1;
    asm volatile("" ::: "memory");
    
    kernel2_anti_dependencies(arr3, N * 2);
    sync = 2;
    asm volatile("" ::: "memory");
    
    kernel3_output_dependencies(res1, res2, ibuf, &ibuf[N], N);
    sync = 3;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(cbuf, ibuf, fbuf, dbuf, N * 2);
    sync = 4;
    asm volatile("" ::: "memory");
    
    kernel5_varying_distances(larr, lbrr, N * 3);
    sync = 5;
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += (unsigned long long)arr2[i][j];
        }
    }
    
    for (int i = 0; i < N * 2; i++) {
        checksum += (unsigned long long)arr3[i];
        checksum += ibuf[i];
        checksum += (unsigned long long)fbuf[i];
        checksum += (unsigned long long)dbuf[i];
    }
    
    for (int i = 0; i < N * 4; i++) {
        checksum += cbuf[i];
    }
    
    for (int i = 0; i < N * 3; i++) {
        checksum += larr[i];
        checksum += lbrr[i];
    }
    
    for (int i = 0; i < N; i++) {
        checksum += res1[i];
        checksum += res2[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
