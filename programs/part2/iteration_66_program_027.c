/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent constant propagation and dead code elimination */
volatile int global_seed = 42;

/* Function 1: High register pressure with multiple calls */
int __attribute__((noinline)) 
test_high_pressure(int x) {
    /* Many live variables across calls */
    int a = x + 1;
    int b = x * 2;
    int c = x - 3;
    int d = x / 4;
    int e = x % 5;
    int f = x << 1;
    int g = x >> 2;
    int h = x | 0xFF;
    int i = x & 0xF0;
    int j = x ^ 0xAA;
    
    /* First call clobbers caller-saved regs */
    clobber_many_regs1();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c;
    
    /* Second call with different clobbers */
    clobber_many_regs2();
    
    /* More variable usage */
    int sum2 = d + e + f;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + g + h + i + j + a + b + c + d + e + f;
}

/* Function 2: Mix of caller/callee saved with control flow */
int __attribute__((noinline))
test_mixed_save(int x, int y) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("") = x * 100;
    register long r2 asm("") = y * 200;
    
    /* Many temporary variables for caller-saved pressure */
    int t1 = x + y;
    int t2 = x - y;
    int t3 = x * y;
    int t4 = x ^ y;
    int t5 = x & y;
    int t6 = x | y;
    int t7 = ~x;
    int t8 = ~y;
    
    if (x > y) {
        /* Branch with call and variable usage */
        clobber_many_regs1();
        t1 = t1 * 2;
        t2 = t2 + 1;
    } else {
        /* Other branch with different call */
        clobber_many_regs2();
        t3 = t3 / 2;
        t4 = t4 | 1;
    }
    
    /* Force address taken to inhibit optimizations */
    int *ptrs[] = {&t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8};
    
    /* Another call after control flow */
    clobber_many_regs3();
    
    /* Complex computation using all variables */
    return (int)(r1 + r2) + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + 
           ptrs[0][0] + ptrs[1][0];
}

/* Function 3: Loop with calls and register pressure */
int __attribute__((noinline))
test_loop_pressure(int iterations) {
    int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int a = i * 2;
        int b = i + 1;
        int c = i - 1;
        int d = i ^ 0x55;
        int e = i & 0xAA;
        
        /* Call inside loop - forces spills/reloads each iteration */
        clobber_many_regs1();
        
        /* Use variables after call */
        accum += a + b + c + d + e;
        
        /* Another call with different clobbers */
        if (i % 2 == 0) {
            clobber_many_regs2();
            accum += i * 3;
        }
    }
    
    /* Final call outside loop */
    clobber_many_regs3();
    
    return accum;
}

/* Function 4: Nested calls with register pressure */
int __attribute__((noinline))
test_nested_pressure(int x) {
    /* Create deep live range across multiple calls */
    int v1 = x * 11;
    int v2 = x * 13;
    int v3 = x * 17;
    int v4 = x * 19;
    int v5 = x * 23;
    
    clobber_many_regs1();
    
    int w1 = v1 + v2;
    int w2 = v3 + v4;
    
    clobber_many_regs2();
    
    int z1 = w1 * w2;
    int z2 = v5 * 2;
    
    clobber_many_regs3();
    
    /* Force all variables to be used */
    return z1 + z2 + v1 + v2 + v3 + v4 + v5 + w1 + w2;
}

/* Function 5: Switch statement with calls in different cases */
int __attribute__((noinline))
test_switch_pressure(int x, int mode) {
    int result = 0;
    
    switch (mode % 4) {
        case 0: {
            int a = x + 1, b = x + 2, c = x + 3;
            clobber_many_regs1();
            result = a + b + c;
            break;
        }
        case 1: {
            int d = x * 2, e = x * 3, f = x * 4;
            clobber_many_regs2();
            result = d + e + f;
            break;
        }
        case 2: {
            int g = x - 1, h = x - 2, i = x - 3;
            clobber_many_regs3();
            result = g + h + i;
            break;
        }
        default: {
            int j = x / 2, k = x / 3, l = x / 4;
            /* Multiple calls in default case */
            clobber_many_regs1();
            clobber_many_regs2();
            result = j + k + l;
            break;
        }
    }
    
    return result;
}

/* External function implementations using inline asm to clobber registers */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber_many_regs1"
        :
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    /* Different clobber set */
    asm volatile (
        "# clobber_many_regs2"
        :
        :
        : "rax", "rdx", "rcx", "rbx", "rsi", "rdi", "r8", "r9",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    /* Yet another clobber pattern */
    asm volatile (
        "# clobber_many_regs3"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    /* Run all test functions with different patterns */
    result += test_high_pressure(seed);
    result += test_mixed_save(seed, seed * 2);
    result += test_loop_pressure(seed % 10 + 5);
    result += test_nested_pressure(seed + 1);
    result += test_switch_pressure(seed, argc);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
