/* caller-save-test.c - ISO C99 program to exercise GCC's caller-save register allocation */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force noinline to prevent unwanted optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with varying signatures to create register pressure */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    /* Use inline asm to clobber caller-saved registers */
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return result;
}

NOINLINE float helper2(float a, float b, float c, float d, 
                       float e, float f, float g, float h) {
    volatile float sum = a + b + c + d + e + f + g + h;
    /* Clobber floating point/SSE registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7");
    return sum * 0.5f;
}

NOINLINE void* helper3(void* p1, void* p2, void* p3, void* p4,
                       void* p5, void* p6, int i1, int i2) {
    volatile uintptr_t addr = (uintptr_t)p1 + (uintptr_t)p2 - 
                             (uintptr_t)p3 + (uintptr_t)p4;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
    return (void*)(addr + i1 + i2);
}

NOINLINE double helper4(double a, double b, double c, double d,
                        double e, double f, double g, double h,
                        double i, double j) {
    /* Many arguments to force stack passing */
    volatile double prod = a * b * c * d * e * f * g * h * i * j;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7",
                      "xmm8", "xmm9", "xmm10", "xmm11");
    return prod;
}

NOINLINE int helper5(int a, int b, int c, int d, int e,
                     float f, float g, double h, double i) {
    /* Mixed types to use different register classes */
    volatile int int_sum = a + b + c + d + e;
    volatile float float_sum = f + g;
    volatile double double_sum = h + i;
    
    /* Clobber multiple register types */
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "xmm0", "xmm1", "xmm2");
    
    return int_sum + (int)float_sum + (int)double_sum;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE void* use_alloca(size_t size) {
    void* ptr = alloca(size);
    __asm__ volatile ("" : : "r"(ptr) : "rax", "rcx");
    return ptr;
}

/* Complex function with control flow */
NOINLINE int complex_helper(int base, int iterations) {
    volatile int a = base * 2;
    volatile int b = base + 1;
    volatile int c = base - 1;
    volatile float f1 = base * 1.5f;
    volatile float f2 = base * 2.5f;
    volatile double d1 = base * 3.14159;
    volatile double d2 = base * 2.71828;
    
    int result = 0;
    
    /* Create basic block with internal calls */
    for (int i = 0; i < iterations; i++) {
        if (i % 3 == 0) {
            /* Call within basic block */
            result += helper1(a, b, c, i, i+1, i+2, i+3, i+4);
            __asm__ volatile ("" : : : "rax", "rcx", "rdx");
        } else if (i % 3 == 1) {
            /* Another call in same basic block */
            float fresult = helper2(f1, f2, f1*2, f2*2, 
                                   f1*3, f2*3, f1*4, f2*4);
            result += (int)fresult;
            __asm__ volatile ("" : : : "xmm0", "xmm1", "rax");
        } else {
            /* Third call pattern */
            double dresult = helper4(d1, d2, d1/2, d2/2,
                                    d1/3, d2/3, d1/4, d2/4,
                                    d1/5, d2/5);
            result += (int)dresult;
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "rax");
        }
        
        /* Modify values to create live ranges across calls */
        a += result;
        b -= i;
        c *= 2;
        f1 += 0.1f;
        f2 -= 0.1f;
        d1 *= 1.01;
        d2 /= 1.01;
    }
    
    return result;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    volatile float* p3 = &f1;
    volatile double* p4 = &d1;
    
    int result = 0;
    
    /* Take addresses to affect frame pointer optimization */
    int local_for_address = 42;
    volatile int* addr_taker = &local_for_address;
    
    /* Use alloca to force frame pointer usage */
    void* dynamic = use_alloca(64);
    __asm__ volatile ("" : : "r"(dynamic) : "rax");
    
    /* Sequence of operations with calls interspersed */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Complex control flow to create basic blocks */
        if (iteration % 10 == 0) {
            /* Block with multiple computations between calls */
            v1 = v1 * 2 + v2;
            v2 = v2 - v3 + iteration;
            v3 = v3 ^ v4;
            
            /* Call that uses many arguments */
            result += helper1(v1, v2, v3, v4, v5, v6, v7, v8);
            
            v4 = v4 | v5;
            v5 = v5 & v6;
            f1 = f1 * 1.1f + f2;
            
            /* Another call with different signature */
            float fres = helper2(f1, f2, f3, f4, f5, f1*2, f2*2, f3*2);
            result += (int)fres;
            
            v6 = v6 << 1;
            v7 = v7 >> 1;
            d1 = d1 * 1.01 + d2;
        } 
        else if (iteration % 10 == 5) {
            /* Different block with pointer operations */
            *p1 = *p1 + *p2;
            *p2 = *p2 - iteration;
            
            /* Call with pointer arguments */
            void* new_ptr = helper3(p1, p2, p3, p4, 
                                   (void*)&v9, (void*)&v10, 
                                   v1, v2);
            __asm__ volatile ("" : : "r"(new_ptr) : "rax", "rbx");
            
            *p3 = *p3 + 0.5f;
            *p4 = *p4 - 0.1;
            
            /* Call with mixed types */
            result += helper5(v1, v2, v3, v4, v5, f1, f2, d1, d2);
            
            v8 = v8 ^ v9;
            v9 = v9 | v10;
        }
        else {
            /* Third block pattern with double operations */
            d2 = d2 * d3 / d4;
            d3 = d3 + d5 - d1;
            
            /* Call with many double arguments (forces stack) */
            double dres = helper4(d1, d2, d3, d4, d5,
                                 d1*2, d2*2, d3*2, d4*2, d5*2);
            result += (int)dres;
            
            d4 = d4 * 0.99;
            d5 = d5 / 1.01;
            f3 = f3 + f4 - f5;
            
            /* Complex helper with internal control flow */
            result += complex_helper(iteration, 5);
            
            f4 = f4 * 1.05f;
            f5 = f5 / 1.05f;
        }
        
        /* Additional inline asm to clobber registers between iterations */
        __asm__ volatile ("" : : : 
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Modify all variables to keep them live */
        v1++; v2--; v3 ^= iteration; v4 |= 1; v5 &= 0xFF;
        v6 <<= 1; v7 >>= 1; v8 += 2; v9 -= 2; v10 *= 3;
        f1 += 0.01f; f2 -= 0.01f; f3 *= 1.001f; f4 /= 1.001f; f5 = -f5;
        d1 += 0.001; d2 -= 0.001; d3 *= 1.0001; d4 /= 1.0001; d5 = -d5;
    }
    
    /* Final checksum */
    int checksum = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5
                 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return checksum != 0 ? 0 : 1;
}
