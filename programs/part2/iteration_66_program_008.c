/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions that clobber caller-saved registers */
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

/* External function to prevent inlining and optimization */
int __attribute__((noinline)) external_func(int x) {
    return x * 2;
}

/* Test function 1: High register pressure with sequential calls */
int __attribute__((noinline)) test_high_pressure_seq(int seed) {
    /* Many local variables all live across calls */
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
    
    /* First call clobbers many registers */
    clobber_many_regs_1();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + i + j + k + l + m + n + o + p + external_func(seed);
}

/* Test function 2: Control flow with branches */
int __attribute__((noinline)) test_branching_pressure(int seed, int flag) {
    /* Many variables that need to survive across calls */
    register int v1 = seed * 1;  /* Hint for register allocation */
    register int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    /* Take addresses to force stack slots */
    int *ptr1 = &v1;
    int *ptr2 = &v2;
    
    if (flag > 0) {
        /* Branch 1: calls with high pressure */
        clobber_many_regs_1();
        v1 = v1 + v3 + v5;
        clobber_many_regs_2();
        v2 = v2 + v4 + v6;
    } else {
        /* Branch 2: different call pattern */
        clobber_many_regs_3();
        v3 = v3 + v7 + v9;
        clobber_many_regs_1();
        v4 = v4 + v8 + v10;
    }
    
    /* Use all variables to keep them live */
    return *ptr1 + *ptr2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 3: Loop with calls inside */
int __attribute__((noinline)) test_loop_pressure(int seed, int iterations) {
    int acc = seed;
    
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
        /* Call that clobbers registers inside loop */
        clobber_many_regs_1();
        
        /* Use variables, keeping them live across call */
        acc += a + b;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
            acc += c + d;
        } else {
            clobber_many_regs_3();
            acc += e + f;
        }
        
        /* More variable usage */
        acc += g + h;
        
        /* Modify variables to prevent optimization */
        a ^= 1;
        b ^= 2;
        c ^= 3;
        d ^= 4;
    }
    
    return acc;
}

/* Test function 4: Nested calls with register pressure */
int __attribute__((noinline)) test_nested_pressure(int seed) {
    /* Even more variables to increase pressure */
    int w1 = seed * 1, w2 = seed * 2, w3 = seed * 3, w4 = seed * 4;
    int x1 = seed * 5, x2 = seed * 6, x3 = seed * 7, x4 = seed * 8;
    int y1 = seed * 9, y2 = seed * 10, y3 = seed * 11, y4 = seed * 12;
    int z1 = seed * 13, z2 = seed * 14, z3 = seed * 15, z4 = seed * 16;
    
    /* Sequence of calls with variable usage in between */
    int t1 = w1 + w2 + w3 + w4;
    clobber_many_regs_1();
    
    int t2 = x1 + x2 + x3 + x4;
    clobber_many_regs_2();
    
    int t3 = y1 + y2 + y3 + y4;
    clobber_many_regs_3();
    
    int t4 = z1 + z2 + z3 + z4;
    clobber_many_regs_1();
    
    /* Use all intermediate results */
    return t1 + t2 + t3 + t4 + external_func(seed);
}

/* Test function 5: Switch statement with different call patterns */
int __attribute__((noinline)) test_switch_pressure(int seed, int choice) {
    /* Variables that must survive across calls in all cases */
    int r1 = seed + 100, r2 = seed + 200, r3 = seed + 300;
    int s1 = seed + 400, s2 = seed + 500, s3 = seed + 600;
    
    switch (choice % 4) {
        case 0:
            clobber_many_regs_1();
            r1 = r1 * 2;
            clobber_many_regs_2();
            r2 = r2 * 3;
            break;
        case 1:
            clobber_many_regs_3();
            r3 = r3 * 4;
            clobber_many_regs_1();
            s1 = s1 * 5;
            break;
        case 2:
            clobber_many_regs_2();
            s2 = s2 * 6;
            clobber_many_regs_3();
            s3 = s3 * 7;
            break;
        default:
            clobber_many_regs_1();
            clobber_many_regs_2();
            clobber_many_regs_3();
            break;
    }
    
    /* Use all variables */
    return r1 + r2 + r3 + s1 + s2 + s3;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Run all test functions to create various pressure patterns */
    result ^= test_high_pressure_seq(seed);
    result ^= test_branching_pressure(seed, argc);
    result ^= test_loop_pressure(seed, 3);
    result ^= test_nested_pressure(seed);
    result ^= test_switch_pressure(seed, argc);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
