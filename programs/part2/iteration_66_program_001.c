/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#elif defined(__arm__)
#define CLOBBER_LIST "r0", "r1", "r2", "r3"
#elif defined(__aarch64__)
#define CLOBBER_LIST "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15"
#else
#define CLOBBER_LIST "memory"
#endif

/* Function that clobbers many caller-saved registers */
void __attribute__((noinline)) clobber_many_regs(void) {
    /* Use asm to clobber registers without being optimized away */
    asm volatile ("" : : : CLOBBER_LIST);
}

/* Another clobbering function with different name */
void __attribute__((noinline)) clobber_more_regs(int x) {
    asm volatile ("" : : "r"(x) : CLOBBER_LIST);
}

/* Function that uses many variables across calls */
int __attribute__((noinline)) test_high_pressure(int seed) {
    /* Declare many local variables to create register pressure */
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
    
    /* Force variables to be live by taking addresses */
    volatile int *ptr_a = &a;
    volatile int *ptr_b = &b;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobbering function */
    clobber_more_regs(sum1);
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs();
    
    /* Final computation using all variables */
    return sum1 + sum2 + *ptr_a + *ptr_b;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_with_branches(int seed, int flag) {
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
        /* Branch 1: calls and computations */
        clobber_many_regs();
        int temp = v1 + v2 + v3 + v4;
        clobber_more_regs(temp);
        return temp + v5 + v6 + v7 + v8 + v9 + v10;
    } else {
        /* Branch 2: different call pattern */
        clobber_more_regs(v1);
        int temp = v2 + v3 + v4 + v5;
        clobber_many_regs();
        clobber_more_regs(v6);
        return temp + v7 + v8 + v9 + v10;
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int seed, int iterations) {
    int accum = seed;
    
    /* Many live variables inside loop */
    int x1 = seed + 1;
    int x2 = seed + 2;
    int x3 = seed + 3;
    int x4 = seed + 4;
    int x5 = seed + 5;
    int x6 = seed + 6;
    int x7 = seed + 7;
    int x8 = seed + 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Call that clobbers registers inside loop */
        clobber_many_regs();
        
        /* Use all variables to keep them live */
        accum += x1 + x2 + x3 + x4;
        
        /* Another call */
        clobber_more_regs(accum);
        
        /* More variable usage */
        accum += x5 + x6 + x7 + x8;
        
        /* Modify variables to prevent optimization */
        x1 ^= i;
        x2 += i;
    }
    
    return accum;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save_types(int seed) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("") = seed * 100;
    register long r2 asm("") = seed * 200;
    register long r3 asm("") = seed * 300;
    
    /* Many other variables for pressure */
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
    
    /* Take addresses to force stack usage */
    volatile int *ptrs[] = {&a, &b, &c, &d, &e, &f, &g, &h, &i, &j};
    
    /* Sequence of calls */
    clobber_many_regs();
    int sum1 = a + b + c + d + e;
    
    clobber_more_regs(sum1);
    int sum2 = f + g + h + i + j;
    
    clobber_many_regs();
    
    /* Use register variables across calls */
    return (int)(r1 + r2 + r3) + sum1 + sum2 + *ptrs[0];
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Run all test functions to create various patterns */
    result += test_high_pressure(seed);
    result += test_with_branches(seed, argc);
    result += test_with_loop(seed, (argc > 2) ? atoi(argv[2]) : 3);
    result += test_mixed_save_types(seed);
    
    /* Add some unpredictable control flow */
    switch (seed % 4) {
        case 0:
            result += test_high_pressure(result);
            break;
        case 1:
            result += test_with_branches(result, argc);
            break;
        case 2:
            result += test_with_loop(result, 2);
            break;
        default:
            result += test_mixed_save_types(result);
            break;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
