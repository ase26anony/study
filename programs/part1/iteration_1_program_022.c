#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with varying attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Force register pressure inside callee */
    volatile int x1 = result * 2;
    volatile int x2 = result / 3;
    volatile int x3 = x1 ^ x2;
    volatile int x4 = x3 << 2;
    volatile int x5 = x4 >> 1;
    return x5;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float* f1, float* f2, float* f3, float* f4, float* f5) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5;
    /* More register pressure */
    volatile float t1 = sum * 1.5f;
    volatile float t2 = t1 / 2.0f;
    volatile float t3 = t2 + 3.14f;
    volatile float t4 = t3 * t3;
    return t4;
}

/* Function that clobbers registers via inline asm */
void __attribute__((noinline))
force_register_clobber(void) {
    /* Explicitly clobber call-clobbered registers */
    __asm__ volatile (
        "# Force clobber\n"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

/* Function using alloca to affect frame pointer */
int __attribute__((noinline))
use_alloca(int size) {
    /* Taking address of alloca result forces frame pointer */
    char* buffer = alloca(size);
    volatile int sum = 0;
    for (int i = 0; i < size && i < 16; i++) {
        buffer[i] = i;
        sum += buffer[i];
    }
    return sum;
}

/* Mixed-type computation function */
static double __attribute__((noinline))
mixed_computation(int a, float b, double c, int* d, float* e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    
    /* Create register pressure with mixed types */
    volatile int i1 = (int)result;
    volatile float f1 = (float)result;
    volatile double d1 = result * 2.0;
    volatile int i2 = i1 ^ 0xABCD;
    volatile float f2 = f1 / 3.0f;
    
    return d1 + (double)i2 + (double)f2;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    
    /* Variable to accumulate results */
    volatile int checksum = 0;
    
    /* Control flow to create basic block boundaries */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        f1 = f1 * 1.5f + (float)iteration;
        d1 = d1 * 2.0 + (double)iteration;
        
        /* Function call with many arguments - will need caller-save */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += sum1;
        
        /* Inline assembly to clobber registers between computations */
        force_register_clobber();
        
        /* More computations keeping values live in registers */
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        f2 = f2 + f1 * 0.5f;
        
        /* Conditional to create another basic block */
        if (iteration % 2 == 0) {
            /* Function with pointer arguments */
            float float_result = process_floats(&f1, &f2, &f3, &f4, &f5);
            checksum += (int)float_result;
            
            /* More computations */
            v5 = v5 << 2;
            v6 = v6 >> 1;
            d2 = d2 * 1.1;
            
            /* Mixed type function call */
            double mixed_result = mixed_computation(v5, f3, d2, &v6, &f4);
            checksum += (int)mixed_result;
        } else {
            /* Alternative path with different calls */
            int alloca_result = use_alloca(32 + iteration);
            checksum += alloca_result;
            
            /* More register-intensive computations */
            v7 = v7 * v1 + v2;
            v8 = v8 / (v3 + 1);
            f3 = f3 * f1 - f2;
        }
        
        /* Another forced clobber */
        __asm__ volatile (
            "# Clobber specific registers\n"
            "mov $0, %%rax\n"
            "mov $0, %%rcx\n"
            "mov $0, %%rdx\n"
            : 
            : 
            : "rax", "rcx", "rdx", "memory"
        );
        
        /* Final computations in the loop */
        v9 = v9 + checksum;
        v10 = v10 - iteration;
        f4 = f4 / (f1 + 1.0f);
        f5 = f5 * f2;
        
        /* Another function call at end of block */
        int sum2 = compute_sum(v10, v9, v8, v7, v6, v5, v4, v3, v2, v1);
        checksum += sum2;
    }
    
    /* Additional complex control flow */
    volatile int counter = 100;
    while (counter > 0) {
        /* Nested conditionals to create more basic blocks */
        if (counter % 3 == 0) {
            int temp = compute_sum(counter, counter+1, counter+2, counter+3, 
                                  counter+4, counter+5, counter+6, counter+7,
                                  counter+8, counter+9);
            checksum += temp;
            
            /* Use alloca occasionally to affect frame pointer */
            if (counter % 9 == 0) {
                use_alloca(counter % 16);
            }
        } else if (counter % 5 == 0) {
            float temp_f = process_floats(&f1, &f2, &f3, &f4, &f5);
            checksum += (int)temp_f;
        }
        
        /* Force register clobbering in loop */
        if (counter % 7 == 0) {
            force_register_clobber();
        }
        
        /* Complex computation keeping many values live */
        v1 = (v1 * 3 + counter) & 0xFFF;
        v2 = (v2 / 2 + counter) | 0xABC;
        v3 = v3 ^ v1 ^ v2;
        v4 = v4 + (v3 << 2);
        f1 = f1 * 1.01f + (float)counter * 0.1f;
        f2 = f2 / 1.01f - (float)counter * 0.01f;
        d1 = d1 + (double)counter * 0.001;
        
        counter--;
    }
    
    /* Final output */
    printf("Final checksum: %d\n", checksum);
    
    /* Use all variables to prevent optimization */
    printf("Values: %d %d %d %d %d %d %d %d %d %d\n", 
           v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
    printf("Floats: %f %f %f %f %f\n", f1, f2, f3, f4, f5);
    printf("Doubles: %lf %lf\n", d1, d2);
    
    return checksum != 0 ? 0 : 1;
}
