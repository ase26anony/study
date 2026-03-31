/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering
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
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int seed) {
    /* Many local variables, all live across calls */
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
    
    /* First call - many registers need saving */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call - force more spill/reload */
    clobber_many_regs_2();
    
    /* More computation keeping variables live */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + a - b + c - d + e - f + g - h + i - j + k - l + m - n + o - p;
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int x, int y) {
    /* Variables that must survive across calls in different paths */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = x + y;
    int v4 = x - y;
    int v5 = x * y;
    int v6 = x ^ y;
    int v7 = x | y;
    int v8 = x & y;
    
    if (x > y) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables */
        v1 = v1 + v2;
        v3 = v3 * v4;
        
        /* Another call */
        clobber_many_regs_2();
        
        v5 = v5 ^ v6;
        v7 = v7 & v8;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        
        v1 = v1 - v2;
        v3 = v3 / (v4 ? v4 : 1);
        
        clobber_many_regs_1();
        
        v5 = v5 | v6;
        v7 = v7 ^ v8;
    }
    
    /* Force all variables to be used */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Function with loops creating multiple basic blocks */
int __attribute__((noinline)) test_loop_pressure(int iterations) {
    int acc = 0;
    
    /* Many live variables inside loop */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int n = 0; n < iterations; n++) {
        /* Call inside loop - forces repeated save/restore */
        clobber_many_regs_1();
        
        /* Complex computation with all variables */
        acc += a * n + b * (n + 1) + c * (n + 2) + d * (n + 3) + e * (n + 4);
        acc += f * (n + 5) + g * (n + 6) + h * (n + 7) + i * (n + 8) + j * (n + 9);
        
        /* Another call with different clobbers */
        if (n % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* Modify variables to keep them live */
        a += 1; b += 2; c += 3; d += 4; e += 5;
        f += 6; g += 7; h += 8; i += 9; j += 10;
    }
    
    return acc + a + b + c + d + e + f + g + h + i + j;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_register_usage(int x) {
    /* Take addresses to force stack allocation or specific registers */
    int var1 = x, var2 = x * 2, var3 = x * 3, var4 = x * 4;
    int var5 = x * 5, var6 = x * 6, var7 = x * 7, var8 = x * 8;
    
    /* Use addresses to inhibit optimizations */
    int *ptr1 = &var1;
    int *ptr2 = &var2;
    int *ptr3 = &var3;
    int *ptr4 = &var4;
    
    /* Multiple calls with live pointer variables */
    clobber_many_regs_1();
    
    /* Access through pointers */
    *ptr1 += *ptr2;
    *ptr3 += *ptr4;
    
    clobber_many_regs_2();
    
    /* More variables */
    int var9 = var1 + var2;
    int var10 = var3 + var4;
    int var11 = var5 + var6;
    int var12 = var7 + var8;
    
    clobber_many_regs_3();
    
    /* Force all to be used */
    return var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10 + var11 + var12;
}

/* Main function that runs all tests with varying inputs */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int iterations = argc > 2 ? atoi(argv[2]) : 10;
    
    printf("Testing caller-save optimization patterns...\n");
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed);
    printf("Test 1 result: %d\n", result);
    
    /* Test 2: Control flow variations */
    result += test_control_flow(seed, seed / 2);
    printf("Test 2 cumulative: %d\n", result);
    
    /* Test 3: Loop pressure */
    result += test_loop_pressure(iterations);
    printf("Test 3 cumulative: %d\n", result);
    
    /* Test 4: Mixed register usage */
    result += test_mixed_register_usage(seed);
    printf("Test 4 cumulative: %d\n", result);
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
