/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -o test_remat test_remat.c */

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

static inline int16_t narrow_copy(int32_t a, int32_t b) {
    int16_t t1 = (int16_t)(a & 0xFFFF);
    int16_t t2 = (int16_t)(b & 0xFFFF);
    FORCE_COPY(t1);
    FORCE_COPY(t2);
    return (int16_t)(t1 + t2);
}

static inline double promote_and_compute(int32_t a, float b) {
    double da = (double)a;
    double db = (double)b;
    FORCE_COPY(da);
    FORCE_COPY(db);
    return da * db + 1.5;
}

/* Complex kernel with mixed operations */
static void compute_kernel(int32_t *int_data, float *float_data, 
                          double *double_data, int size) {
    volatile int outer_counter = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create register pressure with many short-lived values */
        int32_t base = int_data[i];
        float fbase = float_data[i];
        
        /* Inner loop with dependent chain */
        for (int j = 0; j < 8; j++) {
            /* Integer chain with recomputable values */
            int32_t v1 = base + j * 17;
            int32_t v2 = v1 ^ 0x55AA55AA;
            int32_t v3 = v2 * 3 + 1;
            int32_t v4 = copy_and_add(v1, v2, v3);
            
            /* Floating chain */
            float f1 = fbase * (j + 1);
            float f2 = f1 + 3.14159f;
            float f3 = float_copy_mul(f1, f2, fbase);
            
            /* Mixed precision */
            int16_t narrow = narrow_copy(v3, v4);
            double dbl = promote_and_compute(v4, f3);
            
            /* Force register clobbering */
            CLASH_REGISTERS;
            
            /* Consume results to prevent elimination */
            VOLATILE_SINK(v4);
            VOLATILE_SINK(f3);
            VOLATILE_SINK(narrow);
            VOLATILE_SINK(dbl);
            
            /* Conditional block to create control flow complexity */
            if (j & 1) {
                /* Different computation path */
                int32_t alt1 = v1 * 7 - 13;
                float alt2 = f1 / 2.0f + 1.0f;
                double alt3 = (double)alt1 * (double)alt2;
                VOLATILE_SINK(alt1);
                VOLATILE_SINK(alt2);
                VOLATILE_SINK(alt3);
                
                /* More copies */
                int32_t copy1 = alt1;
                float copy2 = alt2;
                FORCE_COPY(copy1);
                FORCE_COPY(copy2);
                VOLATILE_SINK(copy1);
                VOLATILE_SINK(copy2);
            }
            
            /* Another nested conditional */
            if ((i ^ j) & 2) {
                double_data[i] += dbl;
                CLASH_REGISTERS;
            }
        }
        
        outer_counter++;
        if (outer_counter > 100) {
            CLASH_REGISTERS;
            outer_counter = 0;
        }
    }
}

/* Second kernel with different patterns */
static void kernel_with_switches(int32_t *data, int size) {
    for (int i = 0; i < size; i++) {
        int32_t acc = data[i];
        
        /* Switch-like structure with multiple blocks */
        for (int k = 0; k < 4; k++) {
            int32_t temp;
            
            switch (k) {
                case 0:
                    temp = acc * 2 + 1;
                    FORCE_COPY(temp);
                    acc = temp ^ 0xFF;
                    break;
                case 1:
                    temp = acc + 0x1000;
                    FORCE_COPY(temp);
                    acc = temp >> 4;
                    break;
                case 2:
                    temp = acc - 777;
                    FORCE_COPY(temp);
                    acc = temp * 3;
                    break;
                case 3:
                    temp = acc & 0x7F;
                    FORCE_COPY(temp);
                    acc = temp | 0x80;
                    break;
            }
            
            /* Force spill/reload context */
            asm volatile("" : "+r"(acc) : : "memory");
            
            /* Create floating point copies in integer loop */
            if (k & 1) {
                float fcopy = (float)acc;
                double dcopy = (double)fcopy;
                FORCE_COPY(fcopy);
                FORCE_COPY(dcopy);
                VOLATILE_SINK(fcopy);
                VOLATILE_SINK(dcopy);
            }
        }
        
        data[i] = acc;
    }
}

int main(void) {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with different patterns */
    int32_t *int_data = (int32_t*)malloc(SIZE * sizeof(int32_t));
    float *float_data = (float*)malloc(SIZE * sizeof(float));
    double *double_data = (double*)malloc(SIZE * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        return 1;
    }
    
    /* Initialize with patterns that create varied computations */
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = i * 3 - 17;
        float_data[i] = (float)i * 0.5f + 1.0f;
        double_data[i] = (double)i * 0.25;
    }
    
    /* Run multiple kernels to increase optimization opportunities */
    for (int iter = 0; iter < 3; iter++) {
        compute_kernel(int_data, float_data, double_data, SIZE);
        kernel_with_switches(int_data, SIZE);
        
        /* Additional mixed computation between kernels */
        for (int i = 0; i < 100; i++) {
            int idx = i % SIZE;
            int32_t ival = int_data[idx];
            float fval = float_data[idx];
            
            /* Create cross-type copies */
            double d1 = (double)ival;
            double d2 = (double)fval;
            FORCE_COPY(d1);
            FORCE_COPY(d2);
            
            int32_t converted = (int32_t)d1;
            float reconverted = (float)converted;
            FORCE_COPY(converted);
            FORCE_COPY(reconverted);
            
            VOLATILE_SINK(d1);
            VOLATILE_SINK(d2);
            VOLATILE_SINK(converted);
            VOLATILE_SINK(reconverted);
        }
    }
    
    /* Final sink to prevent elimination of entire computation */
    volatile int32_t final_sink = 0;
    for (int i = 0; i < SIZE; i += 64) {
        final_sink += int_data[i];
        final_sink ^= (int32_t)float_data[i];
    }
    VOLATILE_SINK(final_sink);
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
