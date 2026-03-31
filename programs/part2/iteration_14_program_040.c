/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_transform(int32_t a, int32_t b, int32_t c) {
    volatile int32_t sink;
    int32_t tmp = a + b;
    asm volatile("" : "+r"(tmp) : "r"(c));
    sink = tmp;
    return tmp * c;
}

static inline float float_copy(float a, float b, int32_t c) {
    volatile float fsink;
    float ftmp = a * b;
    asm volatile("" : "+f"(ftmp) : "r"(c));
    fsink = ftmp;
    return ftmp + (float)c;
}

static inline int16_t narrow_copy(int32_t a, int32_t b) {
    volatile int16_t sink;
    int16_t tmp = (int16_t)(a + b);
    asm volatile("" : "+r"(tmp));
    sink = tmp;
    return tmp;
}

static inline double promote_and_compute(float a, int32_t b) {
    volatile double dsink;
    double dtmp = (double)a + (double)b;
    asm volatile("" : "+f"(dtmp) : "r"(b));
    dsink = dtmp;
    return dtmp * 1.5;
}

/* Main computation kernel */
void compute_kernel(int32_t *int_data, float *float_data, 
                    int32_t rows, int32_t cols, int32_t iters) {
    volatile int32_t global_sink = 0;
    volatile float float_sink = 0.0f;
    volatile double double_sink = 0.0;
    
    for (int32_t iter = 0; iter < iters; ++iter) {
        /* Outer loop creates register pressure */
        int32_t outer_acc = iter;
        float outer_float = (float)iter * 0.1f;
        
        for (int32_t i = 0; i < rows; ++i) {
            /* Middle loop with mixed operations */
            int32_t row_acc = int_data[i];
            float row_float = float_data[i];
            
            for (int32_t j = 0; j < cols; ++j) {
                /* Innermost loop - maximum register pressure */
                
                /* Chain of dependent integer operations */
                int32_t a = i * j + iter;
                int32_t b = a + int_data[j % rows];
                int32_t c = b - outer_acc;
                int32_t d = copy_and_transform(a, b, c);
                
                /* Mixed-width operations */
                int16_t e = narrow_copy(c, d);
                int32_t f = (int32_t)e * d;
                
                /* Floating point chain */
                float g = row_float * (float)j;
                float h = float_copy(g, outer_float, f);
                
                /* Promotion to double */
                double k = promote_and_compute(h, f);
                
                /* More integer ops with different modes */
                int64_t l = (int64_t)f * (int64_t)d;
                int32_t m = (int32_t)(l >> 16);
                
                /* Force register clobbering */
                asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), 
                             "r"(f), "r"(m), "f"(g), "f"(h), "f"(k));
                
                /* Conditional block to split live ranges */
                if ((i ^ j) & 1) {
                    int32_t n = m + copy_and_transform(f, d, e);
                    float o = h * 2.0f;
                    asm volatile("" : "+r"(n), "+f"(o));
                    global_sink += n;
                    float_sink += o;
                } else {
                    double p = k * 0.5;
                    int16_t q = narrow_copy(m, f);
                    asm volatile("" : "+f"(p), "+r"(q));
                    double_sink += p;
                    global_sink += q;
                }
                
                /* Another dependent chain */
                int32_t r = a + (b >> 3);
                float s = (float)r * 0.25f;
                double t = (double)s + k;
                
                /* Force spills/reloads */
                for (int32_t kk = 0; kk < 2; ++kk) {
                    int32_t u = r + kk;
                    float v = s + (float)kk;
                    asm volatile("" : "+r"(u), "+f"(v));
                    global_sink += u;
                    float_sink += v;
                }
            }
            
            /* Loop-carried dependency */
            outer_acc += row_acc;
            outer_float += row_float;
        }
        
        /* Cross-iteration dependency */
        int_data[iter % rows] = outer_acc;
        float_data[iter % rows] = outer_float;
    }
    
    /* Final sink to prevent elimination */
    asm volatile("" : : "r"(global_sink), "f"(float_sink), "f"(double_sink));
}

/* Initialize with pattern to avoid constant propagation */
void init_data(int32_t *int_data, float *float_data, int32_t size) {
    for (int32_t i = 0; i < size; ++i) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        float_data[i] = (float)int_data[i] * 0.001f;
    }
}

int main(void) {
    const int32_t rows = 32;
    const int32_t cols = 64;
    const int32_t iters = 100;
    
    int32_t *int_data = (int32_t*)malloc(rows * sizeof(int32_t));
    float *float_data = (float*)malloc(rows * sizeof(float));
    
    if (!int_data || !float_data) return 1;
    
    init_data(int_data, float_data, rows);
    
    compute_kernel(int_data, float_data, rows, cols, iters);
    
    /* Use results to prevent dead code elimination */
    volatile int32_t result = 0;
    for (int32_t i = 0; i < rows; ++i) {
        result += int_data[i];
        result += (int32_t)float_data[i];
    }
    
    free(int_data);
    free(float_data);
    
    return result & 1;
}
