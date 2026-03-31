/* caller-save-test.c
 * Program designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering code (lines 905-913)
 * in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);
extern int __attribute__((noinline)) use_some_args(int, int, int, int, int, int);

/* Prevent inlining and constant propagation */
volatile int global_seed = 42;

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int param) {
    /* Many local variables that must live across calls */
    int a = param + 1;
    int b = param + 2;
    int c = param + 3;
    int d = param + 4;
    int e = param + 5;
    int f = param + 6;
    int g = param + 7;
    int h = param + 8;
    int i = param + 9;
    int j = param + 10;
    int k = param + 11;
    int l = param + 12;
    int m = param + 13;
    int n = param + 14;
    int o = param + 15;
    int p = param + 16;
    
    /* Force these to be in registers by taking addresses */
    int *ptr_a = &a;
    int *ptr_b = &b;
    int *ptr_c = &c;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Second call with different clobber pattern */
    clobber_many_regs2();
    
    /* More computations keeping variables live */
    int prod1 = a * b * c * d;
    int prod2 = e * f * g * h;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final use of all variables */
    return sum1 + sum2 + prod1 + prod2 + *ptr_a + *ptr_b + *ptr_c;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_with_branches(int x, int y) {
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
    
    /* Branch that creates different basic blocks */
    if (x > y) {
        /* Call in one branch */
        clobber_many_regs1();
        
        /* Use variables in this branch */
        v1 = v1 + v2;
        v3 = v3 * v4;
    } else {
        /* Different call in other branch */
        clobber_many_regs2();
        
        /* Different variable usage */
        v5 = v5 + v6;
        v7 = v7 | v8;
    }
    
    /* Common code after branch with another call */
    clobber_many_regs3();
    
    /* Use all variables to keep them live */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int iterations) {
    int acc = 0;
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Many live variables inside loop */
        int a = idx * 2;
        int b = idx * 3;
        int c = idx * 5;
        int d = idx * 7;
        int e = idx * 11;
        int f = idx * 13;
        
        /* Call inside loop - forces spills/reloads each iteration */
        if (idx % 2 == 0) {
            clobber_many_regs1();
        } else {
            clobber_many_regs2();
        }
        
        /* Use variables after call */
        acc += a + b + c + d + e + f;
        
        /* Another call with different pattern */
        if (idx % 3 == 0) {
            clobber_many_regs3();
        }
    }
    
    return acc;
}

/* Function that uses many function arguments (uses argument registers) */
int __attribute__((noinline)) test_many_args(void) {
    /* Call function with many arguments - uses argument passing registers */
    int result = use_some_args(
        global_seed + 1,
        global_seed + 2,
        global_seed + 3,
        global_seed + 4,
        global_seed + 5,
        global_seed + 6
    );
    
    /* More local variables */
    int x1 = result * 2;
    int x2 = result * 3;
    int x3 = result * 4;
    int x4 = result * 5;
    int x5 = result * 6;
    int x6 = result * 7;
    
    /* Call that clobbers registers */
    clobber_many_regs1();
    
    /* Use all variables */
    return x1 + x2 + x3 + x4 + x5 + x6 + result;
}

/* Inline assembly to simulate clobbering functions */
void clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber_many_regs1\n\t"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void clobber_many_regs2(void) {
    /* Different clobber pattern */
    asm volatile (
        "# clobber_many_regs2\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

void clobber_many_regs3(void) {
    /* Yet another clobber pattern */
    asm volatile (
        "# clobber_many_regs3\n\t"
        : 
        : 
        : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

int use_some_args(int a, int b, int c, int d, int e, int f) {
    /* Use all arguments to prevent optimization */
    asm volatile (
        "# use_some_args\n\t"
        : 
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f)
        : "memory"
    );
    return a + b + c + d + e + f;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Run all test functions with different patterns */
    total += test_high_pressure(base);
    total += test_with_branches(base, base / 2);
    total += test_with_loop((base % 10) + 5);  /* Small loop to avoid being too slow */
    total += test_many_args();
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
