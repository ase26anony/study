/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent optimization */
static volatile int volatile_sink_int;
static volatile float volatile_sink_float;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int copy_and_add_int(int a, int b, int c) {
    int temp = a + b;
    asm volatile("" : "+r"(temp) : : "memory");
    return temp + c;
}

static inline float copy_and_mul_float(float a, float b, float c) {
    float temp = a * b;
    asm volatile("" : "+f"(temp) : : "memory");
    return temp * c;
}

static inline double promote_and_div(int a, float b) {
    double temp = (double)a / (double)b;
    asm volatile("" : "+f"(temp) : : "memory");
    return temp;
}

/* Function with mixed operations to create register pressure */
static void process_block(int start, int end, 
                         const int* restrict arr_int,
                         const float* restrict arr_float,
                         double* restrict result) {
    /* Local variables with different scopes and lifetimes */
    int local_int1, local_int2, local_int3;
    float local_float1, local_float2;
    double local_double1, local_double2;
    
    /* Loop with multiple dependent computations */
    for (int i = start; i < end; i++) {
        /* First chain: integer operations with type mixing */
        local_int1 = arr_int[i] * 3;
        volatile_sink_int = local_int1;  /* Force spill/reload */
        
        local_int2 = copy_and_add_int(local_int1, i, 17);
        asm volatile("" : : "r"(local_int2) : "memory");
        
        /* Promote to different type */
        local_float1 = (float)local_int2 * 2.5f;
        volatile_sink_float = local_float1;
        
        /* Second chain: floating point with demotion */
        local_float2 = arr_float[i] + 1.0f;
        local_int3 = (int)local_float2;
        
        /* Force copy propagation context */
        int copied_int = local_int3;
        for (int j = 0; j < 3; j++) {
            copied_int = copy_and_add_int(copied_int, j, 1);
            asm volatile("" : : "r"(copied_int) : "r0", "r1", "r2", "r3");
        }
        
        /* Mixed precision calculation */
        local_double1 = promote_and_div(local_int1, local_float2);
        
        /* Third chain: double precision with integer mixing */
        local_double2 = local_double1 * (double)(i % 8 + 1);
        volatile_sink_double = local_double2;
        
        /* Final result with type conversion */
        result[i] = local_double2 + (double)copied_int;
        
        /* Conditional block to split control flow */
        if (i & 1) {
            /* Different register usage in this path */
            float temp = local_float1 * local_float2;
            asm volatile("" : : "f"(temp) : "memory");
            result[i] += (double)temp;
        } else {
            int temp = local_int1 | local_int3;
            asm volatile("" : : "r"(temp) : "memory");
            result[i] -= (double)temp;
        }
    }
}

/* Main driver with nested loops */
int main(void) {
    const int SIZE = 1024;
    const int BLOCK = 64;
    
    /* Initialize with different patterns */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    float* arr_float = (float*)malloc(SIZE * sizeof(float));
    double* result = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr_int || !arr_float || !result) {
        return 1;
    }
    
    /* Fill arrays with varying data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = (i * 37) & 0xFF;
        arr_float[i] = (float)((i * 19) % 100) * 0.1f;
    }
    
    /* Outer loop creating pressure */
    for (int outer = 0; outer < 10; outer++) {
        volatile_sink_int = outer;
        
        /* Process in blocks to create different register contexts */
        for (int block = 0; block < SIZE; block += BLOCK) {
            int end = block + BLOCK;
            if (end > SIZE) end = SIZE;
            
            /* Multiple calls with overlapping ranges */
            process_block(block, end - 4, arr_int, arr_float, result);
            process_block(end - 4, end, arr_int, arr_float, result);
            
            /* Small computation between calls */
            int temp_sum = 0;
            for (int k = 0; k < 8; k++) {
                temp_sum += arr_int[block + (k % BLOCK)];
                asm volatile("" : : "r"(temp_sum) : "cc");
            }
            volatile_sink_int = temp_sum;
        }
        
        /* Modify arrays slightly each outer iteration */
        for (int i = 0; i < SIZE; i += 16) {
            arr_int[i] += outer;
            arr_float[i] += (float)outer * 0.01f;
        }
    }
    
    /* Consume result to prevent elimination */
    double final_sum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += result[i];
        asm volatile("" : : "f"(final_sum));
    }
    volatile_sink_double = final_sum;
    
    free(arr_int);
    free(arr_float);
    free(result);
    
    return (int)final_sum & 1;
}
