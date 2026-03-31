/* test_caller_save.c
 * Designed to trigger GCC's caller-save pass instruction reordering
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
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) + 
           (i * j) - (k * l) + (m * n) - (o * p);
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int x, int y, int z) {
    /* Variables that will be used in different branches */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = z * 4;
    int v4 = x + y + z;
    int v5 = x - y - z;
    int v6 = x * y * z;
    int v7 = x ^ y ^ z;
    int v8 = x | y | z;
    int v9 = x & y & z;
    int v10 = ~x + ~y + ~z;
    
    if (x > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables */
        v1 = v2 + v3;
        v4 = v5 * v6;
        
        /* Another call */
        clobber_many_regs_2();
        
        v7 = v8 ^ v9;
        v10 = v1 + v4;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        
        v2 = v3 - v4;
        v5 = v6 / (x ? x : 1);
        
        clobber_many_regs_1();
        
        v8 = v9 | v10;
        v1 = v2 * v3;
    }
    
    /* Force all variables to be used */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with loops creating multiple basic blocks */
int __attribute__((noinline)) test_loops(int n) {
    int i;
    int accum = 0;
    int tmp1 = 1, tmp2 = 2, tmp3 = 3, tmp4 = 4;
    int tmp5 = 5, tmp6 = 6, tmp7 = 7, tmp8 = 8;
    
    for (i = 0; i < n; i++) {
        /* Call inside loop - creates interesting block boundaries */
        if (i % 3 == 0) {
            clobber_many_regs_1();
            tmp1 = tmp2 + tmp3;
            tmp4 = tmp5 * tmp6;
        } else if (i % 3 == 1) {
            clobber_many_regs_2();
            tmp7 = tmp8 ^ tmp1;
            tmp2 = tmp3 - tmp4;
        } else {
            clobber_many_regs_3();
            tmp5 = tmp6 | tmp7;
            tmp8 = tmp1 & tmp2;
        }
        
        /* Use all temporaries */
        accum += tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8;
        
        /* Modify temporaries for next iteration */
        tmp1 += i;
        tmp2 -= i;
        tmp3 *= (i + 1);
        tmp4 ^= i;
        tmp5 |= i;
        tmp6 &= ~i;
        tmp7 = tmp7 >> 1;
        tmp8 = tmp8 << 1;
    }
    
    return accum;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int a, int b, int c, int d, 
                                              int e, int f, int g, int h) {
    /* These will likely use callee-saved registers */
    register long r12 asm("r12") = a + b;
    register long r13 asm("r13") = c + d;
    register long r14 asm("r14") = e + f;
    register long r15 asm("r15") = g + h;
    
    /* More variables for caller-saved registers */
    int x1 = a * 2;
    int x2 = b * 3;
    int x3 = c * 4;
    int x4 = d * 5;
    int x5 = e * 6;
    int x6 = f * 7;
    int x7 = g * 8;
    int x8 = h * 9;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both register types */
    int sum_callee = (int)(r12 + r13 + r14 + r15);
    int sum_caller = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    r12 += x1;
    r13 += x2;
    x3 += (int)r14;
    x4 += (int)r15;
    
    clobber_many_regs_3();
    
    return sum_callee + sum_caller + x3 + x4 + (int)(r12 + r13);
}

/* Main function that runs all tests with varying inputs */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int iter = argc > 2 ? atoi(argv[2]) : 10;
    
    srand(seed);
    
    for (int i = 0; i < iter; i++) {
        /* Generate varying inputs to create different register pressure patterns */
        int x = rand() % 100;
        int y = rand() % 100;
        int z = rand() % 100;
        
        /* Call test functions with different patterns */
        result ^= test_high_pressure(x, y);
        result ^= test_control_flow(x, y, z);
        result ^= test_loops(5 + (rand() % 10));
        result ^= test_mixed_save(x, y, z, x+y, y+z, z+x, x*y, y*z);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
