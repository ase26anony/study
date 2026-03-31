/* caller-save-test.c
 * Test program to trigger GCC's caller-save optimization pass
 * and exercise the instruction reordering logic in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
}

/* Function that uses many variables across calls - high register pressure */
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
    
    /* Use all variables after call - forces spills/restores */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a - b + c - d + e - f + g - h + 
           i - j + k - l + m - n + o - p + y;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_with_branches(int x, int cond) {
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    if (cond > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        a = a * 2;
        b = b * 2;
    } else {
        /* Call in false branch */
        clobber_many_regs_2();
        c = c * 3;
        d = d * 3;
    }
    
    /* Another call after the branch */
    clobber_many_regs_3();
    
    /* Use variables that must survive across calls */
    return a + b + c + d + e + f + g + h;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int x, int iterations) {
    int a = x;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    int g = x * 7;
    int h = x * 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        /* Modify variables */
        a += i;
        b += i * 2;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        }
        
        c += i * 3;
        d += i * 4;
    }
    
    /* Final call */
    clobber_many_regs_3();
    
    return a + b + c + d + e + f + g + h;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save_types(int x) {
    /* Variables that might use callee-saved registers */
    register long r12 asm("r12") = x * 100;
    register long r13 asm("r13") = x * 200;
    register long r14 asm("r14") = x * 300;
    register long r15 asm("r15") = x * 400;
    
    /* Many regular variables for caller-saved pressure */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    int i = x + 9;
    int j = x + 10;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types */
    int sum1 = a + b + c + d + e;
    long sum2 = r12 + r13 + r14 + r15;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More usage */
    int sum3 = f + g + h + i + j;
    
    return sum1 + (int)sum2 + sum3;
}

/* Function with multiple basic blocks and calls at block boundaries */
int __attribute__((noinline)) test_block_boundaries(int x, int mode) {
    int a = x * 1;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    
    switch (mode) {
        case 0:
            clobber_many_regs_1();
            a = a + 100;
            break;
        case 1:
            clobber_many_regs_2();
            b = b + 200;
            break;
        case 2:
            clobber_many_regs_3();
            c = c + 300;
            break;
        default:
            clobber_many_regs_1();
            clobber_many_regs_2();
            d = d + 400;
            break;
    }
    
    /* Call right before block end */
    clobber_many_regs_3();
    
    return a + b + c + d;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to vary paths and prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Run all test functions with different parameters */
    result += test_high_pressure(seed, seed + 1);
    result += test_with_branches(seed, seed % 3);
    result += test_with_loop(seed, 5 + (seed % 3));
    result += test_mixed_save_types(seed);
    result += test_block_boundaries(seed, seed % 4);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
