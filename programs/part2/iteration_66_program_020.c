/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering code at lines 905-913
 * of caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8");
}

/* Function that creates high register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x * 1;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    int g = x * 7;
    int h = x * 8;
    int i = x * 9;
    int j = x * 10;
    int k = x * 11;
    int l = x * 12;
    int m = x * 13;
    int n = x * 14;
    int o = x * 15;
    int p = x * 16;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call - they must be preserved */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + y;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_with_branches(int x, int cond) {
    /* Many live variables */
    int v1 = x + 1;
    int v2 = x + 2;
    int v3 = x + 3;
    int v4 = x + 4;
    int v5 = x + 5;
    int v6 = x + 6;
    int v7 = x + 7;
    int v8 = x + 8;
    int v9 = x + 9;
    int v10 = x + 10;
    
    if (cond > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables in true branch */
        int t1 = v1 + v2 + v3 + v4;
        int t2 = v5 + v6 + v7 + v8;
        
        /* Another call */
        clobber_many_regs_2();
        
        return t1 + t2 + v9 + v10;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        
        /* Different variable usage pattern */
        int f1 = v1 * v2 * v3;
        int f2 = v4 * v5 * v6;
        
        /* Call at end of block */
        clobber_many_regs_1();
        
        return f1 + f2 + v7 + v8 + v9 + v10;
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int x, int iterations) {
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int l1 = x + i * 1;
        int l2 = x + i * 2;
        int l3 = x + i * 3;
        int l4 = x + i * 4;
        int l5 = x + i * 5;
        int l6 = x + i * 6;
        int l7 = x + i * 7;
        int l8 = x + i * 8;
        
        /* Call inside loop - variables must be preserved */
        if (i % 2 == 0) {
            clobber_many_regs_1();
        } else {
            clobber_many_regs_2();
        }
        
        /* Use variables after call */
        acc += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
        
        /* Another call with different pattern */
        if (i % 3 == 0) {
            clobber_many_regs_3();
        }
    }
    
    return acc;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save_types(int x) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("rbx") = x * 100;
    register long r2 asm("rbp") = x * 200;
    register long r3 asm("r12") = x * 300;
    
    /* Many other variables for caller-saved pressure */
    int c1 = x * 1;
    int c2 = x * 2;
    int c3 = x * 3;
    int c4 = x * 4;
    int c5 = x * 5;
    int c6 = x * 6;
    int c7 = x * 7;
    int c8 = x * 8;
    int c9 = x * 9;
    int c10 = x * 10;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types */
    long callee_sum = r1 + r2 + r3;
    int caller_sum = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10;
    
    /* Another call */
    clobber_many_regs_2();
    
    return (int)(callee_sum % 1000) + caller_sum;
}

/* Function with switch statement for more control flow */
int __attribute__((noinline)) test_with_switch(int x, int mode) {
    int r1 = x + 1;
    int r2 = x + 2;
    int r3 = x + 3;
    int r4 = x + 4;
    int r5 = x + 5;
    int r6 = x + 6;
    
    switch (mode) {
        case 0:
            clobber_many_regs_1();
            return r1 + r2;
        case 1:
            clobber_many_regs_2();
            return r3 + r4;
        case 2:
            clobber_many_regs_3();
            return r5 + r6;
        case 3:
            clobber_many_regs_1();
            clobber_many_regs_2();
            return r1 + r3 + r5;
        default:
            clobber_many_regs_1();
            clobber_many_regs_2();
            clobber_many_regs_3();
            return r2 + r4 + r6;
    }
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Run all test functions with varying inputs */
    result += test_high_pressure(seed, seed + 1);
    result += test_with_branches(seed, seed % 2);
    result += test_with_loop(seed, 5 + (seed % 3));
    result += test_mixed_save_types(seed);
    result += test_with_switch(seed, seed % 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;
}
