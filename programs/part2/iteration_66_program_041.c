/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass instruction reordering
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extremely high register pressure around calls */
unsigned long long __attribute__((noinline)) 
high_pressure_function(int param1, int param2, int param3) {
    /* Many local variables that must be live across calls */
    volatile int a = param1 + 1;
    volatile int b = param2 + 2;
    volatile int c = param3 + 3;
    volatile int d = param1 * 2;
    volatile int e = param2 * 3;
    volatile int f = param3 * 4;
    volatile int g = a + b;
    volatile int h = c + d;
    volatile int i = e + f;
    volatile int j = g + h;
    volatile int k = i + j;
    volatile int l = a + k;
    volatile int m = b + l;
    volatile int n = c + m;
    volatile int o = d + n;
    volatile int p = e + o;
    volatile int q = f + p;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs_1();
    
    /* Use all variables after call to keep them live */
    int r = a + b + c + d;
    int s = e + f + g + h;
    int t = i + j + k + l;
    int u = m + n + o + p;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int v = q + r + s + t;
    int w = u + v + a + b;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return (unsigned long long)a + b + c + d + e + f + g + h + 
           i + j + k + l + m + n + o + p + q + r + s + t + u + v + w;
}

/* Function with control flow variation */
unsigned long long __attribute__((noinline))
control_flow_function(int x, int y, int z) {
    volatile int v1 = x * 2;
    volatile int v2 = y * 3;
    volatile int v3 = z * 4;
    volatile int v4 = v1 + v2;
    volatile int v5 = v2 + v3;
    volatile int v6 = v3 + v1;
    volatile int v7 = v4 + v5;
    volatile int v8 = v5 + v6;
    volatile int v9 = v6 + v4;
    
    /* Conditional that creates multiple basic blocks */
    if (x > y) {
        clobber_many_regs_1();
        v1 = v2 + v3;
        v4 = v5 + v6;
    } else {
        clobber_many_regs_2();
        v2 = v3 + v1;
        v5 = v6 + v4;
    }
    
    /* Loop to create more basic blocks */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        clobber_many_regs_3();
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    }
    
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

/* Function mixing caller-saved and callee-saved usage */
unsigned long long __attribute__((noinline))
mixed_register_function(int base) {
    /* Variables that might use callee-saved registers */
    register long r12_var asm("r12") = base + 1;
    register long r13_var asm("r13") = base + 2;
    register long r14_var asm("r14") = base + 3;
    register long r15_var asm("r15") = base + 4;
    
    /* Many caller-saved register variables */
    volatile int c1 = base * 5;
    volatile int c2 = base * 6;
    volatile int c3 = base * 7;
    volatile int c4 = base * 8;
    volatile int c5 = base * 9;
    volatile int c6 = base * 10;
    volatile int c7 = base * 11;
    volatile int c8 = base * 12;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both caller and callee saved variables */
    long result = r12_var + r13_var + r14_var + r15_var;
    result += c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    result += r12_var * c1 + r13_var * c2 + r14_var * c3 + r15_var * c4;
    
    return result;
}

/* Function with multiple calls in sequence */
unsigned long long __attribute__((noinline))
sequential_calls_function(int seed) {
    volatile int w1 = seed + 1;
    volatile int w2 = seed + 2;
    volatile int w3 = seed + 3;
    volatile int w4 = seed + 4;
    volatile int w5 = seed + 5;
    volatile int w6 = seed + 6;
    volatile int w7 = seed + 7;
    volatile int w8 = seed + 8;
    
    /* Sequence of calls with live variables between */
    clobber_many_regs_1();
    int t1 = w1 + w2 + w3;
    
    clobber_many_regs_2();
    int t2 = w4 + w5 + w6 + t1;
    
    clobber_many_regs_3();
    int t3 = w7 + w8 + t2;
    
    clobber_many_regs_1();
    int t4 = t1 + t2 + t3;
    
    clobber_many_regs_2();
    
    return w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + t1 + t2 + t3 + t4;
}

/* Main function that calls all test functions */
int main(int argc, char *argv[]) {
    unsigned long long total = 0;
    
    /* Use command line arguments to vary input and prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    printf("Testing caller-save optimization patterns...\n");
    
    /* Call each test function multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        total += high_pressure_function(base + i, base + i + 1, base + i + 2);
        total += control_flow_function(base + i * 2, base + i * 3, base + i * 4);
        total += mixed_register_function(base + i * 5);
        total += sequential_calls_function(base + i * 7);
    }
    
    printf("Total checksum: %llu\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
