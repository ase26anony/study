/* test_ddg_coverage.c - Complex loops to trigger DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define SIZE_1D 1024

/* Volatile variables to prevent optimization */
volatile int volatile_seed = 42;
volatile int volatile_barrier = 0;

/* Simple LCG for pseudo-random initialization */
static inline int lcg_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across dimensions */
__attribute__((noinline)) 
void kernel1_flow_dependencies(int arr1[N][M], int arr2[M][P]) {
    int i, j, k;
    
    /* True dependencies (RAW) across i dimension */
    for (i = 1; i < N; i++) {
        for (j = 0; j < M - 2; j++) {
            for (k = 1; k < P; k++) {
                /* Flow dependency with distance 1 in i, 2 in j */
                arr1[i][j] = arr1[i-1][j+2] + arr2[j][k];
                
                /* Additional dependency chain */
                arr2[j][k] = arr2[j][k-1] * 2 + arr1[i][j] / 3;
            }
        }
    }
    
    /* Reverse loop with different stride */
    for (i = N - 2; i >= 0; i--) {
        for (j = M - 1; j >= 1; j--) {
            /* Anti-dependency (WAR) */
            int temp = arr1[i][j];
            arr1[i][j] = arr1[i+1][j-1] + temp;
            arr1[i+1][j-1] = temp;
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies */
__attribute__((noinline))
void kernel2_pointer_aliasing(int *base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];
    int *r = &base_arr[size/2];
    int i;
    
    /* Create aliasing pointers that may overlap */
    for (i = 1; i < size - 5; i++) {
        /* WAR: Write after read with potential aliasing */
        *p = *q + i;
        
        /* WAW: Output dependency */
        *r = *p * 2;
        
        /* Move pointers with overlapping ranges */
        p = &base_arr[i % (size-1)];
        q = &base_arr[(i + 2) % (size-1)];
        r = &base_arr[(i * 3) % (size-1)];
        
        /* Complex dependency chain */
        base_arr[i] = base_arr[i-1] + base_arr[i+1] - *q;
    }
    
    /* Loop-carried dependencies with varying distances */
    for (i = 0; i < size; i++) {
        if (i % 3 == 0 && i >= 2) {
            /* Distance 2 dependency */
            base_arr[i] = base_arr[i-2] + volatile_seed;
        } else if (i % 5 == 0 && i >= 1) {
            /* Distance 1 dependency */
            base_arr[i] = base_arr[i-1] * 2;
        } else if (i >= 3) {
            /* Distance 3 dependency */
            base_arr[i] = base_arr[i-3] / 2;
        }
    }
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
void kernel3_restrict_pointers(double *restrict d1, double *restrict d2, 
                               float *restrict f1, int n) {
    int i, j;
    
    /* Output dependencies (WAW) with restrict */
    for (i = 0; i < n; i++) {
        d1[i] = d2[i] * 3.14159;
        
        /* Inline assembly memory barrier */
        asm volatile("" ::: "memory");
        
        /* Another write creating WAW */
        d1[i] = d1[i] + f1[i % (n/2)];
    }
    
    /* Nested loops with mixed dependencies */
    for (i = 1; i < n/2; i++) {
        for (j = 0; j < 8; j++) {
            /* Flow dependency across outer loop */
            f1[i*8 + j] = (float)(d1[i-1] + d2[j]);
            
            /* Anti-dependency */
            double temp = d2[j];
            d2[j] = f1[i*8 + j] * 2.0;
            f1[i*8 + j] = (float)temp;
        }
    }
}

/* Kernel 4: Mixed data types and volatile operations */
__attribute__((noinline))
void kernel4_mixed_types(char *c_arr, int *i_arr, float *f_arr, 
                         double *d_arr, int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    int i;
    volatile int *volatile_ptr = &volatile_barrier;
    
    /* Type-punning dependencies through union */
    for (i = 0; i < size - 4; i++) {
        /* Write as int, read as float */
        u.i = i_arr[i];
        f_arr[i] = u.f * 0.5f;
        
        /* Memory barrier to enforce ordering */
        asm volatile("" ::: "memory");
        
        /* Write as char array, read as int */
        u.c[0] = c_arr[i];
        u.c[1] = c_arr[i+1];
        u.c[2] = c_arr[i+2];
        u.c[3] = c_arr[i+3];
        i_arr[i+1] = u.i + *volatile_ptr;
        
        /* Cast dependencies */
        d_arr[i] = (double)f_arr[i] + (double)i_arr[i] / 256.0;
        
        /* Bitwise operations creating dependencies */
        i_arr[i] = (i_arr[i] << 3) | (i_arr[i] >> 29);
    }
    
    /* memcpy creating dependencies */
    for (i = 0; i < size - 16; i += 8) {
        memcpy(&c_arr[i], &c_arr[i+8], 8);
        asm volatile("" ::: "memory");
        memcpy(&i_arr[i/4], &c_arr[i], 4);
    }
}

/* Main function with initialization and checksum */
int main() {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[M][P];
    int linear_arr[SIZE_1D];
    double dbl_arr[SIZE_1D];
    float flt_arr[SIZE_1D * 2];
    char char_arr[SIZE_1D * 4];
    
    int seed = 12345;
    int i, j, k;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            arr2[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (i = 0; i < SIZE_1D; i++) {
        linear_arr[i] = lcg_rand(&seed) % 1000;
        dbl_arr[i] = (double)(lcg_rand(&seed) % 1000) / 10.0;
        flt_arr[i] = (float)(lcg_rand(&seed) % 1000) / 5.0f;
    }
    
    for (i = 0; i < SIZE_1D * 4; i++) {
        char_arr[i] = (char)(lcg_rand(&seed) % 256);
    }
    
    /* Execute kernels */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Volatile operation between kernels */
    volatile_seed = linear_arr[0];
    asm volatile("" ::: "memory");
    
    kernel2_pointer_aliasing(linear_arr, SIZE_1D);
    
    volatile_barrier = linear_arr[SIZE_1D/2];
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(dbl_arr, &dbl_arr[SIZE_1D/2], 
                             flt_arr, SIZE_1D/2);
    
    volatile_seed = dbl_arr[0];
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(char_arr, linear_arr, flt_arr, dbl_arr, SIZE_1D);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    for (i = 0; i < SIZE_1D; i++) {
        checksum += linear_arr[i];
        checksum += (long long)dbl_arr[i];
        checksum += (long long)flt_arr[i];
    }
    
    for (i = 0; i < SIZE_1D * 4; i++) {
        checksum += char_arr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
