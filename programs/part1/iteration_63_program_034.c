/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction reordering code in caller-save.cc
 * that manipulates basic block instruction chains.
 *
 * Compilation recommendations:
 *   gcc -O3 -fschedule-insns2 -fno-gcse -fno-inline -fno-inline-small-functions \
 *       -fno-strict-aliasing -march=native -fno-rename-registers \
 *       caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper functions with register pressure ========== */

/* Force caller-save by clobbering many registers */
__attribute__((noinline))
void clobber_registers() {
    /* Clobber multiple caller-saved registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                               "r8", "r9", "r10", "r11", "xmm0", "xmm1",
                               "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                               "xmm8", "xmm9", "xmm10", "xmm11", "xmm12",
                               "xmm13", "xmm14", "xmm15", "memory");
}

/* Function that uses many registers and calls clobbering function */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d,
                           double e, double f, double g, double h) {
    /* Create many live values across the call */
    double t1 = a * b + c * d;
    double t2 = e * f + g * h;
    double t3 = sin(a) * cos(b);
    double t4 = exp(c) * log(d + 1.0);
    double t5 = t1 * t2 + t3 * t4;
    
    /* Call that clobbers registers - forces caller-save */
    clobber_registers();
    
    /* Use all values after call - requires restores */
    double result = t1 + t2 + t3 + t4 + t5;
    result += sin(t1) * cos(t2);
    result += exp(t3) * log(fabs(t4) + 1.0);
    
    return result;
}

/* Function with mixed float/int operations */
__attribute__((noinline))
long long mixed_operations(int a, int b, float c, float d,
                           double e, double f, long long g) {
    volatile int vi1 = a, vi2 = b;
    volatile float vf1 = c, vf2 = d;
    volatile double vd1 = e, vd2 = f;
    volatile long long vl1 = g;
    
    /* Force register usage across call */
    int ires1 = vi1 * vi2 + (int)(vf1 * vf2);
    float fres1 = vf1 * vf2 + (float)(vi1 * vi2);
    double dres1 = vd1 * vd2 + (double)(vi1 + vi2);
    long long llres1 = vl1 * (long long)(vd1 * 100.0);
    
    clobber_registers();
    
    /* More operations after call */
    int ires2 = ires1 * 3 + (int)(fres1 * 2.0f);
    float fres2 = fres1 * 1.5f + (float)(ires1 / 2);
    double dres2 = dres1 * 2.0 + (double)(llres1 % 1000);
    long long llres2 = llres1 + (long long)(dres2 * 10.0);
    
    return llres2 + ires2 + (long long)fres2 + (long long)dres2;
}

/* ========== Functions with control flow ========== */

/* Function with if-else creating multiple basic blocks */
__attribute__((noinline, optimize("O3")))
double conditional_caller_save(double x, double y, int mode) {
    double a = x * x + y * y;
    double b = sin(x) * cos(y);
    double c = exp(x) * log(fabs(y) + 1.0);
    double d = a + b + c;
    
    if (mode == 0) {
        /* First path with clobber */
        clobber_registers();
        d = d * 2.0 + a;
    } else if (mode == 1) {
        /* Second path with different computation then clobber */
        double e = sqrt(a) + pow(b, 2.0);
        clobber_registers();
        d = d + e * 3.0;
    } else {
        /* Third path with nested call */
        double f = complex_calculation(a, b, c, d, x, y, 1.0, 2.0);
        clobber_registers();
        d = d + f / 2.0;
    }
    
    /* Common continuation using all values */
    return a + b + c + d;
}

/* Function with loop containing calls */
__attribute__((noinline))
double loop_with_calls(int iterations) {
    double acc = 0.0;
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Loop-invariant computations that need registers */
        double t1 = a * i + b;
        double t2 = c * i + d;
        double t3 = e * i + f;
        double t4 = g * i + h;
        
        /* Call that clobbers - registers must be saved each iteration */
        clobber_registers();
        
        /* Use values after call */
        acc += t1 + t2 + t3 + t4;
        
        /* Modify some values for next iteration */
        a += 0.1; b += 0.2;
        c += 0.3; d += 0.4;
    }
    
    return acc;
}

/* ========== Switch statement with multiple edges ========== */

__attribute__((noinline, optimize("O3")))
double switch_caller_save(double x, int choice) {
    double a = x * 2.0;
    double b = x * 3.0;
    double c = x * 4.0;
    double d = x * 5.0;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            clobber_registers();
            result = a + b;
            break;
        case 1:
            result = complex_calculation(a, b, c, d, 1.0, 2.0, 3.0, 4.0);
            clobber_registers();
            result += c;
            break;
        case 2:
            result = a * b * c * d;
            clobber_registers();
            result = sqrt(result);
            break;
        case 3:
            clobber_registers();
            result = conditional_caller_save(a, b, 0);
            clobber_registers();
            break;
        default:
            result = a + b + c + d;
            __builtin_unreachable(); /* May affect block termination */
    }
    
    /* Common code using all variables */
    return result + a + b + c + d;
}

/* ========== setjmp/longjmp test ========== */

static jmp_buf env;

__attribute__((noinline))
void function_with_setjmp(int *value) {
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    
    if (setjmp(env) == 0) {
        /* First call - do computations */
        double t1 = a * b + c * d;
        double t2 = e * f + g * h;
        *value = (int)(t1 + t2);
        
        /* Call that might longjmp */
        clobber_registers();
    } else {
        /* After longjmp - use registers again */
        double t3 = a + b + c + d + e + f + g + h;
        *value += (int)t3;
    }
}

/* ========== Main test orchestration ========== */

int main() {
    double total = 0.0;
    long long checksum = 0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Complex calculation with many live values */
    total += complex_calculation(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8);
    
    /* Test 2: Mixed operations */
    checksum += mixed_operations(10, 20, 3.14f, 2.71f, 1.414, 2.718, 1000);
    
    /* Test 3: Conditional paths */
    for (int i = 0; i < 3; i++) {
        total += conditional_caller_save(1.0 + i, 2.0 + i, i);
    }
    
    /* Test 4: Loop with calls */
    total += loop_with_calls(10);
    
    /* Test 5: Switch statement */
    for (int i = 0; i < 4; i++) {
        total += switch_caller_save(1.5 + i, i);
    }
    
    /* Test 6: setjmp/longjmp */
    int jmp_value = 0;
    function_with_setjmp(&jmp_value);
    /* Simulate longjmp */
    longjmp(env, 1);
    function_with_setjmp(&jmp_value);
    checksum += jmp_value;
    
    /* Final validation */
    printf("Total double result: %f\n", total);
    printf("Checksum: %lld\n", checksum);
    
    /* Prevent dead code elimination */
    volatile double sink = total;
    volatile long long sink2 = checksum;
    
    return (sink > 0.0 && sink2 > 0) ? 0 : 1;
}
