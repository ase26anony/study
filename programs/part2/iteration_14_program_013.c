/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -S -o test.s test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(var) asm volatile("" : "+r"(var))
#define VOLATILE_SINK(var) do { volatile int sink = (var); (void)sink; } while(0)
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

/* Helper functions to force copy propagation contexts */
static inline int32_t process_int(int32_t a, int32_t b, int mode) {
    volatile int32_t vol_a = a;
    int32_t result;
    switch(mode & 3) {
        case 0: result = vol_a + b * 2; break;
        case 1: result = vol_a - b / 3; break;
        case 2: result = vol_a ^ b; break;
        default: result = vol_a | (b << 2); break;
    }
    FORCE_COPY(result);
    return result;
}

static inline float process_float(float a, float b, int mode) {
    volatile float vol_a = a;
    float result;
    switch(mode & 3) {
        case 0: result = vol_a + b * 2.5f; break;
        case 1: result = vol_a - b / 1.7f; break;
        case 2: result = vol_a * b; break;
        default: result = vol_a / (b + 0.1f); break;
    }
    asm volatile("" : "+f"(result));
    return result;
}

static inline int16_t narrow_copy(int32_t val) {
    volatile int16_t narrow = (int16_t)CLAMP(val, -32768, 32767);
    int16_t result = narrow + (narrow >> 3);
    FORCE_COPY(result);
    return result;
}

static inline double promote_float(float f) {
    volatile double d = (double)f;
    double result = d * 1.23456789;
    asm volatile("" : "+f"(result));
    return result;
}

/* Main computation kernel */
void compute_kernel(int8_t* restrict arr1, int16_t* restrict arr2, 
                    int32_t* restrict arr3, float* restrict arr4,
                    double* restrict arr5, size_t size) {
    
    for (size_t outer = 0; outer < 3; ++outer) {
        volatile int outer_mod = outer % 7;
        
        for (size_t i = 0; i < size; ++i) {
            /* Create many short-lived recomputable values */
            int32_t base = arr1[i] * 3 + outer_mod;
            FORCE_COPY(base);
            
            /* Chain of dependent integer operations */
            int32_t v1 = base + (i & 0xFF);
            int32_t v2 = v1 * 2 - arr2[i % 256];
            int32_t v3 = v2 ^ (v1 >> 4);
            
            /* Force copy through helper */
            int32_t v4 = process_int(v3, base, i);
            
            /* Narrowing conversion */
            int16_t v5 = narrow_copy(v4);
            
            /* More integer computation */
            int32_t v6 = v5 * 7 + arr3[i % 128];
            int32_t v7 = v6 - (v5 << 2);
            
            VOLATILE_SINK(v7);
            
            /* Floating-point computations with different modes */
            float f1 = arr4[i % 64] * 1.5f + (float)v7;
            float f2 = f1 / (float)((i & 31) + 1);
            
            /* Force float copy */
            float f3 = process_float(f2, f1, i);
            
            /* Mixed precision */
            double d1 = promote_float(f3);
            double d2 = d1 + arr5[i % 32];
            double d3 = d2 * 0.987654321;
            
            /* Conditional block creating control flow complexity */
            if (i % 17 == 0) {
                volatile double temp = d3;
                d3 = temp * 2.0;
                asm volatile("" ::: "memory");
            } else if (i % 23 == 0) {
                volatile float ftemp = f3;
                f3 = ftemp * 1.1f;
                asm volatile("" ::: "xmm0", "xmm1", "xmm2");
            }
            
            /* More computations in different scopes */
            {
                int32_t local_v1 = v7 + (int32_t)f3;
                float local_f1 = f3 + (float)local_v1;
                
                /* Another helper call forcing copies */
                int32_t local_v2 = process_int(local_v1, (int32_t)d3, outer);
                float local_f2 = process_float(local_f1, (float)local_v2, i);
                
                VOLATILE_SINK(local_v2);
                VOLATILE_SINK(local_f2);
            }
            
            /* Store results back (creating more register pressure) */
            if (i < size - 1) {
                arr1[i + 1] = (int8_t)(v7 & 0xFF);
                arr2[(i + 1) % 256] = v5;
                arr3[(i + 1) % 128] = v4;
                arr4[(i + 1) % 64] = f3;
                arr5[(i + 1) % 32] = d3;
            }
        }
        
        /* Additional inner loop with different computation pattern */
        for (size_t j = 0; j < size / 2; ++j) {
            volatile int counter = j;
            
            /* Different type mixing */
            int8_t c1 = arr1[j] ^ (j & 0x7F);
            int16_t c2 = arr2[j % 256] + c1;
            int32_t c3 = arr3[j % 128] * c2;
            float c4 = arr4[j % 64] + (float)c3;
            double c5 = arr5[j % 32] * (double)c4;
            
            /* Force register moves with inline asm */
            asm volatile("" : "+r"(c1), "+r"(c2), "+r"(c3), "+f"(c4), "+f"(c5));
            
            /* Complex expression with many temporaries */
            double result = (double)c1 * 0.5 + 
                           (double)c2 * 0.25 + 
                           (double)c3 * 0.125 + 
                           c4 * 2.0 + 
                           c5;
            
            VOLATILE_SINK(result);
        }
    }
}

/* Initialize with different patterns */
void init_arrays(int8_t* arr1, int16_t* arr2, int32_t* arr3, 
                 float* arr4, double* arr5, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        arr1[i] = (int8_t)((i * 37) & 0xFF);
        arr2[i % 256] = (int16_t)((i * 73) & 0xFFFF);
        arr3[i % 128] = (int32_t)(i * 131);
        arr4[i % 64] = (float)(i * 0.73f);
        arr5[i % 32] = (double)(i * 1.37);
    }
}

int main(void) {
    const size_t size = 1024;
    
    /* Allocate arrays with different alignments */
    int8_t* arr1 = __builtin_aligned_alloc(64, size * sizeof(int8_t));
    int16_t* arr2 = __builtin_aligned_alloc(32, 256 * sizeof(int16_t));
    int32_t* arr3 = __builtin_aligned_alloc(32, 128 * sizeof(int32_t));
    float* arr4 = __builtin_aligned_alloc(32, 64 * sizeof(float));
    double* arr5 = __builtin_aligned_alloc(32, 32 * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5) {
        return 1;
    }
    
    init_arrays(arr1, arr2, arr3, arr4, arr5, size);
    
    /* Run multiple iterations to increase optimization opportunities */
    for (int iter = 0; iter < 10; ++iter) {
        volatile int iter_mod = iter % 5;
        compute_kernel(arr1, arr2, arr3, arr4, arr5, size);
        
        /* Modify arrays slightly between iterations */
        for (size_t i = 0; i < size; i += 17) {
            arr1[i] += iter_mod;
            arr2[i % 256] -= iter_mod;
            arr3[i % 128] ^= iter_mod;
            arr4[i % 64] *= 1.0f + iter_mod * 0.01f;
            arr5[i % 32] /= 1.0 + iter_mod * 0.02;
        }
    }
    
    /* Final volatile sink to prevent elimination */
    volatile int8_t final_sink = arr1[0];
    (void)final_sink;
    
    free(arr5);
    free(arr4);
    free(arr3);
    free(arr2);
    free(arr1);
    
    return 0;
}
