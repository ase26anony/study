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

/* Simple LCG for pseudo-random initialization to avoid compile-time computation */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Volatile sink to prevent optimization */
volatile int volatile_sink;

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[SIZE_2D][SIZE_2D], 
                                      int arr2[SIZE_2D][SIZE_2D]) {
    int i, j, k;
    
    /* Complex 3-level nest with true dependencies (RAW) */
    for (i = 1; i < SIZE_2D - 1; i++) {
        for (j = 1; j < SIZE_2D - 2; j++) {
            /* Flow dependency with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j+2] + arr2[i][j];
            
            for (k = 2; k < SIZE_2D - 3; k += 3) {
                /* Multi-dimensional strided access with loop-carried dependency */
                arr2[i][j] = arr2[i][j] + arr1[i][k] * arr1[i-1][k-2];
                
                /* Additional dependency chain */
                arr1[i][k] = arr1[i][k-1] + arr2[i-2][k+1];
            }
            
            /* Anti-dependency (WAR) within same iteration */
            int temp = arr1[i][j];
            arr1[i][j] = j * 2;
            arr2[i][j] = temp + arr2[i][j-1];
        }
    }
    
    /* Output dependency (WAW) with varying distance */
    for (i = 3; i < SIZE_2D; i++) {
        if (i % 4 == 0) {
            arr1[i][i % SIZE_2D] = arr1[i-3][(i+1) % SIZE_2D] + 1;
        } else {
            arr1[i][i % SIZE_2D] = arr1[i-1][(i+2) % SIZE_2D] * 2;
        }
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases p+1 */
    int *r = &base_arr[size/2]; /* Potential aliasing with p/q depending on size */
    
    int i;
    
    /* Create anti-dependencies through aliased pointers */
    for (i = 1; i < size - 5; i++) {
        /* WAR: Read from p[i], then write to q[i-1] which may alias p[i] */
        int val = p[i];
        q[i-1] = val * 3;
        
        /* Complex aliasing pattern */
        if (i % 3 == 0) {
            r[i % (size/2)] = p[i-2] + q[i+1];
        } else {
            p[i+1] = r[(i-1) % (size/2)] - val;
        }
        
        /* Output dependency through potentially aliased pointers */
        if (i % 5 == 0) {
            q[i-3] = p[i] * 2;
            p[i] = i;  /* WAW if q[i-3] aliases p[i] */
        }
    }
    
    /* Additional loop with pointer arithmetic creating dependencies */
    int *ptr1 = base_arr;
    int *ptr2 = base_arr + 10;
    for (i = 10; i < size - 20; i++) {
        ptr1[i] = ptr2[i-10] + ptr1[i-5];
        ptr2[i-9] = ptr1[i] * ptr2[i-11];
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, 
                                      int* restrict r2,
                                      int* restrict r3,
                                      int size) {
    int i;
    
    /* With restrict, compiler knows no aliasing - tests dependency analysis */
    for (i = 2; i < size - 2; i++) {
        /* Pure flow dependencies without aliasing concerns */
        r1[i] = r1[i-1] + r1[i-2];
        r2[i] = r1[i] * r2[i-1];
        r3[i] = r2[i] + r3[i-2];
    }
    
    /* Output dependencies on restrict-qualified arrays */
    for (i = 0; i < size - 1; i += 2) {
        r1[i] = i * i;
        r1[i] = r1[i] + r2[i/2];  /* WAW on r1[i] */
        
        /* Conditional WAW with different distances */
        if (i % 3 == 0) {
            r2[i] = r1[i-3] + 7;
        } else {
            r2[i] = r1[i-1] * 2;
        }
    }
}

/* ========== KERNEL 4: Mixed data types and assembly barriers ========== */
__attribute__((noinline))
static void kernel4_mixed_types_asm(void) {
    int int_arr[SIZE_1D];
    float float_arr[SIZE_1D];
    double double_arr[SIZE_1D];
    char char_arr[SIZE_1D * 4];
    
    union mixed_union {
        int i;
        float f;
        char bytes[4];
    } u_arr[SIZE_1D/4];
    
    int i;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE_1D; i++) {
        int_arr[i] = lcg_rand() % 1000;
        float_arr[i] = (lcg_rand() % 1000) / 10.0f;
        double_arr[i] = (lcg_rand() % 1000) / 100.0;
    }
    
    /* Dependency chain across different data types */
    for (i = 1; i < SIZE_1D - 1; i++) {
        /* int -> float -> double dependency */
        int temp_int = int_arr[i-1] + int_arr[i];
        
        /* Memory barrier to force dependency edge creation */
        asm volatile("" ::: "memory");
        
        float_arr[i] = (float)temp_int / 2.0f;
        
        /* volatile operation to prevent optimization */
        volatile_sink = i;
        
        double_arr[i] = (double)float_arr[i] * 1.5;
        
        /* Type punning through union creates dependencies */
        u_arr[i % (SIZE_1D/4)].i = temp_int;
        float_arr[i+1] = u_arr[i % (SIZE_1D/4)].f;  /* May create flow dep */
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Bitwise operations with dependencies */
        char_arr[i] = (char)(int_arr[i] & 0xFF);
        int_arr[i] = (int)char_arr[i-1] << 8;
    }
    
    /* memcpy creating dependencies */
    for (i = 0; i < SIZE_1D - 64; i += 32) {
        memcpy(&char_arr[i+16], &char_arr[i], 16);
        asm volatile("" ::: "memory");
        memcpy(&int_arr[i/4 + 4], &int_arr[i/4], 8);
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Allocate multi-dimensional arrays on heap to avoid stack overflow */
    int (*arr1)[SIZE_2D] = malloc(SIZE_2D * SIZE_2D * sizeof(int));
    int (*arr2)[SIZE_2D] = malloc(SIZE_2D * SIZE_2D * sizeof(int));
    int *arr3 = malloc(SIZE_1D * sizeof(int));
    int *arr4 = malloc(SIZE_1D * sizeof(int));
    int *arr5 = malloc(SIZE_1D * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < SIZE_1D; i++) {
        arr3[i] = lcg_rand() % 1000;
        arr4[i] = lcg_rand() % 1000;
        arr5[i] = lcg_rand() % 1000;
    }
    
    /* Execute kernels multiple times to ensure coverage */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify array contents with volatile between kernels */
        volatile_sink = iter;
        
        kernel1_flow_dependencies(arr1, arr2);
        
        /* Volatile operation to prevent cross-kernel optimization */
        asm volatile("" ::: "memory");
        
        kernel2_pointer_aliasing(arr3, SIZE_1D);
        
        /* Another barrier */
        volatile_sink = arr3[SIZE_1D/2];
        
        kernel3_restrict_pointers(arr4, arr5, arr3, SIZE_1D);
        
        /* Force dependency between kernel calls */
        arr3[0] = iter;
        
        kernel4_mixed_types_asm();
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            checksum += (uint64_t)arr1[i][j] + (uint64_t)arr2[i][j];
        }
    }
    
    for (int i = 0; i < SIZE_1D; i++) {
        checksum += (uint64_t)arr3[i] + (uint64_t)arr4[i] + (uint64_t)arr5[i];
    }
    
    checksum += (uint64_t)volatile_sink;
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    
    return 0;
}
