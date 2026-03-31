/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs_1(void);
extern void __attribute__((noinline)) clobber_many_regs_2(void);
extern void __attribute__((noinline)) clobber_many_regs_3(void);

/* External function that uses many arguments */
extern int __attribute__((noinline)) use_many_args(int, int, int, int, int, int, int, int, int, int);

/* Prevent constant propagation and dead code elimination */
volatile int global_seed = 42;

/* Function 1: High register pressure with multiple calls in sequence */
int __attribute__((noinline)) test_high_pressure_seq(int param) {
    /* Many local variables that must stay live across calls */
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
    
    /* Force these to be in registers by taking addresses */
    int *ptr_a = &a;
    int *ptr_b = &b;
    int *ptr_c = &c;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use variables after first call */
    int sum1 = a + b + c + d;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    int result = sum1 + sum2 + i + j + *ptr_a + *ptr_b + *ptr_c;
    
    /* Prevent tail call optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result;
}

/* Function 2: Register pressure with control flow */
int __attribute__((noinline)) test_control_flow(int param) {
    /* Many variables that compete for registers */
    int v1 = param * 1;
    int v2 = param * 2;
    int v3 = param * 3;
    int v4 = param * 4;
    int v5 = param * 5;
    int v6 = param * 6;
    int v7 = param * 7;
    int v8 = param * 8;
    int v9 = param * 9;
    int v10 = param * 10;
    int v11 = param * 11;
    int v12 = param * 12;
    
    /* Use conditionals to create multiple basic blocks */
    if (param & 1) {
        /* Branch 1: Call and use variables */
        clobber_many_regs_1();
        v1 = v1 + v2 + v3;
        v4 = v4 * v5;
    } else {
        /* Branch 2: Different call pattern */
        clobber_many_regs_2();
        v6 = v6 - v7;
        v8 = v8 / (v9 ? v9 : 1);
    }
    
    /* Common code with high register pressure */
    int temp = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Another call in the common path */
    clobber_many_regs_3();
    
    /* Use all variables to keep them live */
    return temp + v11 + v12 + param;
}

/* Function 3: Nested loops with calls */
int __attribute__((noinline)) test_loop_pressure(int param) {
    int acc = 0;
    
    /* Outer loop creates multiple basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Inner loop with register pressure */
        for (int j = 0; j < 2; j++) {
            /* Many live variables across the call */
            int x1 = param + i + j;
            int x2 = param + i * j;
            int x3 = param - i + j;
            int x4 = param * i + j;
            int x5 = param + i * 2 + j;
            int x6 = param + i + j * 3;
            int x7 = param * i * j;
            int x8 = param + i * 4;
            
            /* Call that clobbers registers */
            if ((i + j) & 1) {
                clobber_many_regs_1();
            } else {
                clobber_many_regs_2();
            }
            
            /* Use all variables after call */
            acc += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
        }
    }
    
    return acc;
}

/* Function 4: Mixed caller/callee saved usage */
int __attribute__((noinline)) test_mixed_save(int param) {
    /* Variables that want to be in callee-saved registers */
    register long c1 asm("") = param + 100;
    register long c2 asm("") = param + 200;
    register long c3 asm("") = param + 300;
    
    /* Variables for caller-saved registers */
    int v1 = param * 13;
    int v2 = param * 17;
    int v3 = param * 19;
    int v4 = param * 23;
    int v5 = param * 29;
    int v6 = param * 31;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use mixed variables */
    int sum1 = v1 + v2 + v3 + (int)c1;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    int sum2 = v4 + v5 + v6 + (int)c2 + (int)c3;
    
    return sum1 * sum2;
}

/* Function 5: Many function arguments forcing spills */
int __attribute__((noinline)) test_many_args(int param) {
    /* Create many values to pass as arguments */
    int a1 = param + 1;
    int a2 = param + 2;
    int a3 = param + 3;
    int a4 = param + 4;
    int a5 = param + 5;
    int a6 = param + 6;
    int a7 = param + 7;
    int a8 = param + 8;
    int a9 = param + 9;
    int a10 = param + 10;
    
    /* Additional local variables that must stay live */
    int l1 = param * 2;
    int l2 = param * 3;
    int l3 = param * 4;
    int l4 = param * 5;
    
    /* Call with many arguments - forces register pressure */
    int result = use_many_args(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    
    /* Use local variables after call */
    return result + l1 + l2 + l3 + l4;
}

/* Simulate external functions using inline asm */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Clobber many caller-saved registers */
    asm volatile(
        "# clobber_many_regs_1\n"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    /* Different clobber pattern */
    asm volatile(
        "# clobber_many_regs_2\n"
        : 
        : 
        : "rax", "rdx", "rcx", "rbx", "rsi", "rdi", 
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    /* Yet another clobber pattern */
    asm volatile(
        "# clobber_many_regs_3\n"
        : 
        : 
        : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

int __attribute__((noinline)) use_many_args(int a1, int a2, int a3, int a4, int a5,
                                           int a6, int a7, int a8, int a9, int a10) {
    /* Use all arguments to prevent optimization */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to vary execution paths */
    int base = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    /* Call all test functions to trigger different caller-save scenarios */
    result ^= test_high_pressure_seq(base);
    result ^= test_control_flow(base + 1);
    result ^= test_loop_pressure(base + 2);
    result ^= test_mixed_save(base + 3);
    result ^= test_many_args(base + 4);
    
    /* Additional complex scenario combining multiple patterns */
    for (int i = 0; i < 2; i++) {
        result += test_high_pressure_seq(base + 10 + i);
        if (i & 1) {
            result -= test_control_flow(base + 20 + i);
        } else {
            result += test_loop_pressure(base + 30 + i);
        }
    }
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    return result & 255;
}
