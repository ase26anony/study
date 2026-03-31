/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer test_caller_save.c -o test_caller_save
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Use asm to force register clobbering */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    /* Clobber different caller-saved registers */
    asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    /* Clobber floating point/vector registers too */
    asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "rax", "rcx");
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
    
    /* First call that clobbers many registers */
    clobber_many_regs1();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call */
    clobber_many_regs2();
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    int sum3 = i + j + k + l + m + n + o + p;
    
    /* Complex computation to prevent optimization */
    return sum1 * 3 + sum2 * 7 + sum3 * 11 + (a ^ b) + (c & d) + (e | f);
}

/* Test function 2: Mix of caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int seed) {
    /* Variables that will compete for registers */
    register int r1 asm("rbx") = seed * 2;  /* Hint for callee-saved */
    register int r2 asm("r12") = seed * 3;  /* Another callee-saved hint */
    
    int v1 = seed + 100;
    int v2 = seed + 200;
    int v3 = seed + 300;
    int v4 = seed + 400;
    int v5 = seed + 500;
    int v6 = seed + 600;
    
    /* Take addresses to force memory/stack usage */
    int *p1 = &v1;
    int *p2 = &v2;
    int *p3 = &v3;
    
    /* Call that clobbers caller-saved regs */
    clobber_many_regs1();
    
    /* Use variables through pointers */
    int sum1 = *p1 + *p2 + *p3;
    
    /* Another call */
    clobber_many_regs2();
    
    /* Use register variables */
    int sum2 = r1 + r2 + v4;
    
    /* Final call */
    clobber_many_regs3();
    
    return sum1 + sum2 * 2 + v5 + v6 + (r1 ^ r2);
}

/* Test function 3: Control flow variation */
int __attribute__((noinline)) test_control_flow(int seed, int flag) {
    int a = seed * 2;
    int b = seed * 3;
    int c = seed * 4;
    int d = seed * 5;
    int e = seed * 6;
    int f = seed * 7;
    int g = seed * 8;
    int h = seed * 9;
    
    if (flag > 0) {
        /* Branch 1: calls and computations */
        clobber_many_regs1();
        int sum1 = a + b + c;
        
        clobber_many_regs2();
        int sum2 = d + e + f;
        
        clobber_many_regs3();
        return sum1 * 5 + sum2 * 3 + g + h;
    } else {
        /* Branch 2: different sequence */
        int temp1 = a * b + c;
        clobber_many_regs1();
        
        int temp2 = d * e + f;
        clobber_many_regs2();
        
        int temp3 = g * h;
        clobber_many_regs3();
        
        return temp1 + temp2 * 2 + temp3 * 3;
    }
}

/* Test function 4: Loop with calls */
int __attribute__((noinline)) test_loop_calls(int seed, int iterations) {
    int acc = seed;
    int v1 = seed + 1;
    int v2 = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    
    for (int i = 0; i < iterations; i++) {
        /* Live variables across loop iteration */
        int temp = v1 + v2 + v3;
        
        clobber_many_regs1();
        
        acc += temp + v4;
        
        clobber_many_regs2();
        
        v1 = (v1 * 3) & 0xFF;
        v2 = (v2 * 5) & 0xFF;
        v3 = (v3 * 7) & 0xFF;
        
        clobber_many_regs3();
        
        acc += v5 + v6 + i;
    }
    
    return acc;
}

/* Test function 5: Nested calls with register pressure */
int __attribute__((noinline)) test_nested_pressure(int seed) {
    /* Even more variables to increase pressure */
    int a1 = seed, a2 = seed*2, a3 = seed*3, a4 = seed*4;
    int b1 = seed+10, b2 = seed+20, b3 = seed+30, b4 = seed+40;
    int c1 = seed-1, c2 = seed-2, c3 = seed-3, c4 = seed-4;
    int d1 = seed^0xAA, d2 = seed^0x55, d3 = seed^0xFF, d4 = seed^0xCC;
    
    /* Sequence of calls with variable usage in between */
    int sum1 = a1 + a2 + a3 + a4;
    clobber_many_regs1();
    
    int sum2 = b1 * b2 - b3 + b4;
    clobber_many_regs2();
    
    int sum3 = c1 | c2 | c3 | c4;
    clobber_many_regs3();
    
    int sum4 = d1 ^ d2 ^ d3 ^ d4;
    clobber_many_regs1();  /* Call again */
    
    /* Force all variables to be used in final computation */
    return sum1 + sum2 + sum3 + sum4 + 
           (a1 & b1) + (a2 | b2) + (a3 ^ b3) + (a4 & b4) +
           (c1 & d1) + (c2 | d2) + (c3 ^ d3) + (c4 & d4);
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments to vary execution paths */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    int flag = (argc > 2) ? atoi(argv[2]) : 1;
    int iterations = (argc > 3) ? atoi(argv[3]) : 3;
    
    /* Run all test functions to create various register pressure scenarios */
    total += test_high_pressure(base);
    total += test_mixed_save(base + 100);
    total += test_control_flow(base + 200, flag);
    total += test_loop_calls(base + 300, iterations);
    total += test_nested_pressure(base + 400);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;  /* Return non-constant value */
}
