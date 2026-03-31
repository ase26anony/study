/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must be live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - a + 4;
    int e = d * 5 + b;
    int f = e / (a + 1) + 6;
    int g = f << 2 + 7;
    int h = g ^ a + 8;
    int i = h | b + 9;
    int j = i & c + 10;
    int k = j - d + 11;
    int l = k * e + 12;
    int m = l / f + 13;
    int n = m << 1 + 14;
    int o = n ^ g + 15;
    int p = o | h + 16;
    int q = p & i + 17;
    int r = q - j + 18;
    int s = r * k + 19;
    int t = s / l + 20;
    
    /* Call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t;
    
    /* Another call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More computation keeping variables live */
    result += (a * b) - (c / d) + (e % f) ^ (g & h) | (i << 2);
    
    return result;
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int seed, int iter) {
    int a = seed * 2;
    int b = seed + 3;
    int c = seed - 4;
    int d = seed / 2;
    int e = seed % 7;
    int f = seed ^ 0xFF;
    int g = seed | 0xAA;
    int h = seed & 0x55;
    
    int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        if (i % 3 == 0) {
            clobber_many_regs_1();
            sum += a + b + c;
        } else if (i % 3 == 1) {
            clobber_many_regs_2();
            sum += d + e + f;
        } else {
            clobber_many_regs_3();
            sum += g + h + i;
        }
        
        /* Mix in more variables to increase pressure */
        a += i;
        b -= i * 2;
        c ^= i;
        d |= i;
        e &= ~i;
        f = f * 2 + i;
        g = g / 2 - i;
        h = h % 20 + i;
    }
    
    return sum + a + b + c + d + e + f + g + h;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save_types(int x) {
    /* Variables that might use callee-saved registers */
    register long r12 asm("r12") = x * 2;
    register long r13 asm("r13") = x * 3;
    register long r14 asm("r14") = x * 4;
    register long r15 asm("r15") = x * 5;
    
    /* Many caller-saved variables */
    int a = x + 100;
    int b = x * 2 + 200;
    int c = x * 3 + 300;
    int d = x * 4 + 400;
    int e = x * 5 + 500;
    int f = x * 6 + 600;
    int g = x * 7 + 700;
    int h = x * 8 + 800;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types */
    int result = (int)(r12 + r13 + r14 + r15);
    result += a + b + c + d + e + f + g + h;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    r12 += a;
    r13 += b;
    result += (int)(r12 * r13);
    
    return result;
}

/* Function with multiple consecutive calls */
int __attribute__((noinline)) test_consecutive_calls(int x) {
    int v1 = x * 11;
    int v2 = x * 12;
    int v3 = x * 13;
    int v4 = x * 14;
    int v5 = x * 15;
    int v6 = x * 16;
    int v7 = x * 17;
    int v8 = x * 18;
    int v9 = x * 19;
    int v10 = x * 20;
    
    /* Sequence of calls with live variables between */
    clobber_many_regs_1();
    int t1 = v1 + v2 + v3;
    
    clobber_many_regs_2();
    int t2 = v4 + v5 + v6 + t1;
    
    clobber_many_regs_3();
    int t3 = v7 + v8 + v9 + v10 + t2;
    
    clobber_many_regs_1();
    int t4 = t1 * t2 + t3;
    
    clobber_many_regs_2();
    int t5 = t3 - t2 + t4;
    
    return t1 + t2 * 2 + t3 * 3 + t4 * 4 + t5 * 5;
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int iter = argc > 2 ? atoi(argv[2]) : 5;
    
    int result = 0;
    
    /* Test different patterns to maximize coverage */
    result += test_high_pressure(seed, seed + 1);
    result += test_control_flow(seed, iter);
    result += test_mixed_save_types(seed);
    result += test_consecutive_calls(seed);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
