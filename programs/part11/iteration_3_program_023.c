#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 256

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile variables to prevent optimization */
volatile int volatile_barrier = 0;

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M], double arr3[P]) {
    int i, j, k;
    
    /* True dependencies (RAW) with varying distances */
    for (i = 1; i < N; i++) {
        for (j = 1; j < M - 2; j++) {
            /* Flow dependency with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr2[i][j];
            
            /* Flow dependency with distance 2 in j dimension */
            if (j % 3 == 0) {
                arr2[i][j] = arr2[i][j-2] * 2;
            } else {
                arr2[i][j] = arr2[i][j-1] + 1;
            }
            
            /* Cross-array dependency */
            for (k = 0; k < P; k += 8) {
                /* Strided access with dependency */
                arr3[k] = arr3[k] + arr1[i][j] * 0.5;
            }
        }
    }
    
    /* Anti-dependency (WAR) in reverse traversal */
    for (i = N - 2; i >= 0; i--) {
        for (j = M - 2; j >= 0; j--) {
            int temp = arr1[i][j];  /* Read */
            arr1[i+1][j+1] = temp * 3;  /* Write to different location */
            arr1[i][j] = temp + arr2[i][j];  /* Write after read */
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* arr, float* farr) {
    int *p = &arr[0];
    int *q = &arr[1];  /* q aliases with p+1 */
    int *r = &arr[N/2];
    float *fp = farr;
    
    /* WAR dependencies through aliasing pointers */
    for (int i = 0; i < N - 10; i++) {
        int val = *p;           /* Read from p */
        *q = val * 2;           /* Write to q (aliases p+1) - anti-dependency */
        
        /* Output dependency (WAW) */
        *p = *r + i;            /* Write to p */
        *r = *p / 2;            /* Write to r - flow dependency */
        
        /* Mixed type dependency with casting */
        *fp = (float)(*p) * 0.5f;
        *p = (int)(*fp * 2.0f);
        
        /* Pointer arithmetic with potential aliasing */
        p++;
        q++;
        if (i % 4 == 0) {
            r++;
        }
        fp += 2;  /* Strided access */
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, int* restrict r2, 
                                      double* restrict d1, double* restrict d2) {
    /* With restrict, compiler knows pointers don't alias */
    /* This creates different DDG patterns than kernel2 */
    
    for (int i = 2; i < N - 2; i++) {
        /* Loop-carried flow dependency with distance 2 */
        r1[i] = r1[i-2] + r2[i];
        
        /* Output dependency chain */
        d1[i] = d2[i] * 3.14;
        d2[i] = d1[i] / 2.0;
        d1[i] = d2[i] + 1.0;
        
        /* Conditional dependency distance */
        if (i % 5 == 0) {
            r2[i] = r2[i-3] * 2;  /* Distance 3 */
        } else if (i % 5 == 1) {
            r2[i] = r2[i-1] + r2[i-4];  /* Multiple distances */
        } else {
            r2[i] = r2[i-2] - 1;  /* Distance 2 */
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Kernel 4: Mixed data type dependencies and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(char* carr, short* sarr, int* iarr, 
                                float* farr, double* darr) {
    union mixed_types {
        int i;
        float f;
        char bytes[4];
    } u;
    
    volatile int* volatile_ptr = &volatile_barrier;
    
    for (int i = 4; i < N - 4; i++) {
        /* Type punning through union creates dependencies */
        u.i = iarr[i-1];
        farr[i] = u.f * 2.0f;
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Dependency through volatile */
        (*volatile_ptr)++;
        iarr[i] = *volatile_ptr + iarr[i-2];
        
        /* Bitwise operations with dependencies */
        sarr[i] = (short)((carr[i-1] << 8) | carr[i-3]);
        carr[i] = (char)((sarr[i] & 0xFF) ^ carr[i-2]);
        
        /* Mixed size memory operations */
        if (i % 8 == 0) {
            memcpy(&darr[i], &farr[i-2], sizeof(float));
            darr[i] += (double)iarr[i-4];
        }
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 5: Complex strided access patterns */
__attribute__((noinline))
static void kernel5_strided_access(int arr3d[N][M][8]) {
    int i, j, k;
    
    /* 3D array with strided dependencies */
    for (i = 2; i < N - 2; i += 2) {  /* Stride 2 in outer loop */
        for (j = 3; j < M - 3; j += 3) {  /* Stride 3 in middle loop */
            for (k = 1; k < 7; k++) {
                /* Flow dependency with 3D neighborhood */
                arr3d[i][j][k] = arr3d[i-1][j][k] +      /* Distance 1 in i */
                                 arr3d[i][j-2][k+1] +    /* Distance 2 in j, 1 in k */
                                 arr3d[i-2][j+1][k-1];   /* Distance 2 in i, 1 in j, 1 in k */
                
                /* Anti-dependency in k dimension */
                int temp = arr3d[i][j][k-1];
                arr3d[i][j][k] = arr3d[i][j][k] * temp;
                arr3d[i][j][k-1] = temp + i;
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int (*arr1)[M] = malloc(N * M * sizeof(int));
    int (*arr2)[M] = malloc(N * M * sizeof(int));
    double *arr3 = malloc(P * sizeof(double));
    int *arr_flat = malloc(N * M * sizeof(int));
    float *farr = malloc(N * M * sizeof(float));
    double *darr1 = malloc(N * sizeof(double));
    double *darr2 = malloc(N * sizeof(double));
    char *carr = malloc(N * sizeof(char));
    short *sarr = malloc(N * sizeof(short));
    int *iarr = malloc(N * sizeof(int));
    float *farr2 = malloc(N * sizeof(float));
    int (*arr3d)[M][8] = malloc(N * M * 8 * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            arr_flat[i * M + j] = lcg_rand() % 1000;
            farr[i * M + j] = (float)(lcg_rand() % 1000) * 0.1f;
        }
    }
    
    for (int i = 0; i < P; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) * 0.01;
    }
    
    for (int i = 0; i < N; i++) {
        darr1[i] = (double)(lcg_rand() % 1000) * 0.001;
        darr2[i] = (double)(lcg_rand() % 1000) * 0.001;
        carr[i] = (char)(lcg_rand() % 256);
        sarr[i] = (short)(lcg_rand() % 65536);
        iarr[i] = lcg_rand() % 10000;
        farr2[i] = (float)(lcg_rand() % 1000) * 0.1f;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < 8; k++) {
                arr3d[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    /* Execute kernels with volatile barriers between them */
    volatile_barrier = 1;
    kernel1_flow_dependencies(arr1, arr2, arr3);
    
    asm volatile("" ::: "memory");
    volatile_barrier++;
    kernel2_pointer_aliasing(arr_flat, farr);
    
    asm volatile("" ::: "memory");
    volatile_barrier++;
    kernel3_restrict_pointers(arr_flat, &arr_flat[N/2], darr1, darr2);
    
    asm volatile("" ::: "memory");
    volatile_barrier++;
    kernel4_mixed_types(carr, sarr, iarr, farr2, darr1);
    
    asm volatile("" ::: "memory");
    volatile_barrier++;
    kernel5_strided_access(arr3d);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
            checksum += (unsigned long long)farr[i * M + j];
        }
    }
    
    for (int i = 0; i < P; i++) {
        checksum += (unsigned long long)arr3[i];
    }
    
    for (int i = 0; i < N; i++) {
        checksum += iarr[i];
        checksum += (unsigned long long)darr1[i];
        checksum += (unsigned long long)darr2[i];
        checksum += carr[i];
        checksum += sarr[i];
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < 8; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr_flat);
    free(farr);
    free(darr1);
    free(darr2);
    free(carr);
    free(sarr);
    free(iarr);
    free(farr2);
    free(arr3d);
    
    return 0;
}
