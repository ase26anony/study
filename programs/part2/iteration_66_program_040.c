/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering block
 * in caller-save.cc lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent constant propagation and inlining */
volatile int global_seed = 42;

/* Function 1: High register pressure with single call */
int __attribute__((noinline)) test_high_pressure_single_call(int x) {
    /* Many local variables, all live across the call */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    int i = x + 9;
    int j = x + 10;
    int k = x + 11;
    int l = x + 12;
    int m = x + 13;
    int n = x + 14;
    int o = x + 15;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables after call - forces spills/restores */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
}

/* Function 2: Multiple calls with live variables in between */
int __attribute__((noinline)) test_multiple_calls(int x) {
    int v1 = x * 1;
    int v2 = x * 2;
    int v3 = x * 3;
    int v4 = x * 4;
    int v5 = x * 5;
    
    clobber_many_regs1();
    
    int v6 = v1 + v2;
    int v7 = v3 + v4;
    
    clobber_many_regs2();
    
    int v8 = v5 + v6;
    int v9 = v7 + v8;
    
    clobber_many_regs3();
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

/* Function 3: Mix of caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_register_usage(int x) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("rbx") = x + 100;
    register long r2 asm("r12") = x + 200;
    register long r3 asm("r13") = x + 300;
    
    /* Many caller-saved register variables */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    
    /* Force address taken to inhibit optimizations */
    int *ptr1 = &a;
    int *ptr2 = &b;
    
    clobber_many_regs1();
    
    /* Use all variables */
    int sum = a + b + c + d + e + f;
    sum += (int)(r1 + r2 + r3);
    sum += *ptr1 + *ptr2;
    
    clobber_many_regs2();
    
    return sum + x;
}

/* Function 4: Control flow variation with branches */
int __attribute__((noinline)) test_control_flow(int x, int flag) {
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    
    if (flag) {
        clobber_many_regs1();
        int f = a + b;
        int g = c + d;
        clobber_many_regs2();
        return f + g + e;
    } else {
        clobber_many_regs3();
        int h = b + c;
        int i = d + e;
        clobber_many_regs1();
        return h + i + a;
    }
}

/* Function 5: Loop with calls inside */
int __attribute__((noinline)) test_loop_with_calls(int x, int iterations) {
    int sum = x;
    
    for (int i = 0; i < iterations; i++) {
        int a = sum + i;
        int b = sum + i * 2;
        int c = sum + i * 3;
        
        if (i % 2 == 0) {
            clobber_many_regs1();
        } else {
            clobber_many_regs2();
        }
        
        sum += a + b + c;
        
        /* Force spill by using many temporaries */
        int t1 = sum * 2;
        int t2 = sum * 3;
        int t3 = sum * 4;
        int t4 = sum * 5;
        sum = t1 + t2 + t3 + t4;
    }
    
    clobber_many_regs3();
    return sum;
}

/* Function 6: Nested calls with register pressure */
int __attribute__((noinline)) test_nested_pressure(int x) {
    /* Extreme register pressure */
    int v01 = x + 1;
    int v02 = x + 2;
    int v03 = x + 3;
    int v04 = x + 4;
    int v05 = x + 5;
    int v06 = x + 6;
    int v07 = x + 7;
    int v08 = x + 8;
    int v09 = x + 9;
    int v10 = x + 10;
    int v11 = x + 11;
    int v12 = x + 12;
    int v13 = x + 13;
    int v14 = x + 14;
    int v15 = x + 15;
    
    /* Multiple consecutive calls */
    clobber_many_regs1();
    clobber_many_regs2();
    clobber_many_regs3();
    clobber_many_regs1();
    
    /* Use all variables in complex expression */
    return v01 + v02 + v03 + v04 + v05 + v06 + v07 + v08 +
           v09 + v10 + v11 + v12 + v13 + v14 + v15;
}

/* Inline assembly to simulate clobbering functions */
void clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber many regs\n"
        :
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void clobber_many_regs2(void) {
    /* Different clobber set to force different spill decisions */
    asm volatile (
        "# clobber different regs\n"
        :
        :
        : "rax", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

void clobber_many_regs3(void) {
    /* Minimal clobber but still forces some spills */
    asm volatile (
        "# clobber minimal regs\n"
        :
        :
        : "rax", "rcx", "rdx", "xmm0", "xmm1"
    );
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    /* Call all test functions with different patterns */
    result += test_high_pressure_single_call(seed);
    result += test_multiple_calls(seed + 1);
    result += test_mixed_register_usage(seed + 2);
    result += test_control_flow(seed + 3, seed % 2);
    result += test_loop_with_calls(seed + 4, 5);
    result += test_nested_pressure(seed + 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
