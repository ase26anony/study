/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc lines 905-913.
 *
 * Compilation recommendations:
 *   gcc -O3 -fschedule-insns2 -fno-gcse -fno-inline -fno-inline-small-functions \
 *       -fno-strict-aliasing -march=native -fno-rename-registers \
 *       -fno-sched-interblock caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller-save by clobbering many registers */
__attribute__((noinline))
void clobber_registers(void) {
    /* Clobber multiple caller-saved registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                 "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
                 "xmm4", "xmm5", "xmm6", "xmm7", "memory");
}

/* Function that uses many registers and can't be inlined */
__attribute__((noinline, optimize("O3")))
double complex_float_calc(double a, double b, double c, double d,
                          double e, double f, double g, double h) {
    /* Complex floating point chain to create register pressure */
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = g / h + t1;
    double t4 = t2 * t3 - a;
    double t5 = sin(t4) + cos(t3);
    double t6 = t5 * t1 / t2;
    
    /* Force register usage with volatile */
    volatile double sink = t6;
    return sink + t4 + t5;
}

/* Another noinline function with integer pressure */
__attribute__((noinline))
long long int_pressure(long long a, long long b, long long c,
                       long long d, long long e, long long f) {
    /* Long dependency chain to force register allocation */
    long long x = a * b + c;
    long long y = d * e - f;
    long long z = (x ^ y) & (a | b);
    long long w = (z << 3) | (y >> 2);
    
    /* Clobber registers in the middle */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory");
    
    return w + x + y + z;
}

/* External function declaration to prevent optimization */
extern void external_call(int, double, long long);

/* ========== Test Functions Targeting Specific Patterns ========== */

/* Test 1: Many live variables across a function call */
__attribute__((noinline, optimize("O3")))
double test1_caller_save_chain(void) {
    /* Declare many variables of different types */
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4, d5 = 5.5;
    float f1 = 6.6f, f2 = 7.7f, f3 = 8.8f;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50, i6 = 60;
    long long ll1 = 100, ll2 = 200, ll3 = 300;
    
    /* Use all variables before call to make them live */
    double sum_d = d1 + d2 + d3 + d4 + d5;
    float sum_f = f1 * f2 - f3;
    int sum_i = i1 * i2 + i3 - i4 + i5 * i6;
    long long sum_ll = ll1 ^ ll2 | ll3;
    
    /* Function call that clobbers caller-saved registers */
    clobber_registers();
    
    /* Use all variables after call - forces restore */
    sum_d += d1 * d2 - d3 / d4 + d5;
    sum_f = sum_f + f1 - f2 * f3;
    sum_i = sum_i ^ i1 | i2 & i3;
    sum_ll = (sum_ll << 2) + ll1 - ll2;
    
    /* Complex computation mixing types */
    return sum_d + sum_f + sum_i + sum_ll;
}

/* Test 2: Loop with function calls and live variables */
__attribute__((noinline))
long long test2_loop_caller_save(void) {
    long long acc = 0;
    volatile int counter = 100; /* Prevent loop unrolling */
    
    /* Variables that must survive across loop iterations */
    double d_acc = 0.0;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    float f1 = 1.5f, f2 = 2.5f;
    
    for (int i = 0; i < counter; i++) {
        /* Use variables before call */
        d_acc += (i1 * i2) / (double)(i3 + i4);
        f1 = f1 * 1.1f;
        f2 = f2 - 0.1f;
        
        /* Function call clobbering registers */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3", "memory");
        
        /* Use variables after call - forces restore each iteration */
        i1 = (i1 + i2) ^ i3;
        i2 = i2 * i3 - i4;
        i3 = i3 + i4;
        i4 = i4 ^ i1;
        
        acc += i1 + i2 + i3 + i4;
    }
    
    return acc + (long long)d_acc;
}

/* Test 3: Conditional branches with different caller-save patterns */
__attribute__((noinline, optimize("O3")))
double test3_conditional_saves(int mode) {
    double result = 0.0;
    
    /* Many live variables */
    double d1 = 1.234, d2 = 5.678, d3 = 9.012;
    float f1 = 3.14f, f2 = 2.71f;
    int i1 = 42, i2 = 84, i3 = 168;
    long long ll1 = 1000, ll2 = 2000;
    
    switch (mode % 4) {
        case 0:
            /* Complex computation then call */
            d1 = sin(d1) * cos(d2);
            f1 = f1 * f2 + 1.0f;
            clobber_registers();
            result = d1 + d2 + d3 + f1 + f2 + i1;
            break;
            
        case 1:
            /* Call then complex computation */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "memory");
            d2 = exp(d1) * log(d3);
            f2 = sqrtf(f1) * f2;
            result = d2 * d3 - f2 + i2;
            break;
            
        case 2:
            /* Multiple calls with live variables */
            i1 = i1 * i2 - i3;
            external_call(i1, d1, ll1);
            i2 = i2 ^ i3 | i1;
            clobber_registers();
            i3 = i3 + i1 * i2;
            result = i1 + i2 + i3 + d1 + d2;
            break;
            
        case 3:
            /* Mixed float/int operations with calls */
            d3 = d1 * d2 + d3;
            ll1 = ll1 << (i1 % 8);
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                         "r8", "r9", "r10", "xmm0", "xmm1", "memory");
            f1 = f1 / f2 * 2.0f;
            ll2 = ll2 >> (i2 % 8);
            result = d3 + f1 + ll1 + ll2;
            break;
    }
    
    /* Use all variables at the end to ensure they're live */
    return result + d1 - d2 * d3 + f1 / f2 + i1 * i2 - i3 + ll1 ^ ll2;
}

/* Test 4: setjmp/longjmp caller-save requirements */
static jmp_buf env;
static volatile int jmp_flag = 0;

__attribute__((noinline))
double test4_setjmp_pattern(void) {
    double d1 = 10.0, d2 = 20.0, d3 = 30.0;
    int i1 = 100, i2 = 200;
    float f1 = 5.5f;
    
    if (setjmp(env) == 0) {
        /* First time through */
        d1 = d1 * 2.0;
        d2 = sin(d2);
        i1 = i1 * 3;
        
        /* Call that might longjmp */
        if (jmp_flag == 0) {
            clobber_registers();
        }
        
        d3 = d3 / 2.0;
        i2 = i2 + i1;
        f1 = sqrtf(f1);
        
        /* Simulate longjmp */
        jmp_flag = 1;
        longjmp(env, 1);
    } else {
        /* After longjmp - registers must be restored */
        d1 = d1 + 1.0;
        d2 = cos(d2);
        i1 = i1 ^ 0xFF;
    }
    
    return d1 + d2 + d3 + i1 + i2 + f1;
}

/* Test 5: Unreachable code after call affecting block termination */
__attribute__((noinline, optimize("O3")))
int test5_unreachable_pattern(int x) {
    double d1 = 1.0, d2 = 2.0, d3 = 3.0;
    int i1 = x, i2 = x * 2, i3 = x * 3;
    
    /* Complex computation */
    d1 = d1 * i1 + d2;
    d2 = sin(d2) * cos(d3);
    i2 = i2 ^ i3 | i1;
    
    /* External call */
    external_call(i1, d1, (long long)i2);
    
    if (x > 1000) {
        /* This path might be optimized as unreachable */
        d3 = d3 * 2.0;
        i3 = i3 << 2;
        clobber_registers();
        return i1 + i2 + i3 + (int)d1 + (int)d2 + (int)d3;
    }
    
    /* Use variables to ensure they're live */
    i1 = i1 * 2 - i2;
    d1 = d1 / d2 + d3;
    
    /* Hint about unreachable code */
    if (x < 0) {
        __builtin_unreachable();
    }
    
    return i1 + (int)d1;
}

/* ========== Main Orchestration Function ========== */

int main(void) {
    double checksum = 0.0;
    
    printf("Starting caller-save stress tests...\n");
    
    /* Run test 1 */
    double result1 = test1_caller_save_chain();
    checksum += result1;
    printf("Test 1 result: %f\n", result1);
    
    /* Run test 2 */
    long long result2 = test2_loop_caller_save();
    checksum += (double)result2;
    printf("Test 2 result: %lld\n", result2);
    
    /* Run test 3 with different modes */
    for (int mode = 0; mode < 8; mode++) {
        double result3 = test3_conditional_saves(mode);
        checksum += result3;
        printf("Test 3 mode %d: %f\n", mode, result3);
    }
    
    /* Run test 4 */
    double result4 = test4_setjmp_pattern();
    checksum += result4;
    printf("Test 4 result: %f\n", result4);
    
    /* Run test 5 with different inputs */
    for (int i = 0; i < 10; i++) {
        int result5 = test5_unreachable_pattern(i * 50);
        checksum += (double)result5;
        printf("Test 5 input %d: %d\n", i * 50, result5);
    }
    
    /* Final validation */
    printf("\nFinal checksum: %f\n", checksum);
    printf("All tests completed.\n");
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0.0) {
        printf("ERROR: All computations eliminated!\n");
        return 1;
    }
    
    return 0;
}

/* Dummy external function definition */
void external_call(int a, double b, long long c) {
    /* Do nothing but prevent optimization */
    volatile int sink = a + (int)b + (int)c;
    (void)sink;
}
