/* ISO C99-compliant program to stress GCC's caller-save register allocation */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force noinline to prevent optimization of call sites */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with different signatures and calling conventions */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Force register pressure in callee */
    volatile int x1 = a + b;
    volatile int x2 = c - d;
    volatile int x3 = e * f;
    volatile int x4 = g / (h ? h : 1);
    return x1 + x2 + x3 + x4;
}

NOINLINE float helper2(float a, float b, float c, float d, 
                       float e, float f, float g, float h) {
    /* Mixed float operations */
    volatile float r1 = a + b;
    volatile float r2 = c - d;
    volatile float r3 = e * f;
    volatile float r4 = g / h;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    return r1 + r2 + r3 + r4;
}

NOINLINE static void* helper3(void* p1, void* p2, void* p3, 
                              void* p4, void* p5, void* p6) {
    /* Pointer manipulations */
    volatile uintptr_t v1 = (uintptr_t)p1;
    volatile uintptr_t v2 = (uintptr_t)p2;
    volatile uintptr_t v3 = (uintptr_t)p3;
    __asm__ volatile ("" : : : "rax", "r10", "r11");
    return (void*)(v1 ^ v2 ^ v3 ^ (uintptr_t)p4 ^ (uintptr_t)p5 ^ (uintptr_t)p6);
}

NOINLINE double helper4(double a, double b, double c, double d,
                        int i, int j, int k, int l, int m, int n) {
    /* Many arguments to exceed register passing limits */
    volatile double sum = a + b + c + d;
    volatile int isum = i + j + k + l + m + n;
    __asm__ volatile ("" : : : "xmm4", "xmm5", "xmm6", "xmm7", "r12", "r13");
    return sum * isum;
}

NOINLINE static long helper5(long a, long b, long c, long d,
                             long e, long f, long g, long h,
                             long i, long j, long k, long l) {
    /* Extreme argument count */
    volatile long t1 = a ^ b;
    volatile long t2 = c | d;
    volatile long t3 = e & f;
    volatile long t4 = g << 2;
    volatile long t5 = h >> 1;
    __asm__ volatile ("" : : : "rbx", "rbp", "r14", "r15");
    return t1 + t2 + t3 + t4 + t5 + i + j + k + l;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static int use_alloca(int size) {
    char* buffer = alloca(size);
    volatile int sum = 0;
    for (int i = 0; i < size && i < 16; i++) {
        buffer[i] = i;
        sum += buffer[i];
    }
    return sum;
}

/* Main function with complex control flow */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04;
    volatile void* p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4;
    volatile long l1 = 100, l2 = 200, l3 = 300, l4 = 400;
    
    /* Take addresses to force stack usage */
    int* ptrs[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    /* Complex control flow creating multiple basic blocks */
    int result = 0;
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration == 0) {
            /* First basic block with function calls */
            v1 = helper1(v1, v2, v3, v4, v5, v6, v7, v8);
            
            /* Inline assembly clobbering call-clobbered registers */
            __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", 
                                           "r8", "r9", "r10", "r11");
            
            f1 = helper2(f1, f2, f3, f4, f1, f2, f3, f4);
            
            /* More register pressure between calls */
            v2 = v1 * v2 + v3 - v4;
            f2 = f1 * 2.0f - f3;
            
            p1 = helper3(p1, p2, p3, p4, &f1, &f2);
            
            /* Force spill/reload around call */
            __asm__ volatile ("" : "+r"(v3), "+r"(v4) : : "memory");
            
            d1 = helper4(d1, d2, d3, d4, v1, v2, v3, v4, v5, v6);
        } 
        else if (iteration == 1) {
            /* Second basic block with different call pattern */
            v5 = use_alloca(v5 * 8 + 16);
            
            __asm__ volatile ("" : : : "xmm8", "xmm9", "xmm10", "xmm11",
                                           "xmm12", "xmm13", "xmm14", "xmm15");
            
            l1 = helper5(l1, l2, l3, l4, l1, l2, l3, l4, l1, l2, l3, l4);
            
            /* Complex computation keeping values live */
            v6 = v5 ^ v6 | v7 & ~v8;
            f3 = f2 * f4 / (f1 + 1.0f);
            
            /* Another call with register pressure */
            v7 = helper1(v6, v7, v8, v9, v10, v1, v2, v3);
            
            /* Clobber more registers */
            __asm__ volatile ("" : : : "r12", "r13", "r14", "r15", "rbp");
        } 
        else {
            /* Third basic block with loop and calls */
            for (int i = 0; i < 2; i++) {
                v8 = helper1(v8, v9, v10, v1, v2, v3, v4, v5);
                
                /* Volatile access to prevent optimization */
                *(volatile int*)&v9 = v8 + i;
                
                f4 = helper2(f4, f3, f2, f1, f4, f3, f2, f1);
                
                /* Mix in some alloca usage */
                if (i == 0) {
                    v10 = use_alloca(32);
                }
                
                /* More inline assembly between calls */
                __asm__ volatile ("" : "+m"(v8), "+m"(v9) : : 
                                           "rax", "rbx", "rcx", "rdx");
            }
        }
        
        /* Accumulate results across iterations */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)(f1 + f2 + f3 + f4);
        result += (int)(d1 + d2 + d3 + d4);
        result += (intptr_t)p1 % 256;
        result += l1 % 256;
    }
    
    /* Final computation and output */
    printf("Result checksum: %d\n", result);
    
    /* Verify with expected value for basic correctness */
    if (result != 0) {
        printf("Program executed successfully with non-zero result.\n");
    }
    
    return 0;
}
