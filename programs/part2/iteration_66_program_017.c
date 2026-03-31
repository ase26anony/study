/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute lines 905-913 in caller-save.cc
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
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) + (i * j) +
           (k * l) - (m * n) + (o * p) - (q * r) + (s * t);
}

/* Function with control flow variations */
int __attribute__((noinline)) control_flow_function(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        /* Many live variables in this branch */
        int a1 = x + 1, b1 = y + 2, c1 = z + 3;
        int d1 = a1 * b1, e1 = b1 * c1, f1 = c1 * a1;
        
        clobber_many_regs_1();
        
        /* Use variables after call */
        result += a1 + b1 + c1 + d1 + e1 + f1;
        
        if (y > 0) {
            /* Nested branch with more variables */
            int g1 = d1 + 1, h1 = e1 + 2, i1 = f1 + 3;
            int j1 = g1 * h1, k1 = h1 * i1, l1 = i1 * g1;
            
            clobber_many_regs_2();
            
            result += g1 + h1 + i1 + j1 + k1 + l1;
        }
    } else {
        /* Alternative branch with different pressure */
        int a2 = x - 1, b2 = y - 2, c2 = z - 3;
        int d2 = a2 * b2, e2 = b2 * c2, f2 = c2 * a2;
        int g2 = d2 + 4, h2 = e2 + 5, i2 = f2 + 6;
        
        clobber_many_regs_3();
        
        result += a2 + b2 + c2 + d2 + e2 + f2 + g2 + h2 + i2;
    }
    
    return result;
}

/* Function with loop and calls */
int __attribute__((noinline)) loop_function(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Variables that must be preserved across loop iterations */
        int a = i * 2 + 1;
        int b = i * 3 + 2;
        int c = i * 5 + 3;
        int d = a * b + c;
        int e = b * c + a;
        int f = c * a + b;
        
        /* Call inside loop - variables must be saved/restored */
        if (i % 2 == 0) {
            clobber_many_regs_1();
        } else {
            clobber_many_regs_2();
        }
        
        /* Complex use of variables to keep them live */
        total += a * d - b * e + c * f;
        
        /* Additional variables for next iteration */
        int g = total + i;
        int h = g * a;
        int j = h - b;
        
        clobber_many_regs_3();
        
        total += g + h + j;
    }
    
    return total;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) mixed_register_function(int x) {
    /* Force some variables to potentially use callee-saved registers
     * by taking their addresses */
    int callee_var1 = x * 2;
    int callee_var2 = x * 3;
    int callee_var3 = x * 5;
    int callee_var4 = x * 7;
    
    /* Use addresses to inhibit optimizations */
    int *ptr1 = &callee_var1;
    int *ptr2 = &callee_var2;
    int *ptr3 = &callee_var3;
    int *ptr4 = &callee_var4;
    
    /* Many caller-saved register variables */
    int caller_var1 = x + 1;
    int caller_var2 = x + 2;
    int caller_var3 = x + 3;
    int caller_var4 = x + 4;
    int caller_var5 = x + 5;
    int caller_var6 = x + 6;
    int caller_var7 = x + 7;
    int caller_var8 = x + 8;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use both caller and callee variables */
    int sum1 = *ptr1 + *ptr2 + *ptr3 + *ptr4;
    int sum2 = caller_var1 + caller_var2 + caller_var3 + caller_var4 +
               caller_var5 + caller_var6 + caller_var7 + caller_var8;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More complex usage */
    return sum1 * sum2 + (*ptr1 * caller_var1) - (*ptr2 * caller_var2) +
           (*ptr3 * caller_var3) - (*ptr4 * caller_var4);
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call high pressure function */
    result += high_pressure_function(seed, seed + 1);
    
    /* Call control flow function */
    result += control_flow_function(seed, seed % 10, seed % 20);
    
    /* Call loop function */
    result += loop_function(seed % 5 + 3);
    
    /* Call mixed register function */
    result += mixed_register_function(seed);
    
    /* Additional complex scenario with multiple calls in sequence */
    for (int i = 0; i < 3; i++) {
        int a = seed + i * 2;
        int b = seed + i * 3;
        int c = seed + i * 5;
        int d = a * b + c;
        int e = b * c + a;
        int f = c * a + b;
        int g = d * e + f;
        int h = e * f + d;
        int j = f * d + e;
        
        clobber_many_regs_1();
        clobber_many_regs_2();
        
        result += a + b + c + d + e + f + g + h + j;
        
        clobber_many_regs_3();
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
