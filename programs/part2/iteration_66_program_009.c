/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#elif defined(__arm__)
#define CLOBBER_LIST "r0", "r1", "r2", "r3"
#else
#define CLOBBER_LIST "memory"
#endif

/* Function that clobbers many caller-saved registers */
void __attribute__((noinline)) clobber_many_regs(void) {
    /* Use asm to clobber registers without being optimized away */
    asm volatile ("" : : : CLOBBER_LIST);
}

/* Another clobbering function */
void __attribute__((noinline)) clobber_more_regs(int x) {
    asm volatile ("" : : "r"(x) : CLOBBER_LIST);
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int seed) {
    /* Many local variables that must stay live across calls */
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
    
    /* Force variables to be in registers by taking addresses */
    volatile int *ptr_a = &a;
    volatile int *ptr_b = &b;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call with different clobber pattern */
    clobber_more_regs(e);
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call */
    clobber_many_regs();
    
    /* Complex usage pattern */
    int sum3 = i + j + k + l;
    
    /* Conditional to create basic blocks */
    if (seed % 2) {
        /* Another call in one branch */
        clobber_more_regs(m);
        sum3 += m + n;
    } else {
        /* Different call in other branch */
        clobber_many_regs();
        sum3 += o + p;
    }
    
    /* Final computation using all variables */
    return sum1 + sum2 + sum3 + *ptr_a + *ptr_b;
}

/* Function with loops to increase register pressure */
int __attribute__((noinline)) test_loop_pressure(int iterations) {
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int v1 = i * 1;
        int v2 = i * 2;
        int v3 = i * 3;
        int v4 = i * 4;
        int v5 = i * 5;
        int v6 = i * 6;
        int v7 = i * 7;
        int v8 = i * 8;
        
        /* Call inside loop - forces spills/reloads each iteration */
        clobber_many_regs();
        
        /* Use all variables */
        acc += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        
        /* Another call */
        if (i % 3 == 0) {
            clobber_more_regs(v1);
        }
    }
    
    return acc;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x, int y) {
    /* Variables that should go in callee-saved registers */
    register long c1 asm("") = x * 11;
    register long c2 asm("") = y * 13;
    register long c3 asm("") = x * 17;
    register long c4 asm("") = y * 19;
    
    /* Variables for caller-saved registers */
    int cs1 = x + 1;
    int cs2 = x + 2;
    int cs3 = x + 3;
    int cs4 = x + 4;
    int cs5 = x + 5;
    int cs6 = x + 6;
    
    /* Sequence of calls */
    clobber_many_regs();
    int t1 = cs1 + cs2;
    
    clobber_more_regs(cs3);
    int t2 = cs3 + cs4;
    
    clobber_many_regs();
    int t3 = cs5 + cs6;
    
    /* Use callee-saved variables across all calls */
    return c1 + c2 + c3 + c4 + t1 + t2 + t3;
}

/* Function with switch statement for control flow variation */
int __attribute__((noinline)) test_switch_pressure(int mode) {
    int r1 = mode * 2;
    int r2 = mode * 3;
    int r3 = mode * 5;
    int r4 = mode * 7;
    int r5 = mode * 11;
    int r6 = mode * 13;
    
    switch (mode % 4) {
        case 0:
            clobber_many_regs();
            return r1 + r2;
        case 1:
            clobber_more_regs(r3);
            return r3 + r4;
        case 2:
            clobber_many_regs();
            clobber_more_regs(r5);
            return r5 + r6;
        default:
            /* Multiple calls in sequence */
            clobber_many_regs();
            clobber_more_regs(r1);
            clobber_many_regs();
            return r1 + r2 + r3 + r4 + r5 + r6;
    }
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int iterations = argc > 2 ? atoi(argv[2]) : 100;
    
    /* Run all test functions */
    result += test_high_pressure(seed);
    result += test_loop_pressure(iterations);
    result += test_mixed_save(seed, seed + 1);
    result += test_switch_pressure(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
