/* caller-save-test.c
 * Designed to trigger GCC's caller-save optimization pass instruction reordering
 * Specifically targets lines 905-913 in caller-save.cc
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

/* Function that creates high register pressure around calls */
int __attribute__((noinline)) high_pressure_function(int seed) {
    /* Many local variables that must be live across calls */
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
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a * b - c + d / (e + 1) + f * g - h + i * j + k - l + m * n - o + p;
}

/* Function with control flow variation */
int __attribute__((noinline)) control_flow_function(int x, int y) {
    /* Variables that must live across calls in different branches */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = x + y;
    int v4 = x - y;
    int v5 = x * y;
    int v6 = x / (y + 1);
    int v7 = y * y;
    int v8 = x * x;
    int v9 = v1 + v2;
    int v10 = v3 * v4;
    
    if (x > y) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables */
        v1 = v1 + v3;
        v2 = v2 + v4;
        
        /* Another call */
        clobber_many_regs_2();
        
        v5 = v5 * v6;
        v7 = v7 + v8;
    } else {
        /* Different calls in false branch */
        clobber_many_regs_3();
        
        v9 = v9 - v10;
        v3 = v3 * 2;
        
        clobber_many_regs_1();
        
        v4 = v4 / 2;
        v6 = v6 + 1;
    }
    
    /* Use all variables after conditional */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with loop and calls */
int __attribute__((noinline)) loop_function(int iterations) {
    int acc = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int n = 0; n < iterations; n++) {
        /* Many live variables across loop iteration */
        int temp = a + b + c + d + e;
        
        /* Call that clobbers registers */
        clobber_many_regs_1();
        
        /* Use variables */
        f = f + temp;
        g = g - n;
        
        /* Another call */
        if (n % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        h = h * (n + 1);
        i = i + a;
        j = j - b;
        
        acc += f + g + h + i + j;
        
        /* Modify some variables for next iteration */
        a = a + 1;
        b = b - 1;
        c = c * 2;
    }
    
    return acc + a + b + c + d + e + f + g + h + i + j;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) mixed_register_pressure(int x) {
    /* Variables that might use callee-saved registers */
    register long r12_val asm("r12") = x * 2;
    register long r13_val asm("r13") = x * 3;
    register long r14_val asm("r14") = x * 4;
    register long r15_val asm("r15") = x * 5;
    
    /* Many temporary variables using caller-saved registers */
    int t1 = x + 1;
    int t2 = x + 2;
    int t3 = x + 3;
    int t4 = x + 4;
    int t5 = x + 5;
    int t6 = x + 6;
    int t7 = x + 7;
    int t8 = x + 8;
    int t9 = x + 9;
    int t10 = x + 10;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types of variables */
    long callee_sum = r12_val + r13_val + r14_val + r15_val;
    int caller_sum = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More computations */
    r12_val = r12_val * 2;
    r13_val = r13_val + caller_sum;
    
    return (int)(callee_sum + r12_val + r13_val + r14_val + r15_val) + caller_sum;
}

/* Function with switch statement for more control flow */
int __attribute__((noinline)) switch_function(int mode) {
    int result = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    switch (mode % 4) {
        case 0:
            clobber_many_regs_1();
            result = a + b + c;
            clobber_many_regs_2();
            result += d + e + f;
            break;
        case 1:
            clobber_many_regs_2();
            result = g + h + i + j;
            clobber_many_regs_3();
            result += a * b * c;
            break;
        case 2:
            clobber_many_regs_3();
            result = d * e * f;
            clobber_many_regs_1();
            result += g * h * i;
            break;
        case 3:
            clobber_many_regs_1();
            result = j * a * b;
            clobber_many_regs_2();
            result += c * d * e;
            clobber_many_regs_3();
            result += f * g * h;
            break;
    }
    
    /* Use all variables at the end */
    return result + a + b + c + d + e + f + g + h + i + j;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command line arguments to prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Call all test functions to create various register pressure scenarios */
    result += high_pressure_function(seed);
    result += control_flow_function(seed, seed * 2);
    result += loop_function(seed % 10 + 1);
    result += mixed_register_pressure(seed + 1);
    result += switch_function(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
