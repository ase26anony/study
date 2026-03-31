/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation */
static inline int32_t propagate_int(int32_t a, int32_t b, int32_t c) {
    int32_t t = a + b;
    FORCE_COPY(t);
    return t * c;
}

static inline float propagate_float(float a, float b, float c) {
    float t = a * b;
    FORCE_COPY(t);
    return t + c;
}

static inline double propagate_double(double a, double b, double c) {
    double t = a - b;
    FORCE_COPY(t);
    return t * c;
}

/* Mixed precision operations */
static inline int16_t promote_and_add(int8_t a, int16_t b) {
    int16_t promoted = a;  /* Implicit promotion */
    FORCE_COPY(promoted);
    return promoted + b;
}

static inline float int_to_float_mul(int32_t a, float b) {
    float fa = a;  /* Conversion */
    FORCE_COPY(fa);
    return fa * b;
}

/* Complex chain of operations to create register pressure */
static inline uint64_t compute_chain(uint64_t seed, int iter) {
    uint32_t a = (seed >> 0) & 0xFF;
    uint32_t b = (seed >> 8) & 0xFF;
    uint32_t c = (seed >> 16) & 0xFF;
    uint32_t d = (seed >> 24) & 0xFF;
    
    /* Force many virtual registers */
    uint32_t t1 = a * b + iter;
    uint32_t t2 = b * c + iter;
    uint32_t t3 = c * d + iter;
    uint32_t t4 = d * a + iter;
    
    FORCE_COPY(t1); FORCE_COPY(t2); FORCE_COPY(t3); FORCE_COPY(t4);
    
    uint32_t t5 = propagate_int(t1, t2, t3);
    uint32_t t6 = propagate_int(t2, t3, t4);
    uint32_t t7 = propagate_int(t3, t4, t1);
    
    FORCE_COPY(t5); FORCE_COPY(t6); FORCE_COPY(t7);
    
    return ((uint64_t)t5 << 32) | t6 + t7;
}

int main(void) {
    /* Initialize arrays with different patterns */
    int8_t arr8[256];
    int16_t arr16[256];
    int32_t arr32[256];
    float arrf[256];
    double arrd[256];
    
    volatile int8_t *volatile_arr8 = arr8;
    volatile int16_t *volatile_arr16 = arr16;
    
    for (int i = 0; i < 256; i++) {
        arr8[i] = (i * 13) & 0xFF;
        arr16[i] = (i * 17) & 0xFFFF;
        arr32[i] = i * 23;
        arrf[i] = i * 1.5f;
        arrd[i] = i * 2.7;
    }
    
    uint64_t accumulator = 0;
    int outer_counter = 0;
    
    /* Outer loop creating control flow complexity */
    for (int outer = 0; outer < 100; outer++) {
        CLASH_REGISTERS;
        
        /* First inner loop - integer operations */
        for (int i = 0; i < 128; i++) {
            /* Create many short-lived values */
            int32_t v1 = arr32[i] * 3;
            int32_t v2 = arr32[i + 128] * 5;
            int32_t v3 = propagate_int(v1, v2, outer);
            
            /* Mixed width operations */
            int16_t s1 = promote_and_add(arr8[i], arr16[i]);
            int16_t s2 = promote_and_add(arr8[i + 128], arr16[i + 128]);
            
            /* Force copies between different scopes */
            {
                int32_t temp = s1 * s2;
                FORCE_COPY(temp);
                v3 += temp;
            }
            
            VOLATILE_SINK(v3);
            accumulator += v3;
        }
        
        /* Second inner loop - floating point operations */
        for (int i = 0; i < 128; i++) {
            /* Mix float and double operations */
            float f1 = arrf[i] * 1.1f;
            float f2 = propagate_float(f1, arrf[i + 128], outer * 0.5f);
            
            double d1 = arrd[i] * 1.3;
            double d2 = propagate_double(d1, arrd[i + 128], outer * 0.7);
            
            /* Convert between types */
            float f3 = int_to_float_mul(arr32[i], f2);
            double d3 = f3 + d2;  /* Mixed precision */
            
            FORCE_COPY(f3); FORCE_COPY(d3);
            
            /* Complex condition to create control flow */
            if (i % 3 == 0) {
                accumulator += (uint64_t)(f3 * 1000);
            } else if (i % 3 == 1) {
                accumulator += (uint64_t)(d3 * 1000);
            } else {
                /* Force another computation path */
                int32_t temp = arr32[i] * i;
                VOLATILE_SINK(temp);
            }
        }
        
        /* Third inner loop - complex chain */
        for (int i = 0; i < 64; i++) {
            uint64_t chain_result = compute_chain(accumulator, i);
            
            /* More mixed operations */
            float f_chain = chain_result * 0.001f;
            double d_chain = chain_result * 0.0001;
            
            /* Force register pressure with many live values */
            int32_t t1 = chain_result & 0xFFFFFFFF;
            int32_t t2 = chain_result >> 32;
            float t3 = f_chain * 2.0f;
            double t4 = d_chain * 3.0;
            
            FORCE_COPY(t1); FORCE_COPY(t2); FORCE_COPY(t3); FORCE_COPY(t4);
            
            /* Use volatile arrays to prevent optimization */
            volatile_arr8[i] = t1 & 0xFF;
            volatile_arr16[i] = t2 & 0xFFFF;
            
            /* Conditional computation */
            if (outer % 2 == 0) {
                accumulator += t1 * t2;
            } else {
                accumulator += (uint64_t)(t3 + t4);
            }
        }
        
        /* Fourth loop - more pressure with different modes */
        for (int i = 0; i < 32; i++) {
            /* Create many dependent computations */
            int8_t c1 = arr8[i];
            int16_t s1 = c1 * 2;  /* Promotion */
            int32_t i1 = s1 * 3;  /* Promotion */
            int64_t l1 = i1 * 5LL;  /* Promotion */
            
            float f1 = l1 * 0.01f;  /* Conversion */
            double d1 = f1 * 0.02;  /* Conversion */
            
            /* Force all values to be live simultaneously */
            FORCE_COPY(c1); FORCE_COPY(s1); FORCE_COPY(i1);
            FORCE_COPY(l1); FORCE_COPY(f1); FORCE_COPY(d1);
            
            /* Complex expression with many temporaries */
            double result = (d1 + f1) * (l1 - i1) / (s1 + 1);
            
            /* Use inline asm to clobber registers */
            asm volatile("" : : "r"(c1), "r"(s1), "r"(i1), "r"(l1), 
                         "f"(f1), "f"(d1), "f"(result));
            
            accumulator += (uint64_t)result;
        }
        
        outer_counter++;
        
        /* Conditional break to create more control flow edges */
        if (accumulator > 1000000) {
            /* Reset to continue creating pressure */
            accumulator = accumulator % 1000;
        }
    }
    
    /* Final sink to prevent elimination */
    VOLATILE_SINK(accumulator);
    VOLATILE_SINK(outer_counter);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
