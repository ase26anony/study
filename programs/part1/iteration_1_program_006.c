#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many registers */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b + c + d + e + f + g + h;
    /* Force register pressure inside callee */
    int t1 = result * 2;
    int t2 = t1 + a;
    int t3 = t2 * b;
    int t4 = t3 - c;
    int t5 = t4 / (d ? d : 1);
    int t6 = t5 ^ e;
    int t7 = t6 | f;
    int t8 = t7 & g;
    return t8 + h;
}

/* Function with pointer arguments */
float __attribute__((noinline))
process_floats(float* f1, float* f2, float* f3, float* f4, 
               float* f5, float* f6, float* f7) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5 + *f6 + *f7;
    
    /* Create register pressure with float operations */
    float t1 = sum * 1.5f;
    float t2 = t1 / 2.0f;
    float t3 = t2 + *f1;
    float t4 = t3 - *f2;
    float t5 = t4 * *f3;
    
    /* Inline assembly to clobber floating point registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    
    return t5;
}

/* Function that uses alloca to affect frame pointer */
static void* __attribute__((noinline))
create_buffer(int size) {
    /* alloca forces frame pointer usage */
    char* buffer = (char*)alloca(size + 16);
    
    /* Use the buffer to prevent optimization */
    for (int i = 0; i < size && i < 16; i++) {
        buffer[i] = (char)(i * 3);
    }
    
    volatile int checksum = 0;
    for (int i = 0; i < size && i < 16; i++) {
        checksum += buffer[i];
    }
    
    /* Cast to void* to return, though alloca memory is stack-bound */
    return (void*)buffer;
}

/* Variadic-like function using many arguments */
int __attribute__((noinline))
mixed_operation(int a, float b, int c, float d, 
                int* e, float* f, int g, float h) {
    volatile int int_part = a + c + g + *e;
    volatile float float_part = b + d + h + *f;
    
    /* Force both integer and float register pressure */
    int t1 = int_part * 3;
    float t2 = float_part * 2.5f;
    int t3 = t1 + (int)t2;
    
    /* Clobber specific call-clobbered registers */
    #ifdef __x86_64__
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", 
                      "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7");
    #elif defined(__i386__)
    __asm__ volatile ("" : : : "eax", "ecx", "edx", 
                      "st(0)", "st(1)", "st(2)", "st(3)");
    #endif
    
    return t3;
}

/* Function with loop to create basic block boundaries */
static int __attribute__((noinline))
loop_computation(int iterations, int seed) {
    volatile int result = seed;
    
    /* Create multiple basic blocks within the function */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            result += i * 2;
            /* Function call inside basic block */
            int temp = compute_sum(result, i, seed, i*2, i*3, i*4, i*5, i*6);
            result ^= temp;
        } else if (i % 3 == 1) {
            result -= i * 3;
            /* Inline assembly between computations */
            #ifdef __x86_64__
            __asm__ volatile ("" : : : "r10", "r11");
            #endif
        } else {
            result *= (i + 1);
            /* Another function call */
            float f1 = result * 0.1f;
            float f2 = f1 * 2.0f;
            float f3 = f2 * 3.0f;
            float f4 = f3 * 4.0f;
            float f5 = f4 * 5.0f;
            float f6 = f5 * 6.0f;
            float f7 = f6 * 7.0f;
            float temp_f = process_floats(&f1, &f2, &f3, &f4, &f5, &f6, &f7);
            result += (int)temp_f;
        }
        
        /* Additional control flow */
        switch (i % 4) {
            case 0:
                result |= 0xFF;
                break;
            case 1:
                result &= 0x0F;
                break;
            case 2:
                result ^= 0xAA;
                break;
            default:
                result = ~result;
                break;
        }
    }
    
    return result;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile float f5 = 5.5f;
    int* p1 = &v1;
    int* p2 = &v2;
    float* pf1 = &f1;
    float* pf2 = &f2;
    
    /* Initial computations keeping values in registers */
    int r1 = v1 + v2;
    int r2 = v3 * v4;
    float rf1 = f1 + f2;
    float rf2 = f3 * f4;
    
    /* First function call - many arguments */
    int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8);
    
    /* Inline assembly clobbering registers between calls */
    #ifdef __x86_64__
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
    #elif defined(__i386__)
    __asm__ volatile ("" : : : "eax", "ecx", "edx");
    #endif
    
    /* More computations using results */
    r1 ^= sum1;
    r2 += sum1;
    rf1 *= (float)sum1;
    
    /* Second function call - pointer arguments */
    float float_result = process_floats(&f1, &f2, &f3, &f4, &f5, &rf1, &rf2);
    
    /* Control flow creating basic block boundaries */
    if (r1 > r2) {
        /* Function call inside basic block */
        void* buffer = create_buffer(64);
        (void)buffer; /* Use to avoid unused warning */
        
        r1 = mixed_operation(v1, f1, v2, f2, p1, pf1, v3, f3);
        
        /* More register pressure */
        v9 = r1 * 2;
        v10 = v9 + r2;
        
        #ifdef __x86_64__
        __asm__ volatile ("" : : : "r8", "r9", "r10", "r11");
        #endif
    } else {
        r2 = mixed_operation(v4, f4, v5, f5, p2, pf2, v6, f3);
        
        v9 = r2 / 2;
        v10 = v9 - r1;
        
        #ifdef __x86_64__
        __asm__ volatile ("" : : : "xmm8", "xmm9", "xmm10", "xmm11");
        #endif
    }
    
    /* Loop with function calls to create instruction density */
    int loop_result = 0;
    for (int i = 0; i < 5; i++) {
        /* Varying calls based on loop iteration */
        if (i % 2 == 0) {
            loop_result += loop_computation(3, r1 + i);
        } else {
            loop_result -= loop_computation(2, r2 - i);
        }
        
        /* Additional computations between calls */
        v1 += i;
        v2 *= (i + 1);
        f1 += (float)i * 0.5f;
        
        /* Occasional inline assembly */
        if (i % 3 == 1) {
            #ifdef __x86_64__
            __asm__ volatile ("" : : : "xmm12", "xmm13", "xmm14", "xmm15");
            #endif
        }
    }
    
    /* Final computation using all variables */
    int final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    final_result += (int)(f1 + f2 + f3 + f4 + f5);
    final_result ^= r1;
    final_result |= r2;
    final_result += (int)float_result;
    final_result += loop_result;
    
    /* Use alloca in main to affect frame pointer */
    char* main_buffer = (char*)alloca(32);
    for (int i = 0; i < 32; i++) {
        main_buffer[i] = (char)(final_result >> (i % 8));
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += main_buffer[i];
    }
    checksum += final_result;
    
    printf("Result: %d, Checksum: %d\n", final_result, checksum);
    
    return (checksum % 256) == 0 ? 0 : 1;
}
