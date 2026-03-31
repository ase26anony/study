/* sel-sched-test.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Arrays with potential aliasing */
static double global_array[256];
static float float_array[256];
static int int_array[256];

/* Inline function with hot loop - designed for selective scheduling */
static inline uint64_t __attribute__((always_inline))
compute_loop(int start, int end, double* restrict dst, 
             const double* restrict src1, const float* src2,
             int* restrict int_dst, const int* int_src)
{
    uint64_t checksum = 0;
    double acc_double = 1.0;
    float acc_float = 2.0f;
    int acc_int = 3;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Memory loads with potential aliasing (src2 is not restrict) */
        double dval = src1[i];
        float fval = src2[i];
        int ival = int_src[i];
        
        /* Carried dependency chain - prevents parallelization */
        acc_double = acc_double * 0.999 + dval * 1.001;
        acc_float = acc_float * 0.998f + fval * 1.002f;
        acc_int = acc_int * 997 + ival * 1003;
        
        /* Independent operations to create ILP opportunities */
        double temp1 = dval * dval;
        float temp2 = fval * fval;
        int temp3 = ival * ival;
        
        /* Mix of arithmetic operations */
        if (i % 7 == 0) {
            /* Division creates complex instructions */
            temp1 = temp1 / (dval + 1.0);
            temp2 = temp2 / (fval + 1.0f);
            temp3 = temp3 / ((ival & 0xFF) + 1);
            
            /* Store with conditional */
            dst[i] = temp1 + acc_double;
            float_array[i] = temp2 + acc_float;
        } else if (i % 5 == 0) {
            /* Different operation mix */
            dst[i] = temp1 - acc_double;
            float_array[i] = temp2 - acc_float;
        } else {
            /* Default path */
            dst[i] = temp1;
            float_array[i] = temp2;
        }
        
        /* Integer store with pointer aliasing consideration */
        int_dst[i] = temp3 + acc_int;
        
        /* Volatile operation to prevent dead code elimination */
        g_volatile_counter++;
        
        /* Update checksum to prevent optimization */
        checksum ^= *(uint64_t*)&dst[i];
        checksum ^= *(uint32_t*)&float_array[i];
        checksum ^= int_dst[i];
        
        /* Additional floating-point operations */
        if (i % 11 == 0) {
            double complex_op = (dval * 3.14159) / (fval + 0.0001);
            dst[i] += complex_op;
            checksum ^= *(uint64_t*)&complex_op;
        }
    }
    
    /* Final reduction */
    checksum ^= *(uint64_t*)&acc_double;
    checksum ^= *(uint32_t*)&acc_float;
    checksum ^= acc_int;
    
    return checksum;
}

/* Secondary hot function to increase scheduling complexity */
static inline uint64_t __attribute__((always_inline))
process_block(int block_id, int size)
{
    /* Create local arrays to work with */
    double local_dst[128];
    double local_src[128];
    float local_float[128];
    int local_int_dst[128];
    int local_int_src[128];
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        local_src[i] = (i + block_id) * 0.12345;
        local_float[i] = (i - block_id) * 0.6789f;
        local_int_src[i] = i * block_id * 7919; /* Prime number */
    }
    
    /* Call the main compute loop */
    return compute_loop(0, size, 
                       local_dst, local_src, local_float,
                       local_int_dst, local_int_src);
}

int main(void)
{
    uint64_t final_checksum = 0;
    
    /* Initialize global arrays with data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 0.314159;
        float_array[i] = i * 0.271828f;
        int_array[i] = i * 57721; /* Another prime */
    }
    
    /* Multiple calls to create scheduling regions */
    for (int iter = 0; iter < 100; iter++) {
        /* Process multiple blocks */
        for (int block = 0; block < 4; block++) {
            uint64_t block_result = process_block(block + iter, 64);
            final_checksum ^= block_result;
            
            /* Additional computation on global arrays */
            double* restrict ptr1 = global_array;
            const double* restrict ptr2 = &global_array[128];
            float* fptr = float_array;
            int* iptr = int_array;
            
            /* Another hot loop with different access pattern */
            for (int j = 0; j < 64; j++) {
                /* Mixed loads and stores */
                double val1 = ptr1[j];
                double val2 = ptr2[j];
                
                /* Complex dependency chain */
                val1 = val1 * 1.234 - val2 * 0.789;
                ptr1[j + 64] = val1;
                
                /* Integer operations */
                int ival = iptr[j];
                ival = (ival * 1103515245 + 12345) & 0x7FFFFFFF;
                iptr[j + 64] = ival;
                
                /* Floating-point with conversion */
                fptr[j + 64] = (float)val1 + (float)ival * 0.001f;
                
                /* Update checksum */
                final_checksum ^= *(uint64_t*)&val1;
                final_checksum ^= ival;
            }
        }
    }
    
    /* Use results to prevent optimization */
    printf("Final checksum: 0x%016llX\n", (unsigned long long)final_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Additional computation to extend scheduling region */
    {
        double final_acc = 0.0;
        for (int i = 0; i < 256; i++) {
            final_acc += global_array[i] * float_array[i] + int_array[i];
        }
        printf("Final accumulator: %f\n", final_acc);
    }
    
    return 0;
}
