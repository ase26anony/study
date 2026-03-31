/* test_caller_save.c - Program to trigger caller-save instruction reordering */
#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_x86(void) {
    /* Inline asm to clobber many x86-64 caller-saved registers */
    asm volatile (
        "mov $0, %%rax\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
}

void __attribute__((noinline)) clobber_many_regs_arm(void) {
    /* Inline asm to clobber many ARM64 caller-saved registers */
    asm volatile (
        "mov x0, #0\n"
        "mov x1, #0\n"
        "mov x2, #0\n"
        "mov x3, #0\n"
        "mov x4, #0\n"
        "mov x5, #0\n"
        "mov x6, #0\n"
        "mov x7, #0\n"
        "mov x8, #0\n"
        "mov x9, #0\n"
        "mov x10, #0\n"
        "mov x11, #0\n"
        "mov x12, #0\n"
        "mov x13, #0\n"
        "mov x14, #0\n"
        "mov x15, #0\n"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "memory"
    );
}

/* Test function 1: High register pressure with multiple calls in sequence */
int __attribute__((noinline)) test_high_pressure_seq(int seed) {
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
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_x86();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call */
    clobber_many_regs_x86();
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call */
    clobber_many_regs_x86();
    
    /* Final computation using all variables */
    return sum1 + sum2 + i + j + k + l + m + n + o + p;
}

/* Test function 2: Control flow variation with branches */
int __attribute__((noinline)) test_control_flow(int seed, int flag) {
    /* Many variables, some will need callee-saved registers */
    register int r1 asm("") = seed * 1;
    register int r2 asm("") = seed * 2;
    register int r3 asm("") = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    if (flag > 0) {
        /* Branch with call and variable usage */
        clobber_many_regs_x86();
        int temp = r1 + r2 + r3;
        clobber_many_regs_x86();
        return temp + v4 + v5 + v6;
    } else {
        /* Different branch with different call pattern */
        int temp2 = v7 + v8;
        clobber_many_regs_x86();
        temp2 += v9 + v10;
        clobber_many_regs_x86();
        return temp2 + r1;
    }
}

/* Test function 3: Loop with calls inside */
int __attribute__((noinline)) test_loop_calls(int seed, int iterations) {
    int accum = seed;
    
    /* Force many variables to be live in the loop */
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_x86();
        
        /* Use variables to keep them live */
        accum += a + b;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_x86();
            accum += c + d;
        } else {
            accum += e + f;
        }
        
        /* More variable usage */
        accum += g + h;
    }
    
    return accum;
}

/* Test function 4: Mixed caller/callee saved usage with address taking */
int __attribute__((noinline)) test_mixed_save_types(int seed) {
    /* Variables that might get different register assignments */
    int var1 = seed;
    int var2 = seed * 2;
    int var3 = seed * 3;
    int var4 = seed * 4;
    int var5 = seed * 5;
    int var6 = seed * 6;
    int var7 = seed * 7;
    int var8 = seed * 8;
    
    /* Take addresses to inhibit optimizations and force stack slots */
    int *ptr1 = &var1;
    int *ptr2 = &var2;
    int *ptr3 = &var3;
    int *ptr4 = &var4;
    
    /* Call that clobbers registers */
    clobber_many_regs_x86();
    
    /* Use variables through pointers */
    int sum = *ptr1 + *ptr2;
    
    /* Another call */
    clobber_many_regs_x86();
    
    /* More computation */
    sum += var5 + var6 + var7 + var8;
    
    /* Final call */
    clobber_many_regs_x86();
    
    return sum + *ptr3 + *ptr4;
}

/* Test function 5: Nested calls with register pressure */
int __attribute__((noinline)) test_nested_pressure(int seed) {
    /* Even more variables to exhaust registers */
    int v1 = seed;
    int v2 = seed + 11;
    int v3 = seed + 22;
    int v4 = seed + 33;
    int v5 = seed + 44;
    int v6 = seed + 55;
    int v7 = seed + 66;
    int v8 = seed + 77;
    int v9 = seed + 88;
    int v10 = seed + 99;
    int v11 = seed + 111;
    int v12 = seed + 122;
    int v13 = seed + 133;
    int v14 = seed + 144;
    int v15 = seed + 155;
    int v16 = seed + 166;
    
    /* Sequence of calls with variable usage in between */
    int t1 = v1 + v2 + v3;
    clobber_many_regs_x86();
    
    int t2 = v4 + v5 + v6 + v7;
    clobber_many_regs_x86();
    
    int t3 = v8 + v9 + v10 + v11;
    clobber_many_regs_x86();
    
    int t4 = v12 + v13 + v14 + v15 + v16;
    clobber_many_regs_x86();
    
    return t1 + t2 + t3 + t4;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Run all test functions to create various caller-save scenarios */
    result += test_high_pressure_seq(seed);
    result += test_control_flow(seed, argc);
    result += test_loop_calls(seed, (argc > 2) ? atoi(argv[2]) : 3);
    result += test_mixed_save_types(seed);
    result += test_nested_pressure(seed);
    
    /* Also test ARM version if compiled for ARM */
    #ifdef __arm__ || __aarch64__
    clobber_many_regs_arm();
    #endif
    
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return lower byte to avoid large exit codes */
}
