/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to work with complex control flow and high register pressure */
#define ARRAY_SIZE 256
#define UNROLL_FACTOR 4
#define NUM_VARS 16

/* Volatile function pointer to prevent optimization */
typedef void (*volatile_op_t)(int*, float*);
volatile_op_t volatile_op;

/* Memory barrier */
#define SCHED_BARRIER() asm volatile("" ::: "memory")

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Force register pressure with many local variables */
__attribute__((noinline))
static int compute_path_a(int base, int idx) {
    /* Declare many local variables to increase register pressure */
    int v0 = base + idx;
    int v1 = v0 * 3;
    int v2 = v1 - idx;
    int v3 = v2 ^ 0x55AA55AA;
    int v4 = v3 + v1;
    int v5 = v4 * 7;
    int v6 = v5 >> 3;
    int v7 = v6 & 0xFF;
    int v8 = v7 * v0;
    int v9 = v8 - v2;
    int v10 = v9 | v3;
    int v11 = v10 ^ v4;
    int v12 = v11 + v5;
    int v13 = v12 * 13;
    int v14 = v13 - v6;
    int v15 = v14 ^ v7;
    
    /* Mix integer and floating point operations */
    float f0 = v0 * 1.5f;
    float f1 = v1 * 2.5f;
    float f2 = f0 + f1;
    float f3 = f2 * 3.14f;
    float f4 = f3 - f0;
    float f5 = f4 / 2.0f;
    
    /* Create artificial dependencies */
    v0 += (int)f0;
    v1 += (int)f1;
    v2 += (int)f2;
    v3 += (int)f3;
    
    /* Memory barrier to create serialization point */
    SCHED_BARRIER();
    
    /* Complex final computation with all variables */
    return v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + 
           v10 + v11 + v12 + v13 + v14 + v15 + (int)f4 + (int)f5;
}

__attribute__((noinline))
static int compute_path_b(int base, int idx) {
    /* Different computation pattern to create scheduling complexity */
    int v0 = base - idx;
    int v1 = v0 / 2;
    int v2 = v1 + 0x12345678;
    int v3 = v2 * v0;
    int v4 = v3 ^ v1;
    int v5 = v4 << 2;
    int v6 = v5 | v2;
    int v7 = v6 - v3;
    int v8 = v7 * 11;
    int v9 = v8 ^ v4;
    int v10 = v9 + v5;
    int v11 = v10 >> 1;
    int v12 = v11 & 0xFFFF;
    int v13 = v12 * v6;
    int v14 = v13 - v7;
    int v15 = v14 ^ v8;
    
    /* Different floating point pattern */
    float f0 = v0 * 0.75f;
    float f1 = v1 * 1.25f;
    float f2 = f0 - f1;
    float f3 = f2 * 2.71f;
    float f4 = f3 + f0;
    float f5 = f4 * 0.5f;
    
    /* Create cross dependencies */
    v0 -= (int)f0;
    v1 -= (int)f1;
    v2 += (int)f2 * 2;
    v3 -= (int)f3;
    
    /* Memory barrier */
    SCHED_BARRIER();
    
    /* Different combination */
    return v0 * v1 + v2 - v3 + v4 + v5 * 2 + v6 - v7 + v8 + 
           v9 * 3 + v10 - v11 + v12 + v13 - v14 + v15 + 
           (int)(f4 * f5);
}

/* Function to create volatile operation */
__attribute__((noinline))
static void dummy_op(int* vars, float* fvars) {
    /* Force memory clobber */
    asm volatile("" : : "r"(vars), "r"(fvars) : "memory");
}

int main(void) {
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    uint32_t seed = 42;
    int i, j;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (int)(lcg(&seed) % 1000);
        array2[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Set volatile function pointer */
    volatile_op = dummy_op;
    
    int total_result = 0;
    int threshold = 500;
    
    /* Outer loop with data-dependent branching */
    for (i = 0; i < ARRAY_SIZE - UNROLL_FACTOR; i += UNROLL_FACTOR) {
        /* Unrolled inner loop to create dense basic blocks */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            int idx = i + j;
            int val = array1[idx] + array2[idx];
            
            /* Data-dependent branch - hard to predict at compile time */
            if (__builtin_expect((val & 0x3FF) > threshold, 0)) {
                /* Path A - complex computation */
                int local_vars[NUM_VARS];
                float local_floats[NUM_VARS];
                
                /* Initialize locals with varying values */
                for (int k = 0; k < NUM_VARS; k++) {
                    local_vars[k] = val + k * 17;
                    local_floats[k] = val * 0.1f + k * 0.3f;
                }
                
                /* Call volatile operation to prevent optimization */
                volatile_op(local_vars, local_floats);
                
                /* Perform computation with high register pressure */
                int result = compute_path_a(val, idx);
                
                /* Use result in unpredictable way */
                if (result & 1) {
                    array1[idx] = result;
                } else {
                    array2[idx] = result ^ 0xAAAAAAAA;
                }
                
                total_result += result;
            } else {
                /* Path B - different complex computation */
                int local_vars[NUM_VARS];
                float local_floats[NUM_VARS];
                
                /* Different initialization pattern */
                for (int k = 0; k < NUM_VARS; k++) {
                    local_vars[k] = (val ^ k) * 23;
                    local_floats[k] = (val & 0xFF) * 0.05f + k * 0.7f;
                }
                
                volatile_op(local_vars, local_floats);
                
                int result = compute_path_b(val, idx);
                
                /* Different update pattern */
                if (result % 3 == 0) {
                    array1[idx] = result >> 1;
                } else {
                    array2[idx] = result & 0x55555555;
                }
                
                total_result -= result;
            }
            
            /* Additional control flow complexity with goto */
            if (idx % 7 == 0) {
                goto merge_point;
            }
            
            /* Continue with more operations */
            array1[idx] += array2[idx] * 3;
        }
        
        merge_point:
        /* Common point with memory barrier */
        SCHED_BARRIER();
        
        /* Switch statement for additional control flow complexity */
        switch (i % 5) {
            case 0:
                total_result += array1[i] * 2;
                break;
            case 1:
                total_result -= array2[i];
                break;
            case 2:
                total_result ^= array1[i + 1];
                break;
            case 3:
                total_result |= array2[i + 1];
                break;
            case 4:
                total_result &= 0xFFFFFFFF;
                break;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    checksum += total_result;
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
