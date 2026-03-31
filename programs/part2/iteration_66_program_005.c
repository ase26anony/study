/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test-caller-save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_x86(void) {
    /* Inline asm to clobber many x86-64 caller-saved registers */
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
    /* Inline asm to clobber many ARM caller-saved registers */
    asm volatile (
        "mov r0, #0x12\n\t"
        "mov r1, #0x34\n\t"
        "mov r2, #0x56\n\t"
        "mov r3, #0x78\n\t"
        "mov r12, #0x9A\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r12", "memory"
    );
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
    
    /* Force variables to be in registers by taking addresses */
    volatile int *ptr_a = &a;
    volatile int *ptr_b = &b;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_x86();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_arm();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_x86();
    
    /* Complex expression using all variables */
    return sum1 + sum2 + (*ptr_a) + (*ptr_b) + (a * b) - (c / d) + (e % f);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int seed, int flag) {
    int v1 = seed * 2;
    int v2 = seed * 3;
    int v3 = seed * 4;
    int v4 = seed * 5;
    int v5 = seed * 6;
    int v6 = seed * 7;
    int v7 = seed * 8;
    int v8 = seed * 9;
    
    if (flag > 0) {
        /* Branch 1: calls with many live variables */
        clobber_many_regs_x86();
        int temp1 = v1 + v2 + v3 + v4;
        clobber_many_regs_arm();
        int temp2 = v5 + v6 + v7 + v8;
        return temp1 + temp2;
    } else {
        /* Branch 2: different call pattern */
        clobber_many_regs_arm();
        int temp3 = v1 * v2 * v3;
        clobber_many_regs_x86();
        int temp4 = v4 * v5 * v6;
        clobber_many_regs_x86();
        return temp3 + temp4 + v7 + v8;
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int seed, int iterations) {
    int accum = seed;
    
    /* Many local variables inside loop */
    for (int i = 0; i < iterations; i++) {
        int x1 = accum + 1;
        int x2 = accum + 2;
        int x3 = accum + 3;
        int x4 = accum + 4;
        int x5 = accum + 5;
        int x6 = accum + 6;
        
        /* Call inside loop with live variables */
        clobber_many_regs_x86();
        
        /* Use variables to keep them live */
        accum += x1 + x2 + x3 + x4 + x5 + x6;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_arm();
        }
    }
    
    return accum;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int seed) {
    /* Variables that might go to callee-saved registers */
    register long r1 asm("rbx") = seed + 100;
    register long r2 asm("rbp") = seed + 200;
    register long r3 asm("r12") = seed + 300;
    
    /* Many caller-saved variables */
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_x86();
    
    /* Use both types */
    long callee_sum = r1 + r2 + r3;
    int caller_sum = a + b + c + d + e + f + g + h;
    
    /* Another call */
    clobber_many_regs_arm();
    
    return (callee_sum % 1000) + caller_sum;
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    int result = 0;
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Use command line to vary paths */
    int flag = (argc > 2) ? atoi(argv[2]) : 0;
    int iterations = (argc > 3) ? atoi(argv[3]) : 5;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed);
    
    /* Test 2: Control flow variation */
    result += test_control_flow(seed, flag);
    
    /* Test 3: Loop with pressure */
    result += test_loop_pressure(seed, iterations);
    
    /* Test 4: Mixed caller/callee saved */
    result += test_mixed_save(seed);
    
    /* Prevent dead code elimination */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
