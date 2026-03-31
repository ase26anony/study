/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t tmp1 = a + b;
    int32_t tmp2 = tmp1 ^ c;
    asm volatile("" : "+r"(tmp2) : : "memory");
    return tmp2 * 2;
}

static inline float promote_and_multiply(int16_t s, uint8_t b) {
    float f1 = (float)s * 1.5f;
    float f2 = (float)b * 0.25f;
    asm volatile("" : "+f"(f1), "+f"(f2));
    return f1 * f2;
}

static inline double mixed_calc(int a, float b, double c) {
    double d1 = (double)a * 1.618;
    double d2 = (double)b * 2.718;
    asm volatile("" : "+r"(a), "+f"(d1), "+f"(d2));
    return (d1 + d2) * c;
}

/* Function with complex control flow */
static int process_chunk(int8_t* data, float* fdata, int start, int end, 
                         int stride, int mode) {
    int acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Multiple loops with different induction variables */
    for (int i = start; i < end; i += stride) {
        int8_t val1 = data[i];
        uint16_t val2 = (uint16_t)(val1 * 3);
        
        /* First inner loop - integer operations */
        for (int j = 0; j < 4; ++j) {
            int tmp = val1 * j + val2;
            tmp = copy_and_add(tmp, i, j);
            
            /* Conditional to create different basic blocks */
            if (tmp & 1) {
                float fval = promote_and_multiply(val1, (uint8_t)tmp);
                acc_float += fval;
                volatile_sink_int = tmp; /* Force spill */
            } else {
                double dval = mixed_calc(tmp, acc_float, 1.0);
                acc_double += dval;
                asm volatile("" : : "r"(tmp), "f"(dval));
            }
            
            /* More arithmetic chains */
            int32_t chain1 = tmp * 7;
            int32_t chain2 = chain1 - 13;
            int32_t chain3 = chain2 ^ 0x55AA;
            int32_t chain4 = chain3 + i * j;
            
            /* Use volatile to prevent coalescing */
            volatile int vol_tmp = chain4;
            acc_int += vol_tmp;
        }
        
        /* Second inner loop - mixed precision */
        for (short k = 0; k < 3; ++k) {
            float base = fdata[i % 256];
            double expanded = (double)base * (double)k;
            
            /* Type conversions to generate different modes */
            long long big_val = (long long)acc_int * k;
            float converted = (float)big_val * 0.01f;
            
            /* Complex expression with many temporaries */
            double result = expanded * converted + (double)acc_float;
            result = result / ((double)(i + 1) * 0.5);
            
            /* Force register pressure with many live values */
            asm volatile("" 
                : 
                : "r"(i), "r"(k), "r"(acc_int), 
                  "f"(base), "f"(converted), "f"(result)
                : "memory");
                  
            acc_double += result;
        }
        
        /* Conditional block with different operations */
        if (mode == 0) {
            /* Integer-heavy path */
            int8_t* ptr = data + i;
            int sum = 0;
            for (int m = 0; m < 8; ++m) {
                sum += ptr[m] * m;
            }
            acc_int += sum * i;
            
            /* Inline asm that clobbers registers */
            asm volatile("" : : : "%eax", "%ecx", "%edx");
        } else if (mode == 1) {
            /* Float-heavy path */
            float* fptr = fdata + (i % 128);
            float prod = 1.0f;
            for (int m = 0; m < 4; ++m) {
                prod *= fptr[m] + (float)m;
            }
            acc_float += prod;
            
            /* Clobber floating point registers */
            asm volatile("" : : : "%xmm0", "%xmm1", "%xmm2");
        }
    }
    
    /* Final mixing */
    volatile_sink_int = acc_int;
    volatile_sink_double = acc_double;
    
    return acc_int + (int)acc_float + (int)acc_double;
}

/* Main driver with initialization and multiple passes */
int main(void) {
    const int SIZE = 1024;
    const int ITERS = 100;
    
    /* Allocate and initialize with different patterns */
    int8_t* int_data = (int8_t*)malloc(SIZE * sizeof(int8_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; ++i) {
        int_data[i] = (int8_t)((i * 37) & 0xFF);
        float_data[i] = (float)((i * 19) % 100) * 0.01f;
    }
    
    int total = 0;
    
    /* Outer loop with varying parameters */
    for (int iter = 0; iter < ITERS; ++iter) {
        int mode = iter % 3;
        int stride = 1 + (iter % 5);
        
        /* Process in chunks with different alignments */
        for (int chunk = 0; chunk < SIZE; chunk += 128) {
            int end = chunk + 64 + (iter % 64);
            if (end > SIZE) end = SIZE;
            
            /* This call creates register pressure */
            int result = process_chunk(int_data, float_data, 
                                      chunk, end, stride, mode);
            
            /* More arithmetic to chain dependencies */
            long long big_temp = (long long)result * iter;
            int reduced = (int)(big_temp % 1000);
            
            /* Mix with floating point */
            double dtemp = (double)reduced * 0.12345;
            float ftemp = (float)dtemp;
            int final_val = (int)(ftemp * 100.0f);
            
            /* Use inline asm to force copies */
            asm volatile("" 
                : "+r"(final_val), "+f"(ftemp), "+f"(dtemp)
                : 
                : "memory");
                
            total += final_val;
            
            /* Periodic volatile write to prevent optimization */
            if ((iter * chunk) % 1000 == 0) {
                volatile_sink_int = total;
            }
        }
        
        /* Alternate between different computation patterns */
        if (iter % 2 == 0) {
            /* Integer pattern */
            for (int i = 0; i < 50; ++i) {
                int a = total + i;
                int b = a * 3;
                int c = b ^ 0xABCD;
                int d = c - a;
                total += d % 100;
                asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            }
        } else {
            /* Mixed pattern */
            for (int i = 0; i < 30; ++i) {
                float f1 = (float)total * 0.01f;
                double d1 = (double)f1 * 1.234;
                int i1 = (int)d1;
                double d2 = d1 * (double)i1;
                total += (int)d2;
                asm volatile("" : : "r"(i1), "f"(f1), "f"(d1), "f"(d2));
            }
        }
    }
    
    volatile_sink_int = total;
    
    free(int_data);
    free(float_data);
    
    return total % 256;
}
