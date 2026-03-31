/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer */
#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Helper functions to force copy propagation */
static inline int32_t process_int(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a * b;
    int32_t t2 = t1 + c;
    int32_t t3 = t2 ^ a;
    FORCE_COPY(t3);
    return t3;
}

static inline float process_float(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    float t3 = t2 - a;
    FORCE_COPY(t3);
    return t3;
}

static inline int16_t process_short(int16_t a, int16_t b, int16_t c) {
    int16_t t1 = (int16_t)(a + b);
    int16_t t2 = (int16_t)(t1 * c);
    int16_t t3 = (int16_t)(t2 ^ a);
    FORCE_COPY(t3);
    return t3;
}

/* Mixed precision calculations to create different register modes */
static inline double mixed_calc(int32_t a, float b, double c) {
    double d1 = (double)a;
    double d2 = (double)b;
    double d3 = d1 * d2 + c;
    /* Force register clobbering */
    asm volatile("" : : "r"(a), "x"(d3));
    return d3;
}

/* Complex kernel with nested loops and conditionals */
void compute_kernel(int32_t *int_data, float *float_data, 
                    int16_t *short_data, double *double_data,
                    int size) {
    volatile int outer_sink = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create register pressure with many short-lived values */
        int32_t base = int_data[i];
        float fbase = float_data[i];
        int16_t sbase = short_data[i];
        double dbase = double_data[i];
        
        /* Force copies between different scopes */
        {
            int32_t tmp1 = base * 3;
            int32_t tmp2 = tmp1 + 7;
            int32_t tmp3 = tmp2 ^ 0x55;
            int32_t tmp4 = process_int(tmp3, base, i);
            
            /* Volatile sink to prevent elimination */
            VOLATILE_SINK(tmp4);
        }
        
        for (int j = 0; j < 8; j++) {
            /* Inner loop creates more pressure */
            float f1 = fbase * (float)j;
            float f2 = f1 + (float)(i * j);
            float f3 = process_float(f2, fbase, (float)j);
            
            int16_t s1 = (int16_t)(sbase + j);
            int16_t s2 = (int16_t)(s1 * 2);
            int16_t s3 = process_short(s2, sbase, (int16_t)j);
            
            /* Conditional block creates control flow complexity */
            if ((i + j) & 1) {
                double d1 = mixed_calc(base, f3, dbase);
                double d2 = d1 * (double)s3;
                
                /* More virtual register pressure */
                int32_t t1 = base + j;
                int32_t t2 = t1 * 2;
                int32_t t3 = t2 - s3;
                int32_t t4 = process_int(t3, base, j);
                
                /* Force register moves with inline asm */
                asm volatile("" : : "r"(t4), "x"(d2));
                
                double_data[i] += d2;
                VOLATILE_SINK(t4);
            } else {
                /* Alternative path with different operations */
                float f4 = f3 * 2.0f;
                int16_t s4 = (int16_t)(s3 + 1);
                
                /* Chain of dependent calculations */
                for (int k = 0; k < 4; k++) {
                    int32_t chain1 = base + k;
                    int32_t chain2 = chain1 * (int32_t)f4;
                    int32_t chain3 = chain2 ^ s4;
                    int32_t chain4 = process_int(chain3, chain1, k);
                    
                    /* Force spilling/reloading */
                    asm volatile("" : : "r"(chain4), "r"(k));
                    
                    if (k & 1) {
                        float f5 = f4 * (float)chain4;
                        VOLATILE_SINK(f5);
                    }
                }
            }
            
            /* Cross-type operations to generate different modes */
            int32_t cross1 = (int32_t)f3;
            int16_t cross2 = (int16_t)cross1;
            float cross3 = (float)cross2;
            double cross4 = mixed_calc(cross1, cross3, dbase);
            
            /* Final sink */
            outer_sink += (int)cross4;
        }
        
        /* Another level of nesting with different data types */
        for (int m = 0; m < 4; m++) {
            double dval = double_data[i];
            for (int n = 0; n < 3; n++) {
                int32_t ival = int_data[CLAMP(i + m + n, 0, size-1)];
                float fval = float_data[CLAMP(i - m + n, 0, size-1)];
                
                /* Complex expression with many intermediates */
                double result = (double)ival * dval + (double)fval;
                result = result / (double)(m + n + 1);
                
                /* Force copy propagation context */
                int32_t copied = (int32_t)result;
                int32_t recopied = copied;
                FORCE_COPY(recopied);
                
                /* Use in another calculation */
                float fres = (float)recopied * fval;
                VOLATILE_SINK(fres);
            }
        }
    }
    
    VOLATILE_SINK(outer_sink);
}

/* Initialize with varied patterns */
void init_data(int32_t *int_data, float *float_data,
               int16_t *short_data, double *double_data,
               int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) ^ 0x1234;
        float_data[i] = (float)(i * 2.5) + 1.0f;
        short_data[i] = (int16_t)((i * 19) & 0xFFFF);
        double_data[i] = (double)i * 3.14159;
    }
}

int main(void) {
    const int SIZE = 256;
    
    /* Allocate aligned to avoid unnecessary moves */
    int32_t *int_data = __builtin_alloca(SIZE * sizeof(int32_t));
    float *float_data = __builtin_alloca(SIZE * sizeof(float));
    int16_t *short_data = __builtin_alloca(SIZE * sizeof(int16_t));
    double *double_data = __builtin_alloca(SIZE * sizeof(double));
    
    init_data(int_data, float_data, short_data, double_data, SIZE);
    
    /* Multiple passes to increase optimization opportunities */
    for (int pass = 0; pass < 3; pass++) {
        compute_kernel(int_data, float_data, short_data, double_data, SIZE);
        
        /* Modify data slightly between passes */
        for (int i = 0; i < SIZE; i++) {
            int_data[i] += pass;
            float_data[i] += (float)pass;
        }
    }
    
    return 0;
}
