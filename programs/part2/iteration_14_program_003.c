/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : : "r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "memory", "r0", "r1", "r2", "r3")

/* Helper functions to force copy propagation */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 ^ c;
    int32_t t3 = t2 * 7;
    FORCE_COPY(t3);
    return t3;
}

static inline float float_copy_mul(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    float t3 = t2 - a;
    FORCE_COPY(t3);
    return t3;
}

static inline int16_t narrow_copy(int32_t a, int32_t b) {
    int16_t t1 = (int16_t)(a & 0xFFFF);
    int16_t t2 = (int16_t)(b & 0xFFFF);
    int16_t t3 = t1 + t2;
    FORCE_COPY(t3);
    return t3;
}

/* Main computation kernel */
static void compute_kernel(int32_t *int_data, float *float_data, 
                          int8_t *char_data, double *double_data,
                          int size) {
    volatile int outer_sink = 0;
    
    for (int i = 0; i < size; i++) {
        /* Outer loop creates register pressure */
        int32_t base = int_data[i];
        float fbase = float_data[i];
        
        /* Multiple inner loops with different scopes */
        for (int j = 0; j < 8; j++) {
            /* Create many short-lived values */
            int32_t v1 = base + j * 3;
            int32_t v2 = v1 ^ 0x55AA55AA;
            int32_t v3 = v2 * 13;
            int32_t v4 = v3 - j;
            
            /* Force copies through function calls */
            int32_t v5 = copy_and_add(v1, v2, v3);
            int32_t v6 = copy_and_add(v4, v5, base);
            
            /* Mixed precision operations */
            float f1 = fbase * (j + 1);
            float f2 = float_copy_mul(f1, fbase, float_data[(i + j) % size]);
            
            /* Type conversions create different register modes */
            double d1 = (double)f1 + (double)v1;
            double d2 = d1 * 2.5;
            
            /* Narrow operations */
            int16_t s1 = narrow_copy(v1, v2);
            int16_t s2 = narrow_copy(v3, v4);
            int8_t c1 = (int8_t)(s1 + s2);
            
            /* Volatile sinks prevent elimination */
            VOLATILE_SINK(v6);
            VOLATILE_SINK(f2);
            VOLATILE_SINK(d2);
            VOLATILE_SINK(c1);
            
            /* Register clobbering forces moves */
            if ((j & 3) == 0) {
                CLASH_REGISTERS;
            }
        }
        
        /* Conditional block with different computation */
        if (i % 3 == 0) {
            double d3 = (double)base * 1.7;
            float f3 = (float)d3;
            int32_t v7 = (int32_t)f3;
            
            /* Chain of dependent operations */
            for (int k = 0; k < 4; k++) {
                v7 = v7 * 3 + k;
                f3 = f3 * 1.1f - k;
                d3 = d3 / 2.0 + k;
                
                /* More forced copies */
                int32_t v8 = copy_and_add(v7, k, base);
                float f4 = float_copy_mul(f3, 2.0f, float_data[k]);
                
                VOLATILE_SINK(v8);
                VOLATILE_SINK(f4);
                VOLATILE_SINK(d3);
            }
        } else if (i % 3 == 1) {
            /* Different computation pattern */
            int64_t l1 = (int64_t)base * 1000;
            int64_t l2 = l1 + (i * 37);
            int64_t l3 = l2 ^ 0x123456789ABCDEF0ULL;
            
            /* Force 64-bit operations */
            for (int m = 0; m < 2; m++) {
                l3 = (l3 << 3) | (l3 >> 61);
                VOLATILE_SINK(l3);
            }
        }
        
        outer_sink += base;
    }
    
    VOLATILE_SINK(outer_sink);
}

/* Initialize with different patterns */
static void init_data(int32_t *int_data, float *float_data,
                     int8_t *char_data, double *double_data,
                     int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37 + 123) & 0x7FFF;
        float_data[i] = (float)(i * 0.123 + 45.67);
        char_data[i] = (int8_t)((i * 13) & 0xFF);
        double_data[i] = (double)(i * 0.456 - 23.45);
    }
}

int main(void) {
    const int SIZE = 128;
    
    /* Allocate arrays with different alignments */
    int32_t *int_data = __builtin_assume_aligned(
        malloc(SIZE * sizeof(int32_t)), 16);
    float *float_data = __builtin_assume_aligned(
        malloc(SIZE * sizeof(float)), 16);
    int8_t *char_data = __builtin_assume_aligned(
        malloc(SIZE * sizeof(int8_t)), 16);
    double *double_data = __builtin_assume_aligned(
        malloc(SIZE * sizeof(double)), 16);
    
    if (!int_data || !float_data || !char_data || !double_data) {
        return 1;
    }
    
    init_data(int_data, float_data, char_data, double_data, SIZE);
    
    /* Multiple iterations to increase optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        compute_kernel(int_data, float_data, char_data, double_data, SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            int_data[i] += iter * 11;
            float_data[i] += (float)(iter * 0.1);
        }
    }
    
    /* Final volatile sink */
    volatile int final_sink = int_data[0] + char_data[0];
    (void)final_sink;
    
    free(int_data);
    free(float_data);
    free(char_data);
    free(double_data);
    
    return 0;
}
