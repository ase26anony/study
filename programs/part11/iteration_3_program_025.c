/* test_ddg_coverage.c
 * Complex dependency patterns to exercise GCC's Data Dependency Graph edge creation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define ITER 10

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile counter to prevent optimization */
static volatile int volatile_counter = 0;

/* ========== KERNEL 1: Multi-dimensional array with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* Triple-nested loop with flow dependencies across all dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Flow (RAW) dependency with varying distances */
                arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1] * 2;
                
                /* Cross-array dependency */
                arr2[i][j] = arr1[i][j] + arr2[i-2][j+1];
                
                /* Additional dependency with non-unit stride */
                if ((i + j) % 3 == 0) {
                    arr1[i][j] += arr1[i-2][j] / 3;
                } else {
                    arr1[i][j] += arr1[i-1][j+1] * 2;
                }
            }
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    int i;
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[2];
    volatile int* volatile_ptr = &base_arr[size/2];
    
    /* Loop with anti-dependencies (WAR) */
    for (i = 2; i < size - 2; i++) {
        int temp = *p;           /* Read from p */
        *q = temp + i;           /* Write to q (aliases p+1) - WAR with next iteration */
        *r = *q * 2;             /* Read from q, write to r */
        
        /* Pointer arithmetic that may cause aliasing */
        p = &base_arr[i];
        q = &base_arr[i+1];
        r = &base_arr[i+2];
        
        /* Volatile access creates artificial dependency */
        volatile_counter += *volatile_ptr;
        
        /* Conditional dependency with varying distance */
        if (i % 4 == 0) {
            base_arr[i] = base_arr[i-3] + 1;  /* Distance 3 */
        } else if (i % 4 == 1) {
            base_arr[i] = base_arr[i-2] * 2;  /* Distance 2 */
        } else {
            base_arr[i] = base_arr[i-1] / 3;  /* Distance 1 */
        }
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict dst, 
                                 double* restrict src1, 
                                 double* restrict src2,
                                 int len) {
    int i, j;
    
    /* Output dependencies (WAW) with restrict qualifiers */
    for (i = 0; i < len; i++) {
        double acc = 0.0;
        
        /* Inner reduction loop creates flow dependencies */
        for (j = 0; j < 8; j++) {
            acc += src1[i] * src2[j] - (double)(i * j);
        }
        
        /* Multiple writes to same location - WAW */
        dst[i] = acc;
        dst[i] = dst[i] * 1.5;  /* Overwrites previous value */
        
        /* Conditional output dependency */
        if (i % 5 == 0) {
            dst[i] = src1[i] + src2[i % 8];
        }
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* ========== KERNEL 4: Mixed data types and complex dependencies ========== */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, 
                                 float* float_arr, 
                                 double* double_arr,
                                 int* int_arr,
                                 int size) {
    int i;
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    /* Loop with mixed data type dependencies */
    for (i = 1; i < size - 1; i++) {
        /* Type casting creates data flow between different types */
        int int_val = int_arr[i-1];
        float float_val = (float)int_val / 3.14f;
        
        /* Dependency chain across different arrays */
        float_arr[i] = float_val + float_arr[i-1];
        
        /* Bitwise operations with char array */
        char_arr[i] = (char)(int_arr[i] & 0xFF) ^ char_arr[i-1];
        
        /* Double precision computation with dependency */
        double_arr[i] = (double)float_arr[i] * 2.71828 - double_arr[i-1];
        
        /* Union access creates aliasing possibilities */
        u.i = int_arr[i];
        float_arr[i] += u.f;  /* Type punning */
        
        /* Memory function with overlapping regions */
        if (i % 16 == 0) {
            memcpy(&char_arr[i], &char_arr[i-8], 8);  /* Creates dependencies */
        }
        
        /* Inline assembly barrier every 32 iterations */
        if (i % 32 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* ========== KERNEL 5: Loop-carried dependencies with varying distances ========== */
__attribute__((noinline))
static void kernel5_variable_distances(int arr[N], int stride) {
    int i;
    
    /* Complex loop-carried dependencies with varying distances */
    for (i = 0; i < N; i++) {
        /* Multiple dependency distances in same loop */
        switch (i % 6) {
            case 0:
                arr[i] = arr[i-1] + stride;      /* Distance 1 */
                break;
            case 1:
                arr[i] = arr[i-2] * stride;      /* Distance 2 */
                break;
            case 2:
                arr[i] = arr[i-3] / (stride + 1); /* Distance 3 */
                break;
            case 3:
                arr[i] = arr[i-4] ^ 0xAA;        /* Distance 4 */
                break;
            case 4:
                arr[i] = arr[i-5] | 0x55;        /* Distance 5 */
                break;
            default:
                arr[i] = arr[i-6] & 0xFF;        /* Distance 6 */
                break;
        }
        
        /* Additional flow dependency */
        if (i > 10) {
            arr[i] += arr[i-10] % 7;  /* Distance 10 */
        }
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int i, j;
    
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[1024];
    float arr4[512];
    char arr5[2048];
    int linear_arr[2048];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    /* Initialize with pseudo-random values using LCG */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
            arr2[i][j] = (int)(lcg_rand() % 1000);
        }
    }
    
    for (i = 0; i < 1024; i++) {
        arr3[i] = (double)(lcg_rand() % 10000) / 100.0;
    }
    
    for (i = 0; i < 512; i++) {
        arr4[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    for (i = 0; i < 2048; i++) {
        arr5[i] = (char)(lcg_rand() % 256);
        linear_arr[i] = lcg_rand() % 10000;
    }
    
    /* Execute kernels multiple times to increase coverage chances */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify volatile counter between kernels */
        volatile_counter += iter;
        
        /* Kernel 1: Multi-dimensional flow dependencies */
        kernel1_flow_deps(arr1, arr2);
        
        /* Kernel 2: Pointer aliasing with anti-dependencies */
        kernel2_anti_deps(&linear_arr[0], 2048);
        
        /* Kernel 3: Restrict pointers with output dependencies */
        kernel3_output_deps(arr3, arr3 + 512, arr3 + 256, 256);
        
        /* Kernel 4: Mixed data types */
        kernel4_mixed_types(arr5, arr4, arr3, linear_arr, 512);
        
        /* Kernel 5: Variable distance dependencies */
        kernel5_variable_distances(linear_arr, iter + 1);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += (long long)arr3[i];
    }
    
    for (i = 0; i < 512; i++) {
        checksum += (long long)arr4[i];
    }
    
    for (i = 0; i < 2048; i++) {
        checksum += arr5[i] + linear_arr[i];
    }
    
    checksum += volatile_counter;
    
    printf("Final checksum: %lld\n", checksum);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return 0;
}
