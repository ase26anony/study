/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -o test_remat test_remat.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int copy_and_add(int a, int b, int c) {
    int temp = a + b;
    asm volatile("" : "+r"(temp) : "r"(c));
    return temp + c;
}

static inline float promote_and_multiply(short s, char c) {
    float f1 = (float)s;
    float f2 = (float)c;
    asm volatile("" : "+f"(f1), "+f"(f2));
    return f1 * f2;
}

static inline double mixed_calc(int i, float f, long l) {
    double d1 = (double)i;
    double d2 = (double)f;
    double d3 = (double)l;
    /* Force register moves */
    asm volatile("" : "+r"(i), "+f"(d1), "+r"(l));
    return d1 * d2 + d3;
}

/* Function with complex control flow */
static int process_chunk(int start, int end, const int* data, float* fdata) {
    int acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Multiple basic blocks created by conditionals */
    for (int i = start; i < end; i++) {
        /* First basic block: integer operations */
        int val1 = data[i] * 3;
        int val2 = val1 + (i & 0xFF);
        
        /* Force copy propagation context */
        int val3 = copy_and_add(val1, val2, i);
        
        /* Conditional creates new basic block */
        if (val3 > 1000) {
            /* Mixed precision in true branch */
            short s_val = (short)val3;
            char c_val = (char)(i % 256);
            float f_val = promote_and_multiply(s_val, c_val);
            
            /* More copies */
            float f_val2 = f_val * 2.0f;
            asm volatile("" : "+f"(f_val2));
            
            acc_float += f_val2;
            
            /* Integer chain continues */
            int val4 = val3 - 500;
            val4 = val4 * 2 + (val4 >> 3);
            acc_int += val4;
        } else {
            /* False branch with different operations */
            long l_val = (long)val3 * 7L;
            double d_val = mixed_calc(val3, acc_float, l_val);
            
            /* Volatile write to prevent elimination */
            volatile_sink_double = d_val;
            
            acc_double += d_val;
            
            /* Another integer chain */
            int val4 = val3 * 5 - 123;
            val4 = (val4 << 2) | (val4 & 0xF);
            acc_int ^= val4;
        }
        
        /* Loop-carried dependency with recomputable values */
        int loop_dep = acc_int + i;
        loop_dep = loop_dep * 3 - loop_dep / 2;
        
        /* Force spill/reload context */
        asm volatile("" : "+r"(loop_dep));
        
        /* Store to volatile to create use */
        volatile_sink_int = loop_dep;
        
        /* Floating-point chain */
        float f_temp = (float)loop_dep;
        f_temp = f_temp * 1.5f - f_temp * 0.25f;
        acc_float += f_temp;
        
        /* Another conditional for more basic blocks */
        if ((i & 3) == 0) {
            double d_temp = (double)f_temp * 2.5;
            d_temp = d_temp + (double)acc_int;
            acc_double += d_temp;
            
            /* More register pressure */
            int extra1 = i * 11;
            int extra2 = extra1 + data[i % 16];
            int extra3 = extra2 * 3 - extra1;
            asm volatile("" : "+r"(extra1), "+r"(extra2), "+r"(extra3));
        }
    }
    
    /* Final mixing */
    int result = acc_int + (int)acc_float + (int)acc_double;
    asm volatile("" : "+r"(result));
    return result;
}

/* Main driver with nested loops */
int main(void) {
    /* Initialize with different patterns */
    const int ARRAY_SIZE = 1024;
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    /* Fill with varying data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (i * 37) & 0x7FF;
        float_data[i] = (float)((i * 19) % 100) * 0.1f;
    }
    
    int total_result = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 10; outer++) {
        /* Middle loop with stride */
        for (int chunk = 0; chunk < 8; chunk++) {
            int start = chunk * 128;
            int end = start + 64 + (chunk % 3) * 32;
            
            /* Inner processing with register pressure */
            int chunk_result = process_chunk(start, end, int_data, float_data);
            
            /* More operations on result */
            int modified = chunk_result;
            for (int k = 0; k < 4; k++) {
                modified = (modified * 3) + (modified >> 2);
                modified = modified ^ (outer * 0x1234);
            }
            
            /* Mixed precision accumulation */
            double d_accum = (double)modified;
            for (int k = 0; k < 3; k++) {
                d_accum = d_accum * 1.1 + (double)k;
                float f_part = (float)d_accum;
                f_part = f_part * 2.0f - f_part * 0.5f;
                d_accum += (double)f_part;
            }
            
            total_result += (int)d_accum;
            
            /* More virtual register pressure */
            short s_tmp = (short)total_result;
            char c_tmp = (char)(total_result & 0xFF);
            float f_tmp = promote_and_multiply(s_tmp, c_tmp);
            volatile_sink_double = (double)f_tmp;
        }
        
        /* Between chunks, create more opportunities */
        int inter_val = outer * 100;
        for (int j = 0; j < 20; j++) {
            inter_val = copy_and_add(inter_val, j, outer);
            inter_val = (inter_val << 1) | (inter_val & 1);
            
            /* Force different modes */
            if (j & 1) {
                float f = (float)inter_val;
                f = f * 0.75f;
                asm volatile("" : "+f"(f));
                volatile_sink_int = (int)f;
            } else {
                long l = (long)inter_val * 3L;
                double d = (double)l / 2.0;
                asm volatile("" : "+r"(l), "+f"(d));
            }
        }
    }
    
    /* Final result processing */
    total_result = total_result & 0x7FFFFFFF;
    
    /* One more complex chain */
    int final = total_result;
    for (int i = 0; i < 50; i++) {
        final = final * 1103515245 + 12345;
        final = (final & 0x7FFFFFFF);
        
        /* Frequent mode mixing */
        if (i % 5 == 0) {
            float f_final = (float)final;
            double d_final = (double)f_final;
            long l_final = (long)final;
            
            /* All used to prevent elimination */
            asm volatile("" : "+r"(final), "+f"(f_final), "+f"(d_final), "+r"(l_final));
        }
    }
    
    free(int_data);
    free(float_data);
    
    return final % 1000;
}
