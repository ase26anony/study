/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent optimization */
static volatile int volatile_sink_int;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    volatile int32_t temp = a + b;
    asm volatile("" : "+r"(temp) : : "memory");
    return temp + c;
}

static inline double promote_and_multiply(float f, double d, int32_t i) {
    volatile double temp = (double)f * d;
    asm volatile("" : "+f"(temp) : : "memory");
    return temp * i;
}

static inline int16_t narrow_and_shift(int32_t x, int32_t y) {
    volatile int16_t temp = (int16_t)(x >> 4);
    asm volatile("" : "+r"(temp) : : "memory");
    return temp + (int16_t)y;
}

/* Function with complex control flow */
static int process_chunk(int8_t* data, float* floats, double* doubles, 
                         int start, int end, int stride) {
    int sum = 0;
    double acc_double = 0.0;
    
    /* Multiple basic blocks with different register pressure */
    for (int i = start; i < end; i += stride) {
        /* First basic block: integer operations */
        int32_t base = data[i] * 3;
        int32_t offset = i & 0xFF;
        
        /* Force copy propagation context */
        int32_t val1 = copy_and_add(base, offset, i);
        int32_t val2 = copy_and_add(val1, base, offset);
        
        /* Conditional creates control flow */
        if (val1 > val2) {
            /* Different register modes here */
            float fval = floats[i % 256];
            double dval = doubles[i % 128];
            
            /* Mixed precision calculations */
            double mixed = promote_and_multiply(fval, dval, val1);
            acc_double += mixed;
            
            /* Narrow conversion */
            int16_t narrow = narrow_and_shift(val1, val2);
            sum += narrow;
        } else {
            /* Alternative path with different operations */
            int32_t diff = val2 - val1;
            volatile_sink_int = diff;
            
            /* More copies */
            int32_t scaled = diff * 7;
            int32_t shifted = scaled >> 2;
            
            /* Chain of dependent operations */
            for (int j = 0; j < 3; j++) {
                shifted = copy_and_add(shifted, j, diff);
                volatile_sink_int = shifted;
            }
            sum += shifted;
        }
        
        /* Inner loop with register pressure */
        int32_t temp_sum = 0;
        for (int k = 0; k < 4; k++) {
            /* Short-lived recomputable values */
            int32_t recompute = (i * k) + (sum & 0xF);
            int32_t another = recompute * 13;
            
            /* Force virtual register creation */
            temp_sum += copy_and_add(recompute, another, k);
            
            /* Floating point in inner loop */
            if (k & 1) {
                float f = floats[(i + k) % 256];
                double d = doubles[(i + k) % 128];
                volatile_sink_double = promote_and_multiply(f, d, temp_sum);
            }
        }
        
        /* Use result to prevent elimination */
        sum += temp_sum;
        
        /* More mixed operations */
        if (i % 8 == 0) {
            double d = acc_double * 0.5;
            int32_t truncated = (int32_t)d;
            sum += truncated;
            
            /* Reset accumulator periodically */
            acc_double = d * 0.1;
        }
    }
    
    return sum;
}

/* Main driver with nested loops */
int main(void) {
    /* Initialize arrays with different patterns */
    int8_t data[1024];
    float floats[256];
    double doubles[128];
    
    for (int i = 0; i < 1024; i++) {
        data[i] = (i * 37) & 0xFF;
    }
    for (int i = 0; i < 256; i++) {
        floats[i] = (i * 0.1f) - 12.8f;
    }
    for (int i = 0; i < 128; i++) {
        doubles[i] = (i * 0.05) - 3.2;
    }
    
    int total = 0;
    
    /* Outer loop with multiple iterations */
    for (int outer = 0; outer < 8; outer++) {
        /* Vary parameters to create different register pressure patterns */
        int chunk_size = 64 + (outer * 8);
        int stride = 1 + (outer % 3);
        
        /* Process multiple chunks with overlap */
        for (int chunk = 0; chunk < 4; chunk++) {
            int start = chunk * 128;
            int end = start + chunk_size;
            
            /* This creates the virtual register context */
            int result = process_chunk(data, floats, doubles, 
                                      start, end, stride);
            
            /* Use result to prevent dead code elimination */
            total += result;
            
            /* Modify data slightly between chunks */
            if (chunk % 2 == 0) {
                for (int i = start; i < end && i < 1024; i++) {
                    data[i] += (result & 0x7F);
                }
            }
        }
        
        /* Additional processing with different modes */
        double double_acc = 0.0;
        for (int i = 0; i < 100; i++) {
            /* Mix integer and float operations */
            int idx = (outer * 13 + i) % 256;
            float f = floats[idx];
            double d = doubles[idx % 128];
            
            /* Chain of promotions and demotions */
            int32_t as_int = (int32_t)(f * 100.0f);
            double promoted = (double)as_int;
            double mixed = promoted * d;
            
            /* Force register moves with inline asm */
            asm volatile("" : "+r"(as_int), "+f"(mixed) : : "memory");
            
            double_acc += mixed;
            
            /* Periodic volatile write */
            if (i % 16 == 0) {
                volatile_sink_double = double_acc;
            }
        }
        
        /* Convert back to integer */
        total += (int)double_acc;
    }
    
    /* Final result to prevent optimization */
    volatile_sink_int = total;
    
    return total & 0xFF;
}
