/* test-caller-save.c
 * Designed to trigger GCC's caller-save pass instruction reordering
 * (lines 905-913 in caller-save.cc)
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8");
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * 3;
    int d = b * 4;
    int e = c + d;
    int f = e * 5;
    int g = f - a;
    int h = g + b;
    int i = h * c;
    int j = i / (d + 1);
    int k = j << 2;
    int l = k | 0xFF;
    int m = l & 0x0F;
    int n = m ^ e;
    int o = n + f;
    int p = o - g;
    int q = p * h;
    int r = q / (i + 1);
    int s = r << 3;
    int t = s | 0xAA;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call */
    clobber_many_regs_2();
    
    /* More uses */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a - b + c - d + e - f + g - h + i - j +
           k - l + m - n + o - p + q - r + s - t;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y, int z) {
    int a = x * 2;
    int b = y * 3;
    int c = z * 4;
    int d = a + b;
    int e = b + c;
    int f = c + a;
    int g = d * e;
    int h = e * f;
    int i = f * d;
    
    if (x > y) {
        clobber_many_regs_1();
        a = b + c;
        b = c + d;
        c = d + e;
    } else {
        clobber_many_regs_2();
        d = e + f;
        e = f + g;
        f = g + h;
    }
    
    /* Mix of uses to keep variables live */
    int sum = a + b + c + d + e + f + g + h + i;
    
    switch (z % 3) {
        case 0:
            clobber_many_regs_1();
            sum += a * 2;
            break;
        case 1:
            clobber_many_regs_2();
            sum += b * 3;
            break;
        case 2:
            clobber_many_regs_3();
            sum += c * 4;
            break;
    }
    
    return sum + d + e + f;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int k = 0; k < n; k++) {
        /* Many variables live across loop iterations */
        a += k;
        b += a;
        c += b;
        
        /* Call inside loop - high pressure */
        if (k % 2 == 0) {
            clobber_many_regs_1();
        } else {
            clobber_many_regs_2();
        }
        
        d += c;
        e += d;
        f += e;
        g += f;
        
        /* Another call */
        if (k % 3 == 0) {
            clobber_many_regs_3();
        }
        
        h += g;
        i += h;
        j += i;
    }
    
    /* Use all variables to keep them live */
    return a + b + c + d + e + f + g + h + i + j;
}

/* Function using address-taking to inhibit optimizations */
int __attribute__((noinline)) test_address_taken(int x, int y) {
    int a = x + 1;
    int b = y + 2;
    int c = a * b;
    int d = c + x;
    int e = d * y;
    
    /* Take addresses to force stack/memory usage */
    int *pa = &a;
    int *pb = &b;
    int *pc = &c;
    int *pd = &d;
    int *pe = &e;
    
    /* Use pointers to access variables */
    *pa += 1;
    *pb += 2;
    
    clobber_many_regs_1();
    
    *pc += *pa;
    *pd += *pb;
    
    clobber_many_regs_2();
    
    *pe += *pc + *pd;
    
    /* Force all variables to be used */
    asm volatile ("" : : "r"(pa), "r"(pb), "r"(pc), "r"(pd), "r"(pe) : "memory");
    
    return a + b + c + d + e;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to vary computation and prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Run all test functions with varying inputs */
    result += test_high_pressure(seed, seed + 1);
    result += test_control_flow(seed, seed + 2, seed + 3);
    result += test_loop_pressure(seed % 10 + 5);
    result += test_address_taken(seed + 4, seed + 5);
    
    /* Additional complex scenario with nested calls */
    {
        int a = seed, b = seed * 2, c = seed * 3;
        int d = a + b, e = b + c, f = c + a;
        int g = d * e, h = e * f, i = f * d;
        
        clobber_many_regs_1();
        int j = g + h + i + a + b + c + d + e + f;
        
        clobber_many_regs_2();
        int k = j * 2 - a + b - c + d - e + f - g + h - i;
        
        clobber_many_regs_3();
        result += k + j + i + h + g + f + e + d + c + b + a;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
