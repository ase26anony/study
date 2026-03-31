/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions that clobber caller-saved registers */
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
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + (a * b) - (c / d) + (e % f) ^ (g & h) | (i ^ j) + 
           (k - l) * (m + n) / (o - p) + (q & r) | (s ^ t);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_with_branches(int x, int y, int mode) {
    /* Many variables that compete for registers */
    register int r1 asm("rbx") = x;  /* Hint for callee-saved */
    register int r2 asm("r12") = y;  /* Another callee-saved hint */
    
    int v1 = x * 2;
    int v2 = y + 3;
    int v3 = v1 ^ v2;
    int v4 = v3 << 2;
    int v5 = v4 - x;
    int v6 = v5 | y;
    int v7 = v6 * 3;
    int v8 = v7 / 2;
    int v9 = v8 + v1;
    int v10 = v9 ^ v2;
    
    if (mode > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables to keep them live */
        int branch_sum = v1 + v2 + v3 + v4 + v5 + r1;
        
        /* Another call */
        clobber_many_regs_2();
        
        /* More computation */
        branch_sum += v6 + v7 + v8 + v9 + v10 + r2;
        
        /* Final call in branch */
        clobber_many_regs_3();
        
        return branch_sum + (r1 * r2);
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_2();
        
        int branch_sum = v1 - v2 + v3 - v4 + v5 + r1;
        
        clobber_many_regs_1();
        
        branch_sum += v6 - v7 + v8 - v9 + v10 + r2;
        
        clobber_many_regs_3();
        
        return branch_sum - (r1 * r2);
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int iterations) {
    int accum = 0;
    int live1 = 1, live2 = 2, live3 = 3, live4 = 4, live5 = 5;
    int live6 = 6, live7 = 7, live8 = 8, live9 = 9, live10 = 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        /* Use many live variables */
        accum += live1 + live2 + live3 + live4 + live5;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More variable usage */
        accum += live6 + live7 + live8 + live9 + live10;
        
        /* Modify variables to prevent optimization */
        live1 ^= i;
        live2 += accum;
        live3 -= i * 2;
        live4 |= 0xFF;
        live5 &= 0x0F;
    }
    
    /* Final call after loop */
    clobber_many_regs_1();
    
    return accum + live1 + live2 + live3 + live4 + live5 + 
           live6 + live7 + live8 + live9 + live10;
}

/* Function that takes addresses to inhibit register allocation */
int __attribute__((noinline)) test_with_addresses(int x, int y) {
    int a = x, b = y, c = x + y, d = x * y, e = x ^ y;
    int f = x - y, g = x | y, h = x & y, i = x << 2, j = y >> 1;
    
    /* Take addresses to force stack slots */
    int *ptr1 = &a, *ptr2 = &b, *ptr3 = &c, *ptr4 = &d, *ptr5 = &e;
    
    /* Call that clobbers registers */
    clobber_many_regs_1();
    
    /* Use through pointers */
    int sum = *ptr1 + *ptr2 + *ptr3 + *ptr4 + *ptr5;
    
    /* More variables */
    int k = sum + f, l = sum - g, m = sum * h, n = sum / (i + 1), o = sum % (j + 1);
    int *ptr6 = &f, *ptr7 = &g, *ptr8 = &h, *ptr9 = &i, *ptr10 = &j;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More pointer usage */
    sum += *ptr6 + *ptr7 + *ptr8 + *ptr9 + *ptr10;
    
    /* Final call */
    clobber_many_regs_3();
    
    return sum + k + l + m + n + o + a + b + c + d + e + f + g + h + i + j;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to vary behavior and prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed + 1);
    
    /* Test 2: Branches */
    result += test_with_branches(seed, seed * 2, seed % 3);
    
    /* Test 3: Loop */
    result += test_with_loop(seed % 10 + 5);
    
    /* Test 4: Address taking */
    result += test_with_addresses(seed, seed / 2 + 1);
    
    /* Additional complex scenario mixing everything */
    for (int i = 0; i < 3; i++) {
        int temp = seed + i;
        int a = temp, b = temp * 2, c = temp + 3, d = temp - 4, e = temp ^ 5;
        int f = temp | 6, g = temp & 7, h = temp << 1, i2 = temp >> 2, j = temp * 3;
        
        clobber_many_regs_1();
        
        int sum = a + b + c + d + e + f + g + h + i2 + j;
        
        if (i % 2 == 0) {
            clobber_many_regs_2();
            sum += test_high_pressure(a, b);
        } else {
            clobber_many_regs_3();
            sum += test_with_branches(c, d, e % 2);
        }
        
        result += sum;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
