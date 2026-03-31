/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void clobber_many_regs_1(void) __attribute__((noinline));
extern void clobber_many_regs_2(void) __attribute__((noinline));
extern void clobber_many_regs_3(void) __attribute__((noinline));

/* External function definitions to prevent inlining */
void clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void clobber_many_regs_2(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
}

void clobber_many_regs_3(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8");
}

/* Test function 1: High register pressure with multiple calls */
int test_high_pressure_1(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = b - a + 4;
    int e = c * d + 5;
    int f = d - c + 6;
    int g = e * f + 7;
    int h = f - e + 8;
    int i = g * h + 9;
    int j = h - g + 10;
    int k = i * j + 11;
    int l = j - i + 12;
    int m = k * l + 13;
    int n = l - k + 14;
    int o = m * n + 15;
    int p = n - m + 16;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 * 3 + sum2 * 7 + a - b + c - d + e - f + g - h +
           i - j + k - l + m - n + o - p + x + y;
}

/* Test function 2: Mix of caller-saved and callee-saved usage */
int test_mixed_save_types(int seed) {
    /* Variables that will compete for callee-saved registers */
    register long r12_val asm("r12") = seed * 3;
    register long r13_val asm("r13") = seed * 5;
    register long r14_val asm("r14") = seed * 7;
    register long r15_val asm("r15") = seed * 11;
    
    /* Many temporary variables for caller-saved pressure */
    int t1 = seed + 1;
    int t2 = seed + 2;
    int t3 = seed + 3;
    int t4 = seed + 4;
    int t5 = seed + 5;
    int t6 = seed + 6;
    int t7 = seed + 7;
    int t8 = seed + 8;
    int t9 = seed + 9;
    int t10 = seed + 10;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both register-pinned and regular variables */
    long callee_sum = r12_val + r13_val + r14_val + r15_val;
    int caller_sum = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More computations mixing both types */
    return (callee_sum % 1000) + caller_sum + t1 * t2 - t3 * t4;
}

/* Test function 3: Control flow variation with basic block boundaries */
int test_with_control_flow(int x, int y, int mode) {
    int result = 0;
    
    /* Many live variables */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = x + y;
    int v4 = x - y;
    int v5 = x * y;
    int v6 = x ^ y;
    int v7 = x | y;
    int v8 = x & y;
    int v9 = x << 2;
    int v10 = y >> 1;
    
    if (mode > 0) {
        /* Branch 1: call and computation */
        clobber_many_regs_1();
        result = v1 + v2 + v3 + v4;
        
        /* Another call in this branch */
        clobber_many_regs_2();
        result += v5 + v6 + v7;
    } else {
        /* Branch 2: different call pattern */
        clobber_many_regs_3();
        result = v8 + v9 + v10;
        
        /* Multiple calls in sequence */
        clobber_many_regs_1();
        result *= 2;
        clobber_many_regs_2();
        result += v1 - v2;
    }
    
    /* Common code after if-else with all variables live */
    clobber_many_regs_3();
    return result + v1 * v2 - v3 * v4 + v5 / (v6 ? v6 : 1) + v7 | v8;
}

/* Test function 4: Loop with calls and register pressure */
int test_loop_pressure(int iterations) {
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many variables inside loop that must survive calls */
        int a = i * 2;
        int b = i * 3;
        int c = i * 5;
        int d = i * 7;
        int e = i * 11;
        int f = i * 13;
        int g = i * 17;
        int h = i * 19;
        
        /* Call inside loop - forces spills/reloads each iteration */
        if (i % 2 == 0) {
            clobber_many_regs_1();
        } else {
            clobber_many_regs_2();
        }
        
        /* Use variables after call */
        acc += a + b + c + d + e + f + g + h;
        
        /* Another call with different clobber set */
        clobber_many_regs_3();
        
        /* More computation */
        acc ^= (a * b) | (c * d);
    }
    
    return acc;
}

/* Test function 5: Address-taking to inhibit optimizations */
int test_address_taken(int x) {
    /* Take addresses of variables to force stack allocation */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    int *ptrs[] = {&a, &b, &c, &d, &e, &f, &g, &h};
    
    /* Call between address taking and use */
    clobber_many_regs_1();
    
    /* Use through pointers */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += *ptrs[i];
    }
    
    clobber_many_regs_2();
    
    /* More computation */
    return sum + a * b - c * d + e / (f ? f : 1);
}

/* Main function that runs all tests with command-line variation */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command-line arguments to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Run all test functions */
    total += test_high_pressure_1(base, base + 1);
    total += test_mixed_save_types(base + 2);
    total += test_with_control_flow(base, base + 3, (argc > 2) ? atoi(argv[2]) : 0);
    total += test_loop_pressure((argc > 3) ? atoi(argv[3]) % 10 : 5);
    total += test_address_taken(base + 4);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total % 256;
}
