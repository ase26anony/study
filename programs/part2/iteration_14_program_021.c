/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand -o test_remat test_remat.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile float volatile_sink_float;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int copy_and_add(int a, int b) {
    int tmp = a;
    asm volatile("" : "+r"(tmp) : "r"(b));
    return tmp + b;
}

static inline float promote_and_multiply(short s, char c) {
    float f1 = (float)s;
    float f2 = (float)c;
    asm volatile("" : "+x"(f1), "+x"(f2));
    return f1 * f2;
}

static inline double mixed_calc(int i, float f, long l) {
    double d1 = (double)i;
    double d2 = (double)f;
    double d3 = (double)l;
    /* Force register moves with clobbers */
    asm volatile("# clobber mix" : "+r"(i), "+x"(f), "+r"(l));
    return d1 * d2 - d3;
}

/* Function to create complex control flow */
static int process_chunk(int start, int end, float* farr, char* carr) {
    int acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Multiple basic blocks with different register pressure */
    for (int i = start; i < end; i++) {
        /* First block: integer operations */
        int tmp1 = i * 3;
        int tmp2 = tmp1 + (i & 0xFF);
        int tmp3 = copy_and_add(tmp1, tmp2);
        
        /* Force spill/reload context */
        asm volatile("" : "+r"(tmp3));
        
        /* Second block: floating promotions */
        short s_val = (short)(tmp3 % 256);
        char c_val = (char)(i % 128);
        float f_val = promote_and_multiply(s_val, c_val);
        
        /* Conditional creates control flow merge point */
        if (i & 1) {
            f_val *= 1.5f;
            tmp3 += 1000;
        } else {
            f_val *= 0.75f;
            tmp3 -= 500;
        }
        
        /* Third block: mixed precision */
        long l_val = (long)tmp3 * (long)i;
        double d_val = mixed_calc(tmp3, f_val, l_val);
        
        /* Accumulate with different types */
        acc_int += tmp3;
        acc_float += f_val;
        acc_double += d_val;
        
        /* Array accesses with different strides */
        farr[i % 16] = f_val;
        carr[i % 32] = (char)(tmp3 & 0xFF);
        
        /* Volatile sinks to preserve computations */
        volatile_sink_int = tmp3;
        volatile_sink_float = f_val;
        volatile_sink_double = d_val;
    }
    
    /* Final reduction with type mixing */
    return acc_int + (int)acc_float + (int)acc_double;
}

int main(void) {
    /* Initialize with different patterns */
    int int_array[256];
    float float_array[16];
    char char_array[32];
    double double_array[8];
    
    for (int i = 0; i < 256; i++) {
        int_array[i] = (i * 37) & 0xFFF;
    }
    for (int i = 0; i < 16; i++) {
        float_array[i] = (float)i * 0.123f;
    }
    for (int i = 0; i < 32; i++) {
        char_array[i] = (char)(i * 7);
    }
    for (int i = 0; i < 8; i++) {
        double_array[i] = (double)i * 0.456;
    }
    
    int total = 0;
    
    /* Nested loops with varying trip counts */
    for (int outer = 0; outer < 100; outer++) {
        /* Outer loop creates register pressure across iterations */
        int outer_acc = outer * 2;
        float outer_float = (float)outer * 0.01f;
        
        for (int middle = 0; middle < 50; middle++) {
            /* Middle loop with its own pressure */
            int middle_tmp = middle + outer_acc;
            float middle_float = outer_float + (float)middle * 0.02f;
            
            /* Inner loop - main computation */
            for (int inner = 0; inner < 25; inner++) {
                /* Chain of dependent computations */
                int idx = (inner + middle_tmp) & 0xFF;
                int base = int_array[idx];
                
                /* Multiple recomputable expressions */
                int val1 = base + 17;
                int val2 = val1 * 3;
                int val3 = val2 - (inner & 0xF);
                int val4 = copy_and_add(val3, base);
                
                /* Floating conversions */
                float f1 = (float)val4 * 0.25f;
                float f2 = promote_and_multiply((short)val3, (char)val4);
                float f3 = f1 + f2;
                
                /* Double precision */
                double d1 = (double)f3 * 1.234;
                double d2 = mixed_calc(val4, f3, (long)val4 * 1000L);
                double d3 = d1 - d2;
                
                /* Array updates with different modes */
                float_array[inner % 16] = f3;
                char_array[inner % 32] = (char)(val4 & 0xFF);
                double_array[inner % 8] = d3;
                
                /* Force register clobbering */
                asm volatile("# inner loop clobber" 
                           : "+r"(val4), "+x"(f3), "+r"(d3)
                           : "r"(base), "x"(f1), "r"(val3));
                
                /* Volatile sinks */
                volatile_sink_int = val4;
                volatile_sink_float = f3;
                volatile_sink_double = d3;
                
                total += val4 + (int)f3;
            }
            
            /* Call processing function with different scopes */
            int chunk_result = process_chunk(0, 16, float_array, char_array);
            total += chunk_result;
            
            /* More register pressure */
            asm volatile("# middle loop barrier" : : : "memory");
        }
        
        /* Outer loop cleanup with mixed operations */
        double outer_dbl = (double)outer_acc * outer_float;
        volatile_sink_double = outer_dbl;
    }
    
    /* Final result with all types */
    float final_float = (float)total * 0.001f;
    double final_double = (double)total * 0.0001;
    int result = total + (int)final_float + (int)final_double;
    
    return result & 0x7FFFFFFF; /* Ensure positive return */
}
