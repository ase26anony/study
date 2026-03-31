/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile float volatile_sink_float;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int copy_and_add_int(int a, int b, int c) {
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : :);
    int result = a + b + c;
    asm volatile("" : : "r"(result));
    return result;
}

static inline float copy_and_mul_float(float a, float b, float c) {
    asm volatile("" : "+f"(a), "+f"(b), "+f"(c) : :);
    float result = a * b + c;
    asm volatile("" : : "f"(result));
    return result;
}

static inline double copy_and_mix(double a, int b, float c) {
    asm volatile("" : "+f"(a), "+r"(b), "+f"(c) : :);
    double result = a + (double)b + (double)c;
    asm volatile("" : : "f"(result));
    return result;
}

/* Helper with mixed types to create different register modes */
static inline int16_t promote_and_mask(uint8_t a, int16_t b) {
    asm volatile("" : "+r"(a), "+r"(b) : :);
    int16_t result = (int16_t)a & b;
    asm volatile("" : : "r"(result));
    return result;
}

/* Main computation kernel */
static void compute_kernel(int* int_data, float* float_data, double* double_data, 
                          int size, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop creates pressure */
        int base_int = iter * 7;
        float base_float = (float)iter * 1.5f;
        double base_double = (double)iter * 2.3;
        
        volatile_sink_int = base_int;
        volatile_sink_float = base_float;
        volatile_sink_double = base_double;
        
        /* First inner loop - integer heavy */
        for (int i = 0; i < size; i++) {
            /* Create many short-lived recomputable values */
            int temp1 = int_data[i] + base_int;
            int temp2 = temp1 * 3;
            int temp3 = temp2 - base_int;
            
            /* Force copies through inline function */
            int temp4 = copy_and_add_int(temp1, temp2, temp3);
            
            /* Mix with array access */
            int temp5 = temp4 + int_data[(i + 1) % size];
            int temp6 = temp5 * 2;
            
            /* Use volatile to prevent elimination */
            volatile_sink_int = temp6;
            
            /* Conditional creates control flow complexity */
            if (temp6 > 1000) {
                /* Different computation path */
                int temp7 = temp6 / 2;
                int temp8 = copy_and_add_int(temp7, base_int, i);
                int_data[i] = temp8 & 0xFF;
            } else {
                int temp7 = temp6 * 2;
                uint8_t byte_val = (uint8_t)(temp7 & 0xFF);
                int16_t short_val = promote_and_mask(byte_val, (int16_t)temp7);
                int_data[i] = short_val;
            }
        }
        
        /* Second inner loop - floating point heavy */
        for (int i = 0; i < size; i++) {
            /* Create FP recomputable values */
            float f1 = float_data[i] + base_float;
            float f2 = f1 * 1.25f;
            float f3 = copy_and_mul_float(f1, f2, base_float);
            
            /* Mix precision */
            double d1 = (double)f3 + base_double;
            double d2 = d1 * 0.75;
            
            /* Force register moves with inline asm */
            asm volatile("" : : "r"(i), "f"(f1), "f"(f2), "f"(d1), "f"(d2));
            
            /* Complex conditional with mixed types */
            if (i % 3 == 0) {
                float f4 = f3 * 2.0f;
                double d3 = copy_and_mix(d2, i, f4);
                volatile_sink_double = d3;
                float_data[i] = (float)d3;
            } else if (i % 3 == 1) {
                int int_val = (int)d2;
                float f4 = copy_and_mul_float(float_data[i], base_float, (float)int_val);
                volatile_sink_float = f4;
                float_data[i] = f4;
            } else {
                /* Third path with more computations */
                double d3 = d2 + (double)float_data[i];
                for (int j = 0; j < 2; j++) {  /* Tiny nested loop */
                    d3 = d3 * 1.1 - (double)j;
                }
                double_data[i] = d3;
                volatile_sink_double = d3;
            }
        }
        
        /* Third loop - mixed operations with array dependencies */
        for (int i = 0; i < size; i++) {
            /* Chain of dependent computations */
            int idx1 = (i * 17) % size;
            int idx2 = (i * 13) % size;
            
            int val1 = int_data[idx1];
            float val2 = float_data[idx2];
            double val3 = double_data[i];
            
            /* Mixed-type computation chain */
            int int_result = val1 + (int)val2;
            float float_result = (float)int_result * val2;
            double double_result = copy_and_mix(val3, int_result, float_result);
            
            /* More copies and recomputations */
            int temp_a = int_result * 3;
            int temp_b = copy_and_add_int(temp_a, i, iter);
            float temp_c = float_result * 0.5f;
            double temp_d = double_result + (double)temp_b + (double)temp_c;
            
            /* Force register pressure with many live values */
            asm volatile("" : : "r"(temp_a), "r"(temp_b), "f"(temp_c), "f"(temp_d),
                         "r"(val1), "f"(val2), "f"(val3));
            
            /* Store results back with different types */
            int_data[i] = temp_b;
            float_data[i] = temp_c;
            double_data[i] = temp_d;
        }
    }
}

int main(void) {
    const int size = 256;
    const int iterations = 100;
    
    /* Allocate and initialize with different patterns */
    int* int_data = (int*)malloc(size * sizeof(int));
    float* float_data = (float*)malloc(size * sizeof(float));
    double* double_data = (double*)malloc(size * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with varied data patterns */
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) % 7919;  /* Prime modulus for pattern */
        float_data[i] = (float)((i * 51) % 1024) * 0.123f;
        double_data[i] = (double)((i * 73) % 2048) * 0.456;
    }
    
    /* Run the computation kernel */
    compute_kernel(int_data, float_data, double_data, size, iterations);
    
    /* Final volatile sink to ensure all computations are used */
    int final_sum = 0;
    float final_float_sum = 0.0f;
    double final_double_sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        final_sum += int_data[i];
        final_float_sum += float_data[i];
        final_double_sum += double_data[i];
    }
    
    volatile_sink_int = final_sum;
    volatile_sink_float = final_float_sum;
    volatile_sink_double = final_double_sum;
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
