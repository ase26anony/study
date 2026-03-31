/* caller_save_test.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_x86(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile (
        "mov $0x12345678, %%rax\n\t"
        "mov $0x87654321, %%rcx\n\t"
        "mov $0x11111111, %%rdx\n\t"
        "mov $0x22222222, %%rsi\n\t"
        "mov $0x33333333, %%rdi\n\t"
        "mov $0x44444444, %%r8\n\t"
        "mov $0x55555555, %%r9\n\t"
        "mov $0x66666666, %%r10\n\t"
        "mov $0x77777777, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
}

void __attribute__((noinline)) clobber_many_regs_arm(void) {
    /* ARM version - will be ignored on x86 but provides alternative */
    asm volatile (
        "mov r0, #0x12345678\n\t"
        "mov r1, #0x87654321\n\t"
        "mov r2, #0x11111111\n\t"
        "mov r3, #0x22222222\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "memory"
    );
}

/* Another opaque function */
int __attribute__((noinline)) external_func(int x) {
    return x * 2;
}

/* Test function 1: High register pressure with multiple calls */
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
    
    /* Force variables to be in registers by taking addresses */
    volatile int *ptr_a = &a;
    volatile int *ptr_b = &b;
    
    /* First call that clobbers many registers */
    clobber_many_regs_x86();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f;
    
    /* Second call */
    int tmp = external_func(seed);
    
    /* More variable usage */
    int sum2 = g + h + i + j + k + l;
    
    /* Third call */
    clobber_many_regs_x86();
    
    /* Final usage */
    int sum3 = m + n + o + p + tmp;
    
    /* Complex return to prevent optimization */
    return sum1 + sum2 + sum3 + *ptr_a + *ptr_b;
}

/* Test function 2: Control flow variation */
int __attribute__((noinline)) test_control_flow(int seed, int flag) {
    int v1 = seed * 2;
    int v2 = seed * 3;
    int v3 = seed * 4;
    int v4 = seed * 5;
    int v5 = seed * 6;
    int v6 = seed * 7;
    int v7 = seed * 8;
    int v8 = seed * 9;
    
    if (flag) {
        /* Branch 1: calls and variable usage */
        clobber_many_regs_x86();
        int t1 = v1 + v2 + v3;
        external_func(t1);
        int t2 = v4 + v5 + v6;
        return t1 + t2 + v7 + v8;
    } else {
        /* Branch 2: different pattern */
        int t3 = v1 * v2 * v3;
        clobber_many_regs_x86();
        int t4 = v4 * v5 * v6;
        external_func(t4);
        return t3 + t4 + v7 - v8;
    }
}

/* Test function 3: Loop with calls */
int __attribute__((noinline)) test_loop_calls(int seed, int iterations) {
    int acc = 0;
    int live1 = seed + 1;
    int live2 = seed + 2;
    int live3 = seed + 3;
    int live4 = seed + 4;
    int live5 = seed + 5;
    int live6 = seed + 6;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables across each call */
        acc += live1 + live2;
        clobber_many_regs_x86();
        acc += live3 + live4;
        external_func(acc);
        acc += live5 + live6;
        
        /* Modify live variables to prevent optimization */
        live1 ^= i;
        live2 += i;
        live3 -= i;
        live4 |= i;
        live5 &= ~i;
        live6 ^= ~i;
    }
    
    return acc + live1 + live2 + live3 + live4 + live5 + live6;
}

/* Test function 4: Nested calls with register pressure */
int __attribute__((noinline)) test_nested_pressure(int seed) {
    /* Even more variables */
    register int r1 asm("") = seed * 11;
    register int r2 asm("") = seed * 12;
    register int r3 asm("") = seed * 13;
    int s1 = seed * 14;
    int s2 = seed * 15;
    int s3 = seed * 16;
    int s4 = seed * 17;
    int s5 = seed * 18;
    int s6 = seed * 19;
    int s7 = seed * 20;
    int s8 = seed * 21;
    
    /* Force spilling by taking addresses */
    volatile int *volatile ptr1 = &s1;
    volatile int *volatile ptr2 = &s2;
    
    /* Sequence of calls */
    clobber_many_regs_x86();
    int tmp1 = r1 + r2 + r3;
    
    external_func(tmp1);
    int tmp2 = s1 + s2 + s3 + s4;
    
    clobber_many_regs_x86();
    int tmp3 = s5 + s6 + s7 + s8;
    
    /* Use all variables in final computation */
    return tmp1 + tmp2 + tmp3 + *ptr1 + *ptr2 + r1 - r2 + r3;
}

/* Test function 5: Mixed caller/callee saved usage */
int __attribute__((noinline)) test_mixed_save(int seed) {
    /* Variables that should use callee-saved registers */
    long c1 = seed * 31;
    long c2 = seed * 32;
    long c3 = seed * 33;
    long c4 = seed * 34;
    
    /* Variables for caller-saved registers */
    int x1 = seed + 41;
    int x2 = seed + 42;
    int x3 = seed + 43;
    int x4 = seed + 44;
    int x5 = seed + 45;
    int x6 = seed + 46;
    int x7 = seed + 47;
    int x8 = seed + 48;
    
    /* Use callee-saved variables */
    long callee_sum = c1 + c2 + c3 + c4;
    
    /* Call that clobbers caller-saved */
    clobber_many_regs_x86();
    
    /* Use caller-saved variables */
    int caller_sum = x1 + x2 + x3 + x4;
    
    /* Another call */
    external_func(caller_sum);
    
    /* More usage */
    int more_sum = x5 + x6 + x7 + x8;
    
    /* Final call */
    clobber_many_regs_x86();
    
    return (int)(callee_sum >> 32) + caller_sum + more_sum;
}

int main(int argc, char *argv[]) {
    int result = 0;
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Vary execution paths based on input */
    int flag = (seed % 2) == 0;
    int iterations = 3 + (seed % 5);
    
    printf("Testing caller-save optimization patterns...\n");
    
    /* Run all test functions */
    result ^= test_high_pressure(seed);
    printf("test_high_pressure: %d\n", result);
    
    result ^= test_control_flow(seed, flag);
    printf("test_control_flow: %d\n", result);
    
    result ^= test_loop_calls(seed, iterations);
    printf("test_loop_calls: %d\n", result);
    
    result ^= test_nested_pressure(seed);
    printf("test_nested_pressure: %d\n", result);
    
    result ^= test_mixed_save(seed);
    printf("test_mixed_save: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
