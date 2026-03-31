/* test_caller_save.c - Forces caller-save register spilling at block ends */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE int baz(void);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern double external_func2(double);
extern __m128 external_func3(__m128);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* ========== Test Case 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create massive integer register pressure */
    register int r0  = seed + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + seed;
    register int r3  = r2 ^ r1;
    register int r4  = r3 << 2;
    register int r5  = r4 - r0;
    register int r6  = r5 | r2;
    register int r7  = r6 & r3;
    register int r8  = r7 * 3;
    register int r9  = r8 / 2;
    register int r10 = r9 + r4;
    register int r11 = r10 ^ r5;
    register int r12 = r11 << 1;
    register int r13 = r12 - r6;
    register int r14 = r13 | r7;
    register int r15 = r14 & r8;
    register int r16 = r15 * 5;
    register int r17 = r16 / 3;
    register int r18 = r17 + r9;
    register int r19 = r18 ^ r10;
    register int r20 = r19 << 3;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    volatile int v5 = r5;
    
    /* Complex control flow to create basic block ending with call */
    int result = 0;
    if (seed % 3 == 0) {
        /* This basic block ends with the call to foo() */
        foo();  /* Non-inlineable call */
        
        /* Use all the register variables after call */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                 r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else if (seed % 3 == 1) {
        /* Alternative path */
        result = seed * 2;
    } else {
        /* Another alternative path */
        result = seed / 2;
    }
    
    /* Force use of volatile variables after control flow */
    global_counter += v0 + v1 + v2 + v3 + v4 + v5;
    
    return result;
}

/* ========== Test Case 2: FP/SSE register pressure ========== */
NOINLINE double test_fp_pressure(double angle) {
    /* Create massive floating-point register pressure */
    double d0  = sin(angle);
    double d1  = cos(angle);
    double d2  = tan(angle);
    double d3  = d0 * d1;
    double d4  = d2 + d3;
    double d5  = d4 * d0;
    double d6  = d5 - d1;
    double d7  = d6 / d2;
    double d8  = sin(d3);
    double d9  = cos(d4);
    double d10 = tan(d5);
    double d11 = d8 * d9;
    double d12 = d10 + d11;
    double d13 = d12 * d8;
    double d14 = d13 - d9;
    double d15 = d14 / d10;
    double d16 = sin(d11);
    double d17 = cos(d12);
    double d18 = tan(d13);
    double d19 = d16 * d17;
    double d20 = d18 + d19;
    
    /* SSE/vector register pressure */
    __m128 v0 = _mm_set_ps(d0, d1, d2, d3);
    __m128 v1 = _mm_set_ps(d4, d5, d6, d7);
    __m128 v2 = _mm_set_ps(d8, d9, d10, d11);
    __m128 v3 = _mm_set_ps(d12, d13, d14, d15);
    __m128 v4 = _mm_set_ps(d16, d17, d18, d19);
    
    /* Volatile doubles that must survive */
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    
    /* Inline assembly to clobber caller-saved registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    
    /* Switch statement to create complex CFG */
    double result = 0.0;
    switch ((int)angle % 4) {
        case 0:
            /* This block ends with external call */
            external_func1();
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                     d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            break;
        case 1:
            result = external_func2(d0);
            break;
        case 2:
            result = d0 * d1;
            break;
        default:
            result = d0 - d1;
            break;
    }
    
    /* Use vector variables after call */
    v0 = _mm_add_ps(v0, v1);
    v2 = _mm_mul_ps(v2, v3);
    v4 = _mm_sub_ps(v4, v0);
    
    global_accumulator += vd0 + vd1 + vd2;
    
    return result;
}

/* ========== Test Case 3: Mixed pressure in loop ========== */
NOINLINE int test_mixed_pressure_loop(int iterations) {
    int sum = 0;
    
    /* Partially unrolled loop with call at end of basic block */
    for (int i = 0; i < iterations; i++) {
        /* Integer pressure */
        int a0 = i * 2;
        int a1 = a0 + 1;
        int a2 = a1 * 3;
        int a3 = a2 - i;
        int a4 = a3 ^ a0;
        int a5 = a4 << 1;
        int a6 = a5 | a1;
        int a7 = a6 & a2;
        int a8 = a7 * 5;
        int a9 = a8 / 2;
        
        /* Floating pressure */
        double f0 = sin(i * 0.1);
        double f1 = cos(i * 0.2);
        double f2 = f0 * f1;
        double f3 = f2 + f0;
        double f4 = f3 - f1;
        
        /* Volatile variables */
        volatile int va = a0;
        volatile double vf = f0;
        
        /* Call at potential block end */
        if (i % 2 == 0) {
            bar(a0, f0);  /* Non-inlineable call with args */
            
            /* Use variables after call */
            sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
            global_accumulator += f0 + f1 + f2 + f3 + f4;
        } else {
            sum += baz();
        }
        
        global_counter += va;
        global_accumulator += vf;
    }
    
    return sum;
}

/* ========== Test Case 4: Nested control flow ========== */
NOINLINE double test_nested_blocks(int mode) {
    double result = 0.0;
    
    if (mode > 0) {
        /* Create register pressure in nested scope */
        double d0 = 1.0, d1 = 2.0, d2 = 3.0, d3 = 4.0, d4 = 5.0;
        double d5 = 6.0, d6 = 7.0, d7 = 8.0, d8 = 9.0, d9 = 10.0;
        
        for (int i = 0; i < 3; i++) {
            d0 += sin(i);
            d1 += cos(i);
            d2 += d0 * d1;
            d3 += d2 / d1;
            d4 += d3 - d0;
            
            if (i == 1) {
                /* Call at end of inner block */
                external_func1();
                
                /* Massive use of registers after call */
                d5 = d0 + d1 + d2 + d3 + d4;
                d6 = d5 * d0;
                d7 = d6 - d1;
                d8 = d7 / d2;
                d9 = d8 + d3;
            }
        }
        
        result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
    } else {
        result = -1.0;
    }
    
    return result;
}

/* ========== Main driver ========== */
int main(void) {
    int total = 0;
    double dtotal = 0.0;
    
    /* Run all test cases to trigger different spilling scenarios */
    total += test_integer_pressure(42);
    total += test_integer_pressure(99);
    
    dtotal += test_fp_pressure(0.5);
    dtotal += test_fp_pressure(1.0);
    dtotal += test_fp_pressure(2.0);
    
    total += test_mixed_pressure_loop(10);
    
    dtotal += test_nested_blocks(1);
    dtotal += test_nested_blocks(2);
    
    /* Prevent dead code elimination */
    printf("Integer result: %d\n", total);
    printf("FP result: %f\n", dtotal);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    return (total > 0 && dtotal > 0) ? 0 : 1;
}
