/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must be live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - x + 4;
    int e = d * y + 5;
    int f = e - a + 6;
    int g = f * b + 7;
    int h = g - c + 8;
    int i = h * d + 9;
    int j = i - e + 10;
    int k = j * f + 11;
    int l = k - g + 12;
    int m = l * h + 13;
    int n = m - i + 14;
    int o = n * j + 15;
    int p = o - k + 16;
    int q = p * l + 17;
    int r = q - m + 18;
    int s = r * n + 19;
    int t = s - o + 20;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use all variables after first call */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call */
    clobber_many_regs_2();
    
    /* More computations mixing variables */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) + (i * j) +
           (k * l) - (m * n) + (o * p) - (q * r) + (s * t);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y, int z) {
    int a = x * 2;
    int b = y * 3;
    int c = z * 4;
    int d = a + b;
    int e = b + c;
    int f = c + a;
    
    if (x > y) {
        /* Call in true branch */
        clobber_many_regs_1();
        int g = d * e;
        int h = e * f;
        clobber_many_regs_2();
        return g + h + a + b;
    } else {
        /* Call in false branch */
        clobber_many_regs_2();
        int i = f * d;
        int j = d * c;
        clobber_many_regs_3();
        
        /* Nested condition */
        if (y > z) {
            int k = i * j;
            clobber_many_regs_1();
            return k + i + j + c;
        } else {
            int l = j * i;
            clobber_many_regs_3();
            return l + f + e + d;
        }
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Many live variables across loop iteration */
        int f = a * i + b;
        int g = b * i + c;
        int h = c * i + d;
        
        /* Call inside loop - high pressure */
        clobber_many_regs_1();
        
        /* Use variables after call */
        sum += f + g + h + a + b + c;
        
        /* Update variables for next iteration */
        a = (a + f) & 0xFF;
        b = (b + g) & 0xFF;
        c = (c + h) & 0xFF;
        
        /* Another call */
        if (i % 3 == 0) {
            clobber_many_regs_2();
            d = (d + sum) & 0xFF;
        } else {
            clobber_many_regs_3();
            e = (e + sum) & 0xFF;
        }
    }
    
    return sum + a + b + c + d + e;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_register_usage(int x) {
    /* Variables that might use callee-saved registers */
    register long r12 asm("r12") = x * 2;
    register long r13 asm("r13") = x * 3;
    register long r14 asm("r14") = x * 4;
    register long r15 asm("r15") = x * 5;
    
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
    
    /* Call clobbering caller-saved registers */
    clobber_many_regs_1();
    
    /* Use both register types */
    long sum1 = r12 + r13 + r14 + r15;
    int sum2 = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More computations */
    r12 = r12 * t1;
    r13 = r13 * t2;
    r14 = r14 * t3;
    r15 = r15 * t4;
    
    clobber_many_regs_3();
    
    return (int)(sum1 + sum2 + r12 + r13 + r14 + r15);
}

/* Function with switch statement creating multiple basic blocks */
int __attribute__((noinline)) test_switch_blocks(int x, int op) {
    int a = x * 2;
    int b = x * 3;
    int c = x * 4;
    int d = x * 5;
    int e = x * 6;
    
    switch (op) {
        case 0:
            clobber_many_regs_1();
            return a + b;
        case 1:
            clobber_many_regs_2();
            return b + c + d;
        case 2:
            clobber_many_regs_3();
            return c + d + e;
        case 3:
            clobber_many_regs_1();
            clobber_many_regs_2();
            return a + c + e;
        case 4:
            clobber_many_regs_2();
            clobber_many_regs_3();
            return b + d + e;
        default:
            clobber_many_regs_1();
            clobber_many_regs_2();
            clobber_many_regs_3();
            return a + b + c + d + e;
    }
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Call all test functions with varying inputs */
    result += test_high_pressure(seed, seed + 1);
    result += test_control_flow(seed, seed + 2, seed + 3);
    result += test_loop_pressure(seed + 5);
    result += test_mixed_register_usage(seed + 7);
    result += test_switch_blocks(seed + 11, seed % 6);
    
    /* Additional complex scenario */
    for (int i = 0; i < 3; i++) {
        int a = seed + i * 2;
        int b = seed + i * 3;
        int c = seed + i * 4;
        
        /* Multiple calls in sequence with live variables */
        clobber_many_regs_1();
        int t1 = a * b + c;
        
        clobber_many_regs_2();
        int t2 = b * c + a;
        
        clobber_many_regs_3();
        int t3 = c * a + b;
        
        result += t1 + t2 + t3;
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF; /* Return non-constant result */
}
