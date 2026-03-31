/* caller-save-test.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering code (lines 905-913)
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#elif defined(__arm__)
#define CLOBBER_LIST "r0", "r1", "r2", "r3"
#else
#define CLOBBER_LIST "memory"
#endif

/* External functions that clobber many registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use asm to clobber caller-saved registers */
    asm volatile ("" : : : CLOBBER_LIST);
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : CLOBBER_LIST);
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : CLOBBER_LIST);
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
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
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) +
           (i * j) - (k * l) + (m * n) - (o * p);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y, int z) {
    /* Variables that will be used in different branches */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = z * 4;
    int v4 = x + y + z;
    int v5 = x * y * z;
    int v6 = x - y - z;
    int v7 = y - z - x;
    int v8 = z - x - y;
    
    if (x > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables */
        v1 = v1 + v2;
        v3 = v3 * v4;
        
        /* Another call */
        clobber_many_regs_2();
        
        v5 = v5 - v6;
        v7 = v7 * v8;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        
        v2 = v2 + v3;
        v4 = v4 * v5;
        
        clobber_many_regs_1();
        
        v6 = v6 - v7;
        v8 = v8 * v1;
    }
    
    /* Common code with more calls */
    clobber_many_regs_2();
    
    /* Force all variables to be live */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int n) {
    int i;
    int accum = 0;
    
    /* Variables that must survive loop iterations */
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    
    for (i = 0; i < n; i++) {
        /* Call inside loop - variables must be saved/restored each iteration */
        clobber_many_regs_1();
        
        /* Use variables in computation */
        accum += a + b - c + d;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
            accum += e * f;
        } else {
            clobber_many_regs_3();
            accum += g / (h + 1);
        }
        
        /* Modify variables to prevent optimization */
        a += i;
        b -= i;
        c *= (i + 1);
        d /= (i + 2);
        e ^= i;
        f |= i;
        g &= ~i;
        h = h << 1;
    }
    
    /* Final use of all variables */
    return accum + a + b + c + d + e + f + g + h;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Take addresses to force stack allocation and inhibit optimizations */
    int var1 = x * 2;
    int var2 = x * 3;
    int var3 = x * 4;
    int var4 = x * 5;
    int var5 = x * 6;
    
    int *ptr1 = &var1;
    int *ptr2 = &var2;
    int *ptr3 = &var3;
    int *ptr4 = &var4;
    int *ptr5 = &var5;
    
    /* Call that clobbers registers */
    clobber_many_regs_1();
    
    /* Use through pointers to force memory operations */
    *ptr1 += *ptr2;
    *ptr3 -= *ptr4;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More pointer operations */
    *ptr5 = *ptr1 + *ptr3;
    
    /* Final call */
    clobber_many_regs_3();
    
    return *ptr1 + *ptr2 + *ptr3 + *ptr4 + *ptr5;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Run test functions with different patterns */
    result += test_high_pressure(seed, seed + 1);
    result += test_control_flow(seed, seed - 1, seed + 2);
    result += test_loop_pressure(seed % 10 + 5);
    result += test_mixed_save(seed * 3);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
