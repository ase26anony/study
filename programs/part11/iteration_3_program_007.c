/* test_ddg_coverage.c
 * Complex loop nests designed to trigger GCC's Data Dependency Graph edge creation
 * Specifically targets ddg.cc lines 749-757 (edge structure initialization)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SIZE_2D 256
#define SIZE_1D 1024
#define ITER 100

/* Simple LCG to avoid compile-time computation */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random values */
static void init_arrays(int arr1[SIZE_2D][SIZE_2D], 
                       float arr2[SIZE_1D], 
                       double arr3[SIZE_1D]) {
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
        }
    }
    
    for (int i = 0; i < SIZE_1D; i++) {
        arr2[i] = (float)(lcg_rand() % 1000) / 3.0f;
        arr3[i] = (double)(lcg_rand() % 1000) / 7.0;
    }
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[SIZE_2D][SIZE_2D]) {
    /* Complex strided access with true dependencies (RAW) */
    for (int i = 2; i < SIZE_2D - 2; i++) {
        for (int j = 1; j < SIZE_2D - 3; j += 2) {  /* Non-unit stride */
            for (int k = 0; k < 8; k++) {
                /* Flow dependency: read after write across iterations */
                arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1] * k;
                
                /* Additional dependency chain */
                if (k > 0) {
                    arr1[i-1][j+1] = arr1[i][j] + arr1[i-2][j+3];
                }
            }
        }
    }
    
    /* Loop-carried dependency with varying distance */
    for (int i = 5; i < SIZE_2D - 5; i++) {
        if (i % 3 == 0) {
            arr1[i][i % SIZE_2D] = arr1[i-2][(i+1) % SIZE_2D] + 1;  /* Distance 2 */
        } else if (i % 4 == 0) {
            arr1[i][i % SIZE_2D] = arr1[i-3][(i+2) % SIZE_2D] * 2;  /* Distance 3 */
        } else {
            arr1[i][i % SIZE_2D] = arr1[i-1][(i+3) % SIZE_2D] / 3;  /* Distance 1 */
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(float arr2[SIZE_1D]) {
    float *p = &arr2[0];
    float *q = &arr2[1];  /* q aliases p+1 */
    float *r = &arr2[2];  /* r aliases p+2 */
    
    /* Anti-dependencies through aliasing pointers */
    for (int i = 10; i < SIZE_1D - 10; i++) {
        float temp = p[i];      /* Read from p[i] */
        q[i-1] = temp * 2.0f;   /* Write to q[i-1] which is p[i] */
        r[i-2] = q[i-1] + 1.0f; /* Chain continues */
        
        /* Additional WAR with different distances */
        if (i % 5 == 0) {
            float temp2 = r[i-5];
            p[i-3] = temp2 * 3.0f;
        }
    }
    
    /* Complex pointer arithmetic creating overlapping accesses */
    float *ptr1 = arr2 + 50;
    float *ptr2 = arr2 + 51;
    for (int i = 0; i < 200; i++) {
        ptr1[i*2] = ptr2[i*2 - 1] + ptr1[i*2 - 2];
        ptr2[i*2 + 1] = ptr1[i*2] * 0.5f;
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double arr3[SIZE_1D]) {
    double *restrict r1 = arr3 + 100;
    double *restrict r2 = arr3 + 200;
    
    /* Output dependencies with restrict qualifiers */
    for (int i = 0; i < 300; i++) {
        r1[i] = r1[i] * 2.0 + (double)i;
        r2[i] = r1[i] * 3.0;  /* No aliasing due to restrict */
        
        /* WAW on same location */
        if (i % 7 == 0) {
            r1[i] = r1[i] + 10.0;  /* Second write to r1[i] */
        }
    }
    
    /* Nested loops with output dependencies */
    for (int i = 5; i < 150; i++) {
        for (int j = 0; j < 10; j++) {
            double *tmp = &r1[i*2 + j];
            *tmp = *tmp * (double)j;
            *tmp = *tmp + 1.0;  /* WAW on *tmp */
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(int arr1[SIZE_2D][SIZE_2D],
                               float arr2[SIZE_1D],
                               double arr3[SIZE_1D]) {
    volatile int vol_counter = 0;
    union mixed_data {
        int i;
        float f;
        double d;
        char bytes[8];
    } data_union;
    
    /* Mixed type dependency chain */
    for (int i = 20; i < SIZE_1D - 20; i++) {
        /* Start with int */
        int int_val = (int)arr2[i];
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Convert to float with dependency */
        float float_val = (float)int_val + arr2[i-1];
        
        /* Another barrier */
        asm volatile("" ::: "memory");
        
        /* Convert to double */
        double double_val = (double)float_val + arr3[i-2];
        
        /* Write back through different types */
        data_union.i = int_val;
        arr3[i] = double_val + (double)data_union.f;
        
        /* Volatile forces dependency */
        vol_counter++;
        arr2[i] = float_val + (float)vol_counter;
    }
    
    /* Bitwise operations creating dependencies */
    char *byte_ptr = (char*)arr1;
    for (int i = 1000; i < 5000; i += 8) {
        byte_ptr[i] = byte_ptr[i-8] ^ 0x55;
        byte_ptr[i+1] = byte_ptr[i] & 0xAA;
        byte_ptr[i+2] = byte_ptr[i+1] | 0x33;
        
        /* Memory function creating dependencies */
        if (i % 64 == 0) {
            memcpy(&byte_ptr[i+16], &byte_ptr[i-16], 8);
        }
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[SIZE_2D][SIZE_2D],
                                 float arr2[SIZE_1D],
                                 double arr3[SIZE_1D]) {
    long long sum = 0;
    
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            sum += arr1[i][j];
        }
    }
    
    for (int i = 0; i < SIZE_1D; i++) {
        sum += (long long)arr2[i];
        sum += (long long)arr3[i];
    }
    
    return sum;
}

int main(void) {
    /* Allocate arrays with different alignments */
    int (*arr1)[SIZE_2D] = malloc(sizeof(int) * SIZE_2D * SIZE_2D);
    float *arr2 = malloc(sizeof(float) * SIZE_1D);
    double *arr3 = malloc(sizeof(double) * SIZE_1D);
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    init_arrays(arr1, arr2, arr3);
    
    /* Execute kernels multiple times to increase coverage probability */
    for (int iter = 0; iter < ITER; iter++) {
        kernel1_flow_deps(arr1);
        
        /* Volatile operation between kernels */
        volatile int barrier = iter;
        (void)barrier;
        
        kernel2_anti_deps(arr2);
        
        /* Modify array contents between kernels */
        for (int i = 0; i < 10; i++) {
            arr1[i][i] = lcg_rand() % 100;
        }
        
        kernel3_output_deps(arr3);
        
        /* Another barrier */
        asm volatile("" ::: "memory");
        
        kernel4_mixed_types(arr1, arr2, arr3);
        
        /* Shuffle data between iterations */
        if (iter % 10 == 0) {
            memmove(arr2, arr2 + 100, sizeof(float) * (SIZE_1D - 100));
        }
    }
    
    /* Compute and print checksum */
    long long checksum = compute_checksum(arr1, arr2, arr3);
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
