/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering block
 * (lines 905-913 in caller-save.cc)
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", 
                   "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "r8", "r9", "r10");
}

/* Function with extremely high register pressure around calls */
int __attribute__((noinline)) high_pressure_function(int x, int y) {
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
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 * 3 + sum2 * 5 + a - b + c - d + e - f + g - h +
           i - j + k - l + m - n + o - p + x + y;
}

/* Function with control flow variation */
int __attribute__((noinline)) control_flow_function(int selector, int val) {
    int r1 = val * 2;
    int r2 = val + 100;
    int r3 = val - 50;
    int r4 = val / 3;
    int r5 = val % 7;
    int r6 = val << 2;
    int r7 = val >> 1;
    int r8 = val ^ 0xFF;
    int r9 = val | 0xAA;
    int r10 = val & 0x55;
    
    if (selector > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables in true branch */
        int t1 = r1 + r2 + r3;
        int t2 = r4 * r5 * r6;
        
        clobber_many_regs_2();
        
        int t3 = r7 + r8 + r9 + r10;
        return t1 + t2 + t3 + selector;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        
        /* Different variable usage pattern */
        int f1 = r1 * r3 * r5 * r7 * r9;
        int f2 = r2 + r4 + r6 + r8 + r10;
        
        clobber_many_regs_1();
        
        return f1 - f2 - selector;
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) loop_function(int iterations) {
    int acc = 0;
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int w1 = 6, w2 = 7, w3 = 8, w4 = 9, w5 = 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of caller-saved and callee-saved usage */
        int temp = v1 + v2 + v3 + v4 + v5;
        
        /* Call that clobbers registers */
        if (i % 2 == 0) {
            clobber_many_regs_1();
        } else {
            clobber_many_regs_2();
        }
        
        /* Use variables after call */
        acc += temp + w1 + w2 + w3 + w4 + w5;
        
        /* Modify variables to keep them live */
        v1 += i;
        v2 -= i;
        v3 *= (i + 1);
        v4 ^= i;
        v5 = v5 << 1;
        
        /* Another call */
        if (i % 3 == 0) {
            clobber_many_regs_3();
        }
        
        /* More variable usage */
        w1 = w2 + w3;
        w2 = w3 + w4;
        w3 = w4 + w5;
        w4 = w5 + v1;
        w5 = v1 + v2;
    }
    
    return acc + v1 + v2 + v3 + v4 + v5 + w1 + w2 + w3 + w4 + w5;
}

/* Function that takes addresses to inhibit optimizations */
int __attribute__((noinline)) address_taken_function(int x) {
    int a = x * 2;
    int b = x + 100;
    int c = x - 50;
    int d = x / 3;
    
    /* Taking addresses forces these to memory/stack */
    int *pa = &a;
    int *pb = &b;
    int *pc = &c;
    int *pd = &d;
    
    /* Call that clobbers registers */
    clobber_many_regs_1();
    
    /* Use through pointers */
    int sum = *pa + *pb;
    
    clobber_many_regs_2();
    
    sum += *pc + *pd;
    
    /* More variables */
    int e = sum * 2;
    int f = sum + 100;
    int g = sum - 50;
    int h = sum / 3;
    
    /* Take addresses again */
    int *pe = &e;
    int *pf = &f;
    int *pg = &g;
    int *ph = &h;
    
    clobber_many_regs_3();
    
    return *pe + *pf + *pg + *ph + x;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call high pressure function */
    result += high_pressure_function(seed, seed + 1);
    
    /* Call control flow function with different paths */
    result += control_flow_function(seed % 10, seed);
    
    /* Call loop function */
    result += loop_function(seed % 5 + 3);
    
    /* Call address taken function */
    result += address_taken_function(seed * 2);
    
    /* Additional complex scenario with nested calls */
    for (int i = 0; i < 3; i++) {
        int temp = seed + i * 100;
        result += high_pressure_function(temp, temp + i);
        
        /* Call within loop with many live variables */
        int a = temp * 3, b = temp + 7, c = temp - 9, d = temp / 2;
        clobber_many_regs_1();
        result += a + b + c + d;
        
        int e = a * b, f = c + d, g = b - a, h = d * 3;
        clobber_many_regs_2();
        result += e + f + g + h;
        
        int j = e * f, k = g + h, l = f - e, m = h / 2;
        clobber_many_regs_3();
        result += j + k + l + m;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
