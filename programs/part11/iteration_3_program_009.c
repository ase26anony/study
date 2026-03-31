/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64
#define ITER 10

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random values */
static void init_arrays(int arr1[N][N], float arr2[M][M], double arr3[P][P], 
                       char arr4[N*M], int arr5[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(lcg_rand() % 1000) / 1000.0f;
        }
    }
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < P; j++) {
            arr3[i][j] = (double)(lcg_rand() % 1000) / 1000.0;
        }
    }
    
    for (int i = 0; i < N*M; i++) {
        arr4[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < N; i++) {
        arr5[i] = (int)(lcg_rand() % 1000);
    }
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][N], float arr2[M][M], double arr3[P][P]) {
    /* Complex 3D flow dependencies with varying strides */
    for (int i = 2; i < N-2; i++) {
        for (int j = 1; j < N-1; j += 2) {  /* Non-unit stride */
            for (int k = 0; k < N-3; k += 3) {  /* Another non-unit stride */
                /* Flow (RAW) dependency: read after write with distance 2 in i dimension */
                arr1[i][j] = arr1[i-2][j+1] + arr1[i][k] * 2;
                
                /* Additional flow dependency with distance 1 in j dimension */
                if (j > 2) {
                    arr1[i][j] += arr1[i][j-1] - arr1[i-1][j+2];
                }
                
                /* Cross-array flow dependency */
                if (i < M && j < M) {
                    arr2[i][j] = (float)arr1[i][j] * 0.5f + arr2[i-1][j+1];
                }
                
                /* Another flow dependency with distance 3 in k dimension */
                if (k >= 3 && i < P && k < P) {
                    arr3[i][k] = arr3[i][k-3] * 1.5 + (double)arr1[i][j];
                }
            }
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(int arr5[N], char arr4[N*M]) {
    int *p = &arr5[0];
    int *q = &arr5[1];  /* q aliases with p+1 */
    int *r = &arr5[2];  /* r aliases with p+2 */
    
    char *cp1 = &arr4[0];
    char *cp2 = &arr4[1];  /* cp2 aliases with cp1+1 */
    
    /* Anti-dependencies (WAR): write after read with aliasing pointers */
    for (int i = 1; i < N-2; i++) {
        int temp = *p;      /* Read from p */
        *q = temp + i;      /* Write to q (aliases p+1) - WAR dependency */
        
        char ctemp = *cp1;  /* Read from cp1 */
        *cp2 = ctemp ^ 0x55; /* Write to cp2 (aliases cp1+1) - Another WAR */
        
        /* Complex anti-dependency chain */
        if (i % 4 == 0) {
            int temp2 = *r;   /* Read from r */
            *p = temp2 * 3;   /* Write to p - WAR with distance */
            r = &arr5[i+1];   /* Change pointer - creates complex aliasing */
        }
        
        /* Pointer arithmetic creating overlapping accesses */
        p = &arr5[i];
        q = &arr5[i+1];
        r = &arr5[i+2];
        
        cp1 = &arr4[i*2];
        cp2 = &arr4[i*2 + 1];
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_dependencies(int arr1[N][N], double arr3[P][P]) {
    /* Use restrict to allow better optimization but still create WAW */
    int *restrict rp1 = &arr1[0][0];
    int *restrict rp2 = &arr1[1][0];
    double *restrict rp3 = &arr3[0][0];
    double *restrict rp4 = &arr3[1][0];
    
    /* Output dependencies (WAW) with restrict pointers */
    for (int i = 0; i < N-1; i++) {
        for (int j = 0; j < N-1; j++) {
            int idx = i * N + j;
            
            /* WAW: Multiple writes to same location through restrict pointers */
            rp1[idx] = i * j;
            if (j % 3 == 0) {
                rp1[idx] = rp1[idx] + rp2[idx];  /* Another write - WAW */
            }
            
            /* WAW with different data types through union */
            if (i < P-1 && j < P-1) {
                double dval = (double)(i + j);
                rp3[i * P + j] = dval;
                
                /* Conditional WAW */
                if ((i + j) % 5 == 0) {
                    rp3[i * P + j] = dval * 2.0;  /* Another write to same location */
                }
                
                /* WAW through pointer arithmetic */
                *(rp4 + i * P + j) = rp3[i * P + j] + 1.0;
            }
        }
    }
}

/* Kernel 4: Mixed data types, volatile, and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_dependencies(volatile int* varr, float arr2[M][M], 
                                      double arr3[P][P], char arr4[N*M]) {
    /* Mixed data type dependency chain */
    for (int i = 1; i < M-1; i++) {
        for (int j = 1; j < M-1; j++) {
            /* int -> float -> double dependency chain */
            int int_val = (int)arr2[i][j] * 100;
            float float_val = (float)int_val / 50.0f + arr2[i-1][j];
            
            /* Inline assembly memory barrier */
            asm volatile("" ::: "memory");
            
            /* volatile creates artificial dependencies */
            *varr = int_val;
            int volatile_read = *varr;
            
            /* Continue dependency chain */
            arr2[i][j] = float_val + (float)volatile_read;
            
            /* double with bitwise operations through union */
            if (i < P && j < P) {
                union {
                    double d;
                    unsigned long long ull;
                } u;
                u.d = arr3[i][j];
                u.ull = (u.ull ^ 0xAAAAAAAAAAAAAAAALL) >> 2;
                arr3[i][j] = u.d + (double)float_val;
            }
            
            /* char array with memcpy creating dependencies */
            if (i*M + j < N*M - 16) {
                char temp[16];
                memcpy(temp, &arr4[i*M + j], 16);  /* Read dependency */
                for (int k = 0; k < 16; k++) {
                    temp[k] = (temp[k] + 1) % 256;
                }
                memcpy(&arr4[i*M + j], temp, 16);  /* Write dependency */
            }
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
    }
}

/* Kernel 5: Loop-carried dependencies with varying distances */
__attribute__((noinline))
static void kernel5_varying_distance(int arr5[N], int arr1[N][N]) {
    /* Loop-carried dependencies with varying distances */
    for (int i = 0; i < N; i++) {
        /* Varying dependency distances based on conditions */
        if (i % 4 == 0 && i >= 3) {
            /* Distance 3 dependency */
            arr5[i] = arr5[i-3] * 2 + 1;
        } else if (i % 3 == 0 && i >= 2) {
            /* Distance 2 dependency */
            arr5[i] = arr5[i-2] + arr5[i-1];
        } else if (i >= 1) {
            /* Distance 1 dependency */
            arr5[i] = arr5[i-1] ^ 0xFF;
        } else {
            arr5[i] = i;
        }
        
        /* Nested loop with 2D distance vector */
        if (i < N-1) {
            for (int j = 1; j < N-1; j++) {
                /* Distance vector (1,1) */
                arr1[i][j] = arr1[i-1][j-1] + arr5[i];
                
                /* Another distance vector (0,2) */
                if (j >= 2) {
                    arr1[i][j] += arr1[i][j-2] - arr5[i-1];
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[N][N], float arr2[M][M], 
                                 double arr3[P][P], char arr4[N*M], 
                                 int arr5[N]) {
    long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr1[i][j];
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (long long)(arr2[i][j] * 1000);
        }
    }
    
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < P; j++) {
            checksum += (long long)(arr3[i][j] * 1000);
        }
    }
    
    for (int i = 0; i < N*M; i++) {
        checksum += arr4[i];
    }
    
    for (int i = 0; i < N; i++) {
        checksum += arr5[i];
    }
    
    return checksum;
}

int main(void) {
    /* Allocate arrays with different dimensions and types */
    int arr1[N][N];
    float arr2[M][M];
    double arr3[P][P];
    char arr4[N*M];
    int arr5[N];
    volatile int varr = 0;
    
    /* Initialize with pseudo-random values */
    init_arrays(arr1, arr2, arr3, arr4, arr5);
    
    /* Execute kernels multiple times to increase coverage chance */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify volatile between kernels to prevent cross-kernel optimization */
        varr = iter;
        
        /* Execute all kernels */
        kernel1_flow_dependencies(arr1, arr2, arr3);
        kernel2_anti_dependencies(arr5, arr4);
        kernel3_output_dependencies(arr1, arr3);
        kernel4_mixed_dependencies(&varr, arr2, arr3, arr4);
        kernel5_varying_distance(arr5, arr1);
        
        /* Shuffle data between iterations */
        for (int i = 0; i < N; i++) {
            arr5[i] ^= 0xAA;
        }
    }
    
    /* Compute and print checksum */
    long long checksum = compute_checksum(arr1, arr2, arr3, arr4, arr5);
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
