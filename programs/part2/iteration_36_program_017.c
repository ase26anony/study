/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization */
static volatile int global_seed = 42;
static volatile float global_float_acc = 1.0f;
static volatile double global_double_acc = 1.0;

/* Function with memory aliasing - both restrict and non-restrict pointers */
static inline void compute_loop(int* restrict arr1, int* arr2, float* farr, 
                               double* darr, int start, int end, int modifier) {
    int local_sum = 0;
    float local_float = 2.0f;
    double local_double = 3.0;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Carried dependency - running sum */
        local_sum += arr1[i] * modifier;
        
        /* Independent floating point operations */
        local_float = local_float * 1.01f + farr[i % ARRAY_SIZE];
        local_double = local_double / 1.001 + darr[i % ARRAY_SIZE];
        
        /* Memory store with potential aliasing */
        arr2[i % ARRAY_SIZE] = local_sum;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            local_sum += global_seed;
            local_float *= 0.99f;
            
            /* More complex operation with division */
            if (i % 14 == 0) {
                local_double /= 1.0001;
                arr1[i % ARRAY_SIZE] += (int)(local_double * 100);
            }
        } else if (i % 3 == 0) {
            /* Another branch with different operations */
            local_sum -= modifier;
            local_float += global_float_acc;
        }
        
        /* More arithmetic diversity */
        local_sum = (local_sum * 1103515245 + 12345) & 0x7fffffff;
        
        /* Mix integer and floating point */
        if (i % 5 == 0) {
            farr[i % ARRAY_SIZE] = local_float * 0.5f;
            darr[i % ARRAY_SIZE] = local_double + 0.1;
        }
        
        /* Prevent loop invariant code motion */
        arr1[i % ARRAY_SIZE] = (arr1[i % ARRAY_SIZE] + i) & 0xFF;
    }
    
    /* Update global accumulators */
    global_float_acc += local_float;
    global_double_acc += local_double;
    global_seed = local_sum & 0xFF;
}

/* Another inline function to increase scheduling complexity */
static inline int process_chunk(int* data, float* fdata, double* ddata, 
                               int chunk_size, int offset) {
    int temp[ARRAY_SIZE];
    int result = 0;
    
    for (int i = 0; i < chunk_size; i++) {
        /* Complex addressing with multiple array accesses */
        int idx = (i + offset) % ARRAY_SIZE;
        
        /* Mixed operations */
        temp[i] = data[idx] * 2 - data[(idx + 1) % ARRAY_SIZE];
        fdata[idx] = fdata[idx] * 1.5f + (float)temp[i];
        ddata[idx] = ddata[idx] * 0.9 + (double)(i % 16);
        
        /* Conditional with side effects */
        if (temp[i] > 100) {
            result += temp[i] / 3;
            fdata[idx] -= 10.0f;
        } else {
            result += temp[i] * 2;
            ddata[idx] += 0.5;
        }
        
        /* Memory barrier effect */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

int main() {
    /* Initialize arrays with different patterns */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    float farray[ARRAY_SIZE];
    double darray[ARRAY_SIZE];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FF;
        array2[i] = (i * 1664525 + 1013904223) & 0x3FF;
        farray[i] = (float)(i % 100) * 0.123f;
        darray[i] = (double)(i % 200) * 0.456;
    }
    
    int total_result = 0;
    
    /* Call the hot function multiple times with different parameters */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        int chunk = iter % 8;
        int start = chunk * (ARRAY_SIZE / 8);
        int end = start + (ARRAY_SIZE / 8);
        
        /* This is the main hot loop we want to schedule */
        compute_loop(array1, array2, farray, darray, start, end, iter + 1);
        
        /* Process chunks with the second function */
        if (iter % 100 == 0) {
            total_result += process_chunk(array1, farray, darray, 
                                         ARRAY_SIZE / 4, iter % ARRAY_SIZE);
        }
        
        /* Occasionally modify global state */
        if (iter % 1000 == 0) {
            global_seed = (global_seed * 1664525 + 1013904223) & 0xFF;
        }
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
        checksum ^= *(int*)&farray[i];  /* Treat float bits as int */
    }
    
    /* Use results to prevent dead code elimination */
    total_result += checksum;
    total_result += (int)global_float_acc;
    total_result += (int)global_double_acc;
    
    printf("Result: %d (checksum: %d)\n", total_result, checksum);
    
    return total_result != 0 ? 0 : 1;
}
