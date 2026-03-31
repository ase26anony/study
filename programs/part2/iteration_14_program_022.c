/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand */

#include <stdint.h>
#include <stdlib.h>

/* Force register pressure with mixed types and operations */
#define FORCE_COPY(x) asm volatile("" : : "r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)

/* Helper functions to generate copy operations */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    int32_t t1 = a + b;      /* Creates virtual register */
    int32_t t2 = t1 ^ c;     /* Another virtual register */
    FORCE_COPY(t2);          /* Prevent optimization */
    return t2;
}

static inline double promote_and_multiply(float f, int16_t s, int32_t i) {
    double d1 = (double)f;   /* Conversion creates virtual register */
    double d2 = d1 * (double)s;
    double d3 = d2 + (double)i;
    FORCE_COPY(d3);          /* Force register copy */
    return d3;
}

static inline int64_t mixed_ops(uint8_t b, int16_t s, int32_t i, float f) {
    /* Multiple conversions and operations */
    int32_t t1 = (int32_t)b * (int32_t)s;
    float t2 = (float)t1 + f;
    int64_t t3 = (int64_t)t2 * (int64_t)i;
    FORCE_COPY(t3);
    return t3;
}

/* Complex loop structure to create control flow */
void process_data(int8_t* data1, int16_t* data2, int32_t* data3, 
                  float* data4, double* data5, size_t size) {
    volatile int guard = 0;  /* Prevent loop unrolling */
    
    for (size_t outer = 0; outer < 3; ++outer) {
        /* Outer loop creates register pressure across iterations */
        int32_t accum_int = 0;
        double accum_float = 0.0;
        
        for (size_t i = 0; i < size; ++i) {
            /* Multiple basic blocks with conditionals */
            if (i % 7 == 0) {
                /* Block A: Integer-heavy operations */
                int32_t val1 = data1[i] * 3;
                int32_t val2 = copy_and_add(val1, data2[i], data3[i]);
                
                /* Force rematerialization opportunity */
                for (int j = 0; j < 2; ++j) {
                    int32_t temp = val2 + j * 17;
                    accum_int ^= temp;
                    VOLATILE_SINK(temp);  /* Prevent elimination */
                }
            } else if (i % 5 == 0) {
                /* Block B: Floating-point operations */
                float f1 = data4[i] * 2.5f;
                double d1 = promote_and_multiply(f1, data2[i], data3[i]);
                
                /* Chain of dependent FP operations */
                for (int k = 0; k < 3; ++k) {
                    double temp = d1 * (1.0 + k * 0.1);
                    accum_float += temp;
                    FORCE_COPY(temp);  /* Force register copies */
                }
            } else {
                /* Block C: Mixed operations */
                int64_t val = mixed_ops(data1[i], data2[i], 
                                       data3[i], data4[i]);
                
                /* Complex expression with multiple uses */
                double temp_d = (double)val / (1.0 + i);
                float temp_f = (float)temp_d;
                int32_t temp_i = (int32_t)temp_f;
                
                /* All results used in different ways */
                accum_int += temp_i;
                accum_float += temp_d;
                data5[i] = temp_d;
                
                /* Inline asm to clobber registers */
                asm volatile("" : : "r"(temp_i), "r"(temp_f), "r"(temp_d));
            }
            
            /* Cross-block value that might need rematerialization */
            int32_t cross_val = accum_int % 256;
            if (cross_val > 128) {
                /* Another use of the value in different mode */
                float f_cross = (float)cross_val;
                accum_float += f_cross;
                FORCE_COPY(f_cross);
            }
            
            /* Periodic register pressure spike */
            if (i % 13 == 0) {
                /* Create many short-lived values */
                int32_t t1 = data1[i] * 2;
                int32_t t2 = t1 + data2[i];
                int32_t t3 = t2 ^ data3[i];
                float t4 = (float)t3 + data4[i];
                double t5 = (double)t4 * 1.5;
                
                /* Use all values to prevent elimination */
                accum_int += t3;
                accum_float += t5;
                VOLATILE_SINK(t1);
                VOLATILE_SINK(t2);
                VOLATILE_SINK(t4);
                
                /* Force register shuffling */
                asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
            }
        }
        
        /* Use accumulators to prevent dead code elimination */
        VOLATILE_SINK(accum_int);
        VOLATILE_SINK(accum_float);
        
        /* Modify data to create loop-carried dependencies */
        for (size_t i = 0; i < size; ++i) {
            data1[i] = (data1[i] + accum_int) & 0xFF;
            data4[i] = data4[i] * 0.9f + (float)(accum_int % 100) * 0.01f;
        }
    }
}

/* Initialize with varied patterns */
void init_data(int8_t* d1, int16_t* d2, int32_t* d3, 
               float* d4, double* d5, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        d1[i] = (i * 13) & 0xFF;
        d2[i] = (i * 17) & 0x7FFF;
        d3[i] = i * 23;
        d4[i] = (float)i * 0.123f;
        d5[i] = (double)i * 0.456;
    }
}

int main() {
    const size_t SIZE = 256;
    
    /* Allocate with different alignments */
    int8_t* data1 = (int8_t*)aligned_alloc(16, SIZE * sizeof(int8_t));
    int16_t* data2 = (int16_t*)aligned_alloc(16, SIZE * sizeof(int16_t));
    int32_t* data3 = (int32_t*)aligned_alloc(16, SIZE * sizeof(int32_t));
    float* data4 = (float*)aligned_alloc(16, SIZE * sizeof(float));
    double* data5 = (double*)aligned_alloc(16, SIZE * sizeof(double));
    
    init_data(data1, data2, data3, data4, data5, SIZE);
    
    /* Multiple passes with different data patterns */
    for (int pass = 0; pass < 5; ++pass) {
        process_data(data1, data2, data3, data4, data5, SIZE);
        
        /* Modify pattern slightly each pass */
        for (size_t i = 0; i < SIZE; ++i) {
            data1[i] = (data1[i] + pass) & 0xFF;
            data3[i] ^= pass * 0x1234;
        }
    }
    
    /* Final sink to prevent elimination */
    volatile double final_sink = 0.0;
    for (size_t i = 0; i < SIZE; ++i) {
        final_sink += data5[i];
    }
    VOLATILE_SINK(final_sink);
    
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(data5);
    
    return 0;
}
