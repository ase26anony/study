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
#define ITERATIONS 100

/* Simple LCG for pseudo-random initialization to avoid compile-time computation */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Volatile sink to prevent optimization */
volatile int volatile_sink;

/* ========== KERNEL 1: Multi-dimensional array with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_dependencies(void) {
    int arr1[SIZE_2D][SIZE_2D];
    int arr2[SIZE_2D][SIZE_2D];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            arr1[i][j] = lcg_rand() % 100;
            arr2[i][j] = lcg_rand() % 100;
        }
    }
    
    /* Triple-nested loop with complex flow dependencies */
    for (int t = 0; t < ITERATIONS; t++) {
        for (int i = 1; i < SIZE_2D - 1; i++) {
            for (int j = 1; j < SIZE_2D - 1; j++) {
                /* Flow (RAW) dependencies with varying distances */
                arr1[i][j] = arr1[i-1][j+1] + arr1[i][j-2] * 3;  /* Distance 1 in i, 2 in j */
                arr2[i][j] = arr1[i][j] + arr2[i-2][j] - arr2[i][j-1]; /* Distance 2 in i, 1 in j */
                
                /* Anti-dependency (WAR) */
                int temp = arr1[i][j];  /* Read */
                arr1[i][j] = temp + arr2[i][j];  /* Write after read */
                
                /* Output dependency (WAW) with conditional */
                if ((i + j) % 3 == 0) {
                    arr2[i][j] = arr1[i][j] * 2;
                } else {
                    arr2[i][j] = arr1[i][j] / 2;  /* WAW on arr2[i][j] */
                }
            }
        }
        
        /* Memory barrier to prevent loop fusion */
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            sum += arr1[i][j] + arr2[i][j];
        }
    }
    volatile_sink = sum;
}

/* ========== KERNEL 2: Pointer aliasing without restrict ========== */
__attribute__((noinline))
static void kernel2_pointer_aliasing(void) {
    int array[SIZE_1D * 2];
    int *p = &array[0];
    int *q = &array[1];  /* q aliases p+1 */
    int *r = &array[SIZE_1D];  /* Non-overlapping region */
    
    /* Initialize */
    for (int i = 0; i < SIZE_1D * 2; i++) {
        array[i] = lcg_rand() % 1000;
    }
    
    /* Complex loop with aliasing-induced dependencies */
    for (int i = 2; i < SIZE_1D - 2; i++) {
        /* p and q alias, creating WAR and WAW dependencies */
        int val1 = p[i];      /* Read through p */
        q[i-1] = val1 * 2;    /* Write through q (aliases p[i-1]?) */
        
        /* Flow dependency through aliased pointers */
        p[i+1] = q[i] + p[i-1];  /* q[i] aliases p[i+1] */
        
        /* Output dependency with varying distance */
        if (i % 4 == 0) {
            r[i % SIZE_1D] = p[i] * 3;
        } else if (i % 4 == 1) {
            r[(i-1) % SIZE_1D] = p[i] + 5;  /* Different distance */
        }
        
        /* Anti-dependency chain */
        int temp = r[i % SIZE_1D];
        p[i] = temp - 1;
        r[i % SIZE_1D] = p[i] * 2;
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Checksum */
    int sum = 0;
    for (int i = 0; i < SIZE_1D * 2; i++) {
        sum += array[i];
    }
    volatile_sink = sum;
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_pointers(void) {
    int src[SIZE_1D];
    int dst[SIZE_1D];
    int tmp[SIZE_1D];
    
    /* Initialize */
    for (int i = 0; i < SIZE_1D; i++) {
        src[i] = lcg_rand() % 500;
        dst[i] = lcg_rand() % 500;
        tmp[i] = 0;
    }
    
    /* Use restrict to allow better optimization but create explicit dependencies */
    int *restrict r1 = src;
    int *restrict r2 = dst;
    int *restrict r3 = tmp;
    
    /* Loop with output dependencies (WAW) */
    for (int i = 2; i < SIZE_1D - 2; i++) {
        /* Multiple writes to same location create WAW edges */
        if (i % 5 == 0) {
            r2[i] = r1[i-2] + r1[i+1];  /* First write */
        }
        
        r2[i] = r2[i] * 2;  /* Second write to same location (WAW) */
        
        /* Flow dependency chain with restrict */
        r3[i] = r1[i] + r2[i-1];
        r1[i+1] = r3[i] * 3;  /* Flow to next iteration */
        
        /* Another WAW on r3 */
        r3[i] = r1[i] - r2[i];
    }
    
    /* Nested loop with carried dependency */
    for (int i = 0; i < 10; i++) {
        for (int j = 2; j < SIZE_1D/10; j++) {
            int idx = i * (SIZE_1D/10) + j;
            /* Loop-carried dependency with distance 3 */
            r2[idx] = r2[idx-3] + r1[idx];
        }
        asm volatile("" ::: "memory");
    }
    
    /* Checksum */
    int sum = 0;
    for (int i = 0; i < SIZE_1D; i++) {
        sum += src[i] + dst[i] + tmp[i];
    }
    volatile_sink = sum;
}

/* ========== KERNEL 4: Mixed data types and assembly barriers ========== */
__attribute__((noinline))
static void kernel4_mixed_types(void) {
    /* Mixed type arrays */
    int int_arr[SIZE_1D];
    float float_arr[SIZE_1D];
    double double_arr[SIZE_1D];
    char char_arr[SIZE_1D];
    
    union mixed_union {
        int i;
        float f;
        char bytes[4];
    } union_arr[SIZE_1D/4];
    
    /* Initialize */
    for (int i = 0; i < SIZE_1D; i++) {
        int_arr[i] = lcg_rand() % 100;
        float_arr[i] = (lcg_rand() % 100) / 10.0f;
        double_arr[i] = (lcg_rand() % 100) / 5.0;
        char_arr[i] = lcg_rand() % 256;
    }
    for (int i = 0; i < SIZE_1D/4; i++) {
        union_arr[i].i = lcg_rand();
    }
    
    /* Complex dependency chain across different data types */
    for (int i = 3; i < SIZE_1D - 3; i++) {
        /* Flow dependency with type conversion */
        float f_temp = (float)int_arr[i-1] + float_arr[i-2];
        int_arr[i] = (int)f_temp;  /* RAW: int_arr[i-1] -> int_arr[i] */
        
        /* Anti-dependency with different type */
        double d_temp = double_arr[i];  /* Read */
        double_arr[i] = d_temp * 1.5;   /* Write after read (WAR) */
        
        /* Output dependency through union */
        union_arr[i/4].i = int_arr[i] * 2;  /* First write */
        union_arr[i/4].f = float_arr[i] * 3.0f;  /* Second write (WAW on union) */
        
        /* Dependency through char array with memcpy */
        if (i % 8 == 0) {
            memcpy(&char_arr[i], &int_arr[i], 1);  /* Creates dependencies */
        }
        
        /* Inline assembly barrier creating artificial dependencies */
        asm volatile("" ::: "memory");
        
        /* Mixed operations continuing the chain */
        float_arr[i+1] = (float)union_arr[i/4].i / 10.0f;
        double_arr[i] = (double)float_arr[i+1] + double_arr[i-1];
    }
    
    /* Bitwise operations creating dependencies */
    for (int i = 1; i < SIZE_1D; i++) {
        int_arr[i] = (int_arr[i-1] << 2) | (int_arr[i] & 0xFF);
        char_arr[i] = char_arr[i-1] ^ char_arr[i];
    }
    
    /* Final checksum */
    int sum = 0;
    for (int i = 0; i < SIZE_1D; i++) {
        sum += int_arr[i] + (int)float_arr[i] + (int)double_arr[i] + char_arr[i];
    }
    for (int i = 0; i < SIZE_1D/4; i++) {
        sum += union_arr[i].i;
    }
    volatile_sink = sum;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    printf("Starting DDG edge creation test...\n");
    
    /* Execute all kernels to trigger DDG construction during compilation */
    kernel1_flow_dependencies();
    kernel2_pointer_aliasing();
    kernel3_restrict_pointers();
    kernel4_mixed_types();
    
    printf("Test completed. Check DDG dumps for edge creation coverage.\n");
    printf("volatile_sink = %d\n", volatile_sink);
    
    return 0;
}
