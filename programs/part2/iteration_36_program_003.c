/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;
static volatile double g_volatile_double = 1.0;

/* Arrays with potential aliasing */
static float f_array[SIZE];
static double d_array[SIZE];
static int i_array[SIZE];

/* Inline function with hot loop - designed for selective scheduling */
static inline __attribute__((always_inline)) 
double compute_loop(int start, int end, float* restrict f_ptr, double* d_ptr, int* i_ptr) {
    double local_sum = 0.0;
    float float_acc = 0.0f;
    int int_acc = 0;
    
    /* Complex loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Memory loads with potential aliasing */
        float f_val = f_ptr[i];
        double d_val = d_ptr[i % SIZE];
        int i_val = i_ptr[i];
        
        /* Integer operations with carried dependency */
        int_acc += i_val * 3;
        int_acc -= (i_val >> 2);
        
        /* Floating-point operations with mixed precision */
        float_acc += f_val * 2.5f;
        float_acc = float_acc / 1.1f;
        
        /* Double precision operations */
        d_val = d_val * 1.7 + 0.3;
        local_sum += d_val;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and creates complex RTL */
            local_sum = local_sum / 1.5;
            float_acc = float_acc * 0.8f;
            
            /* Memory store with aliasing consideration */
            d_ptr[(i + 1) % SIZE] = local_sum;
        } else if (i % 13 == 0) {
            /* Another basic block with different operations */
            int_acc = int_acc ^ (i_val << 3);
            f_ptr[i] = float_acc;
        } else {
            /* Default path with arithmetic mix */
            local_sum = local_sum * 0.95 + float_acc;
            int_acc = int_acc | (i_val & 0xFF);
        }
        
        /* Volatile access to prevent reordering */
        g_volatile_counter++;
        g_volatile_double *= 1.000001;
        
        /* Inline assembly with memory clobber to create scheduling barriers */
        asm volatile("" ::: "memory");
        
        /* Additional arithmetic to increase instruction count */
        double temp = (double)int_acc * 0.01;
        local_sum += temp;
        float_acc += (float)(i % 256) * 0.1f;
    }
    
    /* Mix results to create output dependency */
    return local_sum + float_acc + int_acc;
}

/* Another inline function to increase scheduling complexity */
static inline __attribute__((always_inline))
void process_chunk(int chunk_id, float* f_arr, double* d_arr, int* i_arr) {
    int start = chunk_id * (SIZE / 4);
    int end = start + (SIZE / 4);
    
    double result = compute_loop(start, end, f_arr, d_arr, i_arr);
    
    /* Store result with potential aliasing */
    d_arr[chunk_id] = result;
    
    /* More complex operations */
    for (int j = start; j < end; j += 8) {
        /* Unrolled operations with dependencies */
        float f1 = f_arr[j];
        float f2 = f_arr[j+1];
        f_arr[j] = f1 * f2 + 1.0f;
        f_arr[j+1] = f1 - f2;
        
        /* Integer operations */
        i_arr[j] = i_arr[j] * 3 + chunk_id;
        i_arr[j+1] = i_arr[j+1] ^ (chunk_id << 2);
    }
}

int main() {
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        f_array[i] = (float)i * 0.1f;
        d_array[i] = (double)i * 0.01;
        i_array[i] = i * 3;
    }
    
    uint64_t checksum = 0;
    
    /* Multiple calls to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Process in chunks to create control flow */
        for (int chunk = 0; chunk < 4; chunk++) {
            process_chunk(chunk, f_array, d_array, i_array);
        }
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < SIZE; i += 64) {
            checksum ^= *(uint64_t*)&f_array[i];
            checksum ^= *(uint64_t*)&d_array[i];
            checksum ^= i_array[i];
        }
        
        /* Periodic reset to create phase changes */
        if (iter % 1000 == 0) {
            g_volatile_double = 1.0;
            i_array[iter % SIZE] = iter;
        }
    }
    
    /* Final computation with mixed operations */
    double final_result = 0.0;
    for (int i = 0; i < SIZE; i++) {
        final_result += f_array[i] * d_array[i] - i_array[i];
        
        /* Complex conditional */
        if (i % 3 == 0) {
            final_result = final_result * 0.9;
        } else if (i % 5 == 0) {
            final_result = final_result / 1.1;
        }
    }
    
    /* Use results to prevent optimization */
    printf("Checksum: %lu\n", (unsigned long)checksum);
    printf("Final result: %f\n", final_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile double: %f\n", g_volatile_double);
    
    return 0;
}
