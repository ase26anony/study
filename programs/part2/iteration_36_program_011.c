/* sel-sched-test.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -o test sel-sched-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Arrays with potential aliasing */
static double global_array[SIZE];
static float float_array[SIZE];
static int int_array[SIZE];

/* Function with memory aliasing through restrict and non-restrict pointers */
static inline void process_chunk(double *restrict dst, 
                                 const double *restrict src1,
                                 float *src2,  /* non-restrict for aliasing */
                                 int *int_src,
                                 int start, int end) {
    double local_acc = 0.0;
    float float_acc = 0.0f;
    int int_acc = 0;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Memory loads with potential aliasing */
        double d_val = src1[i];
        float f_val = src2[i % SIZE];
        int i_val = int_src[i % SIZE];
        
        /* Multiple arithmetic operations creating dependencies */
        double temp1 = d_val * 1.23456789;
        float temp2 = f_val / 3.14159265f;
        int temp3 = i_val + g_volatile_counter;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            temp1 = temp1 * 2.0;
            temp2 = temp2 + 1.0f;
            temp3 = temp3 | 0x0F;
        } else if (i % 13 == 0) {
            temp1 = temp1 / 1.5;
            temp2 = temp2 - 0.5f;
            temp3 = temp3 & 0xF0;
        }
        
        /* More operations mixing types */
        double d_result = temp1 + (double)temp2;
        float f_result = (float)temp1 * temp2;
        int i_result = temp3 ^ (int)(temp1 * 100.0);
        
        /* Memory stores */
        dst[i] = d_result;
        src2[i % SIZE] = f_result;  /* Potential aliasing store */
        int_src[i % SIZE] = i_result;
        
        /* Accumulators with carried dependencies */
        local_acc += d_result;
        float_acc += f_result;
        int_acc ^= i_result;
        
        /* Additional independent operations for ILP */
        double extra1 = d_val * d_val;
        float extra2 = f_val + f_val;
        int extra3 = i_val * 2;
        
        /* Use results to prevent dead code elimination */
        if (extra1 > 1000.0) {
            float_acc -= 0.1f;
        }
        if (extra3 < 0) {
            int_acc += 1;
        }
    }
    
    /* Store accumulated results to global volatile */
    g_volatile_counter += int_acc;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(local_acc), "+r"(float_acc));
}

/* Main computation function with multiple loops */
static inline uint64_t compute_loop(int iterations) {
    uint64_t checksum = 0;
    double local_buffer[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_array[i] = (double)i / 100.0;
        float_array[i] = (float)i / 50.0f;
        int_array[i] = i * 3;
        local_buffer[i] = 0.0;
    }
    
    /* Multiple calls to hot function with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        /* Call with different slice sizes to create varied scheduling regions */
        int slice_size = SIZE / 4;
        for (int slice = 0; slice < 4; slice++) {
            int start = slice * slice_size;
            int end = (slice == 3) ? SIZE : start + slice_size;
            
            process_chunk(local_buffer,
                         global_array,
                         float_array,
                         int_array,
                         start, end);
        }
        
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < SIZE; i += 8) {
            global_array[i] *= 1.0001;
            float_array[i] += 0.001f;
            int_array[i] ^= iter;
        }
        
        /* Compute checksum to prevent optimization */
        for (int i = 0; i < SIZE; i++) {
            checksum ^= *(uint64_t*)&local_buffer[i];
            checksum += int_array[i];
        }
    }
    
    return checksum;
}

/* Secondary hot function with different operation mix */
static inline void secondary_loop(double *output, const int *input, int n) {
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Complex dependency chain */
        double x = (double)input[i];
        double y = x * x;
        double z = y / (x + 1.0);
        
        /* Conditional with floating point comparison */
        if (z > 50.0) {
            acc1 += z;
            output[i] = z * 0.5;
        } else {
            acc2 += z;
            output[i] = z * 2.0;
        }
        
        /* Additional FP operations */
        acc3 += output[i] * output[i-1 > 0 ? i-1 : 0];
        
        /* Integer operations mixed in */
        int idx = i % 256;
        output[idx] += (double)(g_volatile_counter & 0xFF);
    }
    
    /* Use accumulators */
    output[0] += acc1 + acc2 + acc3;
}

int main(void) {
    uint64_t final_checksum = 0;
    
    printf("Starting selective scheduler test...\n");
    
    /* Warm up - first call with fewer iterations */
    final_checksum ^= compute_loop(ITERATIONS / 10);
    
    /* Main computation - this should trigger selective scheduling */
    final_checksum ^= compute_loop(ITERATIONS);
    
    /* Additional computation with different pattern */
    double output_buffer[256];
    int input_buffer[256];
    for (int i = 0; i < 256; i++) {
        input_buffer[i] = i * i;
    }
    
    secondary_loop(output_buffer, input_buffer, 256);
    
    /* Use results to prevent optimization */
    for (int i = 0; i < 256; i++) {
        final_checksum ^= *(uint64_t*)&output_buffer[i];
    }
    
    /* Final checksum to ensure computation happened */
    final_checksum += g_volatile_counter;
    
    printf("Final checksum: %llu\n", (unsigned long long)final_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    return 0;
}
