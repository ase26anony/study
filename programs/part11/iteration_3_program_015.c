/* test_ddg_coverage.c - Complex dependency patterns for GCC DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Volatile variables to prevent optimization */
volatile int volatile_seed = 42;

/* Simple PRNG to avoid compile-time computation */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across dimensions */
__attribute__((noinline))
void kernel1_flow_deps(int arr1[N][M], int arr2[M][P]) {
    int i, j, k;
    
    /* Flow dependencies with varying distances */
    for (i = 2; i < N - 2; i++) {
        for (j = 1; j < M - 1; j++) {
            for (k = 1; k < P - 1; k++) {
                /* RAW dependency with distance 1 in i, 2 in j */
                arr1[i][j] = arr1[i-1][j] + arr1[i][j-1] * 2;
                
                /* Cross-array dependency with stride */
                arr2[j][k] = arr1[i][j] + arr2[j-1][k+1];
                
                /* Conditional loop-carried dependency */
                if ((i + j + k) % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[j][k];
                } else if ((i + j + k) % 5 == 0) {
                    arr1[i][j] = arr1[i-1][j+1] * 3 - arr2[j][k-1];
                }
            }
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
void kernel2_anti_deps(int *base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases p+1 */
    int *r = &base_arr[size/2]; /* Potential aliasing */
    
    int i;
    for (i = 2; i < size - 2; i++) {
        /* Anti-dependency: read before write to aliased location */
        int temp = p[i] + q[i-1];
        
        /* WAR: Write to location that was read in previous iteration */
        p[i-1] = temp * 2;
        
        /* More complex aliasing pattern */
        if (i % 4 == 0) {
            r[i/2] = p[i] + r[i/2 - 1];  /* Potential WAR with p[i] */
        }
        
        /* Inline assembly barrier to prevent dependency elimination */
        asm volatile("" ::: "memory");
        
        /* Output dependency (WAW) with pointer arithmetic */
        *(p + i) = *(q + i - 2) + 1;
    }
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
void kernel3_restrict_deps(int *restrict r1, int *restrict r2, 
                           int *restrict r3, int len) {
    int i;
    
    /* Output dependencies (WAW) with restrict qualifier */
    for (i = 2; i < len; i++) {
        r1[i] = r1[i] * 2;  /* Self-dependency */
        r2[i] = r1[i-1] + r2[i-2];  /* Flow dependency */
        r3[i] = r2[i] * 3 - r3[i-1];  /* Anti-dependency chain */
        
        /* WAW with different distances */
        if (i % 3 == 0) {
            r1[i] = r1[i-3] + 5;
        }
    }
}

/* Kernel 4: Mixed data types and complex dependencies */
__attribute__((noinline))
void kernel4_mixed_types(double darr[], float farr[], 
                         char carr[], int iarr[], int n) {
    int i;
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    for (i = 2; i < n - 2; i++) {
        /* Type casting creating dependencies */
        converter.i = iarr[i-1];
        farr[i] = converter.f * 1.5f;
        
        /* Mixed-type dependency chain */
        darr[i] = (double)farr[i] + darr[i-1] * 0.5;
        
        /* Bitwise operations with char array */
        carr[i] = (carr[i-1] ^ carr[i-2]) + (i & 0xFF);
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&iarr[i], &iarr[i-4], sizeof(int) * 2);
        }
        
        /* Volatile access to force dependency */
        volatile_seed = i;
        
        /* Inline assembly with memory clobber */
        asm volatile("" ::: "memory");
        
        /* Complex dependency with multiple types */
        converter.f = farr[i];
        iarr[i] = converter.i + (int)darr[i-1];
    }
}

/* Kernel 5: Multi-dimensional strided access with complex indices */
__attribute__((noinline))
void kernel5_strided_access(int arr3d[P][M][N]) {
    int i, j, k;
    
    for (i = 4; i < P - 4; i += 2) {  /* Non-unit stride */
        for (j = 3; j < M - 3; j += 3) {
            for (k = 2; k < N - 2; k += 4) {
                /* Complex strided dependencies */
                arr3d[i][j][k] = arr3d[i-2][j+1][k-1] 
                               + arr3d[i][j-3][k+2] 
                               - arr3d[i-1][j][k];
                
                /* Conditional dependency with varying distance */
                if ((i + j + k) % 7 == 0) {
                    arr3d[i][j][k] = arr3d[i-4][j][k] * 2;
                }
                
                /* Cross-dimensional dependency */
                arr3d[i-1][j+2][k-2] = arr3d[i][j][k] + 1;
            }
        }
    }
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[M][P];
    double darr[1024];
    float farr[512];
    char carr[256];
    int iarr[1024];
    int arr3d[P][M][N];
    
    int seed = 12345;
    int i, j, k;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = simple_rand(&seed) % 1000;
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            arr2[i][j] = simple_rand(&seed) % 1000;
        }
    }
    
    for (i = 0; i < 1024; i++) {
        darr[i] = (double)(simple_rand(&seed) % 1000) / 3.0;
        iarr[i] = simple_rand(&seed) % 1000;
    }
    
    for (i = 0; i < 512; i++) {
        farr[i] = (float)(simple_rand(&seed) % 1000) / 2.0f;
    }
    
    for (i = 0; i < 256; i++) {
        carr[i] = simple_rand(&seed) % 256;
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = simple_rand(&seed) % 1000;
            }
        }
    }
    
    /* Execute kernels with volatile barrier between them */
    kernel1_flow_deps(arr1, arr2);
    
    volatile_seed = 1;
    asm volatile("" ::: "memory");
    
    kernel2_anti_deps(&arr1[0][0], N * M);
    
    volatile_seed = 2;
    asm volatile("" ::: "memory");
    
    kernel3_restrict_deps(&arr1[100][0], &arr1[150][0], &arr2[50][0], M);
    
    volatile_seed = 3;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(darr, farr, carr, iarr, 512);
    
    volatile_seed = 4;
    asm volatile("" ::: "memory");
    
    kernel5_strided_access(arr3d);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            checksum += arr2[i][j];
        }
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += (long long)darr[i];
        checksum += iarr[i];
    }
    
    for (i = 0; i < 512; i++) {
        checksum += (long long)farr[i];
    }
    
    for (i = 0; i < 256; i++) {
        checksum += carr[i];
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}
