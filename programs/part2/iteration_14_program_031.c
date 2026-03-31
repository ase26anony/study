/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation */
static inline int32_t copy_and_transform(int32_t a, int32_t b) {
    int32_t tmp = a + b * 3;
    FORCE_COPY(tmp);
    return tmp - (b >> 2);
}

static inline float float_copy_transform(float a, float b, int c) {
    float tmp = a * 1.5f + b;
    FORCE_COPY(tmp);
    return tmp + (float)c * 0.25f;
}

static inline int16_t narrow_transform(int32_t a, int32_t b) {
    int16_t narrow = (int16_t)((a & 0xFFFF) + (b & 0xFF));
    FORCE_COPY(narrow);
    return narrow;
}

/* Main computation kernel */
void __attribute__((noinline)) 
compute_kernel(int32_t* restrict arr_int, float* restrict arr_float, 
               uint8_t* restrict arr_byte, int size) {
    int i, j, k;
    
    /* Multiple nested loops to create complex CFG */
    for (i = 0; i < size; i += 8) {
        int32_t accum_int = arr_int[i];
        float accum_float = arr_float[i];
        uint8_t accum_byte = arr_byte[i];
        
        /* Inner loop with register pressure */
        for (j = 0; j < 128; j++) {
            int32_t tmp1 = accum_int + j * 7;
            float tmp2 = accum_float + j * 0.125f;
            
            /* Force copies between different modes */
            int32_t copied1 = copy_and_transform(tmp1, j);
            float copied2 = float_copy_transform(tmp2, accum_float, j);
            
            /* Mixed precision calculations */
            int16_t narrow1 = narrow_transform(copied1, j);
            double promoted = (double)copied2 * 2.0;
            
            /* Conditional block to split control flow */
            if (j & 1) {
                accum_int = copied1 ^ narrow1;
                accum_float = (float)promoted + 1.0f;
                
                /* Another inner loop for more pressure */
                for (k = 0; k < 4; k++) {
                    int32_t loop_tmp = accum_int + k * 11;
                    float float_tmp = accum_float * (k + 1);
                    
                    /* More copies with different modes */
                    int32_t recopied = copy_and_transform(loop_tmp, k);
                    float refloat = float_copy_transform(float_tmp, accum_float, k);
                    
                    /* Force register clobbering */
                    CLASH_REGISTERS;
                    
                    accum_int = recopied - (k << 2);
                    accum_float = refloat * 0.9f;
                    
                    VOLATILE_SINK(accum_int);
                    VOLATILE_SINK(accum_float);
                }
            } else {
                accum_int = copied1 | (narrow1 << 8);
                accum_float = (float)promoted - 0.5f;
                
                /* Different computation path */
                double double_tmp = promoted * 1.5;
                int64_t widened = (int64_t)accum_int * 3;
                
                /* Force mode mixing */
                float mixed = (float)double_tmp + (float)widened;
                accum_float = mixed * 0.75f;
                
                /* More inline assembly to prevent optimization */
                asm volatile("" : "+r"(accum_int), "+r"(widened));
            }
            
            /* Byte operations with different mode */
            accum_byte ^= (uint8_t)(accum_int & 0xFF);
            accum_byte += (uint8_t)j;
            
            /* Periodic volatile sinks */
            if (j % 16 == 0) {
                VOLATILE_SINK(accum_int);
                VOLATILE_SINK(accum_float);
                VOLATILE_SINK(accum_byte);
            }
            
            /* Force another copy chain */
            int32_t final_copy = copy_and_transform(accum_int, accum_byte);
            accum_int = final_copy + (accum_byte << 4);
        }
        
        /* Store results back */
        arr_int[i] = accum_int;
        arr_float[i] = accum_float;
        arr_byte[i] = accum_byte;
    }
}

/* Additional helper with complex parameter passing */
static inline int32_t __attribute__((always_inline))
complex_helper(int32_t a, float b, int16_t c, double d) {
    int32_t tmp1 = a + (int32_t)b;
    float tmp2 = b + (float)c + (float)d;
    
    /* Force multiple copies */
    int32_t copied1 = copy_and_transform(tmp1, c);
    float copied2 = float_copy_transform(tmp2, b, a);
    
    /* Mixed mode operation */
    double result = (double)copied1 * 0.25 + (double)copied2;
    return (int32_t)result;
}

/* Second kernel with different pattern */
void __attribute__((noinline))
second_kernel(int32_t* data, int size) {
    for (int i = 0; i < size; i++) {
        int32_t base = data[i];
        
        /* Chain of dependent computations */
        for (int rep = 0; rep < 8; rep++) {
            int32_t v1 = base + rep * 3;
            int32_t v2 = v1 << (rep & 3);
            int32_t v3 = v2 - (rep * 5);
            int32_t v4 = v3 ^ (v1 >> 2);
            int32_t v5 = v4 + (rep * 7);
            
            /* Force copies through helper */
            int32_t copied = complex_helper(v1, (float)v2, (int16_t)v3, (double)v4);
            
            /* Conditional with different computation */
            if (v5 & 1) {
                base = copied + v5;
            } else {
                base = copied - v5;
                
                /* Extra loop for more pressure */
                for (int k = 0; k < 2; k++) {
                    int32_t inner = base + k * 13;
                    float float_inner = (float)inner * 1.1f;
                    double double_inner = (double)float_inner * 0.9;
                    
                    /* More mode mixing */
                    int32_t remat = (int32_t)(double_inner * 100.0);
                    base = copy_and_transform(remat, k);
                    
                    VOLATILE_SINK(base);
                }
            }
            
            /* Clobber registers periodically */
            if (rep % 3 == 0) {
                CLASH_REGISTERS;
            }
        }
        
        data[i] = base;
    }
}

int main(void) {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t* arr_int = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* arr_float = (float*)malloc(SIZE * sizeof(float));
    uint8_t* arr_byte = (uint8_t*)malloc(SIZE * sizeof(uint8_t));
    int32_t* data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_float[i] = i * 0.25f;
        arr_byte[i] = i & 0xFF;
        data[i] = i * 5 - 2;
    }
    
    /* Run kernels multiple times */
    for (int iter = 0; iter < 3; iter++) {
        compute_kernel(arr_int, arr_float, arr_byte, SIZE);
        second_kernel(data, SIZE);
        
        /* Mix data between kernels */
        for (int i = 0; i < SIZE; i += 4) {
            arr_int[i] ^= data[i];
            arr_float[i] += (float)data[i];
        }
    }
    
    /* Final volatile sink of results */
    volatile int32_t final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr_int[i] + (int32_t)arr_float[i] + arr_byte[i];
    }
    
    free(arr_int);
    free(arr_float);
    free(arr_byte);
    free(data);
    
    return (int)(final_sum & 0x7FFFFFFF);
}
