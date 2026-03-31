/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-tree-pre -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Helper functions to force copies and prevent optimization */
static inline int32_t copy_and_add(int32_t a, int32_t b, volatile int32_t* sink) {
    int32_t result = a + b;
    *sink = result;  /* Volatile write to prevent elimination */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

static inline float promote_and_multiply(int16_t s, float f, volatile float* sink) {
    float promoted = (float)s;
    float result = promoted * f;
    *sink = result;
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

static inline double mixed_calc(int32_t i, float f, double d, volatile double* sink) {
    double from_i = (double)i;
    double from_f = (double)f;
    double result = from_i * from_f + d;
    *sink = result;
    /* Force register clobbering */
    asm volatile("" : : "r"(result) : "%xmm0", "%xmm1", "%xmm2");
    return result;
}

/* Function with complex control flow to create many basic blocks */
static void process_data(int8_t* data1, int16_t* data2, float* data3, 
                         double* data4, int size, volatile int* global_sink) {
    int i, j, k;
    
    /* Outer loop - creates register pressure */
    for (i = 0; i < size; i++) {
        int32_t accum_int = 0;
        float accum_float = 0.0f;
        double accum_double = 0.0;
        
        /* First inner loop with integer operations */
        for (j = 0; j < 8; j++) {
            /* Create dependent chain of integer computations */
            int32_t temp1 = data1[i] * 3;
            int32_t temp2 = data2[(i + j) % size] & 0xFF;
            
            /* Force copy propagation context */
            int32_t sum1 = copy_and_add(temp1, temp2, global_sink);
            
            /* Conditional to create different basic blocks */
            if (sum1 > 128) {
                int32_t scaled = sum1 * 2;
                accum_int = copy_and_add(accum_int, scaled, global_sink);
                
                /* Mixed precision calculation */
                float f_temp = (float)scaled * 0.5f;
                accum_float = promote_and_multiply(data2[j], f_temp, (volatile float*)global_sink);
            } else {
                int32_t shifted = sum1 << 1;
                accum_int = copy_and_add(accum_int, shifted, global_sink);
                
                /* Different mixed precision path */
                float f_temp = (float)shifted * 0.25f;
                accum_float = promote_and_multiply(data2[(j + 1) % size], 
                                                  f_temp, (volatile float*)global_sink);
            }
        }
        
        /* Second inner loop with floating-point operations */
        for (k = 0; k < 4; k++) {
            /* Create floating-point computation chain */
            float base = data3[(i + k) % size];
            float scaled = base * (float)(k + 1);
            
            /* Force another copy context */
            float result_f = promote_and_multiply(data2[k], scaled, (volatile float*)global_sink);
            
            /* Mixed integer/float/double computation */
            accum_double = mixed_calc(accum_int, result_f, accum_double, 
                                     (volatile double*)global_sink);
            
            /* Complex conditional with multiple blocks */
            if (accum_double > 1000.0) {
                double reduced = accum_double * 0.9;
                accum_double = mixed_calc(data1[k], result_f, reduced, 
                                         (volatile double*)global_sink);
            } else if (accum_double < -1000.0) {
                double increased = accum_double * 1.1;
                accum_double = mixed_calc(data1[(k + 1) % size], result_f, increased, 
                                         (volatile double*)global_sink);
            } else {
                /* Another computation path */
                int32_t temp_int = data1[i] + data2[k];
                accum_double = mixed_calc(temp_int, result_f, accum_double, 
                                         (volatile double*)global_sink);
            }
        }
        
        /* Final store with volatile to prevent elimination */
        *global_sink = accum_int;
        *(volatile float*)global_sink = accum_float;
        *(volatile double*)global_sink = accum_double;
    }
}

/* Another function to create more register pressure */
static void alternate_path(uint64_t* data1, int32_t* data2, float* data3, 
                          int size, volatile int* sink) {
    int i, j;
    
    for (i = 0; i < size; i++) {
        uint64_t u_acc = 0;
        int32_t s_acc = 0;
        float f_acc = 0.0f;
        
        for (j = 0; j < 16; j++) {
            /* Mix of 64-bit and 32-bit operations */
            uint64_t temp64 = data1[(i + j) % size];
            int32_t temp32 = data2[(i * j) % size];
            
            /* Force conversions and copies */
            u_acc += (uint64_t)temp32 * temp64;
            s_acc += (int32_t)(temp64 & 0xFFFFFFFF);
            
            /* Floating point from mixed sources */
            float f1 = (float)(temp64 >> 32);
            float f2 = (float)temp32;
            f_acc = promote_and_multiply((int16_t)f1, f2, (volatile float*)sink);
            
            /* Inline asm to clobber registers */
            asm volatile("" : : "r"(u_acc), "r"(s_acc), "r"(f_acc) : 
                        "%rax", "%rbx", "%rcx", "%xmm0", "%xmm1");
        }
        
        /* Volatile sinks */
        *sink = (int)u_acc;
        *(volatile float*)sink = f_acc;
    }
}

int main(void) {
    const int SIZE = 256;
    
    /* Allocate and initialize arrays with different patterns */
    int8_t* data1 = (int8_t*)malloc(SIZE * sizeof(int8_t));
    int16_t* data2 = (int16_t*)malloc(SIZE * sizeof(int16_t));
    float* data3 = (float*)malloc(SIZE * sizeof(float));
    double* data4 = (double*)malloc(SIZE * sizeof(double));
    uint64_t* data5 = (uint64_t*)malloc(SIZE * sizeof(uint64_t));
    int32_t* data6 = (int32_t*)malloc(SIZE * sizeof(int32_t));
    
    volatile int global_sink = 0;
    
    /* Initialize with varied data patterns */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (i * 37) & 0xFF;
        data2[i] = (i * 73) & 0xFFFF;
        data3[i] = (float)(i * 0.12345);
        data4[i] = (double)(i * 0.6789);
        data5[i] = ((uint64_t)i << 32) | (i * 0xABCD);
        data6[i] = i * 0x1234;
    }
    
    /* Call processing functions multiple times to increase pressure */
    for (int iter = 0; iter < 10; iter++) {
        process_data(data1, data2, data3, data4, SIZE, &global_sink);
        alternate_path(data5, data6, data3, SIZE, &global_sink);
        
        /* Additional mixed computation in main */
        int32_t temp_acc = 0;
        for (int i = 0; i < 100; i++) {
            float f_temp = data3[i % SIZE];
            double d_temp = data4[i % SIZE];
            
            /* Complex expression with many temporaries */
            int32_t t1 = data1[i % SIZE] + i;
            int32_t t2 = data2[i % SIZE] * 2;
            int32_t t3 = copy_and_add(t1, t2, &global_sink);
            
            float t4 = promote_and_multiply((int16_t)t3, f_temp, (volatile float*)&global_sink);
            double t5 = mixed_calc(t3, t4, d_temp, (volatile double*)&global_sink);
            
            temp_acc += (int32_t)t5;
            
            /* Force spill/reload context */
            asm volatile("" : : "r"(temp_acc) : "%rax", "%rbx");
        }
        global_sink = temp_acc;
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(data5);
    free(data6);
    
    return global_sink & 0xFF;
}
