/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -S -o test.s test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : : "r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0","r1","r2","r3","r4","r5","r6","r7")

/* Helper functions to force copy propagation contexts */
static inline int32_t copy_and_transform(int32_t a, int32_t b, char modifier) {
    volatile int32_t temp = a + b;  /* Force memory traffic */
    int32_t result = temp * modifier;
    FORCE_COPY(result);
    return result;
}

static inline double promote_and_compute(float f, short s, int i) {
    double d1 = (double)f;
    double d2 = (double)s;
    double d3 = (double)i;
    volatile double vd = d1;  /* Block optimization */
    double result = vd * d2 + d3;
    FORCE_COPY(result);
    return result;
}

static inline int64_t mixed_width_op(uint8_t b, uint16_t w, uint32_t d) {
    int64_t q1 = (int64_t)b * 37;
    int64_t q2 = (int64_t)w * 41;
    int64_t q3 = (int64_t)d * 43;
    volatile int64_t vq = q1;  /* Prevent coalescing */
    int64_t result = vq + q2 - q3;
    FORCE_COPY(result);
    return result;
}

/* Main computation kernel */
void compute_kernel(int32_t *int_data, float *float_data, 
                    double *double_data, int iterations) {
    /* Multiple local variables to create register pressure */
    register int32_t r0 asm("r0");
    register int32_t r1 asm("r1");
    register float f0 asm("s0");
    register double d0 asm("d0");
    
    /* Outer loop with multiple induction variables */
    for (int outer = 0; outer < iterations; outer += 4) {
        /* First inner loop - integer operations */
        for (int i = 0; i < 128; i++) {
            /* Chain of dependent arithmetic operations */
            int32_t a = int_data[i] * 3;
            int32_t b = a + outer;
            int32_t c = b ^ 0x55AA55AA;
            int32_t d = c - i;
            
            /* Force copy propagation with helper */
            int32_t e = copy_and_transform(d, outer, (char)(i & 0xFF));
            
            /* Mix with volatile to prevent optimization */
            volatile int32_t v = e;
            r0 = v * 7;
            r1 = r0 + 1;
            
            /* Store result back with different mode */
            int_data[i] = (int16_t)r1;  /* Truncation creates new mode */
            
            CLASH_REGISTERS;  /* Force register shuffling */
        }
        
        /* Second inner loop - floating point operations */
        for (int j = 0; j < 64; j += 2) {
            /* Mixed precision calculations */
            float f1 = float_data[j] * 1.5f;
            float f2 = float_data[j + 1] * 2.5f;
            
            /* Promote to double and compute */
            double d1 = promote_and_compute(f1, (short)j, outer);
            double d2 = promote_and_compute(f2, (short)(j + 1), outer + 1);
            
            /* Dependent chain with mode changes */
            f0 = (float)d1;
            d0 = (double)f0 + d2;
            
            /* Force spill/reload context */
            volatile double vd = d0;
            double_data[j] = vd * 0.5;
            
            /* More register pressure */
            asm volatile("" : : "r"(j), "r"(outer) : "memory");
        }
        
        /* Third inner loop - mixed width operations */
        for (int k = 0; k < 32; k++) {
            /* Operations with different integer widths */
            uint8_t b = (uint8_t)(k * 3);
            uint16_t w = (uint16_t)(k * 5);
            uint32_t d = (uint32_t)(k * 7);
            
            /* This creates virtual registers with different modes */
            int64_t q = mixed_width_op(b, w, d);
            
            /* Conditional that creates control flow complexity */
            if (k & 1) {
                int32_t t1 = (int32_t)(q >> 32);
                int32_t t2 = (int32_t)(q & 0xFFFFFFFF);
                int32_t t3 = copy_and_transform(t1, t2, (char)k);
                VOLATILE_SINK(t3);
            } else {
                float ft = (float)(q & 0xFF);
                double dt = promote_and_compute(ft, (short)k, outer);
                VOLATILE_SINK((int)dt);
            }
            
            /* Array access with stride to prevent optimization */
            int_data[(k * 3) & 127] = (int32_t)q;
        }
        
        /* Complex conditional block */
        if (outer & 1) {
            /* Block A - more register pressure */
            for (int m = 0; m < 16; m++) {
                int32_t x = int_data[m] * 11;
                int32_t y = x + m;
                int32_t z = y ^ x;
                float fz = (float)z;
                double dz = (double)fz * 1.2345;
                VOLATILE_SINK((int)dz);
                
                /* Force copies between different scopes */
                {
                    volatile int32_t vx = x;
                    int32_t x2 = vx;
                    int32_t x3 = copy_and_transform(x2, y, (char)m);
                    int_data[m] = x3;
                }
            }
        } else {
            /* Block B - different pattern */
            for (int m = 0; m < 16; m++) {
                double d = double_data[m];
                float f = (float)d;
                int32_t i = (int32_t)f;
                int64_t q = mixed_width_op((uint8_t)i, (uint16_t)i, (uint32_t)i);
                VOLATILE_SINK((int)q);
            }
        }
    }
}

/* Initialize with pattern to avoid constant propagation */
void init_data(int32_t *int_data, float *float_data, 
               double *double_data, int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) ^ 0x12345678;
        float_data[i] = (float)((i * 51) % 100) / 3.14f;
        double_data[i] = (double)((i * 73) % 200) / 6.28;
    }
}

int main() {
    /* Large enough to cause register pressure but fit in cache */
    const int DATA_SIZE = 256;
    
    /* Dynamic allocation to prevent stack optimization */
    int32_t *int_data = (int32_t*)malloc(DATA_SIZE * sizeof(int32_t));
    float *float_data = (float*)malloc(DATA_SIZE * sizeof(float));
    double *double_data = (double*)malloc(DATA_SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    init_data(int_data, float_data, double_data, DATA_SIZE);
    
    /* Multiple iterations to create steady state */
    for (int iter = 0; iter < 100; iter++) {
        compute_kernel(int_data, float_data, double_data, DATA_SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < DATA_SIZE; i++) {
            int_data[i] += iter;
            float_data[i] += (float)iter * 0.01f;
            double_data[i] += (double)iter * 0.005;
        }
    }
    
    /* Consume final result to prevent elimination */
    volatile int32_t final_sum = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        final_sum += int_data[i];
    }
    VOLATILE_SINK(final_sum);
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
