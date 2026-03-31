/* Compile with: gcc -O2 -fno-expensive-optimizations -fdump-rtl-early_remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -finline-small-functions -fno-tree-pre -fdump-rtl-expand */

#include <stdint.h>
#include <stdlib.h>

/* Volatile sinks to prevent elimination */
static volatile int volatile_sink_int;
static volatile float volatile_sink_float;
static volatile double volatile_sink_double;

/* Inline functions to force copies */
static inline int copy_and_add_int(int a, int b, int c) {
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : :);
    int temp = a + b;
    asm volatile("" : "+r"(temp) : :);
    return temp + c;
}

static inline float copy_and_mul_float(float a, float b, float c) {
    asm volatile("" : "+f"(a), "+f"(b), "+f"(c) : :);
    float temp = a * b;
    asm volatile("" : "+f"(temp) : :);
    return temp * c;
}

static inline double copy_and_mix(double a, int b, float c) {
    asm volatile("" : "+f"(a), "+r"(b), "+f"(c) : :);
    double temp = a + (double)b;
    asm volatile("" : "+f"(temp) : :);
    return temp * (double)c;
}

/* Function to create register pressure with mixed types */
static void process_block(int start, int end, 
                         const int* int_data,
                         const float* float_data,
                         const double* double_data) {
    /* Local variables with different scopes and lifetimes */
    int local_int1, local_int2, local_int3;
    float local_float1, local_float2;
    double local_double1, local_double2;
    
    /* Outer loop creating control flow complexity */
    for (int i = start; i < end; i++) {
        /* Conditional block 1 */
        if (i & 1) {
            /* Integer chain with recomputable values */
            local_int1 = int_data[i] * 3;
            local_int2 = local_int1 + 7;
            local_int3 = local_int2 - int_data[i-1];
            
            /* Force copy propagation context */
            local_int1 = copy_and_add_int(local_int1, local_int2, local_int3);
            
            /* Mixed precision calculation */
            local_float1 = (float)local_int1 * 1.5f;
            local_float2 = local_float1 + float_data[i];
            
            /* Volatile sink */
            volatile_sink_int = local_int1;
            volatile_sink_float = local_float2;
            
            /* Inline asm clobbering */
            asm volatile("" : : "r"(local_int2), "r"(local_int3) : "memory");
        }
        
        /* Conditional block 2 */
        if (i & 2) {
            /* Floating point chain */
            local_float1 = float_data[i] * 2.0f;
            local_float2 = local_float1 / 3.14159f;
            
            /* Double precision chain with conversions */
            local_double1 = (double)local_float2;
            local_double2 = double_data[i] + local_double1;
            
            /* Mixed type operation */
            local_double1 = copy_and_mix(local_double2, i, local_float1);
            
            /* Volatile sink */
            volatile_sink_double = local_double1;
            
            /* Force register moves */
            asm volatile("" : : "f"(local_double2), "r"(i) : "memory");
        }
        
        /* Inner loop for additional pressure */
        for (int j = 0; j < 4; j++) {
            /* Short-lived recomputable values */
            int temp1 = i * j + 5;
            int temp2 = temp1 * 2 - j;
            float temp3 = (float)temp2 / 10.0f;
            
            /* Chain of dependent operations */
            temp1 = copy_and_add_int(temp1, temp2, j);
            temp3 = copy_and_mul_float(temp3, 2.0f, (float)temp1);
            
            /* Prevent optimization */
            asm volatile("" : "+r"(temp1), "+f"(temp3) : :);
            
            /* Conditional within inner loop */
            if (j & 1) {
                volatile_sink_int = temp1;
            } else {
                volatile_sink_float = temp3;
            }
        }
        
        /* Another block with different register usage */
        {
            /* Use different integer sizes */
            char c1 = (char)i;
            short s1 = (short)(i * 2);
            long l1 = (long)i * 3L;
            
            /* Operations forcing promotions */
            int mixed1 = c1 + s1;
            long mixed2 = l1 + mixed1;
            
            /* More inline asm to prevent coalescing */
            asm volatile("" : : "r"(c1), "r"(s1), "r"(l1) :);
            asm volatile("" : "+r"(mixed1), "+r"(mixed2) : :);
            
            volatile_sink_int = mixed1;
        }
    }
}

/* Main function creating the required context */
int main(void) {
    /* Initialize arrays with different patterns */
    const int ARRAY_SIZE = 1024;
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Fill arrays with varying data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 - 7;
        float_data[i] = (float)i * 0.5f + 1.0f;
        double_data[i] = (double)i * 0.25 - 0.5;
    }
    
    /* Multiple processing blocks to create complex CFG */
    for (int block = 0; block < 8; block++) {
        int start = block * 128;
        int end = start + 128;
        
        /* Process with different parameter patterns */
        process_block(start, end, int_data, float_data, double_data);
        
        /* Additional computation between blocks */
        if (block & 1) {
            /* Extra pressure between blocks */
            int temp = 0;
            for (int k = 0; k < 100; k++) {
                temp += copy_and_add_int(k, block, temp);
                asm volatile("" : "+r"(temp) : :);
            }
            volatile_sink_int = temp;
        }
    }
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
