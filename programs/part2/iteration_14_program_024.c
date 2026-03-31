/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -S -o test.s test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5")

/* Helper functions to force copy propagation contexts */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 ^ c;
    FORCE_COPY(t2);
    return t2 * 7;
}

static inline float float_copy_mul(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    FORCE_COPY(t2);
    return t2 * 2.5f;
}

static inline int16_t narrow_compute(int32_t a, int32_t b, char c) {
    int16_t t1 = (int16_t)(a & 0xFFFF);
    int16_t t2 = (int16_t)(b & 0xFF);
    int16_t t3 = t1 + t2 + c;
    FORCE_COPY(t3);
    return t3;
}

/* Mixed precision calculations to generate different modes */
static inline double promote_and_compute(float f1, float f2, int32_t i) {
    double d1 = (double)f1;
    double d2 = (double)f2;
    double d3 = d1 * d2 + (double)i;
    FORCE_COPY(d3);
    return d3;
}

int main(void) {
    /* Initialize arrays with different patterns */
    volatile int32_t init_seed = 42;
    int32_t int_array[256];
    float float_array[256];
    int16_t short_array[256];
    
    for (int i = 0; i < 256; i++) {
        int_array[i] = (i * 37 + init_seed) & 0xFFF;
        float_array[i] = (float)(i * 0.1f + init_seed * 0.01f);
        short_array[i] = (int16_t)((i * 19) & 0x7FFF);
    }
    
    /* Outer loop creating register pressure */
    volatile int outer_iter = 8;
    double total_result = 0.0;
    
    for (int outer = 0; outer < outer_iter; outer++) {
        /* Multiple inner loops with different computation patterns */
        
        /* Pattern 1: Integer chain with many virtual registers */
        for (int i = 0; i < 128; i++) {
            int32_t a = int_array[i];
            int32_t b = int_array[i + 128];
            int32_t c = a + b;
            int32_t d = c * 3;
            int32_t e = d ^ (i * 7);
            int32_t f = e - outer;
            
            /* Force copies between computations */
            int32_t g = copy_and_add(f, a, b);
            int32_t h = copy_and_add(g, c, d);
            
            /* Use volatile to prevent elimination */
            VOLATILE_SINK(h);
            
            /* Mixed type computation */
            float fa = float_array[i];
            float fb = float_array[i + 128];
            float fc = float_copy_mul(fa, fb, (float)h);
            
            /* Narrow computation */
            int16_t sa = narrow_compute(g, h, (char)i);
            short_array[i] = sa;
            
            /* Promote and compute */
            double partial = promote_and_compute(fa, fc, sa);
            total_result += partial;
            
            /* Clobber registers to force spills/remats */
            if ((i & 15) == 0) {
                CLASH_REGISTERS;
            }
        }
        
        /* Pattern 2: Floating-point intensive with conversions */
        for (int j = 0; j < 64; j++) {
            float f1 = float_array[j * 2];
            float f2 = float_array[j * 2 + 1];
            float f3 = f1 * f2;
            float f4 = f3 + (float)int_array[j];
            
            /* Convert and back */
            double d1 = (double)f4;
            double d2 = d1 * 1.5;
            float f5 = (float)d2;
            
            /* Chain of dependent operations */
            for (int k = 0; k < 3; k++) {
                f5 = f5 * 0.9f + (float)k;
                FORCE_COPY(f5);
            }
            
            VOLATILE_SINK(f5);
            
            /* Integer side computation */
            int32_t ix = int_array[j] ^ (j * 11);
            int32_t iy = ix + outer;
            int32_t iz = iy * 7;
            
            /* Force another copy context */
            int32_t iw = copy_and_add(iz, ix, iy);
            float_array[j] = (float)iw * 0.01f;
            
            total_result += (double)f5 + (double)iw;
        }
        
        /* Pattern 3: Complex conditional with mixed types */
        for (int m = 0; m < 96; m++) {
            int32_t base = int_array[m];
            float fbase = float_array[m];
            
            /* Conditional computation creating different paths */
            if ((m & 3) == 0) {
                /* Path A: Integer heavy */
                int32_t t1 = base * 3;
                int32_t t2 = t1 + outer * 7;
                int32_t t3 = t2 ^ 0xABCD;
                int32_t t4 = copy_and_add(t3, t1, t2);
                
                /* Convert to float */
                float ft = (float)t4;
                float_array[m] = ft * 0.5f;
                
                total_result += (double)t4;
            } else if ((m & 3) == 1) {
                /* Path B: Float heavy */
                float ft1 = fbase * 2.0f;
                float ft2 = ft1 + (float)outer;
                float ft3 = float_copy_mul(ft2, fbase, ft1);
                
                /* Convert to int */
                int32_t it = (int32_t)ft3;
                int_array[m] = it & 0xFFF;
                
                total_result += (double)ft3;
            } else {
                /* Path C: Mixed with narrow */
                int16_t s1 = short_array[m];
                int32_t extended = (int32_t)s1 * 2;
                float converted = (float)extended * 0.25f;
                int16_t s2 = narrow_compute(extended, (int32_t)converted, (char)m);
                
                short_array[m] = s2;
                total_result += (double)s2;
            }
            
            /* Force register clobbering periodically */
            if ((m & 7) == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        /* Small loop with high register pressure */
        for (int n = 0; n < 32; n++) {
            /* Many live values */
            int32_t v1 = int_array[n];
            int32_t v2 = int_array[n + 32];
            int32_t v3 = int_array[n + 64];
            int32_t v4 = int_array[n + 96];
            
            float fv1 = float_array[n];
            float fv2 = float_array[n + 32];
            float fv3 = float_array[n + 64];
            float fv4 = float_array[n + 96];
            
            /* Chain of dependent operations */
            int32_t r1 = v1 + v2;
            int32_t r2 = r1 * v3;
            int32_t r3 = r2 ^ v4;
            int32_t r4 = copy_and_add(r3, r1, r2);
            
            float fr1 = fv1 * fv2;
            float fr2 = fr1 + fv3;
            float fr3 = float_copy_mul(fr2, fv4, fr1);
            
            /* Cross-type computation */
            double dr = promote_and_compute(fr3, (float)r4, r3);
            total_result += dr;
            
            /* Force all values to be live */
            VOLATILE_SINK(r4);
            VOLATILE_SINK(fr3);
            
            /* More computations to increase pressure */
            for (int p = 0; p < 2; p++) {
                r4 = r4 + p;
                fr3 = fr3 * (1.0f + p * 0.1f);
                FORCE_COPY(r4);
                FORCE_COPY(fr3);
            }
        }
    }
    
    /* Final volatile sink to prevent elimination of total_result */
    VOLATILE_SINK(total_result);
    
    return (int)(total_result * 0.001) & 0xFF;
}
