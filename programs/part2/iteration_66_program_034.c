/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass to reorder
 * instructions, specifically covering lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extremely high register pressure around calls */
int __attribute__((noinline)) high_pressure_function(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - a + 4;
    int e = d * b + 5;
    int f = e - c + 6;
    int g = f * a + 7;
    int h = g - d + 8;
    int i = h * e + 9;
    int j = i - f + 10;
    int k = j * g + 11;
    int l = k - h + 12;
    int m = l * i + 13;
    int n = m - j + 14;
    int o = n * k + 15;
    int p = o - l + 16;
    int q = p * m + 17;
    int r = q - n + 18;
    int s = r * o + 19;
    int t = s - p + 20;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More computations keeping many variables live */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) + (i * j) -
           (k * l) + (m * n) - (o * p) + (q * r) - (s * t);
}

/* Function with control flow variations */
int __attribute__((noinline)) control_flow_function(int x, int argc) {
    int a = x * 2;
    int b = x + 3;
    int c = x - 4;
    int d = x * x + 5;
    int e = x / 2 + 6;
    int f = x % 7 + 8;
    int g = x << 2 + 9;
    int h = x >> 1 + 10;
    
    /* Conditional with calls in both branches */
    if (argc > 2) {
        clobber_many_regs_1();
        a = b + c;
        d = e * f;
        clobber_many_regs_2();
        g = h + a;
    } else {
        clobber_many_regs_2();
        b = c + d;
        e = f * g;
        clobber_many_regs_3();
        h = a + b;
    }
    
    /* Loop with calls inside */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            clobber_many_regs_1();
        } else {
            clobber_many_regs_3();
        }
        sum += a + b + c + d + e + f + g + h;
        a++;
        b--;
    }
    
    return sum;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) mixed_register_function(int x) {
    /* Variables that might use callee-saved registers */
    register long r12_var asm("r12") = x + 100;
    register long r13_var asm("r13") = x + 200;
    register long r14_var asm("r14") = x + 300;
    register long r15_var asm("r15") = x + 400;
    
    /* Many temporary variables using caller-saved registers */
    int t1 = x * 2;
    int t2 = x + 3;
    int t3 = x - 4;
    int t4 = x * 5;
    int t5 = x + 6;
    int t6 = x - 7;
    int t7 = x * 8;
    int t8 = x + 9;
    int t9 = x - 10;
    int t10 = x * 11;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use both caller and callee saved variables */
    int result = (int)(r12_var + r13_var + r14_var + r15_var) +
                 t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    result += (int)(r12_var * 2 - r13_var + r14_var / 2 - r15_var);
    result += t1 * t2 - t3 + t4 / 2 - t5 + t6 * t7 - t8 + t9 / 2 - t10;
    
    return result;
}

/* Function with multiple basic blocks and calls at block boundaries */
int __attribute__((noinline)) multi_block_function(int x, int y) {
    int a = x + y;
    int b = x - y;
    int c = x * y;
    int d = x ^ y;
    
    switch (x % 4) {
        case 0:
            clobber_many_regs_1();
            a = b + c;
            break;
        case 1:
            clobber_many_regs_2();
            b = c + d;
            break;
        case 2:
            clobber_many_regs_3();
            c = d + a;
            break;
        default:
            clobber_many_regs_1();
            clobber_many_regs_2();
            d = a + b;
            break;
    }
    
    /* Create another basic block with call at end */
    int result = 0;
    for (int i = 0; i < 2; i++) {
        result += a + b + c + d;
        if (i == 0) {
            clobber_many_regs_3();
            a++;
            b--;
        } else {
            clobber_many_regs_1();
            c++;
            d--;
        }
    }
    
    return result;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = 0;
    for (int i = 0; i < argc; i++) {
        for (char *p = argv[i]; *p; p++) {
            seed += *p;
        }
    }
    
    /* Call all test functions with varying inputs */
    result += high_pressure_function(seed, argc);
    result += control_flow_function(seed, argc);
    result += mixed_register_function(seed);
    result += multi_block_function(seed, argc);
    
    /* Additional calls to increase pressure */
    for (int i = 0; i < 3; i++) {
        clobber_many_regs_1();
        clobber_many_regs_2();
        clobber_many_regs_3();
        result += i;
    }
    
    printf("Result: %d\n", result);
    return result % 256;
}
