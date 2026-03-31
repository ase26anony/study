/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force memory dependencies and prevent optimization */
static volatile int memory_barrier;

/* Complex loop with multiple dependencies and operations */
static inline uint64_t compute_loop(int start, int end, 
                                   float* restrict arr_f, 
                                   double* arr_d, 
                                   int* arr_i,
                                   int* restrict out) 
{
    double sum_d = 0.0;
    float prod_f = 1.0f;
    int acc_i = start;
    uint64_t checksum = 0;
    
    /* Hot loop with carried dependencies and mixed operations */
    for (int i = start; i < end; i++) {
        /* Memory load with potential aliasing (arr_d not restrict) */
        double val_d = arr_d[i % 256];
        
        /* Integer arithmetic with carried dependency */
        acc_i = (acc_i * 1103515245 + 12345) & 0x7fffffff;
        
        /* Floating-point operations */
        prod_f *= (arr_f[i % 256] + 1.0f) * 0.5f;
        
        /* Complex FP operation with dependency chain */
        sum_d += val_d / ((i & 31) + 1);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            sum_d /= 1.0001;
            prod_f *= 0.9999f;
        } else if (i % 13 == 0) {
            /* Another basic block path */
            acc_i ^= (i << 3);
        } else {
            /* Default path with mixed operations */
            sum_d = sum_d * 0.99 + val_d * 0.01;
        }
        
        /* Memory store with pointer arithmetic */
        out[i % 128] = acc_i + (int)(prod_f * 100);
        
        /* More arithmetic with different data types */
        arr_i[i % 256] = (int)(sum_d * 1000) ^ acc_i;
        
        /* Volatile to prevent reordering */
        memory_barrier = i;
        
        /* Update checksum with all values */
        checksum ^= *(uint64_t*)&sum_d;
        checksum ^= *(uint32_t*)&prod_f;
        checksum ^= acc_i;
    }
    
    return checksum;
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(float* fptr, double* dptr, int* iptr, 
                                int* optr, int size, int iter) 
{
    uint64_t chunk_sum = 0;
    
    for (int j = 0; j < iter; j++) {
        /* Call the hot loop multiple times with different offsets */
        chunk_sum ^= compute_loop(j * 64, j * 64 + size, 
                                 fptr, dptr, iptr, optr);
        
        /* Modify array contents to create new dependencies */
        for (int k = 0; k < 256; k++) {
            fptr[k] = fptr[k] * 0.9f + (float)(k * j) * 0.1f;
            dptr[k] = dptr[(k + j) % 256] * 0.95;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    if (chunk_sum == 0x12345678) {
        printf("Impossible condition\n");
    }
}

int main(void) 
{
    /* Allocate and initialize arrays with different patterns */
    float* arr_f = (float*)aligned_alloc(64, 256 * sizeof(float));
    double* arr_d = (double*)aligned_alloc(64, 256 * sizeof(double));
    int* arr_i = (int*)aligned_alloc(64, 256 * sizeof(int));
    int* out = (int*)aligned_alloc(64, 128 * sizeof(int));
    
    /* Initialize with non-uniform data */
    for (int i = 0; i < 256; i++) {
        arr_f[i] = (float)(i * 1.2345);
        arr_d[i] = (double)(i * 0.9876);
        arr_i[i] = i * 1103515245;
    }
    
    uint64_t final_checksum = 0;
    
    /* Multiple phases of computation to create scheduling regions */
    for (int phase = 0; phase < 4; phase++) {
        /* Vary loop bounds and parameters */
        int size = 64 + phase * 32;
        int iter = 8 - phase * 2;
        
        process_chunk(arr_f, arr_d, arr_i, out, size, iter);
        
        /* Cross-check between arrays */
        for (int i = 0; i < 128; i++) {
            final_checksum ^= out[i];
            final_checksum ^= *(uint64_t*)&arr_d[i];
            final_checksum += arr_i[i * 2];
        }
    }
    
    /* Additional complex loop in main */
    double running_sum = 0.0;
    for (int i = 0; i < 1000; i++) {
        /* Mix of operations with dependencies */
        running_sum = running_sum * 0.9 + arr_d[i % 256] * 0.1;
        
        /* Conditional with side effects */
        if (running_sum > 1000.0) {
            arr_f[i % 256] = (float)running_sum;
            running_sum *= 0.5;
        }
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile (
            "mov %[val], %%eax\n\t"
            "imul $1103515245, %%eax, %%eax\n\t"
            "add $12345, %%eax\n\t"
            : [val] "+r" (arr_i[i % 256])
            : 
            : "eax", "cc"
        );
        
        final_checksum ^= *(uint32_t*)&arr_f[i % 256];
    }
    
    /* Print result to prevent optimization */
    printf("Final checksum: 0x%016llx\n", (unsigned long long)final_checksum);
    
    /* Cleanup */
    free(arr_f);
    free(arr_d);
    free(arr_i);
    free(out);
    
    return 0;
}
