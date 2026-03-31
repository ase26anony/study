/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Force register pressure with volatile operations */
static volatile int volatile_sink;

/* Helper to prevent optimization */
static inline void use_value(volatile int x) {
    volatile_sink = x;
}

/* Force copies between different types */
static inline int32_t promote_and_add(int16_t a, int8_t b) {
    return (int32_t)a + (int32_t)b;
}

/* Another helper that forces register copies */
static inline float int_to_float_and_add(int a, float b) {
    float temp = (float)a;
    asm volatile("" : "+r"(a), "+f"(temp) : : "memory");
    return temp + b;
}

/* Function with mixed operations to create virtual registers */
static inline double complex_calc(int idx, float f, char c, short s) {
    /* Multiple dependent computations */
    int t1 = idx * 3;
    int t2 = t1 + (int)c;
    float t3 = (float)t2 * f;
    double t4 = (double)t3 + (double)s;
    
    /* Force register copies with inline asm */
    asm volatile("" : "+r"(t1), "+r"(t2) : : "memory");
    
    return t4 * 2.0;
}

/* Main computation kernel */
void compute_kernel(int* int_arr, float* float_arr, 
                    double* double_arr, int size) {
    int i, j, k;
    
    /* Outer loop - creates control flow complexity */
    for (i = 0; i < size; i += 4) {
        /* Multiple local variables to increase register pressure */
        int local_int[8];
        float local_float[4];
        double local_double[2];
        
        /* Initialize with array values */
        for (j = 0; j < 8; j++) {
            local_int[j] = int_arr[(i + j) % size];
        }
        
        /* First inner loop - integer operations */
        for (j = 0; j < 100; j++) {
            /* Chain of dependent computations */
            int a = local_int[0] + j;
            int b = a * 3;
            short c = (short)(b & 0xFFFF);
            char d = (char)(c + j);
            
            /* Force copy propagation context */
            int e = promote_and_add(c, d);
            float f = int_to_float_and_add(e, float_arr[j % size]);
            
            /* More computations with different types */
            double g = complex_calc(e, f, d, c);
            
            /* Store results back to local arrays */
            local_float[j % 4] = f;
            local_double[j % 2] = g;
            
            /* Use volatile to prevent elimination */
            use_value(e);
            
            /* Inline asm to clobber registers */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "f"(f), "f"(g) : 
                        "xmm0", "xmm1", "xmm2", "rax", "rbx");
        }
        
        /* Second inner loop - floating point operations */
        for (j = 0; j < 50; j++) {
            /* Mixed precision calculations */
            float f1 = local_float[0] + (float)j;
            float f2 = f1 * 1.5f;
            double d1 = (double)f2 + local_double[0];
            double d2 = d1 * 3.14159;
            
            /* Integer conversion and back */
            int i1 = (int)d2;
            int i2 = i1 ^ local_int[j % 8];
            float f3 = (float)i2 + f2;
            
            /* Force register pressure with many live values */
            asm volatile("" : "+f"(f1), "+f"(f2), "+f"(f3), 
                        "+r"(i1), "+r"(i2) : : "memory");
            
            /* Store to volatile to prevent optimization */
            volatile_sink = i1 + i2;
            
            /* More computations to increase register pressure */
            for (k = 0; k < 3; k++) {
                double temp = d2 + (double)k;
                float ftemp = f3 * (float)temp;
                int itemp = (int)ftemp;
                
                /* Force copies between virtual registers */
                itemp = promote_and_add((short)itemp, (char)k);
                ftemp = int_to_float_and_add(itemp, ftemp);
                
                /* Use results */
                double_arr[(i + k) % size] = temp + (double)ftemp;
            }
        }
        
        /* Conditional block to create control flow edges */
        if (i % 8 == 0) {
            /* Different computation pattern */
            for (j = 0; j < 4; j++) {
                int x = local_int[j] * 2;
                float y = (float)x / 3.0f;
                double z = complex_calc(x, y, (char)j, (short)x);
                
                /* Force register clobbering */
                asm volatile("" : : "r"(x), "f"(y), "f"(z) : 
                            "xmm3", "xmm4", "rcx");
                
                float_arr[(i + j) % size] = y;
                double_arr[(i + j) % size] = z;
            }
        } else {
            /* Alternative path */
            for (j = 0; j < 4; j++) {
                double base = double_arr[(i + j) % size];
                float derived = (float)base * 2.0f;
                int converted = (int)derived;
                
                /* Chain of copies and computations */
                int result = promote_and_add((short)converted, (char)j);
                result = result * 3 - 1;
                
                /* Force virtual register creation */
                asm volatile("" : "+r"(result) : : "memory");
                
                int_arr[(i + j) % size] = result;
            }
        }
    }
}

/* Initialize arrays with patterns */
void init_arrays(int* int_arr, float* float_arr, 
                 double* double_arr, int size) {
    for (int i = 0; i < size; i++) {
        int_arr[i] = (i * 37) % 101;
        float_arr[i] = (float)(i * 19) / 7.0f;
        double_arr[i] = (double)(i * 23) / 11.0;
    }
}

int main() {
    const int SIZE = 256;
    
    /* Allocate arrays with different alignments */
    int* int_arr = (int*)aligned_alloc(64, SIZE * sizeof(int));
    float* float_arr = (float*)aligned_alloc(64, SIZE * sizeof(float));
    double* double_arr = (double*)aligned_alloc(64, SIZE * sizeof(double));
    
    /* Initialize with pattern */
    init_arrays(int_arr, float_arr, double_arr, SIZE);
    
    /* Run computation multiple times to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        compute_kernel(int_arr, float_arr, double_arr, SIZE);
        
        /* Modify arrays slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            int_arr[i] += iter;
            float_arr[i] += (float)iter * 0.1f;
        }
    }
    
    /* Final volatile use to prevent dead code elimination */
    use_value(int_arr[0] + (int)float_arr[0]);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
