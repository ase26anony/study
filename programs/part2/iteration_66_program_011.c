/* caller-save-test.c
 * Designed to trigger instruction reordering in GCC's caller-save pass
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
}

/* Function with extreme register pressure around calls */
unsigned long __attribute__((noinline)) test_high_pressure(int param) {
    /* Many local variables that must live across calls */
    unsigned long a = param + 1;
    unsigned long b = param + 2;
    unsigned long c = param + 3;
    unsigned long d = param + 4;
    unsigned long e = param + 5;
    unsigned long f = param + 6;
    unsigned long g = param + 7;
    unsigned long h = param + 8;
    unsigned long i = param + 9;
    unsigned long j = param + 10;
    unsigned long k = param + 11;
    unsigned long l = param + 12;
    unsigned long m = param + 13;
    unsigned long n = param + 14;
    unsigned long o = param + 15;
    unsigned long p = param + 16;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    unsigned long sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    unsigned long sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + a * b + c * d + e * f + g * h + i * j + k * l + m * n + o * p;
}

/* Function with control flow variation */
unsigned long __attribute__((noinline)) test_with_branches(int param, int flag) {
    /* Many local variables */
    register unsigned long v1 = param * 1;  /* Hint for register allocation */
    register unsigned long v2 = param * 2;
    unsigned long v3 = param * 3;
    unsigned long v4 = param * 4;
    unsigned long v5 = param * 5;
    unsigned long v6 = param * 6;
    unsigned long v7 = param * 7;
    unsigned long v8 = param * 8;
    
    /* Take addresses to force stack slots for some variables */
    unsigned long *pv3 = &v3;
    unsigned long *pv5 = &v5;
    
    if (flag > 0) {
        /* Branch with calls and variable usage */
        clobber_many_regs_1();
        
        /* Complex computation keeping many vars live */
        v1 = v1 + v2 + *pv3 + v4 + *pv5 + v6 + v7 + v8;
        
        clobber_many_regs_2();
        
        v2 = v1 * v2 * *pv3 * v4;
    } else {
        /* Alternative branch */
        clobber_many_regs_3();
        
        v3 = v1 - v2 + v4 - *pv5;
        
        clobber_many_regs_1();
        
        v4 = v3 * v2 / (v1 + 1);
    }
    
    /* Use all variables at the end */
    return v1 + v2 + v3 + v4 + *pv5 + v6 + v7 + v8;
}

/* Function with loop and calls */
unsigned long __attribute__((noinline)) test_with_loop(int param, int iterations) {
    unsigned long accum = 0;
    
    /* Many live variables inside loop */
    unsigned long a = param + 1;
    unsigned long b = param + 2;
    unsigned long c = param + 3;
    unsigned long d = param + 4;
    unsigned long e = param + 5;
    unsigned long f = param + 6;
    unsigned long g = param + 7;
    unsigned long h = param + 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        /* Use all variables */
        accum += a + b + c + d;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More variable usage */
        accum += e + f + g + h;
        
        /* Modify variables to prevent optimization */
        a += i;
        b += accum;
    }
    
    return accum + a + b + c + d + e + f + g + h;
}

/* Function mixing caller-saved and callee-saved usage */
unsigned long __attribute__((noinline)) test_mixed_save_types(int param) {
    /* Variables that ideally go in callee-saved registers */
    register unsigned long callee1 asm("rbx") = param * 2;
    register unsigned long callee2 asm("rbp") = param * 3;
    register unsigned long callee3 asm("r12") = param * 4;
    
    /* Many caller-saved variables */
    unsigned long caller1 = param + 1;
    unsigned long caller2 = param + 2;
    unsigned long caller3 = param + 3;
    unsigned long caller4 = param + 4;
    unsigned long caller5 = param + 5;
    unsigned long caller6 = param + 6;
    unsigned long caller7 = param + 7;
    unsigned long caller8 = param + 8;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types */
    unsigned long sum1 = callee1 + callee2 + callee3;
    unsigned long sum2 = caller1 + caller2 + caller3 + caller4;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    sum1 += caller5 + caller6 + caller7 + caller8;
    sum2 += callee1 * callee2 * callee3;
    
    /* Final call */
    clobber_many_regs_3();
    
    return sum1 + sum2 + callee1 + callee2 + callee3;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    unsigned long total = 0;
    
    /* Use argc to vary behavior and prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Run all test functions */
    total += test_high_pressure(base);
    total += test_with_branches(base, argc);
    total += test_with_loop(base, argc > 2 ? atoi(argv[2]) % 10 : 5);
    total += test_mixed_save_types(base);
    
    /* Additional complex scenario with nested calls */
    for (int i = 0; i < 3; i++) {
        total += test_high_pressure(base + i);
        total += test_with_branches(base + i, i);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lu\n", total);
    
    return (int)(total % 256);
}
