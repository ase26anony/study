/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -fno-schedule-insns */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_COPY(x) asm volatile("" : "+r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)
#define CLASH_REGISTERS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7")

/* Helper functions to force copy propagation contexts */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;
    int32_t t2 = t1 ^ c;
    FORCE_COPY(t2);
    return t2 * 3;
}

static inline float float_copy_mul(float a, float b, float c) {
    float t1 = a * b;
    float t2 = t1 + c;
    FORCE_COPY(t2);
    return t2 * 2.0f;
}

static inline int16_t narrow_compute(int32_t a, int32_t b) {
    int16_t s1 = (int16_t)(a & 0xFFFF);
    int16_t s2 = (int16_t)(b & 0xFFFF);
    int16_t result = s1 - s2;
    FORCE_COPY(result);
    return result;
}

static inline double promote_and_compute(float f1, float f2, int32_t i) {
    double d1 = (double)f1;
    double d2 = (double)f2;
    double d3 = d1 * d2 + (double)i;
    FORCE_COPY(d3);
    return d3 * 0.5;
}

/* Main computation kernel */
void compute_kernel(int32_t* int_data, float* float_data, 
                    int16_t* short_data, double* double_data,
                    int size, int iterations) {
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop creates register pressure */
        int32_t outer_acc = iter;
        float outer_facc = (float)iter * 0.1f;
        
        for (int i = 0; i < size - 4; i++) {
            /* Multiple dependent chains with different types */
            
            /* Integer chain with rematerializable constants */
            int32_t a = int_data[i] + 17;  /* Constant 17 is rematerializable */
            int32_t b = a * 3;
            int32_t c = b - int_data[i + 1];
            int32_t d = c ^ 0x55AA55AA;
            
            FORCE_COPY(d);
            
            /* Force copy propagation through function call */
            int32_t e = copy_and_add(d, int_data[i + 2], outer_acc);
            
            /* Floating-point chain */
            float f = float_data[i] * 2.5f;  /* Constant is rematerializable */
            float g = f + float_data[i + 1];
            float h = float_copy_mul(g, outer_facc, float_data[i + 2]);
            
            /* Mixed-type computations */
            int32_t i_val = (int32_t)h;
            int16_t s = narrow_compute(e, i_val);
            short_data[i] = s;
            
            /* Another chain with type promotions */
            double dbl = promote_and_compute(f, g, e);
            double_data[i] = dbl;
            
            /* Conditional block to split control flow */
            if (i % 3 == 0) {
                /* Different computation in this path */
                int32_t t1 = e * 2;
                float t2 = h * 3.0f;
                double t3 = dbl * 1.5;
                
                /* Use volatile to prevent optimization */
                VOLATILE_SINK(t1);
                VOLATILE_SINK(t2);
                VOLATILE_SINK(t3);
                
                /* Force register clobbering */
                CLASH_REGISTERS;
            } else if (i % 3 == 1) {
                /* Another path with different operations */
                int64_t big_val = (int64_t)e * (int64_t)d;
                float float_val = (float)big_val * 0.01f;
                
                FORCE_COPY(big_val);
                FORCE_COPY(float_val);
            }
            
            /* Update accumulators with rematerializable values */
            outer_acc += 5;  /* Constant 5 is rematerializable */
            outer_facc += 0.25f;  /* Constant is rematerializable */
            
            /* More register pressure with temporary values */
            int32_t tmp1 = outer_acc * 2;
            int32_t tmp2 = tmp1 + i;
            float tmp3 = outer_facc * (float)i;
            float tmp4 = tmp3 * 3.14159f;
            
            VOLATILE_SINK(tmp2);
            VOLATILE_SINK(tmp4);
        }
        
        /* Inner loop with different access pattern */
        for (int j = size - 1; j > 0; j -= 2) {
            int32_t x = int_data[j];
            int32_t y = int_data[j - 1];
            float fx = float_data[j];
            float fy = float_data[j - 1];
            
            /* Cross-type computations */
            int32_t cross1 = (int32_t)(fx * 100.0f) + x;
            float cross2 = (float)y * 0.01f + fy;
            
            /* Force copies between different scopes */
            {
                int32_t local_copy = cross1;
                float local_fcopy = cross2;
                FORCE_COPY(local_copy);
                FORCE_COPY(local_fcopy);
                
                int_data[j] = local_copy ^ 0xFF;
                float_data[j] = local_fcopy * 0.5f;
            }
        }
    }
}

/* Initialize with different patterns to avoid constant propagation */
void init_data(int32_t* int_data, float* float_data, 
               int16_t* short_data, double* double_data, int size) {
    for (int i = 0; i < size; i++) {
        int_data[i] = (i * 37) & 0xFFF;
        float_data[i] = (float)((i * 19) % 100) * 0.1f;
        short_data[i] = (int16_t)(i * 3);
        double_data[i] = (double)(i % 50) * 0.01;
    }
}

int main() {
    const int SIZE = 256;
    const int ITERATIONS = 1000;
    
    /* Allocate aligned to avoid unnecessary instructions */
    int32_t* int_data = __builtin_aligned_alloc(16, SIZE * sizeof(int32_t));
    float* float_data = __builtin_aligned_alloc(16, SIZE * sizeof(float));
    int16_t* short_data = __builtin_aligned_alloc(16, SIZE * sizeof(int16_t));
    double* double_data = __builtin_aligned_alloc(16, SIZE * sizeof(double));
    
    if (!int_data || !float_data || !short_data || !double_data) {
        return 1;
    }
    
    init_data(int_data, float_data, short_data, double_data, SIZE);
    
    /* Multiple calls with different parameters to prevent inlining heuristics */
    for (int run = 0; run < 10; run++) {
        compute_kernel(int_data, float_data, short_data, double_data, 
                      SIZE, ITERATIONS / 10);
        
        /* Modify data slightly between runs */
        for (int i = 0; i < SIZE; i++) {
            int_data[i] += run;
            float_data[i] += (float)run * 0.1f;
        }
    }
    
    /* Final volatile sink to prevent elimination of entire computation */
    volatile int32_t final_check = int_data[0] + short_data[0];
    VOLATILE_SINK(final_check);
    
    free(int_data);
    free(float_data);
    free(short_data);
    free(double_data);
    
    return 0;
}
