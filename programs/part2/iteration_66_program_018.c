/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs_1(void);
extern void __attribute__((noinline)) clobber_many_regs_2(void);
extern void __attribute__((noinline)) clobber_many_regs_3(void);

/* Use asm to force register clobbering */
void clobber_many_regs_1(void) {
    /* Clobber multiple caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void clobber_many_regs_2(void) {
    /* Different clobber set to force different spill decisions */
    asm volatile ("" : : : "rax", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2");
}

void clobber_many_regs_3(void) {
    /* Mix integer and floating point clobbers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
}

/* Test function 1: High register pressure with multiple calls */
int __attribute__((noinline)) test_high_pressure(int seed) {
    /* Many live variables across calls */
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
    
    /* First call - many registers live */
    clobber_many_regs_1();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call with different clobbers */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = e + f + g + h + sum1;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + sum1 + sum2;
}

/* Test function 2: Control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y) {
    /* Variables that must live across calls */
    int a = x * 2;
    int b = y * 3;
    int c = x + y;
    int d = x - y;
    int e = x * y;
    int f = x ^ y;
    int g = x & y;
    int h = x | y;
    
    if (x > y) {
        /* Branch with call and variable usage */
        clobber_many_regs_1();
        a = b + c;
        d = e - f;
    } else {
        /* Different branch, different call */
        clobber_many_regs_2();
        g = h * a;
        b = c + d;
    }
    
    /* Another call after the branch */
    clobber_many_regs_3();
    
    /* Complex computation to use all variables */
    return (a * b) + (c * d) - (e * f) + (g * h);
}

/* Test function 3: Loop with calls and register pressure */
int __attribute__((noinline)) test_loop_pressure(int iterations) {
    int acc = 0;
    
    /* Many local variables that need to be preserved */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be spilled/reloaded */
        clobber_many_regs_1();
        
        /* Use variables in computation */
        acc += v1 + v2 + v3;
        
        /* Another call with different clobbers */
        if (i % 2 == 0) {
            clobber_many_regs_2();
            acc += v4 + v5 + v6;
        } else {
            clobber_many_regs_3();
            acc += v7 + v8 + v9 + v10;
        }
        
        /* Modify variables to prevent optimization */
        v1 += i;
        v2 -= i;
        v3 ^= i;
    }
    
    return acc + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 4: Mix of caller/callee saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Force variables to different registers using addresses */
    int a = x;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    
    /* Take addresses to inhibit register allocation optimizations */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    int *ptr4 = &d;
    int *ptr5 = &e;
    
    /* Sequence of calls */
    clobber_many_regs_1();
    
    /* Use through pointers */
    *ptr1 += 1;
    *ptr2 += 2;
    
    clobber_many_regs_2();
    
    *ptr3 += 3;
    *ptr4 += 4;
    
    clobber_many_regs_3();
    
    *ptr5 += 5;
    
    /* Final computation */
    return a + b + c + d + e + (int)(ptr1) + (int)(ptr2);
}

/* Test function 5: Nested calls with pressure */
int __attribute__((noinline)) test_nested_pressure(int x) {
    /* Even more variables */
    int a1 = x, a2 = x+1, a3 = x+2, a4 = x+3, a5 = x+4;
    int b1 = x*2, b2 = x*3, b3 = x*4, b4 = x*5, b5 = x*6;
    int c1 = x^1, c2 = x^2, c3 = x^3, c4 = x^4, c5 = x^5;
    
    /* Call sequence with computations in between */
    int sum1 = a1 + a2 + a3;
    clobber_many_regs_1();
    
    int sum2 = b1 + b2 + b3 + sum1;
    clobber_many_regs_2();
    
    int sum3 = c1 + c2 + c3 + sum2;
    clobber_many_regs_3();
    
    /* Use all variables in final result */
    return a1 + a2 + a3 + a4 + a5 +
           b1 + b2 + b3 + b4 + b5 +
           c1 + c2 + c3 + c4 + c5 +
           sum1 + sum2 + sum3;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int iterations = argc > 2 ? atoi(argv[2]) : 5;
    
    /* Run all tests to create various caller-save scenarios */
    result += test_high_pressure(seed);
    result += test_control_flow(seed, seed * 2);
    result += test_loop_pressure(iterations);
    result += test_mixed_save(seed);
    result += test_nested_pressure(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
