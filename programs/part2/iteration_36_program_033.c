/* sel-sched-dump-coverage.c
 * Designed to trigger debug_insn_rtx() in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force memory dependencies and prevent optimization */
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Complex loop with mixed operations and dependencies */
static inline uint64_t compute_loop(int start, int end, 
                                   float* restrict arr_f, 
                                   double* arr_d,
                                   int* arr_i,
                                   volatile int* counter) {
    double sum_d = 0.0;
    float prod_f = 1.0f;
    int acc_i = start;
    uint64_t checksum = 0;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Memory load with potential aliasing (arr_d not restrict) */
        double val_d = arr_d[i % 256];
        
        /* Integer arithmetic with carried dependency */
        acc_i = (acc_i * 1103515245 + 12345) & 0x7fffffff;
        
        /* Floating-point operations */
        prod_f *= (arr_f[i % 256] + 1.0f) * 0.5f;
        
        /* Mixed-type computation */
        sum_d += (double)acc_i * val_d / (i + 1);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive */
            sum_d /= 1.0001;
            (*counter)++;
            MEM_BARRIER();
        } else if (i % 13 == 0) {
            /* Another basic block */
            prod_f -= 0.001f;
            arr_i[i % 256] = acc_i;
        }
        
        /* Memory store with aliasing consideration */
        if (i % 5 == 0) {
            arr_f[i % 256] = prod_f;
        }
        
        /* Complex checksum calculation */
        checksum ^= (uint64_t)((int)sum_d ^ (int)(prod_f * 1000) ^ acc_i);
    }
    
    /* Final mixing */
    checksum ^= (uint64_t)(sum_d * 1000000);
    checksum ^= (uint64_t)(prod_f * 1000000);
    
    return checksum;
}

/* Wrapper to force inlining and create scheduling regions */
static inline uint64_t process_chunk(int id, int size, 
                                    float* restrict f1, 
                                    double* d1,
                                    int* i1,
                                    volatile int* cnt) {
    uint64_t result = 0;
    
    /* Multiple loop calls with different parameters */
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = id * 256 + chunk * 64;
        int end = start + 64;
        
        result ^= compute_loop(start, end, f1, d1, i1, cnt);
        
        /* Additional operations between loops */
        if (chunk % 2 == 0) {
            /* More arithmetic to create scheduling opportunities */
            for (int j = 0; j < 16; j++) {
                f1[(start + j) % 256] *= 1.01f;
                d1[(start + j) % 256] += 0.001;
            }
        }
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays with non-zero values */
    float arr_f[256];
    double arr_d[256];
    int arr_i[256];
    volatile int counter = 0;
    
    for (int i = 0; i < 256; i++) {
        arr_f[i] = (i % 37) * 0.1f + 0.5f;
        arr_d[i] = (i % 73) * 0.01 + 0.25;
        arr_i[i] = i * 3;
    }
    
    uint64_t final_checksum = 0;
    
    /* Call the hot function multiple times to create scheduling regions */
    for (int iter = 0; iter < 100; iter++) {
        /* Process multiple chunks to increase scheduling complexity */
        for (int chunk_id = 0; chunk_id < 4; chunk_id++) {
            final_checksum ^= process_chunk(chunk_id, 256, 
                                           arr_f, arr_d, arr_i, &counter);
        }
        
        /* Modify arrays between iterations to prevent complete optimization */
        for (int i = 0; i < 256; i += 8) {
            arr_f[i] += 0.001f;
            arr_d[i] *= 1.00001;
        }
        
        MEM_BARRIER();
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final checksum: %llu\n", (unsigned long long)final_checksum);
    printf("Counter: %d\n", counter);
    
    /* Additional computation using array values */
    double verify_sum = 0.0;
    for (int i = 0; i < 256; i++) {
        verify_sum += arr_f[i] + arr_d[i] + arr_i[i];
    }
    printf("Verification sum: %f\n", verify_sum);
    
    return 0;
}
