/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -o test_remat test_remat.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : : "r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation contexts */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 ^ c;
    FORCE_COPY(t1);
    return t2 * 7;
}

static inline float float_copy_mul(float a, float b, int scale) {
    float t = a * b;
    VOLATILE_SINK(t);
    return t * scale;
}

static inline double mixed_calc(double d, int32_t i, char c) {
    double temp = d + i;
    FORCE_COPY(temp);
    return temp * c;
}

/* Function to create register pressure with mixed types */
static void process_block(int32_t* int_data, float* float_data, 
                         double* double_data, size_t size) {
    volatile int guard = 0;
    
    for (size_t i = 0; i < size; i++) {
        /* Create many short-lived values with dependencies */
        int32_t base = int_data[i] & 0xFF;
        FORCE_COPY(base);
        
        /* Chain of integer computations - each creates virtual regs */
        int32_t v1 = base + 1;
        int32_t v2 = v1 * 2;
        int32_t v3 = v2 - base;
        int32_t v4 = v3 ^ 0x55;
        int32_t v5 = copy_and_add(v4, v2, v3);
        
        /* Mix with floating point - different modes */
        float f1 = float_data[i] + 1.0f;
        float f2 = f1 * 2.0f;
        float f3 = float_copy_mul(f2, f1, v5);
        
        /* More integer variants */
        int16_t s1 = (v5 >> 8) & 0xFFFF;
        int32_t v6 = v5 + s1;
        int32_t v7 = v6 * 3;
        
        /* Double precision calculations */
        double d1 = double_data[i];
        double d2 = mixed_calc(d1, v7, (char)(i & 0xFF));
        
        /* Conditional block to split control flow */
        if (i % 3 == 0) {
            int32_t v8 = v7 + guard;
            float f4 = f3 * 1.5f;
            double d3 = d2 * 0.5;
            
            /* Force copies between different scopes */
            int32_t v9 = copy_and_add(v8, (int32_t)f4, (int32_t)d3);
            VOLATILE_SINK(v9);
            
            CLASH_REGISTERS;
        } else if (i % 3 == 1) {
            /* Different computation pattern */
            int64_t l1 = (int64_t)v7 * 1000;
            float f5 = float_copy_mul(f3, 0.25f, (int)l1);
            VOLATILE_SINK(f5);
        }
        
        /* Store results back to prevent elimination */
        int_data[i] = v5 + (int32_t)f3;
        float_data[i] = f3 + (float)v7;
        double_data[i] = d2;
        
        /* Update volatile to prevent loop optimizations */
        guard += i & 1;
    }
}

/* Outer loops to create nested control flow */
static void nested_processing(int32_t* data1, float* data2, 
                             double* data3, size_t outer, size_t inner) {
    for (size_t o = 0; o < outer; o++) {
        volatile int outer_guard = o;
        
        /* Initialize with pattern */
        for (size_t i = 0; i < inner; i++) {
            data1[i] = (i * 3 + o) & 0xFFF;
            data2[i] = (float)(i + o) * 0.1f;
            data3[i] = (double)(i * o) * 0.01;
        }
        
        /* Process with register pressure */
        for (int iter = 0; iter < 3; iter++) {
            process_block(data1, data2, data3, inner);
            
            /* Additional computations between calls */
            int32_t temp_sum = 0;
            for (size_t i = 0; i < inner; i += 4) {
                temp_sum += data1[i] + (int32_t)data2[i];
                FORCE_COPY(temp_sum);
            }
            VOLATILE_SINK(temp_sum);
            
            CLASH_REGISTERS;
        }
        
        /* Final mixing pass */
        for (size_t i = 0; i < inner; i++) {
            double d = data3[i];
            float f = data2[i];
            int32_t n = data1[i];
            
            /* Mixed-type expression chain */
            double result = mixed_calc(d, n, (char)(f * 10));
            float f_result = float_copy_mul(f, (float)result, n);
            int32_t i_result = copy_and_add(n, (int32_t)f_result, (int32_t)result);
            
            data1[i] = i_result;
            data2[i] = f_result;
            data3[i] = result;
        }
        
        outer_guard += 1;
    }
}

/* Main driver with initialization */
int main(void) {
    const size_t INNER_SIZE = 256;
    const size_t OUTER_LOOPS = 8;
    
    /* Allocate with different alignments */
    int32_t* int_data = (int32_t*)aligned_alloc(16, INNER_SIZE * sizeof(int32_t));
    float* float_data = (float*)aligned_alloc(16, INNER_SIZE * sizeof(float));
    double* double_data = (double*)aligned_alloc(16, INNER_SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with varied patterns */
    for (size_t i = 0; i < INNER_SIZE; i++) {
        int_data[i] = (i * 17) & 0x7FF;
        float_data[i] = (float)(i % 37) * 0.27f;
        double_data[i] = (double)(i % 53) * 0.13;
    }
    
    /* Perform nested processing to create complex DFG */
    nested_processing(int_data, float_data, double_data, OUTER_LOOPS, INNER_SIZE);
    
    /* Final reduction to prevent dead code elimination */
    int32_t final_sum = 0;
    float final_float_sum = 0.0f;
    double final_double_sum = 0.0;
    
    for (size_t i = 0; i < INNER_SIZE; i++) {
        final_sum += int_data[i];
        final_float_sum += float_data[i];
        final_double_sum += double_data[i];
        
        /* Force intermediate computations */
        if (i % 8 == 0) {
            int32_t temp = copy_and_add(final_sum, (int32_t)final_float_sum, 
                                       (int32_t)final_double_sum);
            VOLATILE_SINK(temp);
        }
    }
    
    /* Use results */
    VOLATILE_SINK(final_sum);
    VOLATILE_SINK(final_float_sum);
    VOLATILE_SINK(final_double_sum);
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
