/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Volatile variables to prevent optimization */
volatile int volatile_seed = 12345;

/* Simple LCG for pseudo-random initialization */
static inline int lcg_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) across i dimension */
    for (i = 1; i < N; i++) {
        for (j = 0; j < M - 2; j++) {
            for (k = 1; k < P; k++) {
                /* Flow dependency with distance 1 in i, 2 in j */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j] * k;
                
                /* Additional dependency with varying distance */
                if (k % 3 == 0) {
                    arr2[i][j] = arr2[i][j] + arr1[i][j-1] * 2;
                } else {
                    arr2[i][j] = arr2[i][j] + arr1[i][j] / 2;
                }
            }
        }
    }
    
    /* Reverse loop with different stride */
    for (i = N - 2; i >= 0; i--) {
        for (j = M - 1; j >= 1; j--) {
            /* Anti-dependency (WAR) */
            int temp = arr1[i][j];
            arr1[i][j] = arr1[i+1][j-1] * 3;
            arr2[i][j] = temp + arr2[i][j];
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies */
__attribute__((noinline))
void kernel2_pointer_aliasing(int *base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];
    int *r = &base_arr[size/2];
    int *s = &base_arr[size/2 + 1];
    
    /* These pointers may alias - compiler must assume dependencies */
    for (int i = 0; i < size - 10; i++) {
        /* Anti-dependency chain */
        int read_val = *p;
        *q = read_val * 2 + i;
        
        /* Output dependency (WAW) */
        *r = *p + *q;
        *s = *r - i;
        
        /* Pointer arithmetic that may cause aliasing */
        p = &base_arr[i % (size/4)];
        q = &base_arr[(i + 1) % (size/4)];
        
        /* Conditional aliasing */
        if (i % 7 == 0) {
            r = &base_arr[i % size];
            s = &base_arr[(i + 3) % size];
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
void kernel3_restrict_pointers(int *restrict r1, int *restrict r2, 
                               int *restrict r3, int n) {
    /* With restrict, compiler knows these don't alias */
    for (int i = 2; i < n; i++) {
        /* Output dependencies (WAW) */
        r1[i] = r1[i-1] + r1[i-2];
        r2[i] = r1[i] * r2[i-1];
        
        /* Loop-carried dependency with varying distance */
        if (i % 4 == 0) {
            r3[i] = r3[i-3] + r2[i];
        } else if (i % 4 == 1) {
            r3[i] = r3[i-2] * r1[i];
        } else {
            r3[i] = r3[i-1] - r2[i];
        }
    }
    
    /* Nested loop with restrict */
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < n/2; j++) {
            /* Complex index calculation */
            int idx = (i * 17 + j * 13) % n;
            int idx2 = (i * 11 + j * 7) % n;
            
            r1[idx] = r2[idx2] + r3[(idx + idx2) % n];
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly */
__attribute__((noinline))
void kernel4_mixed_types(double *darr, float *farr, int *iarr, char *carr, int len) {
    union mixed_data {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile int *volatile_ptr = (volatile int *)iarr;
    
    for (int i = 1; i < len; i++) {
        /* Type casting creating dependencies */
        float fval = (float)darr[i-1];
        farr[i] = fval * 1.5f + (float)iarr[i];
        
        /* Bitwise operations with type punning */
        u.i = iarr[i];
        u.f = u.f * 0.5f;
        iarr[i] = u.i;
        
        /* Char array with dependencies */
        carr[i] = carr[i-1] + (i % 128);
        
        /* Volatile access creates memory barrier */
        *volatile_ptr = *volatile_ptr + 1;
        volatile_ptr = (volatile int *)&iarr[(i + 1) % len];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Mixed type dependency chain */
        darr[i] = (double)farr[i] + (double)iarr[i] * 0.01;
    }
    
    /* Memory function with overlapping regions */
    if (len > 10) {
        memcpy(&carr[5], &carr[0], 20);
        memset(&iarr[len/2], 0, sizeof(int) * 10);
    }
}

/* Kernel 5: Complex loop-carried dependencies with varying distances */
__attribute__((noinline))
void kernel5_variable_distance(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        /* Dependency distance varies based on condition */
        if (i % 5 == 0 && i >= 4) {
            arr[i] = arr[i-4] * 3 + arr[i-2];  /* Distance 4 and 2 */
        } else if (i % 3 == 0 && i >= 3) {
            arr[i] = arr[i-3] + arr[i-1] * 2;  /* Distance 3 and 1 */
        } else if (i >= 2) {
            arr[i] = arr[i-2] - arr[i-1];      /* Distance 2 and 1 */
        } else if (i >= 1) {
            arr[i] = arr[i-1] + i;             /* Distance 1 */
        }
        
        /* Additional dependency with modulo pattern */
        if (i >= 7) {
            arr[i % 7] = arr[i] + arr[(i-7) % 7];
        }
    }
    
    /* Second loop with stride access pattern */
    for (int i = 8; i < size; i += 2) {
        arr[i] = arr[i-8] + arr[i-4] + arr[i-2];
    }
}

int main() {
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[1024];
    float arr4[512];
    int arr5[2048];
    char arr6[4096];
    
    int seed = 12345;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand(&seed) % 1000;
            arr2[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        arr3[i] = (double)(lcg_rand(&seed) % 10000) / 100.0;
    }
    
    for (int i = 0; i < 512; i++) {
        arr4[i] = (float)(lcg_rand(&seed) % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 2048; i++) {
        arr5[i] = lcg_rand(&seed) % 10000;
    }
    
    for (int i = 0; i < 4096; i++) {
        arr6[i] = (char)(lcg_rand(&seed) % 256);
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Modify with volatile to prevent cross-kernel optimization */
    volatile_seed = arr1[0][0];
    
    kernel2_pointer_aliasing(&arr1[0][0], N * M);
    
    /* Memory barrier between kernels */
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(arr5, &arr5[512], &arr5[1024], 1024);
    
    volatile_seed = arr5[100];
    
    kernel4_mixed_types(arr3, arr4, arr5, arr6, 512);
    
    kernel5_variable_distance(arr5, 2048);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (long long)arr3[i];
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)arr4[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += arr5[i];
    }
    
    for (int i = 0; i < 4096; i++) {
        checksum += arr6[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    printf("Volatile seed value: %d\n", volatile_seed);
    
    return 0;
}
