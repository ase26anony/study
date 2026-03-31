/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7")

/* Helper functions to force copy propagation */
static inline int32_t copy_and_transform(int32_t a, int32_t b, int mode) {
    volatile int32_t temp = a + b;
    FORCE_COPY(temp);
    return (mode == 0) ? temp * 2 : temp / 2;
}

static inline float float_copy_transform(float a, float b, int scale) {
    volatile float temp = a * b;
    FORCE_COPY(temp);
    return (scale > 0) ? temp + 1.0f : temp - 1.0f;
}

static inline int16_t narrow_copy(int32_t val) {
    volatile int16_t narrowed = (int16_t)(val & 0xFFFF);
    FORCE_COPY(narrowed);
    return narrowed;
}

/* Complex calculation chain that creates many virtual registers */
static inline int32_t compute_chain(int32_t base, int8_t offset, 
                                   float fscale, double dscale) {
    int32_t step1 = base * 3;
    FORCE_COPY(step1);
    
    float fstep = (float)step1 * fscale;
    VOLATILE_SINK(fstep);
    
    int32_t step2 = step1 + offset * 7;
    FORCE_COPY(step2);
    
    double dstep = (double)step2 * dscale;
    VOLATILE_SINK(dstep);
    
    int32_t step3 = step2 ^ (int32_t)dstep;
    FORCE_COPY(step3);
    
    return step3 + (int32_t)fstep;
}

int main(void) {
    /* Initialize arrays with different patterns */
    int32_t int_array[256];
    float float_array[256];
    int16_t short_array[256];
    volatile int8_t volatile_seed = 42;
    
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 3 + 1;
        float_array[i] = i * 0.5f;
        short_array[i] = (i * 7) & 0xFF;
    }
    
    int32_t accumulator = 0;
    float f_accumulator = 0.0f;
    double d_accumulator = 0.0;
    
    /* Outer loop - creates register pressure */
    for (int outer = 0; outer < 100; outer++) {
        VOLATILE_SINK(outer);
        CLASH_REGISTERS;
        
        /* First inner loop - integer operations */
        for (int i = 0; i < 128; i++) {
            int32_t temp1 = int_array[i] * 2;
            int32_t temp2 = int_array[i + 128] * 3;
            
            /* Force copy propagation through function call */
            int32_t copied = copy_and_transform(temp1, temp2, i & 1);
            FORCE_COPY(copied);
            
            /* Mixed-type computation */
            float ftemp = float_array[i] * 2.0f;
            int32_t mixed = copied + (int32_t)ftemp;
            
            /* Narrowing conversion forces mode change */
            int16_t narrow = narrow_copy(mixed);
            accumulator += narrow * (i % 16);
            
            /* Volatile read creates barrier */
            accumulator += volatile_seed;
        }
        
        /* Second inner loop - floating point operations */
        for (int j = 0; j < 64; j++) {
            float f1 = float_array[j * 2];
            float f2 = float_array[j * 2 + 1];
            
            /* Force float copy propagation */
            float fcopied = float_copy_transform(f1, f2, j & 1);
            FORCE_COPY(fcopied);
            
            /* Mixed precision */
            double dval = (double)fcopied * 1.5;
            int32_t iconv = (int32_t)dval;
            
            /* Complex chain with many virtual registers */
            int32_t chain_result = compute_chain(
                iconv, 
                (int8_t)j,
                fcopied,
                dval
            );
            
            f_accumulator += fcopied * (j % 8);
            d_accumulator += dval;
            accumulator += chain_result;
            
            /* Artificial clobbering */
            CLASH_REGISTERS;
        }
        
        /* Conditional block with different register usage */
        if (outer % 3 == 0) {
            for (int k = 0; k < 32; k++) {
                /* Different operation pattern */
                int64_t wide = (int64_t)accumulator * k;
                float fwide = (float)wide * 0.25f;
                double dwide = (double)fwide * 2.0;
                
                /* Force copies between different modes */
                int32_t truncated = (int32_t)dwide;
                int16_t halved = (int16_t)(truncated >> 1);
                
                accumulator += halved * k;
                f_accumulator += fwide;
                d_accumulator += dwide;
                
                /* More register clobbering */
                asm volatile("" : : : "memory");
            }
        } else if (outer % 3 == 1) {
            /* Alternative path with different computations */
            for (int k = 0; k < 16; k++) {
                int32_t base = short_array[k] * 11;
                float fbase = (float)base * 0.1f;
                
                /* Chain of dependent operations */
                for (int m = 0; m < 4; m++) {
                    base = base * 2 + m;
                    fbase = fbase * 1.1f + (float)m;
                    
                    VOLATILE_SINK(base);
                    VOLATILE_SINK(fbase);
                    
                    /* Force copy at each iteration */
                    int32_t copied_base = copy_and_transform(base, m, m & 1);
                    accumulator += copied_base;
                }
            }
        }
        
        /* Final barrier to prevent optimization across iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    VOLATILE_SINK(accumulator);
    VOLATILE_SINK(f_accumulator);
    VOLATILE_SINK(d_accumulator);
    
    return (accumulator > 0) ? 0 : 1;
}
