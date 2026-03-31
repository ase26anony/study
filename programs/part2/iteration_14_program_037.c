/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

#define FORCE_USE(x) asm volatile("" : : "r"(x))
#define VOLATILE_SINK(x) do { volatile int sink = (x); (void)sink; } while(0)

/* Helper functions to force copy propagation */
static inline int32_t copy_and_add(int32_t a, int32_t b, int32_t c) {
    volatile int32_t tmp1 = a + b;
    int32_t tmp2 = tmp1;
    FORCE_USE(tmp2);
    return tmp2 + c;
}

static inline float promote_and_multiply(int16_t s, uint8_t b) {
    float f1 = (float)s;
    float f2 = (float)b;
    volatile float vf = f1;
    return vf * f2;
}

static inline double mixed_calc(int32_t i, float f, char c) {
    double d1 = (double)i;
    double d2 = (double)f;
    double d3 = (double)c;
    FORCE_USE(d3);
    return d1 * d2 + d3;
}

/* Function to create complex control flow */
static int process_chunk(int32_t *arr_int, float *arr_float, 
                         uint8_t *arr_byte, int start, int end) {
    int result = 0;
    volatile int guard = start;
    
    for (int i = start; i < end; i++) {
        /* Multiple basic blocks created by conditionals */
        if (i & 1) {
            /* Block A: Integer operations with rematerializable values */
            int32_t base = arr_int[i] * 3;  /* Cheap to recompute */
            int32_t offset = i * 7;         /* Also cheap */
            
            /* Chain of dependent operations */
            int32_t val1 = base + offset;
            int32_t val2 = val1 >> 2;
            int32_t val3 = copy_and_add(val2, offset, base);
            
            /* Force register pressure with many live values */
            FORCE_USE(val1);
            FORCE_USE(val2);
            VOLATILE_SINK(val3);
            
            /* Mixed-type calculations */
            float f1 = promote_and_multiply((int16_t)val1, arr_byte[i]);
            double d1 = mixed_calc(val3, f1, (char)val2);
            
            result += (int)d1;
        } else {
            /* Block B: Different operations to create different modes */
            float base_f = arr_float[i] * 2.5f;
            volatile float vf = base_f;
            
            for (int j = 0; j < 3; j++) {
                /* Inner loop increases register pressure */
                float scaled = vf * (float)j;
                int32_t as_int = (int32_t)scaled;
                
                /* More copies and conversions */
                double d = (double)scaled + (double)as_int;
                int64_t big = (int64_t)d * 11LL;
                
                /* Inline asm that clobbers registers */
                asm volatile("" : "+r"(as_int) : : "xmm0", "xmm1");
                
                result += (int)big;
                VOLATILE_SINK(scaled);
            }
        }
        
        /* Another conditional block */
        if ((i % 7) == 0) {
            /* Force spilling/rematerialization with many temporaries */
            int32_t t1 = arr_int[i] * 2;
            int32_t t2 = t1 + i;
            int32_t t3 = t2 * 3;
            int32_t t4 = t3 - i;
            int32_t t5 = t4 / 2;
            int32_t t6 = t5 + arr_byte[i];
            int32_t t7 = t6 * 5;
            int32_t t8 = t7 - t1;
            
            /* All used to prevent elimination */
            FORCE_USE(t1); FORCE_USE(t2); FORCE_USE(t3);
            FORCE_USE(t4); FORCE_USE(t5); FORCE_USE(t6);
            FORCE_USE(t7); VOLATILE_SINK(t8);
            
            result += t8;
        }
    }
    
    return result;
}

/* Main driver with nested loops */
int main(void) {
    const int SIZE = 1024;
    const int CHUNK = 64;
    
    /* Arrays with different data patterns */
    int32_t *arr_int = malloc(SIZE * sizeof(int32_t));
    float *arr_float = malloc(SIZE * sizeof(float));
    uint8_t *arr_byte = malloc(SIZE * sizeof(uint8_t));
    
    /* Initialize with varied data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = (i * 37) & 0xFFF;
        arr_float[i] = (float)(i * 0.12345);
        arr_byte[i] = (uint8_t)(i * 19);
    }
    
    int total = 0;
    volatile int outer_counter = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < 8; outer++) {
        outer_counter++;
        
        /* Process in chunks to create more control flow edges */
        for (int chunk_start = 0; chunk_start < SIZE; chunk_start += CHUNK) {
            int chunk_end = chunk_start + CHUNK;
            if (chunk_end > SIZE) chunk_end = SIZE;
            
            /* Call processing function - creates new scope */
            int chunk_result = process_chunk(arr_int, arr_float, 
                                            arr_byte, chunk_start, chunk_end);
            
            /* More mixed-type operations */
            double scaled_result = (double)chunk_result * 1.5;
            int64_t big_result = (int64_t)scaled_result;
            
            /* Force copies between different representations */
            volatile int32_t as_int32 = (int32_t)big_result;
            float as_float = (float)as_int32;
            double as_double = (double)as_float;
            
            /* Inline asm that forces register moves */
            asm volatile("" : "+r"(as_int32), "+x"(as_double));
            
            total += (int)as_double + as_int32;
            VOLATILE_SINK(big_result);
        }
        
        /* Modify arrays slightly each outer iteration */
        for (int i = 0; i < SIZE; i += 17) {
            arr_int[i] += outer;
            arr_float[i] *= 1.01f;
            arr_byte[i] ^= 0x55;
        }
    }
    
    /* Final sink to prevent elimination */
    VOLATILE_SINK(total);
    
    free(arr_int);
    free(arr_float);
    free(arr_byte);
    
    return total & 0xFF;
}
