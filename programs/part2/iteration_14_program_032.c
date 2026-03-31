/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Force register pressure with volatile operations */
#define FORCE_COPY(var) asm volatile("" : : "r"(var))
#define VOLATILE_SINK(var) do { volatile int sink = (var); (void)sink; } while(0)

/* Helper functions to create copy contexts */
static inline int32_t copy_and_transform(int32_t a, int32_t b) {
    /* Creates copy propagation opportunities */
    int32_t tmp = a + b;
    FORCE_COPY(tmp);
    return tmp * 2;
}

static inline float promote_and_compute(int16_t a, int8_t b) {
    /* Mixed-type operations create different register modes */
    float fa = (float)a;
    float fb = (float)b;
    FORCE_COPY(fa);
    FORCE_COPY(fb);
    return fa * 0.5f + fb * 1.5f;
}

static inline double compute_pressure(int64_t x, int32_t y, float z) {
    /* Multiple parameter types force register moves */
    double dx = (double)x;
    double dy = (double)y;
    double dz = (double)z;
    
    /* Inline asm to prevent optimization */
    asm volatile("# Pressure point %0 %1 %2" : : "r"(x), "r"(y), "r"(z));
    
    return dx * 0.25 + dy * 0.75 + dz;
}

/* Main kernel designed to trigger early rematerialization */
void remat_kernel(int32_t *arr_int, int16_t *arr_short, 
                  float *arr_float, double *arr_double, int n) {
    volatile int trigger = 0;
    
    for (int outer = 0; outer < 3; ++outer) {
        /* Outer loop creates multiple basic blocks */
        int32_t base = arr_int[outer % n];
        FORCE_COPY(base);
        
        for (int i = 0; i < n; ++i) {
            /* First inner loop - integer operations */
            int32_t val1 = arr_int[i];
            int16_t val2 = arr_short[i % n];
            
            /* Chain of dependent computations */
            int32_t tmp1 = val1 * 2 + 7;
            int32_t tmp2 = tmp1 - val2;
            int32_t tmp3 = copy_and_transform(tmp2, base);
            
            /* Force register pressure with volatile */
            VOLATILE_SINK(tmp3);
            
            /* Conditional block creates control flow complexity */
            if (tmp3 > 1000) {
                float fval = arr_float[i % n];
                double dval = arr_double[i % n];
                
                /* Mixed precision calculations */
                float ftmp = promote_and_compute(val2, (int8_t)tmp3);
                double dtmp = compute_pressure(tmp3, val1, fval);
                
                /* More register pressure */
                arr_float[i % n] = ftmp * 0.7f;
                arr_double[i % n] = dtmp + dval;
                
                /* Inline asm clobbers registers */
                asm volatile("# Clobber point" : : : "memory", "r0", "r1", "r2", "r3");
            }
            
            /* Second computation path */
            for (int j = 0; j < 4; ++j) {
                /* Nested loop increases pressure */
                int32_t loop_tmp = tmp3 + j * 17;
                float float_tmp = (float)loop_tmp * 0.123f;
                
                /* More copy propagation opportunities */
                int32_t copied = loop_tmp;
                FORCE_COPY(copied);
                
                float promoted = float_tmp;
                FORCE_COPY(promoted);
                
                /* Complex expression with many temporaries */
                double result = (double)copied * 0.456 + 
                               (double)promoted * 0.789 +
                               (double)(outer * i * j);
                
                VOLATILE_SINK(result);
                
                /* Trigger volatile write */
                trigger = result > 0.5 ? 1 : 0;
            }
        }
        
        /* Another inner loop with different operations */
        for (int i = n - 1; i >= 0; --i) {
            int64_t big_val = (int64_t)arr_int[i] * 1000000LL;
            float small_val = arr_float[i % n];
            
            /* More mixed-mode operations */
            double mixed = compute_pressure(big_val, arr_int[i], small_val);
            int32_t truncated = (int32_t)mixed;
            
            /* Force copies between different register types */
            int32_t copy1 = truncated;
            int32_t copy2 = copy1;
            int32_t copy3 = copy_and_transform(copy2, base);
            
            arr_int[i] = copy3 % 1000;
            
            /* More inline asm for register pressure */
            asm volatile("# Second clobber" : : "r"(mixed), "r"(truncated) : "r4", "r5");
        }
    }
}

/* Initialize with varied patterns */
void init_arrays(int32_t *arr_int, int16_t *arr_short, 
                 float *arr_float, double *arr_double, int n) {
    for (int i = 0; i < n; ++i) {
        arr_int[i] = (i * 37 + 123) % 1000;
        arr_short[i] = (i * 53 + 456) % 30000;
        arr_float[i] = (float)(i * 71 + 789) / 100.0f;
        arr_double[i] = (double)(i * 97 + 321) / 50.0;
    }
}

int main() {
    const int N = 256;
    
    /* Allocate arrays with different types */
    int32_t *arr_int = (int32_t*)malloc(N * sizeof(int32_t));
    int16_t *arr_short = (int16_t*)malloc(N * sizeof(int16_t));
    float *arr_float = (float*)malloc(N * sizeof(float));
    double *arr_double = (double*)malloc(N * sizeof(double));
    
    if (!arr_int || !arr_short || !arr_float || !arr_double) {
        return 1;
    }
    
    /* Initialize with patterns */
    init_arrays(arr_int, arr_short, arr_float, arr_double, N);
    
    /* Run kernel multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; ++iter) {
        remat_kernel(arr_int, arr_short, arr_float, arr_double, N);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < N; ++i) {
            arr_int[i] = (arr_int[i] * 3 + 7) % 1000;
        }
    }
    
    /* Consume results to prevent dead code elimination */
    volatile int sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += arr_int[i] + (int)arr_short[i];
    }
    
    free(arr_int);
    free(arr_short);
    free(arr_float);
    free(arr_double);
    
    return sum > 0 ? 0 : 1;
}
