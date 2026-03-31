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
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int seed) {
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
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a * b - c * d + e / (f + 1) + g % (h + 1) +
           i * j - k * l + m / (n + 1) + o % (p + 1);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_with_branches(int x, int y) {
    /* Many variables to create register pressure */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = x + y;
    int v4 = x - y;
    int v5 = x * y;
    int v6 = x + 1;
    int v7 = y + 1;
    int v8 = x * 3;
    int v9 = y * 2;
    int v10 = x + y + 1;
    
    /* Call in one branch */
    if (x > y) {
        clobber_many_regs_1();
        v1 = v2 + v3;
        v4 = v5 - v6;
    } else {
        clobber_many_regs_2();
        v7 = v8 + v9;
        v10 = v1 * v2;
    }
    
    /* Another call after the branch */
    clobber_many_regs_3();
    
    /* Use all variables in complex expression */
    return v1 + v2 * 2 - v3 / 3 + v4 % 5 + v5 * v6 - v7 / 8 + 
           v8 % 9 + v9 * 10 - v10 / 11;
}

/* Function with loops and calls */
int __attribute__((noinline)) test_with_loop(int iterations) {
    int acc = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int n = 0; n < iterations; n++) {
        /* Call inside loop with many live variables */
        clobber_many_regs_1();
        
        /* Use all variables in computation */
        acc += a + b - c + d - e + f - g + h - i + j;
        
        /* Modify variables to prevent optimization */
        a += n;
        b -= n;
        c *= (n + 1);
        d /= (n + 2);
        
        /* Another call with different clobber */
        if (n % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More variable usage */
        e += acc;
        f -= n;
        g *= (n + 3);
        h /= (n + 4);
        i += b;
        j -= c;
    }
    
    return acc + a + b + c + d + e + f + g + h + i + j;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save_types(int x) {
    /* Variables that might use callee-saved registers */
    register long r12_val asm("r12") = x * 2;
    register long r13_val asm("r13") = x * 3;
    register long r14_val asm("r14") = x * 4;
    register long r15_val asm("r15") = x * 5;
    
    /* Many regular variables for caller-saved pressure */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types of variables */
    long callee_sum = r12_val + r13_val + r14_val + r15_val;
    int caller_sum = a + b + c + d + e + f + g + h;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* Complex usage pattern */
    return (callee_sum % 1000) + caller_sum + 
           (r12_val % 10) * (r13_val % 20) - 
           (r14_val % 30) / (r15_val % 40 + 1);
}

/* Function with multiple basic blocks and calls at block boundaries */
int __attribute__((noinline)) test_multiple_blocks(int x) {
    int result = 0;
    
    /* Block 1 */
    int a1 = x * 1, b1 = x * 2, c1 = x * 3;
    clobber_many_regs_1();
    result += a1 + b1 + c1;
    
    /* Block 2 - conditional */
    if (x > 100) {
        int a2 = x * 4, b2 = x * 5, c2 = x * 6;
        clobber_many_regs_2();
        result += a2 - b2 + c2;
        
        /* Nested condition */
        if (x > 200) {
            int a3 = x * 7, b3 = x * 8, c3 = x * 9;
            clobber_many_regs_3();
            result += a3 * b3 - c3;
        }
    } else {
        int a4 = x * 10, b4 = x * 11, c4 = x * 12;
        clobber_many_regs_1();
        result += a4 / (b4 + 1) + c4;
    }
    
    /* Block 3 - after conditional */
    int a5 = x * 13, b5 = x * 14, c5 = x * 15;
    clobber_many_regs_2();
    result += a5 % (b5 + 1) + c5;
    
    return result;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line arguments to prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Run all test functions to create various caller-save scenarios */
    result += test_high_pressure(seed);
    result += test_with_branches(seed, seed * 2);
    result += test_with_loop(seed % 10 + 1);
    result += test_mixed_save_types(seed);
    result += test_multiple_blocks(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
