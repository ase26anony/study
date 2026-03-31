/* test_caller_save.c - Trigger GCC caller-save instruction reordering */
#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs_x86(void);
extern void __attribute__((noinline)) clobber_many_regs_arm(void);
extern int __attribute__((noinline)) external_func(int, int, int, int, int, int);

/* Prevent constant propagation and inlining */
volatile int global_seed = 42;

/* Function 1: High register pressure with multiple calls in sequence */
int __attribute__((noinline)) test_high_pressure_seq(int arg) {
    /* Many local variables, all live across calls */
    int a = arg + 1;
    int b = arg + 2;
    int c = arg + 3;
    int d = arg + 4;
    int e = arg + 5;
    int f = arg + 6;
    int g = arg + 7;
    int h = arg + 8;
    int i = arg + 9;
    int j = arg + 10;
    int k = arg + 11;
    int l = arg + 12;
    int m = arg + 13;
    int n = arg + 14;
    int o = arg + 15;
    int p = arg + 16;
    
    /* First call - clobbers caller-saved registers */
#ifdef __x86_64__
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
#elif defined(__arm__)
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r12");
#endif
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call with different clobbers */
#ifdef __x86_64__
    asm volatile("" ::: "rax", "rdx", "rcx", "rsi", "rdi");
#elif defined(__arm__)
    asm volatile("" ::: "r0", "r1", "r2", "r3");
#endif
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call - external function with many arguments */
    int ret = external_func(i, j, k, l, m, n);
    
    /* Final computation using all variables */
    return sum1 + sum2 + ret + o + p + global_seed;
}

/* Function 2: Control flow variation with branches */
int __attribute__((noinline)) test_branching_pressure(int arg, int cond) {
    /* Many variables, some will need callee-saved registers */
    register int r1 asm("") = arg * 2;
    register int r2 asm("") = arg * 3;
    int v1 = arg + 100;
    int v2 = arg + 200;
    int v3 = arg + 300;
    int v4 = arg + 400;
    int v5 = arg + 500;
    int v6 = arg + 600;
    int v7 = arg + 700;
    int v8 = arg + 800;
    
    /* Take addresses to force stack allocation for some */
    int *ptr1 = &v1;
    int *ptr2 = &v2;
    
    if (cond > 0) {
        /* Branch 1: Call and computation */
#ifdef __x86_64__
        asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
#elif defined(__arm__)
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r12");
#endif
        
        /* Use variables across call */
        *ptr1 = v3 + v4;
        int temp = v5 + v6;
        
        /* Another call */
        int ret = external_func(v1, v2, v3, v4, v5, v6);
        
        /* Complex computation */
        r1 = r1 + ret + temp;
    } else {
        /* Branch 2: Different call pattern */
#ifdef __x86_64__
        asm volatile("" ::: "rax", "rcx", "rdx");
        asm volatile("" ::: "rsi", "rdi", "r8", "r9", "r10");
#elif defined(__arm__)
        asm volatile("" ::: "r0", "r1", "r2");
        asm volatile("" ::: "r3", "r12");
#endif
        
        /* Different variable usage pattern */
        *ptr2 = v7 + v8;
        int temp = v1 + v2 + v3;
        
        /* Call with different arguments */
        int ret = external_func(v7, v8, v1, v2, v3, v4);
        
        r2 = r2 + ret + temp;
    }
    
    /* Use register variables at the end */
    return r1 + r2 + *ptr1 + *ptr2 + global_seed;
}

/* Function 3: Loop with calls and high pressure */
int __attribute__((noinline)) test_loop_pressure(int arg, int iterations) {
    int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int a = arg + i * 1;
        int b = arg + i * 2;
        int c = arg + i * 3;
        int d = arg + i * 4;
        int e = arg + i * 5;
        int f = arg + i * 6;
        int g = arg + i * 7;
        int h = arg + i * 8;
        
        /* Call that clobbers registers inside loop */
#ifdef __x86_64__
        asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi");
#elif defined(__arm__)
        asm volatile("" ::: "r0", "r1", "r2", "r3");
#endif
        
        /* Use all variables after call */
        accum += a + b + c + d + e + f + g + h;
        
        /* Another call with different clobbers */
        if (i % 2 == 0) {
#ifdef __x86_64__
            asm volatile("" ::: "r8", "r9", "r10", "r11");
#elif defined(__arm__)
            asm volatile("" ::: "r12");
#endif
            accum += external_func(a, b, c, d, e, f);
        }
    }
    
    return accum + global_seed;
}

/* Function 4: Mixed caller/callee saved with nested calls */
int __attribute__((noinline)) test_mixed_save(int arg) {
    /* Variables that should use callee-saved registers */
    int callee1 = arg * 10;
    int callee2 = arg * 20;
    int callee3 = arg * 30;
    int callee4 = arg * 40;
    
    /* Variables for caller-saved registers */
    int caller1 = arg + 1000;
    int caller2 = arg + 2000;
    int caller3 = arg + 3000;
    int caller4 = arg + 4000;
    int caller5 = arg + 5000;
    int caller6 = arg + 6000;
    int caller7 = arg + 7000;
    int caller8 = arg + 8000;
    
    /* First call - clobbers caller-saved */
#ifdef __x86_64__
    asm volatile("" ::: "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
#endif
    
    /* Use mix of variables */
    int sum1 = callee1 + callee2 + caller1 + caller2;
    
    /* Second call */
    int ret = external_func(caller3, caller4, caller5, caller6, caller7, caller8);
    
    /* More computation */
    int sum2 = callee3 + callee4 + ret;
    
    /* Third call with different clobbers */
#ifdef __x86_64__
    asm volatile("" ::: "rax", "rcx", "rdx");
#endif
    
    return sum1 + sum2 + callee1 + callee2 + callee3 + callee4 + global_seed;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int iter = (argc > 2) ? atoi(argv[2]) : 5;
    int cond = (argc > 3) ? atoi(argv[3]) : 1;
    
    /* Update global seed to affect all functions */
    global_seed = seed;
    
    /* Run all test functions */
    result += test_high_pressure_seq(seed);
    result += test_branching_pressure(seed, cond);
    result += test_loop_pressure(seed, iter);
    result += test_mixed_save(seed);
    
    /* Also test with different conditions */
    result += test_branching_pressure(seed + 1, 0);
    result += test_loop_pressure(seed + 2, 3);
    
    printf("Result: %d\n", result);
    return result & 0xFF; /* Return non-zero to indicate execution */
}

/* Dummy external function definitions */
void __attribute__((noinline)) clobber_many_regs_x86(void) {
    /* Empty but clobbers registers via asm */
    asm volatile("" ::: "memory");
}

void __attribute__((noinline)) clobber_many_regs_arm(void) {
    asm volatile("" ::: "memory");
}

int __attribute__((noinline)) external_func(int a, int b, int c, int d, int e, int f) {
    /* Simple computation that can't be optimized away */
    volatile int result = (a * b) + (c * d) - (e * f) + global_seed;
    return result;
}
