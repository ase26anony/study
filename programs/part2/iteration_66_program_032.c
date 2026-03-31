/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test-caller-save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use asm to ensure registers are clobbered */
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
    /* Many local variables that must stay live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - x + 4;
    int e = d * y + 5;
    int f = e + a + 6;
    int g = f - b + 7;
    int h = g * c + 8;
    int i = h / (a + 1) + 9;
    int j = i + d + 10;
    int k = j * e + 11;
    int l = k - f + 12;
    int m = l + g + 13;
    int n = m * h + 14;
    int o = n - i + 15;
    int p = o + j + 16;
    int q = p * k + 17;
    int r = q / (l + 1) + 18;
    int s = r + m + 19;
    int t = s * n + 20;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call - they must be preserved */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int prod1 = a * b * c * d % 1000;
    int prod2 = e * f * g * h % 1000;
    int prod3 = i * j * k * l % 1000;
    int prod4 = m * n * o * p % 1000;
    int prod5 = q * r * s * t % 1000;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + prod1 + prod2 + prod3 + prod4 + prod5 + x + y;
}

/* Function with control flow variations */
int __attribute__((noinline)) test_with_branches(int x, int y, int mode) {
    int a = x * 2;
    int b = y * 3;
    int c = a + b;
    int d = b - a;
    int e = c * d;
    int f = e + x;
    int g = f - y;
    int h = g * 2;
    int i = h / 3;
    int j = i + a;
    
    if (mode > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        int k = j * 5;
        int l = k + b;
        int m = l - c;
        
        /* Another call */
        clobber_many_regs_2();
        
        int n = m * d;
        int o = n + e;
        return o + f + g + h + i + j + k + l + m + n;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        int p = j * 7;
        int q = p - b;
        int r = q + c;
        
        clobber_many_regs_1();
        
        int s = r * d;
        int t = s - e;
        return t + f + g + h + i + j + p + q + r + s;
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_with_loop(int x, int iterations) {
    int a = x;
    int b = x + 1;
    int c = x + 2;
    int d = x + 3;
    int e = x + 4;
    
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Many live variables at loop start */
        int f = a + i;
        int g = b * i;
        int h = c - i;
        
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        int j = d + f;
        int k = e + g;
        int l = h * 2;
        
        clobber_many_regs_2();
        
        sum += f + g + h + j + k + l;
        
        /* Modify variables for next iteration */
        a = (a + f) % 100;
        b = (b + g) % 100;
        c = (c + h) % 100;
    }
    
    return sum + a + b + c + d + e;
}

/* Function using addresses to force register allocation decisions */
int __attribute__((noinline)) test_with_addresses(int x, int y) {
    int a = x + 1;
    int b = y + 2;
    int c = a * b;
    int d = c - x;
    int e = d + y;
    
    /* Taking addresses inhibits some optimizations */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    clobber_many_regs_1();
    
    /* Use through pointers */
    int f = *ptr1 + *ptr2;
    int g = *ptr3 * d;
    
    clobber_many_regs_2();
    
    int h = e + f + g;
    
    /* More variables */
    int i = h * 2;
    int j = i / 3;
    int k = j + a;
    int l = k - b;
    int m = l * c;
    
    clobber_many_regs_3();
    
    return m + d + e + f + g + h + i + j + k + l;
}

/* Main function that exercises all test patterns */
int main(int argc, char *argv[]) {
    /* Use argc to vary computations and prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    int result = 0;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed + 1);
    
    /* Test 2: With branches */
    result += test_with_branches(seed, seed + 2, seed % 2);
    
    /* Test 3: With loop */
    result += test_with_loop(seed, (seed % 5) + 3);
    
    /* Test 4: With address taking */
    result += test_with_addresses(seed, seed + 3);
    
    /* Additional mixed test */
    for (int i = 0; i < 3; i++) {
        result += test_high_pressure(seed + i, result % 100);
    }
    
    printf("Result: %d\n", result);
    return result % 256;
}
