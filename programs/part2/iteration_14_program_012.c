/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand -o test_remat test_remat.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t tmp1 = a + b;
    int32_t tmp2 = tmp1 * c;
    FORCE_COPY(tmp2);
    return tmp2 - a;
}

static inline float float_copy_mul(float a, float b, float c) {
    float tmp = a * b;
    FORCE_COPY(tmp);
    return tmp + c;
}

static inline int16_t narrow_copy(int32_t a, int32_t b) {
    int16_t narrow = (int16_t)(a + b);
    FORCE_COPY(narrow);
    return narrow;
}

static inline double promote_and_div(float a, int32_t b) {
    double promoted = (double)a;
    FORCE_COPY(promoted);
    return promoted / (b + 1.0);
}

/* Complex calculation chain that creates many virtual registers */
static inline int32_t calculation_chain(int32_t base, float fbase, 
                                       int8_t offset, uint16_t mask) {
    /* Mixed-type calculations to create different register modes */
    int32_t v1 = base * 3;
    float v2 = fbase * 2.5f;
    int16_t v3 = narrow_copy(v1, offset);
    double v4 = promote_and_div(v2, v1);
    
    FORCE_COPY(v1);
    FORCE_COPY(v2);
    FORCE_COPY(v3);
    
    /* Force register clobbering */
    CLASH_REGISTERS;
    
    int32_t v5 = copy_and_add(v1, v3, offset);
    float v6 = float_copy_mul(v2, (float)v5, fbase);
    
    /* More mixed operations */
    int32_t v7 = (int32_t)(v6 * 100.0f) ^ mask;
    double v8 = v4 * (double)v7;
    
    VOLATILE_SINK(v8);
    
    return v7 + (int32_t)v8;
}

int main(void) {
    /* Initialize arrays with different patterns */
    int32_t int_array[256];
    float float_array[256];
    int8_t byte_array[256];
    uint16_t mask_array[256];
    
    volatile int seed = 42;
    for (int i = 0; i < 256; i++) {
        int_array[i] = (i * 37 + seed) % 1000;
        float_array[i] = (float)(i * 0.12345f + seed * 0.01f);
        byte_array[i] = (int8_t)((i ^ seed) & 0xFF);
        mask_array[i] = (uint16_t)(i * 0xABCD);
    }
    
    /* Outer loop creates register pressure */
    volatile int outer_acc = 0;
    for (int outer = 0; outer < 100; outer++) {
        int32_t outer_var = outer * 7;
        float outer_float = (float)outer * 0.7f;
        
        /* Middle loop with conditional blocks */
        for (int mid = 0; mid < 50; mid++) {
            int32_t mid_base = int_array[mid % 256] + outer_var;
            float mid_float = float_array[mid % 256] + outer_float;
            
            /* Inner loop with complex calculations */
            for (int inner = 0; inner < 25; inner++) {
                /* Create many short-lived values */
                int32_t idx = (inner + mid) % 256;
                
                /* Chain of dependent calculations */
                int32_t val1 = int_array[idx] * 2;
                float val2 = float_array[idx] * 3.0f;
                int8_t val3 = byte_array[idx];
                uint16_t val4 = mask_array[idx];
                
                FORCE_COPY(val1);
                FORCE_COPY(val2);
                FORCE_COPY(val3);
                FORCE_COPY(val4);
                
                /* Force copy propagation context */
                int32_t result1 = copy_and_add(val1, inner, val3);
                float result2 = float_copy_mul(val2, (float)result1, outer_float);
                
                /* Mixed precision operations */
                int16_t narrow1 = narrow_copy(result1, inner);
                double promoted1 = promote_and_div(result2, result1);
                
                /* Complex chain that should trigger rematerialization */
                int32_t final_result = calculation_chain(
                    result1 + narrow1,
                    result2 + (float)narrow1,
                    val3,
                    val4
                );
                
                /* Use volatile to prevent elimination */
                VOLATILE_SINK(final_result);
                VOLATILE_SINK(promoted1);
                
                /* Conditional block to split control flow */
                if (final_result % 7 == 0) {
                    /* Different calculation path */
                    int32_t alt_calc = copy_and_add(final_result, outer, mid);
                    float alt_float = float_copy_mul((float)alt_calc, 0.5f, mid_float);
                    VOLATILE_SINK(alt_calc);
                    VOLATILE_SINK(alt_float);
                    
                    /* More register pressure */
                    for (int k = 0; k < 3; k++) {
                        int32_t temp = alt_calc * k;
                        float ftemp = alt_float * k;
                        VOLATILE_SINK(temp);
                        VOLATILE_SINK(ftemp);
                    }
                }
                
                /* Force register clobbering periodically */
                if (inner % 5 == 0) {
                    CLASH_REGISTERS;
                }
            }
            
            /* Another conditional block */
            if (mid % 7 == 0) {
                /* Different type conversions */
                double dbl_sum = 0.0;
                for (int j = 0; j < 10; j++) {
                    dbl_sum += promote_and_div(mid_float, mid_base + j);
                }
                VOLATILE_SINK(dbl_sum);
            }
        }
        
        /* Accumulate to prevent loop elimination */
        outer_acc += outer_var;
    }
    
    return outer_acc % 1000;
}
