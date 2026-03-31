/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent constant propagation and inlining */
volatile int global_seed = 42;

/* Function 1: High register pressure with multiple calls */
int __attribute__((noinline)) 
test_high_pressure(int x, int y) {
    /* Many local variables, all live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = b - a + 4;
    int e = c ^ d + 5;
    int f = d * e + 6;
    int g = e / (a + 1) + 7;
    int h = f - g + 8;
    int i = g * h + 9;
    int j = h ^ i + 10;
    
    /* First call clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables after call */
    int sum1 = a + b + c + d + e;
    
    /* More computation creating new live values */
    int k = sum1 * 2 + 11;
    int l = sum1 / 2 + 12;
    int m = k ^ l + 13;
    
    /* Second call */
    clobber_many_regs2();
    
    /* Use more variables */
    int sum2 = f + g + h + i + j + k + l + m;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + (a * b) - (c / d) + (e % f) ^ (g & h) | (i << 2);
}

/* Function 2: Mix of caller-saved and callee-saved usage with control flow */
int __attribute__((noinline))
test_control_flow(int cond, int x) {
    /* Variables that might go in callee-saved registers */
    register long r1 asm("rbx") = x * 2;
    register long r2 asm("r12") = x * 3;
    register long r3 asm("r13") = x * 4;
    
    /* Many temporary variables for caller-saved pressure */
    int t1 = x + 1;
    int t2 = x + 2;
    int t3 = x + 3;
    int t4 = x + 4;
    int t5 = x + 5;
    int t6 = x + 6;
    int t7 = x + 7;
    int t8 = x + 8;
    
    if (cond > 0) {
        /* Branch with call and variable usage */
        clobber_many_regs1();
        
        /* Complex computation using both register types */
        r1 = r1 + t1 + t2;
        r2 = r2 * t3 - t4;
        int branch_sum = t5 + t6 + t7 + t8;
        
        clobber_many_regs2();
        
        /* More usage */
        t1 = t1 ^ branch_sum;
        t2 = t2 | r1;
        r3 = r3 & t3;
    } else {
        /* Different computation in else branch */
        clobber_many_regs3();
        
        r1 = r1 - t8;
        r2 = r2 + t7;
        int else_prod = t1 * t2 * t3;
        
        clobber_many_regs1();
        
        t4 = t4 ^ else_prod;
        t5 = t5 | r2;
        r3 = r3 - t6;
    }
    
    /* Force all variables to be used */
    return (int)(r1 + r2 + r3 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8);
}

/* Function 3: Loop with calls and register pressure */
int __attribute__((noinline))
test_loop_pressure(int iterations) {
    int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int v1 = i * 2 + 1;
        int v2 = i * 3 + 2;
        int v3 = i * 4 + 3;
        int v4 = i * 5 + 4;
        int v5 = i * 6 + 5;
        int v6 = i * 7 + 6;
        int v7 = i * 8 + 7;
        int v8 = i * 9 + 8;
        
        /* Call that clobbers registers */
        if (i % 2 == 0) {
            clobber_many_regs1();
        } else {
            clobber_many_regs2();
        }
        
        /* Use variables after call */
        accum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        
        /* More computation */
        int w1 = accum ^ v1;
        int w2 = accum | v2;
        int w3 = accum & v3;
        
        /* Another call */
        if (i % 3 == 0) {
            clobber_many_regs3();
        }
        
        accum = w1 + w2 + w3;
    }
    
    return accum;
}

/* Function 4: Nested calls with register pressure */
int __attribute__((noinline))
test_nested_pressure(int x) {
    /* Force many values to be live */
    int a1 = x * 1, a2 = x * 2, a3 = x * 3, a4 = x * 4;
    int b1 = x * 5, b2 = x * 6, b3 = x * 7, b4 = x * 8;
    int c1 = x * 9, c2 = x * 10, c3 = x * 11, c4 = x * 12;
    
    /* Sequence of calls */
    clobber_many_regs1();
    int sum1 = a1 + a2 + a3 + a4;
    
    clobber_many_regs2();
    int sum2 = b1 + b2 + b3 + b4 + sum1;
    
    clobber_many_regs3();
    int sum3 = c1 + c2 + c3 + c4 + sum2;
    
    /* Use address-taking to inhibit optimizations */
    int *ptr1 = &a1, *ptr2 = &b1, *ptr3 = &c1;
    
    clobber_many_regs1();
    
    return sum3 + *ptr1 + *ptr2 + *ptr3;
}

/* External function definitions using inline asm to clobber registers */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber many regs\n"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    /* Different clobber pattern */
    asm volatile (
        "# clobber many regs 2\n"
        : 
        : 
        : "rax", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    /* Yet another clobber pattern */
    asm volatile (
        "# clobber many regs 3\n"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

int main(int argc, char *argv[]) {
    /* Use argc to vary computation paths */
    int seed = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    int result = 0;
    
    /* Call all test functions to create various pressure scenarios */
    result += test_high_pressure(seed, seed * 2);
    result += test_control_flow(seed % 2, seed);
    result += test_loop_pressure((seed % 5) + 3);
    result += test_nested_pressure(seed + 1);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
