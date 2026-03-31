/* test_ddg_coverage.c
 * Complex dependency patterns to trigger DDG edge creation in GCC
 * Compile with: gcc -O2 -fmodulo-sched -fdump-ddg test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Prevent inlining to ensure separate DDG construction per kernel */
__attribute__((noinline))
void kernel1_flow_dependencies(int arr1[N][N], int arr2[N][N]) {
    /* Triple-nested loop with flow dependencies across all dimensions */
    volatile int barrier = 0;
    
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < N-1; j++) {
            for (int k = 1; k < P; k++) {
                /* Flow dependencies with varying distances */
                if (k % 3 == 0) {
                    /* Distance 2 dependency */
                    arr1[i][j] = arr1[i-2][j+1] + arr2[i][j];
                } else if (k % 5 == 0) {
                    /* Distance 1 dependency with stride */
                    arr1[i][j] = arr1[i-1][j+2] * arr2[i-1][j];
                } else {
                    /* Complex multi-dimensional dependency */
                    arr1[i][j] = arr1[i][j-1] + arr1[i-1][j] - arr2[i][j];
                }
                
                /* Anti-dependency (WAR) */
                arr2[i][j] = arr1[i][j] + k;
                
                /* Memory barrier to prevent optimization */
                asm volatile("" ::: "memory");
            }
        }
    }
}

__attribute__((noinline))
void kernel2_pointer_aliasing(float* arr3, float* arr4, int size) {
    /* Pointer aliasing without restrict - forces conservative dependency analysis */
    float* p = &arr3[0];
    float* q = &arr3[1];  /* q aliases p+1 */
    float* r = &arr4[0];
    
    volatile float v = 0.0f;
    
    for (int i = 2; i < size-2; i++) {
        /* True dependency through aliased pointers */
        *p = *q + 1.5f;
        
        /* Anti-dependency chain */
        float temp = *r;
        *r = *p * 2.0f;
        arr4[i] = temp + v;
        
        /* Output dependency (WAW) */
        *p = arr3[i-1] + arr3[i-2];
        
        /* Shift pointers to create moving aliasing pattern */
        if (i % 4 == 0) {
            p = &arr3[i];
            q = &arr3[i+1];
        }
        
        /* Mixed data type access through casting */
        int* int_view = (int*)&arr3[i];
        *int_view += i;
    }
}

__attribute__((noinline))
void kernel3_restrict_pointers(double* restrict d1, double* restrict d2, 
                               double* restrict d3, int len) {
    /* restrict qualified pointers allow aggressive optimization */
    /* but still create output dependencies */
    
    for (int i = 1; i < len; i++) {
        /* Output dependency chain */
        d1[i] = d2[i] * 3.14;
        d1[i] = d3[i] + 2.71;  /* WAW on d1[i] */
        
        /* Loop-carried dependency with varying distance */
        if (i % 7 == 0) {
            d2[i] = d2[i-3] * d1[i-3];  /* Distance 3 */
        } else if (i % 11 == 0) {
            d2[i] = d2[i-5] + d1[i-5];  /* Distance 5 */
        } else {
            d2[i] = d2[i-1] - d1[i-1];  /* Distance 1 */
        }
        
        /* Complex expression with multiple dependencies */
        d3[i] = (d1[i] + d2[i]) * (d3[i-1] - d2[i-2]);
    }
}

__attribute__((noinline))
void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, 
                         double* dbuf, int size) {
    /* Mixed data types in dependency chains */
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    volatile int sync = 0;
    
    for (int i = 1; i < size; i++) {
        /* Type casting creating dependencies */
        converter.i = ibuf[i-1];
        fbuf[i] = converter.f * 1.1f;
        
        /* Bitwise operations with dependencies */
        ibuf[i] = (ibuf[i-1] << 2) | (ibuf[i-2] & 0xFF);
        
        /* Memory operations with dependencies */
        memcpy(&cbuf[i*4], &ibuf[i], sizeof(int));
        
        /* Double precision chain */
        dbuf[i] = dbuf[i-1] * 1.01 + (double)fbuf[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Volatile access creating artificial dependency */
        sync = i;
        cbuf[i] = (char)sync;
        
        /* Complex index calculations with modulo */
        int idx = (i * 7) % size;
        fbuf[idx] = fbuf[(idx-1+size)%size] + 0.5f;
    }
}

__attribute__((noinline))
void kernel5_stride_access(short arr5[N][M][P]) {
    /* Multi-dimensional strided access patterns */
    
    for (int i = 2; i < N-2; i += 2) {
        for (int j = 3; j < M-3; j += 3) {
            for (int k = 4; k < P-4; k += 4) {
                /* Strided flow dependencies */
                arr5[i][j][k] = arr5[i-2][j+1][k-1] + 
                               arr5[i][j-3][k+2] - 
                               arr5[i+1][j][k-2];
                
                /* Cross-iteration dependencies with different strides */
                if ((i + j + k) % 8 == 0) {
                    arr5[i][j][k] += arr5[i-1][j][k] * 2;
                }
                
                /* Anti-dependency with stride */
                short temp = arr5[i][j][k];
                arr5[i+1][j+1][k+1] = temp * 3;
                arr5[i][j][k] = arr5[i-1][j-1][k-1] + temp;
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][N];
    int arr2[N][N];
    float arr3[N*2];
    float arr4[N*2];
    double darr1[N];
    double darr2[N];
    double darr3[N];
    char cbuf[N*8];
    int ibuf[N];
    float fbuf[N];
    double dbuf[N];
    short arr5[N][M][P];
    
    /* Initialize with pseudo-random values using LCG */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
        }
        arr3[i] = (lcg_rand() % 1000) / 10.0f;
        arr4[i] = (lcg_rand() % 1000) / 10.0f;
        darr1[i] = (lcg_rand() % 1000) / 100.0;
        darr2[i] = (lcg_rand() % 1000) / 100.0;
        darr3[i] = (lcg_rand() % 1000) / 100.0;
        ibuf[i] = lcg_rand() % 1000;
        fbuf[i] = (lcg_rand() % 1000) / 10.0f;
        dbuf[i] = (lcg_rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < N*8; i++) {
        cbuf[i] = lcg_rand() % 256;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr5[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    /* Execute kernels with complex dependency patterns */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Volatile operation between kernels */
    volatile int inter_kernel_barrier = 0;
    inter_kernel_barrier = 1;
    
    kernel2_pointer_aliasing(arr3, arr4, N*2);
    
    inter_kernel_barrier = 2;
    kernel3_restrict_pointers(darr1, darr2, darr3, N);
    
    inter_kernel_barrier = 3;
    kernel4_mixed_types(cbuf, ibuf, fbuf, dbuf, N);
    
    inter_kernel_barrier = 4;
    kernel5_stride_access(arr5);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
        checksum += (int)(arr3[i] * 100) + (int)(arr4[i] * 100);
        checksum += (int)(darr1[i] * 100) + (int)(darr2[i] * 100) + (int)(darr3[i] * 100);
        checksum += ibuf[i] + (int)(fbuf[i] * 100) + (int)(dbuf[i] * 100);
    }
    
    for (int i = 0; i < N*8; i++) {
        checksum += cbuf[i];
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                checksum += arr5[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}
