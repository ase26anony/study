#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_ops(float a, float b, float c, int iter) {
    volatile float result = 0.0f;
    for (int i = 0; i < (iter & 0x3F); i++) {
        result += a * b - c;
        a = b + 1.0f;
        b = c * 2.0f;
        c = result * 0.5f;
    }
    return result;
}

__attribute__((noinline))
int helper_int_ops(int a, int b, int c, int* mem) {
    volatile int result = 0;
    for (int i = 0; i < (a & 0x1F); i++) {
        result += (a * b) ^ c;
        a = (b << 2) | (c & 0xFF);
        b = c + mem[i & 0xF];
        c = result >> 1;
    }
    return result;
}

__attribute__((noinline))
double helper_mixed_ops(double d, int i, float f, volatile int* counter) {
    double result = d;
    for (int j = 0; j < (*counter & 0x7); j++) {
        result = result * f + i;
        f = f * 0.9f + j * 0.1f;
        i = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        asm volatile("" ::: "memory");  // Barrier
    }
    return result;
}

int main() {
    /* High register pressure: many local variables */
    volatile int outer_limit = 1000;  // Prevent constant propagation
    volatile int checksum = 0;
    
    /* Mixed type variables to stress scheduler */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    int* mem_buffer = (int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = i * 3 + 1;
    }
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Nested loops with variable bounds */
        for (int mid = 0; mid < (outer & 0x1F) + 10; mid++) {
            volatile int inner_limit = (mid * 7 + outer) & 0x3F;
            
            for (int inner = 0; inner < inner_limit; inner++) {
                /* Complex dependency chains across different operation types */
                
                /* Integer arithmetic chain */
                v1 = v2 + v3 * v4 - v5;
                v2 = v3 ^ v6 | v7;
                v3 = (v8 << 3) + (v9 >> 2);
                v4 = v10 * v11 - v12;
                
                /* Memory access chain */
                v5 = mem_buffer[(v1 + inner) & 0xFF];
                v6 = mem_buffer[(v2 + mid) & 0xFF];
                v7 = mem_buffer[(v3 + outer) & 0xFF];
                
                /* Floating point chain with integer conversion */
                f1 = f2 * f3 + (float)v1;
                f2 = f4 - f5 * (float)v2;
                f3 = (float)v3 * 0.5f + f6;
                
                /* Double precision chain */
                d1 = d2 * 1.1 + (double)f1;
                d2 = d3 / 2.0 - (double)v4;
                
                /* Barrier to prevent reordering */
                asm volatile("" ::: "memory");
                
                /* Conditional execution paths */
                switch ((inner + mid) & 0x7) {
                    case 0:
                        /* FP intensive path */
                        f4 = f5 * f7 + f8;
                        f5 = helper_float_ops(f1, f2, f3, inner);
                        d3 = d4 * d5 + 1.234;
                        break;
                    case 1:
                        /* Integer bit manipulation path */
                        v8 = (v9 << 4) ^ (v10 >> 2);
                        v9 = v11 & v12 | v13;
                        v10 = helper_int_ops(v1, v2, v3, mem_buffer);
                        break;
                    case 2:
                        /* Mixed operations path */
                        f6 = (float)v14 * 0.25f + f9;
                        v11 = (int)(f6 * 100.0f) ^ v15;
                        d4 = helper_mixed_ops(d1, v16, f10, &inner_limit);
                        break;
                    case 3:
                        /* Memory intensive path */
                        for (int k = 0; k < 4; k++) {
                            mem_buffer[(inner + k) & 0xFF] = 
                                mem_buffer[(mid + k) & 0xFF] + 
                                mem_buffer[(outer + k) & 0xFF];
                        }
                        break;
                    default:
                        /* Default arithmetic path */
                        v12 = v13 + v14 * v15 - v16;
                        v13 = v17 ^ v18 | v19;
                        f7 = f8 * f9 + (float)v20;
                        break;
                }
                
                /* More dependency chains */
                v14 = v15 + v16 * v17;
                v15 = (v18 << 2) | (v19 & 0xF);
                f8 = f9 * 2.0f - f10;
                f9 = (float)v20 * 0.333f;
                
                /* Another barrier */
                asm volatile("" ::: "memory");
                
                /* Function calls with scheduling side effects */
                if ((inner & 0x3) == 0) {
                    v16 = helper_int_ops(v1, v2, v3, mem_buffer);
                } else if ((inner & 0x3) == 1) {
                    f10 = helper_float_ops(f1, f2, f3, inner);
                } else {
                    d5 = helper_mixed_ops(d1, v4, f4, &inner);
                }
                
                /* Update checksum with all variables */
                checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
                checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
                checksum ^= *(int*)&f1 ^ *(int*)&f2 ^ *(int*)&f3;
                checksum ^= *(int*)&f4 ^ *(int*)&f5 ^ *(int*)&f6;
                checksum ^= *(int*)&f7 ^ *(int*)&f8 ^ *(int*)&f9 ^ *(int*)&f10;
                checksum ^= (int)(d1 * 1000) ^ (int)(d2 * 1000);
                checksum ^= (int)(d3 * 1000) ^ (int)(d4 * 1000) ^ (int)(d5 * 1000);
            }
            
            /* Additional operations between inner loops */
            v17 = v18 * v19 + v20;
            v18 = helper_int_ops(v17, v19, v20, mem_buffer);
            f1 = helper_float_ops(f1, f2, f3, mid);
            
            asm volatile("" ::: "memory");
        }
        
        /* Update memory buffer periodically */
        if ((outer & 0x3F) == 0) {
            for (int i = 0; i < 64; i++) {
                mem_buffer[(outer + i) & 0xFF] = 
                    mem_buffer[i] + mem_buffer[(i + 32) & 0xFF] + checksum;
            }
        }
    }
    
    /* Final checksum calculation */
    volatile int final_checksum = checksum;
    for (int i = 0; i < 256; i++) {
        final_checksum ^= mem_buffer[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    free(mem_buffer);
    return final_checksum & 0xFF;
}
