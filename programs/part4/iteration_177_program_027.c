#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, int iter) {
    volatile float result = 0.0f;
    for (int i = 0; i < (iter & 0x3F) + 1; i++) {
        result += a * b - c;
        a = b + 1.0f;
        b = c * 0.5f;
        c = result + i;
    }
    return result;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, int* mem) {
    volatile int result = 0;
    for (int i = 0; i < 3; i++) {
        result += (a ^ b) | c;
        a = (b << 2) + i;
        b = c * 3;
        c = result & 0xFF;
        *mem = result;  // Memory side effect
        asm volatile("" ::: "memory");  // Barrier
    }
    return result;
}

__attribute__((noinline))
double helper_mixed_ops(double d, int i, float f, volatile int* counter) {
    double result = d;
    for (int j = 0; j < (*counter & 0x7) + 1; j++) {
        result += (double)i * 1.5 + (double)f * 2.0;
        i = (int)(result * 10.0) ^ j;
        f = (float)result * 0.3f;
        asm volatile("" ::: "memory");
    }
    (*counter)++;
    return result;
}

/* Main complex function with high register pressure */
void complex_scheduling_function(volatile int outer_iterations) {
    /* Many local variables to create register pressure (30+) */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6;
    int* mem_ptr = (int*)malloc(64 * sizeof(int));
    volatile int checksum = 0;
    volatile int loop_counter = 0;
    
    /* Initialize variables with different values */
    for (int i = 0; i < 64; i++) {
        mem_ptr[i] = i * 3 + 1;
    }
    
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05; d6 = 6.06;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Nested loops with variable bounds */
        for (int i = 0; i < (outer & 0xF) + 2; i++) {
            for (int j = 0; j < (i * 3 + 1) & 0x7; j++) {
                /* Mixed operation dependency chains */
                v1 = v2 + v3 * v4;
                f1 = (float)v1 * 0.5f + f2;
                mem_ptr[(v1 + j) & 0x3F] = (int)(f1 * 100.0f);
                asm volatile("" ::: "memory");  // Barrier
                
                d1 = (double)v1 + (double)f1 * 2.0;
                v2 = (int)d1 ^ v3;
                f2 = f3 - f4 * (float)v2;
                
                /* Call helper functions with dependencies */
                f3 = helper_float_ops(f2, f3, f4, v2 + j);
                asm volatile("" ::: "memory");
                
                v3 = helper_int_ops(v2, v3, v4, &mem_ptr[(v2 + i) & 0x3F]);
                
                d2 = helper_mixed_ops(d1, v3, f3, &loop_counter);
                
                /* More mixed operations */
                v4 = v5 | (v6 & v7);
                f4 = f5 * f6 - f7;
                mem_ptr[(v4 + i + j) & 0x3F] = v3 + (int)f4;
                
                d3 = d4 * d5 + d6;
                v5 = (int)(d3 * 10.0) % 256;
                f5 = (float)v5 * 0.01f + f8;
                
                /* Conditional execution paths */
                switch ((i + j + outer) & 0x3) {
                    case 0:
                        /* FP math path */
                        f6 = f7 * f8 + f9;
                        d4 = d5 * 2.0 - d6;
                        v6 = (int)(f6 * d4);
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        v6 = (v7 << 3) ^ (v8 >> 2);
                        v7 = ~v6 & 0xFFFF;
                        v8 = v6 | v7;
                        break;
                    case 2:
                        /* Memory intensive path */
                        for (int k = 0; k < 2; k++) {
                            mem_ptr[(v9 + k) & 0x3F] = 
                                mem_ptr[(v10 + k) & 0x3F] * 2 + 1;
                        }
                        v6 = mem_ptr[v9 & 0x3F] + mem_ptr[v10 & 0x3F];
                        break;
                    default:
                        /* Mixed path */
                        f6 = helper_float_ops(f9, f10, f1, v9);
                        v6 = helper_int_ops(v9, v10, v1, &mem_ptr[v2 & 0x3F]);
                        break;
                }
                
                /* Update checksum with all variables */
                checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6;
                checksum ^= ((int)f1) ^ ((int)f2) ^ ((int)f3) ^ ((int)f4);
                checksum ^= ((int)d1) ^ ((int)d2) ^ ((int)d3) ^ ((int)d4);
                
                /* More operations to extend dependency chains */
                v7 = v8 + v9 * v10;
                f7 = f8 / f9 + f10;
                d5 = d6 * 1.1 + d1;
                
                v8 = v9 ^ v10;
                f8 = f9 * 2.0f - f10;
                d6 = d1 + d2 + d3;
                
                v9 = v10 + (v1 << 1);
                f9 = f10 + (float)v9 * 0.5f;
                
                v10 = v1 + v2 + v3;
                f10 = (float)v10 * 0.25f + f1;
                
                asm volatile("" ::: "memory");  // Another barrier
            }
        }
        
        /* Additional operations between outer loop iterations */
        if (outer & 1) {
            d1 = helper_mixed_ops(d2, v10, f10, &loop_counter);
            v1 = helper_int_ops(v2, v3, v4, &mem_ptr[outer & 0x3F]);
        } else {
            f1 = helper_float_ops(f2, f3, f4, v5);
            v6 = helper_int_ops(v7, v8, v9, &mem_ptr[(outer + 1) & 0x3F]);
        }
        
        checksum += outer * 31;
    }
    
    /* Final checksum calculation using all variables */
    volatile int final_checksum = checksum;
    final_checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    final_checksum ^= ((int)f1) ^ ((int)f2) ^ ((int)f3) ^ ((int)f4) ^ ((int)f5);
    final_checksum ^= ((int)f6) ^ ((int)f7) ^ ((int)f8) ^ ((int)f9) ^ ((int)f10);
    final_checksum ^= ((int)d1) ^ ((int)d2) ^ ((int)d3) ^ ((int)d4) ^ ((int)d5) ^ ((int)d6);
    
    /* Use memory values in final calculation */
    for (int i = 0; i < 16; i++) {
        final_checksum += mem_ptr[i * 4];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    free(mem_ptr);
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int iterations = 1000;
    
    /* Call the complex scheduling function */
    complex_scheduling_function(iterations);
    
    return 0;
}
