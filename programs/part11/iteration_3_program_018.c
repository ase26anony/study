#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Volatile variables to prevent optimization */
volatile int vol_seed = 12345;
volatile int vol_barrier = 0;

/* Simple LCG for pseudo-random initialization */
static inline int lcg_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) with varying distances */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j-1];
                arr2[i][j] = arr1[i][j-3] * arr2[i-2][j];
                
                /* Loop-carried dependency with varying distance */
                if (i % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i-1][j+1];
                } else if (i % 5 == 0) {
                    arr1[i][j] = arr1[i-3][j] * arr2[i-4][j];
                } else {
                    arr1[i][j] = arr1[i-1][j] + arr2[i-2][j];
                }
            }
        }
    }
    
    /* Anti-dependencies (WAR) in reverse traversal */
    for (i = N-2; i > 0; i--) {
        for (j = M-2; j > 0; j--) {
            int temp = arr1[i][j];
            arr1[i][j] = arr2[i][j];
            arr2[i][j] = temp + arr1[i+1][j-1];  /* WAR: arr1 read after write */
        }
    }
}

/* Kernel 2: Pointer aliasing with and without restrict */
__attribute__((noinline))
void kernel2_pointer_aliasing(int *base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* May alias with p+1 */
    int *r = &base_arr[size/2];
    int *s = &base_arr[size/2 + 1];
    
    /* Aliasing pointers create complex dependencies */
    for (int i = 1; i < size-5; i++) {
        /* Output dependency (WAW) */
        p[i] = q[i-1] * 2;
        q[i] = p[i+1] + 3;
        
        /* Anti-dependency (WAR) through aliasing */
        int tmp = r[i];
        r[i] = s[i] * 7;
        s[i] = tmp - r[i-2];
        
        /* Flow dependency with pointer arithmetic */
        *(p + i) = *(q + i - 2) + *(r + i - 1);
    }
    
    /* Restrict qualified pointers - different dependency pattern */
    int *restrict rp = &base_arr[size/4];
    int *restrict rq = &base_arr[3*size/4];
    
    for (int i = 2; i < size/4 - 2; i++) {
        /* Compiler knows these don't alias */
        rp[i] = rp[i-1] + rp[i-2];  /* Simple flow dependency */
        rq[i] = rq[i-3] * rq[i-4];  /* Longer distance dependency */
    }
}

/* Kernel 3: Mixed data types and type punning */
__attribute__((noinline))
void kernel3_mixed_types(double darr[], float farr[], char carr[], int iarr[]) {
    union type_pun {
        int i;
        float f;
        char c[4];
    } u;
    
    for (int i = 4; i < 1020; i++) {
        /* Type casting creates dependencies through memory */
        u.i = iarr[i];
        farr[i] = (float)u.i + farr[i-1];  /* Flow: float dependency */
        
        /* Different data type chain */
        darr[i] = (double)farr[i] * darr[i-2];  /* Flow: double dependency */
        
        /* Char array with byte-wise dependencies */
        carr[i] = carr[i-1] + carr[i-3] - carr[i-4];
        
        /* Type punning through union */
        u.f = farr[i];
        iarr[i+1] = u.i ^ iarr[i-1];  /* Bitwise dependency */
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&carr[i], &carr[i-8], 8);  /* Creates multiple dependencies */
        }
    }
    
    /* Inline assembly memory barrier */
    asm volatile("" ::: "memory");
    
    /* Volatile operations enforce dependencies */
    for (int i = 0; i < 100; i++) {
        vol_barrier = iarr[i];
        farr[i] += vol_barrier;
    }
}

/* Kernel 4: Complex loop nests with conditional dependencies */
__attribute__((noinline))
void kernel4_conditional_deps(int arr3d[N][M][8]) {
    int i, j, k;
    
    /* 3D array with conditional loop-carried dependencies */
    for (i = 2; i < N-2; i++) {
        for (j = 2; j < M-2; j++) {
            for (k = 1; k < 7; k++) {
                /* Conditional dependency distances */
                if ((i + j + k) % 4 == 0) {
                    arr3d[i][j][k] = arr3d[i-2][j+1][k-1] 
                                   + arr3d[i-1][j-2][k+1];
                } else if ((i + j + k) % 4 == 1) {
                    arr3d[i][j][k] = arr3d[i-3][j][k] 
                                   * arr3d[i][j-3][k];
                } else if ((i + j + k) % 4 == 2) {
                    arr3d[i][j][k] = arr3d[i-1][j+2][k] 
                                   - arr3d[i][j-1][k-2];
                } else {
                    arr3d[i][j][k] = arr3d[i-4][j+1][k+1] 
                                   | arr3d[i-2][j-2][k-1];
                }
                
                /* Output dependency within same iteration */
                int tmp = arr3d[i][j][k];
                arr3d[i][j][k] = tmp * 2;
                arr3d[i][j][k] = arr3d[i][j][k] + 1;  /* WAW */
            }
            
            /* Memory barrier every 8th row */
            if (j % 8 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Volatile write to prevent cross-iteration optimization */
        vol_barrier = i;
    }
}

/* Checksum function to prevent dead code elimination */
__attribute__((noinline))
unsigned long long compute_checksum(int arr1[N][M], int arr2[N][M], 
                                   double darr[], float farr[], 
                                   char carr[], int iarr[],
                                   int arr3d[N][M][8]) {
    unsigned long long checksum = 0;
    int i, j, k;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum = checksum * 31 + arr1[i][j];
            checksum = checksum * 31 + arr2[i][j];
            
            for (k = 0; k < 8; k++) {
                checksum = checksum * 31 + arr3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 1024; i++) {
        checksum = checksum * 31 + (unsigned long long)darr[i];
        checksum = checksum * 31 + (unsigned long long)farr[i];
        checksum = checksum * 31 + (unsigned char)carr[i];
        checksum = checksum * 31 + iarr[i];
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays */
    static int arr1[N][M];
    static int arr2[N][M];
    static double darr[1024];
    static float farr[1024];
    static char carr[1024];
    static int iarr[1024];
    static int arr3d[N][M][8];
    
    int seed = vol_seed;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand(&seed) % 1000;
            arr2[i][j] = lcg_rand(&seed) % 1000;
            
            for (int k = 0; k < 8; k++) {
                arr3d[i][j][k] = lcg_rand(&seed) % 1000;
            }
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        darr[i] = (double)(lcg_rand(&seed) % 1000) / 3.0;
        farr[i] = (float)(lcg_rand(&seed) % 1000) / 7.0f;
        carr[i] = lcg_rand(&seed) % 256;
        iarr[i] = lcg_rand(&seed) % 1000;
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Modify data between kernels using volatile */
    vol_barrier = 1;
    for (int i = 0; i < 100; i++) {
        arr1[i][i] += vol_barrier;
    }
    
    kernel2_pointer_aliasing(&arr1[0][0], N*M);
    
    /* Another memory barrier */
    asm volatile("" ::: "memory");
    vol_barrier = 2;
    
    kernel3_mixed_types(darr, farr, carr, iarr);
    
    /* Scramble data between kernels */
    for (int i = 0; i < 100; i++) {
        int idx = lcg_rand(&seed) % 1024;
        darr[idx] += vol_barrier;
    }
    
    kernel4_conditional_deps(arr3d);
    
    /* Final checksum to prevent optimization */
    unsigned long long checksum = compute_checksum(arr1, arr2, darr, farr, 
                                                  carr, iarr, arr3d);
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
