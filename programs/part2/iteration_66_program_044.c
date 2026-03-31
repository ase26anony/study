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
    int c = x + 3;
    int d = x * 4;
    int e = x + 5;
    int f = x * 6;
    int g = x + 7;
    int h = x * 8;
    int i = x + 9;
    int j = x * 10;
    int k = x + 11;
    int l = x * 12;
    int m = x + 13;
    int n = x * 14;
    int o = x + 15;
    int p = x * 16;
    
    /* First call that clobbers caller-saved regs */
    clobber_many_regs1();
    
    /* Use all variables after call */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call */
    clobber_many_regs2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a - b + c - d + e - f + g - h +
           i - j + k - l + m - n + o - p;
}

/* Function 2: Mix of caller/callee saved with control flow */
int __attribute__((noinline))
test_control_flow(int x, int y) {
    /* Variables that might go to callee-saved registers */
    int callee1 = x * 3;
    int callee2 = y * 7;
    int callee3 = x + y;
    int callee4 = x - y;
    
    /* Many caller-saved candidates */
    int caller1 = x + 1;
    int caller2 = y + 2;
    int caller3 = x * 2;
    int caller4 = y * 3;
    int caller5 = x + 10;
    int caller6 = y + 20;
    int caller7 = x * 5;
    int caller8 = y * 7;
    
    /* Control flow creates multiple basic blocks */
    if (x > y) {
        clobber_many_regs1();
        /* Use mix of variables */
        int tmp = callee1 + callee2 + caller1 + caller2;
        clobber_many_regs2();
        return tmp + caller3 + caller4;
    } else {
        clobber_many_regs2();
        /* Different variable usage pattern */
        int tmp = callee3 + callee4 + caller5 + caller6;
        clobber_many_regs3();
        
        /* Nested condition for more blocks */
        if (x < 0) {
            return tmp + caller7 + caller8;
        } else {
            clobber_many_regs1();
            return tmp - caller7 - caller8;
        }
    }
}

/* Function 3: Loop with calls and register pressure */
int __attribute__((noinline))
test_loop_pressure(int iterations) {
    int acc = 0;
    
    /* Loop creates many live ranges */
    for (int i = 0; i < iterations; i++) {
        /* High pressure within loop body */
        int a = i * 2;
        int b = i + 3;
        int c = i * 5;
        int d = i + 7;
        int e = i * 11;
        int f = i + 13;
        
        /* Call inside loop - forces spills */
        clobber_many_regs1();
        
        /* Use variables after call */
        acc += a + b - c + d - e + f;
        
        /* Another call */
        if (i % 3 == 0) {
            clobber_many_regs2();
            acc += i;
        }
    }
    
    clobber_many_regs3();
    return acc;
}

/* Function 4: Address-taking forces specific register allocation */
int __attribute__((noinline))
test_address_taken(int x) {
    /* Taking addresses inhibits optimizations */
    int v1 = x + 1, v2 = x + 2, v3 = x + 3, v4 = x + 4;
    int v5 = x + 5, v6 = x + 6, v7 = x + 7, v8 = x + 8;
    
    /* Force variables to memory/stack */
    int *p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4;
    int *p5 = &v5, *p6 = &v6, *p7 = &v7, *p8 = &v8;
    
    /* Calls between address uses */
    clobber_many_regs1();
    
    /* Dereference and use */
    int sum = *p1 + *p2 + *p3 + *p4;
    
    clobber_many_regs2();
    
    sum += *p5 + *p6 + *p7 + *p8;
    
    clobber_many_regs3();
    
    /* More computation to keep values live */
    return sum + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8;
}

/* Function 5: Complex expression with many temporaries */
int __attribute__((noinline))
test_complex_expr(int x, int y, int z) {
    /* Create many intermediate values */
    int t1 = x * y + z;
    int t2 = y * z + x;
    int t3 = z * x + y;
    int t4 = x * x + y * y;
    int t5 = y * y + z * z;
    int t6 = z * z + x * x;
    int t7 = t1 + t2 + t3;
    int t8 = t4 + t5 + t6;
    
    /* Call in the middle of computation */
    clobber_many_regs1();
    
    int t9 = t7 * t8 - x;
    int t10 = t8 * t7 - y;
    
    clobber_many_regs2();
    
    int t11 = t9 + t10 + z;
    int t12 = t10 - t9 - x;
    
    clobber_many_regs3();
    
    return t11 * t12 + t1 - t2 + t3 - t4 + t5 - t6 + t7 - t8;
}

/* Inline assembly to clobber caller-saved registers */
void clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber caller-saved regs\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void clobber_many_regs2(void) {
    asm volatile (
        "# clobber more caller-saved regs\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

void clobber_many_regs3(void) {
    asm volatile (
        "# clobber remaining caller-saved regs\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use argc to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : global_seed;
    
    /* Run all test functions with different parameters */
    result += test_high_pressure(seed);
    result += test_control_flow(seed, seed * 2);
    result += test_loop_pressure(seed % 10 + 5);
    result += test_address_taken(seed + 100);
    result += test_complex_expr(seed, seed + 1, seed + 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0;
}
