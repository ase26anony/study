/* test_caller_save.c
 * Designed to trigger GCC's caller-save pass instruction reordering
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function that creates extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure_1(int seed) {
    /* Many local variables that must live across calls */
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    int i = seed + 9;
    int j = seed + 10;
    int k = seed + 11;
    int l = seed + 12;
    int m = seed + 13;
    int n = seed + 14;
    int o = seed + 15;
    int p = seed + 16;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call - variables still live */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a * b - c + d / (e + 1) + f * g - h + i * j + k - l + m * n - o + p;
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int seed, int flag) {
    /* Many live variables */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    if (flag > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Complex computation in true branch */
        int t1 = v1 + v2 + v3 + v4;
        int t2 = v5 * v6 - v7 + v8;
        
        /* Another call */
        clobber_many_regs_2();
        
        /* More computation */
        int t3 = v9 * v10 + t1 - t2;
        
        /* Use all variables */
        return t1 + t2 + t3 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9 - v10;
    } else {
        /* Different computation in false branch */
        clobber_many_regs_3();
        
        int f1 = v1 * v2 + v3 * v4;
        int f2 = v5 * v6 * v7 * v8;
        
        /* Another call in false branch */
        clobber_many_regs_1();
        
        int f3 = v9 + v10 + f1 - f2;
        
        /* Use all variables differently */
        return f1 * 2 + f2 / 3 + f3 * 4 + v1 + v3 + v5 + v7 + v9;
    }
}

/* Function with loops creating multiple basic blocks */
int __attribute__((noinline)) test_with_loops(int seed, int iterations) {
    int result = seed;
    
    /* Many live variables inside loop */
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        /* Complex computation using all variables */
        result += a * i + b * (i + 1) + c * (i + 2) + d * (i + 3);
        result += e * (i * 2) + f * (i * 3) + g * (i * 4) + h * (i * 5);
        
        /* Another call inside loop */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More computation */
        result -= a + b + c + d + e + f + g + h;
    }
    
    return result;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_register_usage(int seed) {
    /* Force some variables to potentially use callee-saved registers
     * by taking their addresses */
    int callee1 = seed * 2;
    int callee2 = seed * 3;
    int callee3 = seed * 4;
    int callee4 = seed * 5;
    
    /* Reference addresses to inhibit optimizations */
    volatile int *ptr1 = &callee1;
    volatile int *ptr2 = &callee2;
    volatile int *ptr3 = &callee3;
    volatile int *ptr4 = &callee4;
    
    /* Many caller-saved register candidates */
    int caller1 = seed + 10;
    int caller2 = seed + 20;
    int caller3 = seed + 30;
    int caller4 = seed + 40;
    int caller5 = seed + 50;
    int caller6 = seed + 60;
    int caller7 = seed + 70;
    int caller8 = seed + 80;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use both caller and callee variables */
    int sum1 = *ptr1 + *ptr2 + caller1 + caller2;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    int sum2 = *ptr3 + *ptr4 + caller3 + caller4 + caller5 + caller6;
    
    /* Final call */
    clobber_many_regs_3();
    
    return sum1 + sum2 + caller7 + caller8 + *ptr1 - *ptr2 + *ptr3 - *ptr4;
}

/* Function with sequential calls and minimal computation between them */
int __attribute__((noinline)) test_sequential_calls(int seed) {
    /* Maximum register pressure: many live variables */
    int r1 = seed * 1, r2 = seed * 2, r3 = seed * 3, r4 = seed * 4;
    int r5 = seed * 5, r6 = seed * 6, r7 = seed * 7, r8 = seed * 8;
    int r9 = seed * 9, r10 = seed * 10, r11 = seed * 11, r12 = seed * 12;
    int r13 = seed * 13, r14 = seed * 14, r15 = seed * 15, r16 = seed * 16;
    
    /* Back-to-back calls with all variables live */
    clobber_many_regs_1();
    clobber_many_regs_2();
    clobber_many_regs_3();
    clobber_many_regs_1();
    clobber_many_regs_2();
    
    /* Use all variables in complex expression */
    return r1 + r2 - r3 + r4 - r5 + r6 - r7 + r8 - r9 + r10 
           - r11 + r12 - r13 + r14 - r15 + r16
           + r1 * r2 + r3 * r4 + r5 * r6 + r7 * r8
           + r9 * r10 + r11 * r12 + r13 * r14 + r15 * r16;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    int flag = argc > 2 ? atoi(argv[2]) : 1;
    int iterations = argc > 3 ? atoi(argv[3]) : 5;
    
    /* Run all test functions to create various caller-save scenarios */
    result += test_high_pressure_1(base);
    result += test_control_flow(base + 1, flag);
    result += test_with_loops(base + 2, iterations);
    result += test_mixed_register_usage(base + 3);
    result += test_sequential_calls(base + 4);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
