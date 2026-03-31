/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -fno-omit-frame-pointer -o test_remat test_remat.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile float volatile_sink_float;
static volatile double volatile_sink_double;

/* Inline functions to force copies and parameter passing */
static inline int32_t compute_int(int32_t a, int32_t b, int32_t c) {
    /* Force register copies with mixed operations */
    int32_t t1 = a + b;
    int32_t t2 = t1 * c;
    int32_t t3 = t2 - (a << 2);
    /* Use inline asm to prevent coalescing */
    asm volatile("" : "+r"(t3) : : "memory");
    return t3;
}

static inline float compute_float(float a, float b, int c) {
    /* Mixed precision calculations */
    float t1 = a * b;
    float t2 = t1 + (float)c;
    float t3 = t2 / (a + 1.0f);
    /* Force register move */
    asm volatile("" : "+r"(t3) : : "memory");
    return t3;
}

static inline double compute_double(double a, double b, float c) {
    /* More mixed precision */
    double t1 = a + (double)c;
    double t2 = t1 * b;
    double t3 = t2 - a;
    /* Clobber registers to force spills/remats */
    asm volatile("" : "+r"(t3) : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    return t3;
}

/* Helper with multiple parameters to increase register pressure */
static inline int mixed_computation(int a, short b, char c, float d, double e) {
    int t1 = a + (int)b;
    float t2 = d * (float)t1;
    double t3 = e + (double)t2;
    int t4 = (int)t3 + c;
    
    /* Multiple volatile writes to prevent optimizations */
    volatile_sink_int = t1;
    volatile_sink_float = t2;
    volatile_sink_double = t3;
    
    /* Inline asm that uses the value */
    asm volatile("" : : "r"(t4), "r"(a), "r"(b) : "memory");
    return t4;
}

/* Main computation kernel */
void compute_kernel(int32_t* int_data, float* float_data, double* double_data, 
                   int size, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop creates multiple basic blocks */
        int start = iter % 4;
        
        for (int i = start; i < size; i += 3) {
            /* First basic block: integer computations */
            int32_t a = int_data[i];
            int32_t b = int_data[(i + 1) % size];
            int32_t c = int_data[(i + 2) % size];
            
            /* Chain of dependent computations */
            int32_t r1 = compute_int(a, b, c);
            int32_t r2 = compute_int(r1, a, b);
            int32_t r3 = compute_int(r2, c, r1);
            
            /* Conditional to split control flow */
            if (r3 > 1000) {
                /* Second basic block: floating point */
                float f1 = float_data[i];
                float f2 = float_data[(i + 1) % size];
                
                float fr1 = compute_float(f1, f2, r3);
                float fr2 = compute_float(fr1, f1, r2);
                
                /* Mixed computation forcing copies */
                int mr1 = mixed_computation(r1, (short)r2, (char)r3, fr1, (double)fr2);
                
                /* Another conditional */
                if (mr1 < 500) {
                    /* Third basic block: double precision */
                    double d1 = double_data[i];
                    double d2 = double_data[(i + 3) % size];
                    
                    double dr1 = compute_double(d1, d2, fr2);
                    double dr2 = compute_double(dr1, d1, fr1);
                    
                    /* More register pressure */
                    int32_t final1 = compute_int(mr1, (int)dr1, (int)dr2);
                    float final2 = compute_float((float)dr1, (float)dr2, final1);
                    
                    /* Volatile sinks */
                    volatile_sink_int = final1;
                    volatile_sink_float = final2;
                    volatile_sink_double = dr2;
                } else {
                    /* Alternative path */
                    int32_t alt1 = compute_int(r3, mr1, a);
                    float alt2 = compute_float((float)alt1, (float)mr1, b);
                    
                    volatile_sink_int = alt1;
                    volatile_sink_float = alt2;
                }
            } else {
                /* Another alternative path */
                int32_t alt_r1 = compute_int(c, b, a);
                int32_t alt_r2 = compute_int(alt_r1, r1, r2);
                
                /* Force different mode registers */
                short s1 = (short)alt_r1;
                char c1 = (char)alt_r2;
                
                /* Mixed computation with different types */
                int mixed = mixed_computation(alt_r1, s1, c1, 
                                            (float)alt_r2, 
                                            (double)alt_r1);
                
                volatile_sink_int = mixed;
            }
            
            /* Small inner loop to increase pressure */
            for (int j = 0; j < 3; j++) {
                int temp = compute_int(r3 + j, i, iter);
                float ftemp = compute_float((float)temp, (float)j, iter);
                
                /* Inline asm that clobbers registers */
                asm volatile("" : : "r"(temp), "r"(ftemp) : 
                           "r0", "r1", "r2", "r3", "r4", "r5");
            }
        }
        
        /* Additional loop with different stride */
        for (int i = size - 1; i >= 0; i -= 2) {
            double d1 = double_data[i];
            double d2 = double_data[(i + 2) % size];
            
            double dr = compute_double(d1, d2, float_data[i]);
            int ir = compute_int((int)d1, (int)d2, (int)dr);
            
            /* More volatile and asm to prevent optimization */
            asm volatile("" : "+r"(ir) : : "memory");
            volatile_sink_double = dr;
            volatile_sink_int = ir;
        }
    }
}

/* Initialize arrays with patterns */
void init_arrays(int32_t* int_arr, float* float_arr, double* double_arr, int size) {
    for (int i = 0; i < size; i++) {
        int_arr[i] = (i * 37 + 123) % 1001;
        float_arr[i] = (float)(i * 19 + 456) / 100.0f;
        double_arr[i] = (double)(i * 53 + 789) / 50.0;
    }
}

int main() {
    const int SIZE = 128;
    const int ITERATIONS = 100;
    
    /* Allocate arrays */
    int32_t* int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with patterns */
    init_arrays(int_data, float_data, double_data, SIZE);
    
    /* Run computation kernel multiple times */
    for (int run = 0; run < 5; run++) {
        compute_kernel(int_data, float_data, double_data, SIZE, ITERATIONS);
    }
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
