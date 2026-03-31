/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering block
 * in caller-save.cc lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - a + 4;
    int e = d * b + 5;
    int f = e / (a + 1) + 6;
    int g = f - c + 7;
    int h = g * d + 8;
    int i = h + e + 9;
    int j = i - f + 10;
    int k = j * g + 11;
    int l = k - h + 12;
    int m = l + i + 13;
    int n = m * j + 14;
    int o = n - k + 15;
    int p = o + l + 16;
    int q = p * m + 17;
    int r = q - n + 18;
    int s = r + o + 19;
    int t = s * p + 20;
    
    /* First call that clobbers many registers */
    clobber_many_regs1();
    
    /* Use all variables after call - forces spills before call */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    
    /* Second call */
    clobber_many_regs2();
    
    /* More variable uses */
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + (a * c) - (b * d) + (e * f) / (g + 1) + 
           (h * i) - (j * k) + (l * m) / (n + 1) + (o * p) - (q * r) + 
           (s * t) / (a + b + 1);
}

/* Function with control flow variations */
int __attribute__((noinline)) test_control_flow(int x, int y, int mode) {
    /* Variables that must live across conditional calls */
    int v1 = x * 2;
    int v2 = y + 3;
    int v3 = v1 - v2;
    int v4 = v1 * v2;
    int v5 = v3 + v4;
    int v6 = v4 - v3;
    int v7 = v5 * v6;
    int v8 = v7 / (v1 + 1);
    int v9 = v8 + v5;
    int v10 = v9 * v6;
    
    if (mode > 0) {
        /* Call in true branch */
        clobber_many_regs1();
        
        /* Use variables */
        v1 = v1 + v2;
        v3 = v3 * v4;
        v5 = v5 - v6;
        
        /* Another call */
        clobber_many_regs2();
        
        v7 = v7 + v8;
        v9 = v9 * v10;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs3();
        
        v2 = v2 - v1;
        v4 = v4 / (v3 + 1);
        v6 = v6 + v5;
        
        clobber_many_regs1();
        
        v8 = v8 * v9;
        v10 = v10 - v7;
    }
    
    /* Common code with more calls */
    clobber_many_regs2();
    
    /* Force all variables to be used */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int iterations) {
    int accum = 0;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Many live variables across loop calls */
        a = a + idx;
        b = b - idx;
        c = c * (idx + 1);
        d = d / (idx + 2);
        e = e + a;
        
        /* Call that clobbers registers inside loop */
        clobber_many_regs1();
        
        f = f + b;
        g = g - c;
        h = h * (d + 1);
        i = i / (e + 2);
        j = j + f;
        
        /* Another call */
        clobber_many_regs2();
        
        accum += a + b + c + d + e + f + g + h + i + j;
    }
    
    return accum;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Variables that might use callee-saved registers */
    register long r12 asm("r12") = x * 2;
    register long r13 asm("r13") = x * 3;
    register long r14 asm("r14") = x * 4;
    register long r15 asm("r15") = x * 5;
    
    /* Many temporary variables using caller-saved regs */
    int t1 = x + 1;
    int t2 = x + 2;
    int t3 = x + 3;
    int t4 = x + 4;
    int t5 = x + 5;
    int t6 = x + 6;
    int t7 = x + 7;
    int t8 = x + 8;
    int t9 = x + 9;
    int t10 = x + 10;
    
    /* Call that clobbers caller-saved but not callee-saved */
    clobber_many_regs1();
    
    /* Use both types */
    t1 = t1 + (int)r12;
    t2 = t2 + (int)r13;
    t3 = t3 + (int)r14;
    t4 = t4 + (int)r15;
    
    /* Another call */
    clobber_many_regs2();
    
    /* More mixing */
    r12 = r12 + t5;
    r13 = r13 + t6;
    r14 = r14 + t7;
    r15 = r15 + t8;
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + 
           (int)r12 + (int)r13 + (int)r14 + (int)r15;
}

/* Function with pointer variables to inhibit optimizations */
int __attribute__((noinline)) test_pointer_vars(int x) {
    int a = x, b = x+1, c = x+2, d = x+3, e = x+4;
    int f = x+5, g = x+6, h = x+7, i = x+8, j = x+9;
    
    /* Take addresses to force stack allocation */
    int *pa = &a, *pb = &b, *pc = &c, *pd = &d, *pe = &e;
    int *pf = &f, *pg = &g, *ph = &h, *pi = &i, *pj = &j;
    
    /* Use through pointers */
    *pa = *pa * 2;
    *pb = *pb + *pa;
    
    /* Call */
    clobber_many_regs1();
    
    *pc = *pc - *pb;
    *pd = *pd * *pc;
    
    /* Another call */
    clobber_many_regs2();
    
    *pe = *pe / (*pd + 1);
    *pf = *pf + *pe;
    
    /* Third call */
    clobber_many_regs3();
    
    *pg = *pg - *pf;
    *ph = *ph * *pg;
    *pi = *pi / (*ph + 1);
    *pj = *pj + *pi;
    
    return a + b + c + d + e + f + g + h + i + j;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed * 2);
    
    /* Test 2: Control flow variations */
    result += test_control_flow(seed, seed + 1, seed % 2);
    
    /* Test 3: Loop with pressure */
    result += test_loop_pressure(seed % 10 + 3);
    
    /* Test 4: Mixed caller/callee saved */
    result += test_mixed_save(seed);
    
    /* Test 5: Pointer variables */
    result += test_pointer_vars(seed + 5);
    
    /* Additional complex scenario combining everything */
    for (int i = 0; i < 3; i++) {
        int temp = test_high_pressure(seed + i, result);
        temp += test_control_flow(temp, seed, i);
        result += temp;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
