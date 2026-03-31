/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");

/* Force register allocation with explicit register variables */
#ifdef __x86_64__
register long r12_var asm("r12");
register long r13_var asm("r13");
register long r14_var asm("r14");
register long r15_var asm("r15");
#elif defined(__arm__)
register int r8_var asm("r8");
register int r9_var asm("r9");
register int r10_var asm("r10");
register int r11_var asm("r11");
#endif

/* Complex structure with mixed types */
struct Nested {
    int a[3];
    long b;
    float c;
    struct {
        short x;
        short y;
    } inner;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  long b1, long b2, long b3, long b4, long b5,
                  float f1, float f2, double d1, double d2) {
    
    /* Many local variables to exhaust registers */
    int v1 = a1 + barrier(a2);
    int v2 = a3 * barrier(a4);
    int v3 = a5 ^ barrier(a6);
    int v4 = a7 | barrier(a8);
    int v5 = a9 & barrier(a10);
    
    long l1 = b1 + barrier(b2);
    long l2 = b3 * barrier(b4);
    long l3 = b5 + barrier(v1);
    long l4 = v2 * barrier(v3);
    long l5 = v4 ^ barrier(v5);
    
    /* Force spills with many live variables */
    int t1 = v1 + v2;
    int t2 = v3 + v4;
    int t3 = v5 + t1;
    int t4 = t2 + t3;
    int t5 = t4 * 7;
    
    long m1 = l1 + l2;
    long m2 = l3 + l4;
    long m3 = l5 + m1;
    long m4 = m2 + m3;
    long m5 = m4 * 13;
    
    /* Complex array access with SIB-like addressing */
    int array[100][100];
    volatile int *volatile ptr = (volatile int*)array;
    
    /* Force complex addressing mode */
    int array_val = ptr[idx1 * 40 + idx2 * 20 + idx3];
    
    /* Mixed float/int operations */
    union {
        float f;
        int i;
    } pun;
    pun.f = f1 + f2;
    int int_from_float = pun.i;
    
    /* Inline assembly with many clobbered registers */
    asm volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[res1]\n\t"
        "add %[val2], %[res1]\n\t"
        : [res1] "=r" (t1)
        : [val1] "r" (t2), [val2] "r" (t3)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Secondary reload stress: memory to register with complex address */
    volatile long mem_var = 0x12345678;
    register long reg_var asm("ebx");
    
    /* Force secondary reload by using complex memory operand */
    asm volatile (
        "mov %1, %0\n\t"
        : "=r" (reg_var)
        : "m" (*(volatile long*)((char*)&mem_var + idx1 * 4))
        : "memory"
    );
    
    /* More arithmetic to keep variables live */
    t1 = t1 + array_val;
    t2 = t2 + int_from_float;
    t3 = t3 + (int)reg_var;
    
    /* Atomic operations that need reloads */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, t1, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Complex structure access */
    struct Nested nested[10];
    int nested_val = nested[idx1].a[idx2] + nested[idx2].inner.x;
    
    /* Use explicit register variables */
#ifdef __x86_64__
    r12_var = t1;
    r13_var = t2;
    r14_var = t3;
    r15_var = t4;
    t5 = (int)(r12_var + r13_var + r14_var + r15_var);
#elif defined(__arm__)
    r8_var = t1;
    r9_var = t2;
    r10_var = t3;
    r11_var = t4;
    t5 = r8_var + r9_var + r10_var + r11_var;
#endif
    
    /* Long dependency chain */
    for (int i = 0; i < 5; i++) {
        t1 = t2 + barrier(t3);
        t2 = t3 + barrier(t4);
        t3 = t4 + barrier(t5);
        t4 = t5 + barrier(t1);
        t5 = t1 + barrier(t2);
    }
    
    /* Final computation using all variables */
    long result = (long)t1 + t2 + t3 + t4 + t5 +
                  l1 + l2 + l3 + l4 + l5 +
                  m1 + m2 + m3 + m4 + m5 +
                  atomic_val + nested_val + array_val + int_from_float;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int a1 = argc + 1;
    int a2 = argc + 2;
    int a3 = argc + 3;
    int a4 = argc + 4;
    int a5 = argc + 5;
    int a6 = argc + 6;
    int a7 = argc + 7;
    int a8 = argc + 8;
    int a9 = argc + 9;
    int a10 = argc + 10;
    
    long b1 = (long)argc * 100;
    long b2 = (long)argc * 200;
    long b3 = (long)argc * 300;
    long b4 = (long)argc * 400;
    long b5 = (long)argc * 500;
    
    float f1 = (float)argc * 1.1f;
    float f2 = (float)argc * 2.2f;
    double d1 = (double)argc * 3.3;
    double d2 = (double)argc * 4.4;
    
    /* Call test function multiple times with different args */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        total += test_reloads(a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
                             a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
                             b1 + i, b2 + i, b3 + i, b4 + i, b5 + i,
                             f1 + i, f2 + i, d1 + i, d2 + i);
    }
    
    printf("Result: %ld\n", total);
    return 0;
}

/* Dummy barrier function definition */
int barrier(int x) {
    return x ^ 0x55;
}
