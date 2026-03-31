/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test-caller-save.c -o test.o
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
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must stay live across calls */
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
    return sum1 * 2 + sum2 * 3 + (a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k ^ l ^ m ^ n ^ o ^ p);
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int x, int y, int z) {
    int a = x * 2;
    int b = y * 3;
    int c = z * 4;
    int d = a + b;
    int e = b + c;
    int f = c + a;
    
    if (x > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        int g = d * e;
        int h = e * f;
        int i = f * d;
        
        /* Another call */
        clobber_many_regs_2();
        
        a = g + h;
        b = h + i;
        c = i + g;
    } else {
        /* Different calls in false branch */
        clobber_many_regs_3();
        
        int j = d - e;
        int k = e - f;
        int l = f - d;
        
        clobber_many_regs_1();
        
        a = j * k;
        b = k * l;
        c = l * j;
    }
    
    /* Use variables that might be in different registers */
    clobber_many_regs_2();
    
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int iter) {
    int accum = 0;
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    for (int i = 0; i < iter; i++) {
        /* Many live variables at loop start */
        int t1 = v1 * i;
        int t2 = v2 * (i + 1);
        int t3 = v3 * (i + 2);
        
        /* Call that clobbers registers */
        clobber_many_regs_1();
        
        /* Use variables after call */
        accum += t1 + t2 + t3;
        
        /* Update variables keeping them live */
        v1 = t1 ^ v4;
        v2 = t2 ^ v5;
        v3 = t3 ^ v6;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More computations */
        v4 = v1 * v7;
        v5 = v2 * v8;
        v6 = v3 * v9;
        
        /* Final call in loop */
        if (i % 3 == 0) {
            clobber_many_regs_1();
        }
    }
    
    return accum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Variables that might use callee-saved registers */
    register long r12 asm("r12") = x * 2;
    register long r13 asm("r13") = x * 3;
    register long r14 asm("r14") = x * 4;
    
    /* Many caller-saved-only variables */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    /* Call clobbering caller-saved regs */
    clobber_many_regs_1();
    
    /* Use both register types */
    int sum1 = (int)(r12 + r13 + r14);
    int sum2 = a + b + c + d + e + f + g + h;
    
    /* More calls */
    clobber_many_regs_2();
    
    /* Update and use */
    r12 += sum1;
    r13 += sum2;
    a = b * c;
    b = d * e;
    
    clobber_many_regs_3();
    
    return (int)(r12 + r13 + r14) + a + b + sum1 + sum2;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Run test functions with varying inputs */
    result ^= test_high_pressure(seed, seed + 1);
    result ^= test_control_flow(seed, seed + 2, seed + 3);
    result ^= test_loop_pressure(seed % 10 + 5);
    result ^= test_mixed_save(seed + 4);
    
    /* Add more complex calling patterns */
    for (int i = 0; i < 3; i++) {
        result += test_high_pressure(seed + i, seed + i * 2);
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
