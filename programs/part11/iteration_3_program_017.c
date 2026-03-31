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
                arr1[i][j] = arr1[i-1][j+1] + arr1[i][j-1] * 2 - arr1[i-2][j];
                /* Cross-array dependency */
                arr2[i][j] = arr1[i][j] + arr2[i-1][j+2] * 3;
                
                /* Conditional loop-carried dependency with varying distance */
                if ((i + j + k) % 4 == 0) {
                    arr1[i][j] = arr1[i-3][j] + arr2[i][j-2];
                } else if ((i + j + k) % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j+1] * arr2[i-1][j];
                } else {
                    arr1[i][j] = arr1[i-1][j-1] + arr2[i][j+1];
                }
            }
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    barrier = 1;
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(float* arr, double* darr) {
    volatile float v1 = 0.0f;
    volatile double v2 = 0.0;
    
    /* Create aliasing pointers */
    float* p1 = &arr[0];
    float* p2 = &arr[1];
    float* p3 = &arr[N/2];
    
    /* Anti-dependencies through aliased pointers */
    for (int i = 2; i < N-2; i++) {
        /* Read before write with aliasing - creates WAR dependencies */
        float temp = *p1 + *p2;
        
        /* Write to memory that was just read through different pointer */
        *p3 = temp * 2.5f;
        
        /* Update pointers to create moving aliases */
        p1 = &arr[i-1];
        p2 = &arr[i+1];
        p3 = &arr[i];
        
        /* Mixed-type dependency chain */
        darr[i] = (double)arr[i] * darr[i-1] + (double)arr[i-2];
        
        /* Conditional with varying dependency distance */
        if (i % 5 == 0) {
            arr[i] = arr[i-4] * 1.1f;
        } else if (i % 3 == 0) {
            arr[i] = arr[i-2] + arr[i-1];
        }
    }
    
    v1 = *p1;
    v2 = darr[N-1];
    asm volatile("" ::: "memory");
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(int* restrict r1, int* restrict r2, 
                                int* restrict r3) {
    volatile int sync = 0;
    
    /* Output dependencies (WAW) with restrict qualification */
    for (int i = 2; i < N-2; i++) {
        /* Multiple writes to same location through computation chain */
        r1[i] = i * 2;
        r1[i] = r1[i] + r2[i-1];  // WAW on r1[i]
        r1[i] = r3[i+1] - r1[i];  // Another WAW
        
        /* Parallel chains with restrict pointers */
        r2[i] = r1[i-1] * r2[i-2];
        r3[i] = r2[i+1] + r3[i-1];
        
        /* Nested output dependencies in inner loop */
        for (int j = 0; j < 8; j++) {
            r1[i] = r1[i] + j;
            r2[i] = r2[i] - j;
        }
    }
    
    sync = 1;
    asm volatile("" ::: "memory");
}

/* Kernel 4: Mixed data types, unions, and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* carr, int* iarr, 
                                float* farr, double* darr) {
    /* Union to create type-aliasing dependencies */
    union mixed {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile union mixed vu;
    
    /* Complex dependency chain with mixed types */
    for (int i = 4; i < N-4; i++) {
        /* Type punning through union creates dependencies */
        u.i = iarr[i-1];
        farr[i] = u.f * 0.5f;
        
        /* Bitwise operations creating dependencies */
        iarr[i] = (iarr[i-2] << 3) | (iarr[i-1] >> 2);
        iarr[i] = iarr[i] ^ 0xAAAAAAAA;
        
        /* Casting chain: char -> int -> float -> double */
        int temp_int = (int)carr[i] + (int)carr[i-1];
        float temp_float = (float)temp_int * 1.5f;
        darr[i] = (double)temp_float + darr[i-2];
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&carr[i], &carr[i-4], 4);
        }
        
        /* Inline assembly barrier every 16 iterations */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Volatile access to force dependency */
        vu.i = iarr[i];
        carr[i] = vu.c[0];
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
}

/* Helper to initialize arrays with pseudo-random values */
static void init_arrays(int arr1[N][M], int arr2[N][M], float farr[N], 
                       double darr[N], char carr[N], int iarr[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
        farr[i] = (float)(lcg_rand() % 1000) / 10.0f;
        darr[i] = (double)(lcg_rand() % 1000) / 10.0;
        carr[i] = (char)(lcg_rand() % 256);
        iarr[i] = (int)lcg_rand() % 10000;
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[N][M], int arr2[N][M], 
                                 float farr[N], double darr[N],
                                 char carr[N], int iarr[N]) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr1[i][j];
            sum += arr2[i][j];
        }
        sum += (long long)farr[i];
        sum += (long long)darr[i];
        sum += (int)carr[i];
        sum += iarr[i];
    }
    
    return sum;
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    float farr[N];
    double darr[N];
    char carr[N];
    int iarr[N];
    
    /* Initialize with pseudo-random values */
    init_arrays(arr1, arr2, farr, darr, carr, iarr);
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Modify array contents between kernels using volatile */
    volatile int mod = 0;
    for (int i = 0; i < 10; i++) {
        arr1[i][i] = mod;
        mod++;
    }
    
    kernel2_anti_deps(farr, darr);
    
    /* More inter-kernel modifications */
    for (int i = 0; i < N; i += 16) {
        iarr[i] = lcg_rand() % 1000;
    }
    
    kernel3_output_deps(iarr, &arr1[0][0], &arr2[0][0]);
    
    /* Final modifications before last kernel */
    memset(carr, 0, sizeof(carr[0]) * 10);
    
    kernel4_mixed_types(carr, iarr, farr, darr);
    
    /* Compute and print checksum to prevent optimization */
    long long checksum = compute_checksum(arr1, arr2, farr, darr, carr, iarr);
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
