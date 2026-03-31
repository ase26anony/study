/* Program to trigger early rematerialization copy_propagate logic */
#include <stdint.h>
#include <stdlib.h>

#define FORCE_USE(x) asm volatile("" : : "r"(x))
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

/* Helper functions to force copy propagation */
static inline int32_t process_int(int32_t a, int32_t b, int32_t c) {
    volatile int32_t temp = a + b;
    return (temp * c) >> 3;
}

static inline float process_float(float a, float b, float c) {
    volatile float temp = a * b;
    return temp + c * 0.5f;
}

static inline int16_t narrow_convert(int32_t val) {
    volatile int32_t tmp = val;
    return CLAMP(tmp, -32768, 32767);
}

static inline double promote_and_compute(float f, int32_t i) {
    volatile double d = (double)f;
    volatile int64_t li = (int64_t)i;
    return d * li;
}

/* Main computation kernel */
__attribute__((noinline))
static void compute_kernel(int32_t* int_data, float* float_data, 
                          int16_t* short_data, double* double_data,
                          int size, int iterations) {
    volatile int sink = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop creates register pressure */
        int32_t accum_int = iter;
        float accum_float = iter * 0.1f;
        double accum_double = 0.0;
        
        /* First inner loop - integer operations */
        for (int i = 0; i < size; i++) {
            /* Chain of dependent integer operations */
            int32_t val1 = int_data[i] + accum_int;
            int32_t val2 = process_int(val1, i, iter);
            int32_t val3 = val2 * 7 - 13;
            
            /* Force copy between different scopes */
            {
                volatile int32_t temp = val3;
                val1 = temp + int_data[(i + 1) % size];
            }
            
            int32_t val4 = val1 ^ val2 ^ val3;
            accum_int = process_int(accum_int, val4, i);
            
            /* Mixed type conversion */
            float fval = (float)val4 * 0.01f;
            accum_float = process_float(accum_float, fval, val1);
            
            /* Narrow conversion creates different mode */
            int16_t sval = narrow_convert(val3);
            short_data[i] = sval;
            
            FORCE_USE(accum_int);
            FORCE_USE(accum_float);
        }
        
        /* Second inner loop - floating point operations */
        for (int i = size - 1; i >= 0; i--) {
            /* Different control flow to create complex CFG */
            if (i % 3 == 0) {
                float f1 = float_data[i];
                float f2 = process_float(f1, accum_float, i);
                float f3 = f2 * f2 - f1;
                
                /* Force register copy with volatile */
                volatile float ftemp = f3;
                float_data[i] = ftemp + 0.5f;
                
                /* Promotion to double */
                accum_double += promote_and_compute(f3, accum_int);
            } else if (i % 3 == 1) {
                /* Integer path */
                int32_t ival = int_data[i] ^ accum_int;
                ival = process_int(ival, i, iter);
                
                /* Multiple uses to prevent elimination */
                volatile int32_t v1 = ival;
                volatile int32_t v2 = ival * 2;
                int_data[i] = v1 + v2;
            } else {
                /* Mixed path */
                double dval = (double)float_data[i] * (double)int_data[i];
                volatile double dtemp = dval;
                double_data[i] = dtemp * 0.99;
            }
            
            /* Complex expression with many temporaries */
            int32_t t1 = int_data[i] + i;
            int32_t t2 = t1 * t1 - 17;
            int32_t t3 = process_int(t2, accum_int, t1);
            int32_t t4 = t3 | (t2 & 0xFF);
            accum_int = t4 ^ (iter << 3);
            
            FORCE_USE(accum_double);
        }
        
        /* Third loop with nested conditionals */
        for (int i = 0; i < size; i += 2) {
            int32_t base = int_data[i];
            
            for (int j = 0; j < 4; j++) {  /* Small inner loop */
                /* Create many short-lived values */
                int32_t tmp1 = base + j * 11;
                int32_t tmp2 = tmp1 * tmp1 - 19;
                int32_t tmp3 = process_int(tmp2, j, tmp1);
                
                /* Conditional creates different basic blocks */
                if (tmp3 > 1000) {
                    float ftmp = (float)tmp3 * 0.001f;
                    volatile float fv = ftmp;
                    accum_float += process_float(fv, accum_float, j);
                } else {
                    int16_t stmp = narrow_convert(tmp3);
                    volatile int16_t sv = stmp;
                    short_data[i] = sv;
                }
                
                /* Another dependent chain */
                tmp1 = tmp3 + accum_int;
                tmp2 = process_int(tmp1, i, j);
                tmp3 = tmp2 ^ (tmp1 << 2);
                base = tmp3;
                
                /* Inline asm to clobber registers */
                asm volatile("" : : "r"(tmp1), "r"(tmp2), "r"(tmp3));
            }
            
            int_data[i] = base;
            sink += accum_int + (int)accum_float;
        }
        
        /* Final mixing */
        accum_double += promote_and_compute(accum_float, accum_int);
        double_data[iter % size] = accum_double;
        
        FORCE_USE(sink);
    }
}

/* Initialize with different patterns */
static void init_data(int32_t* int_data, float* float_data,
                     int16_t* short_data, double* double_data, int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) ^ 0x12345678;
        float_data[i] = (float)i * 0.73f - 12.5f;
        short_data[i] = (int16_t)((i * 59) & 0xFFFF);
        double_data[i] = (double)i * 1.234567;
    }
}

int main(void) {
    const int size = 256;
    const int iterations = 1000;
    
    /* Allocate aligned to avoid unnecessary checks */
    int32_t* int_data = __builtin_alloca(size * sizeof(int32_t));
    float* float_data = __builtin_alloca(size * sizeof(float));
    int16_t* short_data = __builtin_alloca(size * sizeof(int16_t));
    double* double_data = __builtin_alloca(size * sizeof(double));
    
    init_data(int_data, float_data, short_data, double_data, size);
    
    /* Multiple passes with different patterns */
    for (int pass = 0; pass < 3; pass++) {
        compute_kernel(int_data, float_data, short_data, double_data, 
                      size, iterations / (pass + 1));
        
        /* Modify data between passes */
        for (int i = 0; i < size; i++) {
            int_data[i] ^= 0xAAAAAAAA;
            float_data[i] *= -1.0f;
        }
    }
    
    /* Final checksum */
    volatile int32_t checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += int_data[i] + (int32_t)float_data[i] + short_data[i];
    }
    
    return checksum & 0xFF;
}
