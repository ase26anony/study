#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and takes many arguments (exceeding register limits) */
int __attribute__((noinline)) compute_sum(int a, int b, int c, int d, int e,
                                          int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber call-clobbered registers */
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return result;
}

/* Static function that might be inlined */
static float __attribute__((noinline)) process_float(float a, float b, float c, 
                                                     float d, float e) {
    volatile float temp = a * b + c / d - e;
    /* Clobber floating point registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    return temp * 2.0f;
}

/* Function using alloca to affect frame pointer */
void* __attribute__((noinline)) allocate_temp(int size) {
    void* ptr = alloca(size);
    /* Force register pressure with computation */
    volatile int x = size * 2;
    volatile int y = size + 100;
    __asm__ volatile ("" : : "r"(x), "r"(y) : "rax", "rcx");
    return ptr;
}

/* Function with pointer arguments */
int __attribute__((noinline)) process_pointers(int* a, float* b, double* c, 
                                               char* d, short* e) {
    volatile int sum = *a + (int)(*b) + (int)(*c) + *d + *e;
    /* Clobber multiple registers */
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
    return sum;
}

/* Function that returns a double */
double __attribute__((noinline)) compute_double(double a, double b, double c,
                                                double d, double e) {
    volatile double result = (a + b) * (c - d) / e;
    /* Clobber floating point and integer registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "rax", "rcx");
    return result;
}

/* Main computation with high register pressure */
int main(void) {
    /* Declare many local variables of mixed types to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile char c1 = 'a', c2 = 'b', c3 = 'c';
    volatile short s1 = 100, s2 = 200, s3 = 300;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    volatile double* p3 = &d1;
    
    int result = 0;
    
    /* Create control flow with basic blocks containing function calls */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            /* First basic block with multiple calls */
            v1 = compute_sum(v1, v2, v3, v4, v5, i, i+1, i+2, i+3, i+4);
            
            /* Inline assembly between computations to create live ranges */
            __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
            
            f1 = process_float(f1, f2, f3, f4, f5);
            
            /* More register pressure */
            __asm__ volatile ("mov %0, %%rax\n\t"
                             "add %1, %%rax" 
                             : : "r"(v1), "r"(v2) : "rax", "rcx");
        } else {
            /* Second basic block with different calls */
            void* temp = allocate_temp(i * 16 + 8);
            
            /* Use the allocated memory to prevent optimization */
            *(int*)temp = i;
            
            /* More computations keeping values in registers */
            d1 = compute_double(d1, d2, d3, d4, d5);
            
            /* Complex expression with many intermediate values */
            v2 = v1 * v3 + v4 / (v5 + 1) - i * 2;
            f2 = f1 * 3.14f + f3 / 2.0f - f4 * f5;
            
            /* Another inline assembly clobber */
            __asm__ volatile ("" : : : "r10", "r11", "xmm4", "xmm5", "xmm6", "xmm7");
        }
        
        /* Common code with another function call */
        if (i % 3 == 0) {
            int ptr_result = process_pointers((int*)&v1, (float*)&f1, 
                                             (double*)&d1, (char*)&c1, (short*)&s1);
            result += ptr_result;
            
            /* More register-intensive computation */
            d2 = d1 * 1.5 + d3 / 2.0 - d4 * 0.75;
            v3 = v2 ^ v4 | v5 & i;
        }
        
        /* Loop-carried dependencies to keep values live across iterations */
        v4 = v3 + v2 - v1;
        f3 = f2 * f1 - f4 + f5;
        d3 = d2 + d1 * 0.5;
        
        /* Final inline assembly in the loop */
        __asm__ volatile ("" : : : "rax", "rbx", "xmm0", "xmm1");
    }
    
    /* Additional control flow after the loop */
    {
        /* Take address of locals to affect frame pointer */
        int* addr_array[5];
        addr_array[0] = &v1;
        addr_array[1] = &v2;
        addr_array[2] = &v3;
        addr_array[3] = &v4;
        addr_array[4] = &v5;
        
        /* Complex expression using addresses */
        for (int j = 0; j < 5; j++) {
            *addr_array[j] += j * 10;
        }
        
        /* Final function call with many arguments */
        result += compute_sum(v1, v2, v3, v4, v5, 
                             (int)f1, (int)f2, (int)f3, (int)f4, (int)f5);
    }
    
    /* Compute and print checksum */
    int checksum = result + v1 + v2 + v3 + v4 + v5 + 
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return 0;
}
