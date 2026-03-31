#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to force scheduler state saves */
__attribute__((noinline)) 
float helper_float_op(float a, float b, float c) {
    volatile float v1 = a * b;
    volatile float v2 = b + c;
    asm volatile("" ::: "memory");
    return v1 / (v2 + 1.0f);
}

__attribute__((noinline))
int helper_int_op(int a, int b, int c) {
    volatile int v1 = a ^ b;
    volatile int v2 = b | c;
    asm volatile("" ::: "memory");
    return (v1 & v2) + (c << 3);
}

__attribute__((noinline))
double helper_double_op(double a, double b, int c) {
    volatile double d1 = a * 1.234567;
    volatile double d2 = b / 0.987654;
    asm volatile("" ::: "memory");
    return d1 + d2 + (double)c;
}

__attribute__((noinline))
void* helper_mem_op(void* ptr, int offset, int value) {
    volatile int* vptr = (volatile int*)ptr;
    vptr[offset] = value;
    asm volatile("" ::: "memory");
    return ptr;
}

/* Main complex function with high register pressure */
void complex_scheduling_function(volatile int outer_iterations) {
    /* High register pressure: 30+ local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.111, d2 = 2.222, d3 = 3.333, d4 = 4.444, d5 = 5.555;
    volatile int* mem_ptr = (volatile int*)malloc(1024 * sizeof(int));
    volatile float* fptr = (volatile float*)malloc(512 * sizeof(float));
    volatile int counter = 0;
    volatile int checksum = 0;
    
    /* Initialize memory */
    for (int i = 0; i < 1024; i++) {
        mem_ptr[i] = i;
    }
    for (int i = 0; i < 512; i++) {
        fptr[i] = i * 0.5f;
    }
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_iterations; outer++) {
        /* Nested loops with variable bounds */
        volatile int middle_limit = (outer % 10) + 5;
        
        for (volatile int middle = 0; middle < middle_limit; middle++) {
            /* Inner loop count depends on outer and middle */
            volatile int inner_limit = ((outer * middle) % 8) + 2;
            
            for (volatile int inner = 0; inner < inner_limit; inner++) {
                /* Mixed operation dependency chains */
                
                /* Integer to float chain */
                v1 = v2 + v3;
                f1 = (float)v1 * f2;
                asm volatile("" ::: "memory");
                
                /* Float to memory chain */
                fptr[inner] = f1 + f3;
                v4 = (int)fptr[inner];
                asm volatile("" ::: "memory");
                
                /* Memory to integer chain */
                v5 = mem_ptr[v4 % 1024] + v6;
                d1 = (double)v5 * d2;
                asm volatile("" ::: "memory");
                
                /* Double to integer chain */
                v7 = (int)d1;
                mem_ptr[(v7 + inner) % 1024] = v8;
                asm volatile("" ::: "memory");
                
                /* Conditional execution paths */
                switch ((outer + middle + inner) % 4) {
                    case 0:
                        /* FP math branch */
                        f4 = helper_float_op(f1, f2, f3);
                        d3 = helper_double_op(d1, d2, v9);
                        v10 = (int)(f4 * d3);
                        break;
                    case 1:
                        /* Integer bit manipulation branch */
                        v11 = helper_int_op(v1, v2, v3);
                        v12 = (v11 << 2) | (v4 >> 1);
                        v13 = v12 ^ 0xABCDEF;
                        break;
                    case 2:
                        /* Memory intensive branch */
                        helper_mem_op((void*)mem_ptr, v5 % 256, v6);
                        v14 = mem_ptr[v7 % 256] + mem_ptr[v8 % 256];
                        f5 = (float)v14 * 0.123f;
                        break;
                    case 3:
                        /* Mixed operations branch */
                        v15 = helper_int_op(v9, v10, v11);
                        f3 = helper_float_op(f2, f3, f4);
                        helper_mem_op((void*)mem_ptr, v15 % 128, (int)f3);
                        break;
                }
                
                /* More dependency chains across types */
                v2 = v3 * v4;
                f2 = f3 + (float)v2;
                d2 = d3 * (double)f2;
                v3 = (int)d2;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
                
                /* Function call with scheduling side effects */
                if ((inner % 3) == 0) {
                    d4 = helper_double_op(d1, d2, v5);
                    v6 = helper_int_op(v7, v8, (int)d4);
                }
                
                /* Update checksum with all live variables */
                checksum ^= v1 ^ v2 ^ v3 ^ v4 ^ v5;
                checksum ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
                checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15;
                checksum ^= (int)f1 ^ (int)f2 ^ (int)f3 ^ (int)f4 ^ (int)f5;
                checksum ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5;
                
                counter++;
            }
            
            /* Additional operations between loop levels */
            v8 = v9 + v10;
            f3 = helper_float_op(f4, f5, (float)v8);
            asm volatile("" ::: "memory");
        }
        
        /* Complex operation between outer iterations */
        v9 = mem_ptr[outer % 256] * 2;
        d5 = helper_double_op(d4, (double)v9, outer);
        mem_ptr[(outer + 128) % 256] = (int)d5;
        
        /* Force memory synchronization */
        asm volatile("" ::: "memory");
    }
    
    /* Final complex calculation to ensure all code paths matter */
    volatile int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result += mem_ptr[i % 256];
        final_result ^= (int)fptr[i % 128];
    }
    
    checksum ^= final_result;
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d, Iterations: %d\n", checksum, counter);
    
    /* Cleanup */
    free((void*)mem_ptr);
    free((void*)fptr);
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int iterations = 1000;
    
    /* Call the complex scheduling function */
    complex_scheduling_function(iterations);
    
    /* Additional simpler calls to potentially trigger different scheduling contexts */
    for (int i = 0; i < 10; i++) {
        volatile int temp = i * 100;
        complex_scheduling_function(temp % 50 + 10);
    }
    
    return 0;
}
