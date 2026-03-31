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
volatile int external_var = 0;

/* Function 1: High register pressure with multiple calls */
int __attribute__((noinline)) 
test_high_pressure(int x, int y) {
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
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    return sum1 * 3 + sum2 * 7 + external_var;
}

/* Function 2: Control flow variation with calls in branches */
int __attribute__((noinline))
test_control_flow(int cond, int x) {
    int v1 = x * 2 + 1;
    int v2 = x * 3 + 2;
    int v3 = x * 4 + 3;
    int v4 = x * 5 + 4;
    int v5 = x * 6 + 5;
    int v6 = x * 7 + 6;
    int v7 = x * 8 + 7;
    int v8 = x * 9 + 8;
    
    if (cond > 0) {
        /* Call in true branch */
        clobber_many_regs1();
        v1 = v1 * 2;
        v2 = v2 * 3;
        v3 = v3 * 4;
        
        /* Another call */
        clobber_many_regs2();
        v4 = v4 * 5;
        v5 = v5 * 6;
    } else {
        /* Different calls in false branch */
        clobber_many_regs3();
        v6 = v6 * 7;
        v7 = v7 * 8;
        
        clobber_many_regs1();
        v8 = v8 * 9;
    }
    
    /* Use all variables to keep them live */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + external_var;
}

/* Function 3: Loop with calls and register pressure */
int __attribute__((noinline))
test_loop_pressure(int iterations) {
    int accum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int a = i * 2 + 1;
        int b = i * 3 + 2;
        int c = i * 4 + 3;
        int d = i * 5 + 4;
        int e = i * 6 + 5;
        int f = i * 7 + 6;
        
        /* Call that clobbers registers */
        clobber_many_regs1();
        
        /* Use variables after call */
        accum += a + b + c;
        
        /* Another call */
        clobber_many_regs2();
        
        /* More variable usage */
        accum += d + e + f;
        
        /* Force spill/reload by using many temporaries */
        int t1 = accum * 2;
        int t2 = accum * 3;
        int t3 = accum * 4;
        int t4 = accum * 5;
        
        clobber_many_regs3();
        
        accum = t1 + t2 + t3 + t4;
    }
    
    return accum + external_var;
}

/* Function 4: Mixed caller/callee saved usage with address taking */
int __attribute__((noinline))
test_mixed_save(int x, int y) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("r12") = x * 100;
    register long r2 asm("r13") = y * 200;
    
    /* Many caller-saved variables */
    int a = x + 1;
    int b = y + 2;
    int c = a * b;
    int d = b - a;
    int e = c * d;
    int f = d - c;
    
    /* Take addresses to inhibit optimizations */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs1();
    
    /* Use all variables including register ones */
    *ptr1 = *ptr1 + 1;
    *ptr2 = *ptr2 + 2;
    *ptr3 = *ptr3 + 3;
    
    int result = a + b + c + d + e + f + r1 + r2;
    
    clobber_many_regs2();
    
    return result + external_var;
}

/* Function 5: Nested calls with register pressure */
int __attribute__((noinline))
test_nested_pressure(int x) {
    int v1 = x * 11;
    int v2 = x * 12;
    int v3 = x * 13;
    int v4 = x * 14;
    
    clobber_many_regs1();
    
    int v5 = v1 * 2 + v2;
    int v6 = v3 * 3 + v4;
    
    clobber_many_regs2();
    
    int v7 = v5 * 4 + v6;
    int v8 = v6 * 5 + v5;
    
    clobber_many_regs3();
    
    /* Create dependency chain forcing specific register allocation */
    for (int i = 0; i < 3; i++) {
        v1 = v1 + v2;
        v2 = v2 + v3;
        v3 = v3 + v4;
        v4 = v4 + v5;
        v5 = v5 + v6;
        v6 = v6 + v7;
        v7 = v7 + v8;
        
        if (i % 2 == 0) {
            clobber_many_regs1();
        } else {
            clobber_many_regs2();
        }
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + external_var;
}

/* Main function that calls all tests */
int main(int argc, char **argv) {
    /* Use argv to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    int result = 0;
    
    /* Call all test functions with different parameters */
    result += test_high_pressure(seed, seed * 2);
    result += test_control_flow(seed % 2, seed * 3);
    result += test_loop_pressure(3 + (seed % 5));
    result += test_mixed_save(seed * 4, seed * 5);
    result += test_nested_pressure(seed * 6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* Definitions of clobber functions using inline assembly */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber_many_regs1"
        :
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    external_var++;
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    /* Different clobber pattern */
    asm volatile (
        "# clobber_many_regs2"
        :
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    external_var--;
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    /* Yet another clobber pattern */
    asm volatile (
        "# clobber_many_regs3"
        :
        : 
        : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    external_var += 2;
}
