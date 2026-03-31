#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
static float helper_float_op(float a, float b, float c) {
    volatile float barrier = a;
    asm volatile("" : "+f"(barrier) : : "memory");
    return (a * b) + (c / (barrier + 1.0f));
}

__attribute__((noinline))
static int helper_int_op(int a, int b, int c) {
    volatile int barrier = a;
    asm volatile("" : "+r"(barrier) : : "memory");
    return (a ^ b) | (c & ~barrier);
}

__attribute__((noinline))
static double helper_double_op(double a, double b, int c) {
    volatile double barrier = a;
    asm volatile("" : "+f"(barrier) : : "memory");
    return (a + b) * (c % 256) / (barrier + 1.0);
}

__attribute__((noinline))
static void* helper_mem_op(void* ptr, int offset, int value) {
    volatile int* vptr = (volatile int*)ptr;
    vptr[offset] = value;
    asm volatile("" : : "r"(vptr) : "memory");
    return ptr;
}

/* Main complex scheduling function */
static uint64_t complex_scheduling_function(void) {
    /* High register pressure: 30+ local variables of mixed types */
    volatile int outer_limit = 1000;  /* Prevent constant propagation */
    int i, j, k;
    
    /* Integer variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Floating point variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Pointer and memory variables */
    int array[256];
    volatile int* volatile_ptr = array;
    float f_array[128];
    double d_array[64];
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        array[i] = i;
    }
    for (i = 0; i < 128; i++) {
        f_array[i] = i * 0.1f;
    }
    for (i = 0; i < 64; i++) {
        d_array[i] = i * 0.01;
    }
    
    /* Result accumulator */
    volatile uint64_t checksum = 0;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        /* Nested loop level 1 - variable bounds */
        volatile int inner_limit1 = (i % 50) + 10;
        for (j = 0; j < inner_limit1; j++) {
            /* Mixed operation dependency chain */
            v1 = v2 + v3;
            f1 = f2 * f3;
            asm volatile("" : : : "memory");  /* Barrier */
            
            /* Inter-type dependency: int -> float */
            f2 = (float)v1 * 0.5f;
            v2 = (int)f1;
            
            /* Memory access dependency */
            v3 = array[v1 % 256];
            array[v2 % 256] = v3 + 1;
            
            /* Floating point operation chain */
            d1 = d2 + d3;
            d2 = d4 * d5;
            asm volatile("" : : : "memory");  /* Barrier */
            
            /* Nested loop level 2 - deeper nesting */
            volatile int inner_limit2 = (j % 10) + 5;
            for (k = 0; k < inner_limit2; k++) {
                /* Conditional execution paths with different operation mixes */
                switch ((i + j + k) % 4) {
                    case 0:  /* FP math path */
                        f3 = helper_float_op(f1, f2, f3);
                        d3 = helper_double_op(d1, d2, v3);
                        f_array[k % 128] = f3;
                        break;
                        
                    case 1:  /* Integer bit manipulation path */
                        v4 = helper_int_op(v1, v2, v3);
                        v5 = (v4 << 3) | (v5 >> 2);
                        v6 = v4 ^ v5 ^ v6;
                        break;
                        
                    case 2:  /* Memory intensive path */
                        helper_mem_op(array, (v1 + k) % 256, v2);
                        v7 = array[(v2 + k) % 256];
                        v8 = array[(v3 + k) % 256];
                        break;
                        
                    case 3:  /* Mixed type computation path */
                        f4 = (float)v4 * 0.25f;
                        d4 = (double)v5 * 0.125;
                        v9 = (int)(f4 * d4);
                        f5 = helper_float_op(f4, f5, (float)v9);
                        break;
                }
                
                /* Additional dependency chains across iterations */
                v10 = v11 * v12;
                f1 = f1 + f_array[k % 128];
                asm volatile("" : : : "memory");  /* Barrier */
                
                v11 = v10 ^ v13;
                d5 = d_array[k % 64] + d1;
                
                /* More mixed operations */
                v12 = (int)(d5 * 100.0);
                f2 = (float)v12 / 50.0f;
            }
            
            /* Function calls with scheduling side effects */
            if (j % 3 == 0) {
                v13 = helper_int_op(v10, v11, v12);
                f3 = helper_float_op(f1, f2, f3);
            } else if (j % 3 == 1) {
                d1 = helper_double_op(d1, d2, v13);
                helper_mem_op(array, j % 256, v13);
            }
            
            /* Update checksum with various values */
            checksum ^= (uint64_t)v1;
            checksum ^= (uint64_t)(*(uint32_t*)&f1);
            checksum ^= (uint64_t)(*(uint64_t*)&d1);
            checksum ^= (uint64_t)array[j % 256];
        }
        
        /* Periodic complex operation block */
        if (i % 100 == 0) {
            /* Long dependency chain */
            v14 = v15 * v1;
            f4 = f5 * f1;
            d2 = d3 + d4;
            asm volatile("" : : : "memory");  /* Barrier */
            
            v15 = v14 ^ v2;
            f5 = f4 / f2;
            d3 = d2 * d5;
            
            /* Memory update with barrier */
            for (int m = 0; m < 16; m++) {
                array[(i + m) % 256] = v15 + m;
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* Final complex computation to ensure all variables are used */
    int final_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15;
    float final_float = f1 + f2 + f3 + f4 + f5;
    double final_double = d1 + d2 + d3 + d4 + d5;
    
    checksum ^= (uint64_t)final_int;
    checksum ^= (uint64_t)(*(uint32_t*)&final_float);
    checksum ^= (uint64_t)(*(uint64_t*)&final_double);
    
    return checksum;
}

int main(void) {
    printf("Starting complex scheduling test...\n");
    
    uint64_t result = complex_scheduling_function();
    
    printf("Checksum result: 0x%016llx\n", (unsigned long long)result);
    printf("Test completed.\n");
    
    return 0;
}
