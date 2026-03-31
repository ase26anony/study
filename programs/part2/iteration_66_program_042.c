/* caller-save-test.c
 * Designed to trigger GCC's caller-save optimization pass to reorder
 * instructions around function calls under high register pressure.
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
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

/* Function that creates high register pressure with multiple calls */
int __attribute__((noinline)) test_high_pressure_1(int seed) {
    /* Many local variables that must live across calls */
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
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d;
    
    /* Second call */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = e + f + g + h;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + i + j + k + l + m + n + o + p;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y) {
    int a = x * 2;
    int b = x * 3;
    int c = x * 4;
    int d = x * 5;
    int e = x * 6;
    int f = x * 7;
    int g = x * 8;
    int h = x * 9;
    
    if (x > y) {
        clobber_many_regs_1();
        a = b + c;
        d = e + f;
        clobber_many_regs_2();
        g = h * 2;
    } else {
        clobber_many_regs_3();
        a = c + d;
        e = f + g;
        clobber_many_regs_1();
        h = a * 3;
    }
    
    /* Force all variables to be used */
    return a + b + c + d + e + f + g + h;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int iterations) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int n = 0; n < iterations; n++) {
        /* Mix of caller-saved and callee-saved usage */
        int temp = a + b + c;
        
        /* Call that clobbers registers */
        clobber_many_regs_1();
        
        /* Use variables to keep them live */
        d = d + temp;
        e = e + n;
        
        /* Another call */
        if (n % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More computations */
        f = f + a;
        g = g + b;
        h = h + c;
        i = i + d;
        j = j + e;
    }
    
    return a + b + c + d + e + f + g + h + i + j;
}

/* Function with mixed register usage (taking addresses forces spills) */
int __attribute__((noinline)) test_mixed_register_usage(int x) {
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    /* Taking addresses forces variables to memory or specific registers */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    clobber_many_regs_1();
    
    /* Use through pointers */
    *ptr1 = *ptr1 + 1;
    *ptr2 = *ptr2 + 2;
    *ptr3 = *ptr3 + 3;
    
    clobber_many_regs_2();
    
    /* More computations */
    d = d + a;
    e = e + b;
    f = f + c;
    
    clobber_many_regs_3();
    
    g = g + d;
    h = h + e;
    
    return a + b + c + d + e + f + g + h;
}

/* Function with switch statement creating multiple basic blocks */
int __attribute__((noinline)) test_switch_blocks(int x) {
    int a = x * 1;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    int g = x * 7;
    int h = x * 8;
    
    switch (x % 4) {
        case 0:
            clobber_many_regs_1();
            a = b + c;
            break;
        case 1:
            clobber_many_regs_2();
            d = e + f;
            break;
        case 2:
            clobber_many_regs_3();
            g = h + a;
            break;
        default:
            clobber_many_regs_1();
            b = c + d;
            break;
    }
    
    /* Force all variables to be live across the switch */
    return a + b + c + d + e + f + g + h;
}

/* Main function that runs all tests with command-line variation */
int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Use command-line arguments to prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Run all test functions to create various register pressure scenarios */
    result += test_high_pressure_1(seed);
    result += test_control_flow(seed, seed + 1);
    result += test_loop_pressure(3 + (seed % 5));
    result += test_mixed_register_usage(seed);
    result += test_switch_blocks(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;
}
