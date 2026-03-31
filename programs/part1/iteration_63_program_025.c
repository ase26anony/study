/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc lines 905-913.
 *
 * Compilation recommendations:
 *   gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-inline-small-functions caller-save-test.c -o caller-save-test
 *   gcc -O3 -fschedule-insns2 -fno-gcse -fno-strict-aliasing caller-save-test.c -o caller-save-test
 *   gcc -O2 -march=native -fno-rename-registers -fno-sched-interblock caller-save-test.c -o caller-save-test
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

/* Function that uses many registers and cannot be inlined */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d,
                           double e, double f, double g, double h) {
    /* Create register pressure with many live values */
    double t1 = a * b + c * d;
    double t2 = e * f + g * h;
    double t3 = t1 / (t2 + 1.0);
    double t4 = sin(t1) * cos(t2);
    double t5 = t3 * t4 + a * c * e * g;
    
    /* Force caller-save around asm */
    asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3), "+r"(t4) : : "memory");
    
    return t5 * b * d * f * h;
}

/* Function with mixed float/int operations */
__attribute__((noinline))
long long mixed_operations(int a, int b, float c, double d, 
                           long long e, int f, float g, double h) {
    volatile int vi = a;  /* Prevent optimization */
    volatile float vf = c;
    
    /* Operations using different register banks */
    double d1 = d * h + (double)c * (double)g;
    long long ll1 = e * (long long)(a + b) * f;
    float f1 = c * g * (float)d * (float)h;
    int i1 = a * b * f + (int)(c * g);
    
    /* Clobber registers to force saves */
    asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
    
    return ll1 + (long long)d1 + (long long)f1 + i1;
}

/* ========== Test Functions with Different Patterns ========== */

/* Test 1: Many live variables across a function call */
__attribute__((noinline, optimize("O3")))
double test1_caller_save_chain(void) {
    /* Many variables that must survive across the call */
    double v1 = 1.1, v2 = 2.2, v3 = 3.3, v4 = 4.4;
    double v5 = 5.5, v6 = 6.6, v7 = 7.7, v8 = 8.8;
    double v9 = 9.9, v10 = 10.10, v11 = 11.11, v12 = 12.12;
    
    /* Use them in computations to keep them live */
    double sum1 = v1 + v2 + v3 + v4;
    double sum2 = v5 + v6 + v7 + v8;
    double sum3 = v9 + v10 + v11 + v12;
    
    /* Call that clobbers registers - forces caller-save */
    clobber_registers();
    
    /* Continue using the variables - forces restore */
    double result = sum1 * sum2 / sum3;
    result += v1 * v5 * v9;
    result += v2 * v6 * v10;
    result += v3 * v7 * v11;
    result += v4 * v8 * v12;
    
    /* Another call to create multiple restore points */
    double temp = complex_calculation(v1, v2, v3, v4, v5, v6, v7, v8);
    result += temp;
    
    return result;
}

/* Test 2: Loop with function calls and live variables */
__attribute__((noinline))
long long test2_loop_caller_save(void) {
    long long accumulator = 0;
    volatile int loop_counter = 10; /* Prevent loop unrolling */
    
    /* Variables that must survive across each iteration */
    int a = 1, b = 2, c = 3;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    
    for (int i = 0; i < loop_counter; i++) {
        /* Use variables before call */
        int temp_int = a * i + b * (i + 1) + c * (i + 2);
        float temp_float = f1 * i + f2 * (i + 1) + f3 * (i + 2);
        double temp_double = d1 * i + d2 * (i + 1) + d3 * (i + 2);
        
        /* Function call that clobbers registers */
        clobber_registers();
        
        /* Use variables after call - forces restore */
        accumulator += temp_int + (long long)temp_float + (long long)temp_double;
        
        /* Modify variables for next iteration */
        a += i;
        b += i * 2;
        c += i * 3;
        f1 *= 1.1f;
        f2 *= 1.2f;
        f3 *= 1.3f;
        d1 *= 1.01;
        d2 *= 1.02;
        d3 *= 1.03;
    }
    
    return accumulator;
}

/* Test 3: Conditional branches with different caller-save patterns */
__attribute__((noinline, optimize("O3")))
double test3_conditional_caller_save(int mode) {
    double result = 0.0;
    
    /* Many live variables */
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    
    /* Complex conditional structure */
    if (mode == 0) {
        double temp1 = a * b + c * d;
        clobber_registers();
        double temp2 = e * f + g * h;
        result = temp1 + temp2;
    } 
    else if (mode == 1) {
        double temp1 = a / b + c / d;
        /* Call that uses many registers */
        double complex = complex_calculation(a, b, c, d, e, f, g, h);
        clobber_registers();
        double temp2 = e / f + g / h;
        result = temp1 + temp2 + complex;
    }
    else if (mode == 2) {
        for (int i = 0; i < 5; i++) {
            double loop_temp = a * i + b * (i + 1);
            clobber_registers();
            result += loop_temp + c * d;
            a += 0.1;
            b += 0.2;
        }
    }
    else {
        /* Default case with switch-like structure */
        double temp1 = a + b + c + d;
        double temp2 = e + f + g + h;
        clobber_registers();
        result = temp1 * temp2;
        
        /* Unreachable hint might affect block analysis */
        if (mode > 100) {
            __builtin_unreachable();
        }
    }
    
    /* Common continuation using all variables */
    result += a * c * e * g;
    result += b * d * f * h;
    
    return result;
}

/* Test 4: setjmp/longjmp with caller-save requirements */
static jmp_buf jump_buffer;
static volatile int jmp_value = 0;

__attribute__((noinline))
void function_with_setjmp(void) {
    /* Variables that must be saved across longjmp */
    volatile double d1 = 3.14159;
    volatile double d2 = 2.71828;
    volatile int i1 = 42;
    volatile int i2 = 100;
    
    if (setjmp(jump_buffer) == 0) {
        /* First call - do some computation */
        double temp = d1 * d2 + i1 * i2;
        
        /* Call that clobbers registers */
        clobber_registers();
        
        /* Use variables */
        jmp_value = (int)(temp + d1 + d2 + i1 + i2);
    } else {
        /* After longjmp - variables should be restored */
        double temp = d1 / d2 + i1 - i2;
        clobber_registers();
        jmp_value += (int)temp;
    }
}

/* Test 5: Multiple successive calls with overlapping register usage */
__attribute__((noinline, optimize("O3")))
double test5_chained_calls(void) {
    /* Chain of computations with calls in between */
    double x1 = 1.0, x2 = 2.0, x3 = 3.0, x4 = 4.0;
    
    /* First computation */
    double step1 = x1 * x2 + x3 * x4;
    
    /* Call that clobbers registers */
    clobber_registers();
    
    /* Second computation depends on first */
    double step2 = step1 * x1 / x2;
    
    /* Another call */
    double temp = complex_calculation(x1, x2, x3, x4, step1, step2, 5.0, 6.0);
    
    /* Third computation uses all previous results */
    double step3 = step1 + step2 + temp + x1 + x2 + x3 + x4;
    
    /* Final call */
    clobber_registers();
    
    return step1 * step2 * step3 * temp;
}

/* ========== Main Function ========== */

int main(void) {
    double checksum = 0.0;
    long long int_checksum = 0;
    
    printf("Starting caller-save stress tests...\n");
    
    /* Test 1: Basic caller-save chain */
    printf("Test 1: Caller-save chain...\n");
    double result1 = test1_caller_save_chain();
    checksum += result1;
    printf("  Result: %f\n", result1);
    
    /* Test 2: Loop with caller-save */
    printf("Test 2: Loop caller-save...\n");
    long long result2 = test2_loop_caller_save();
    int_checksum += result2;
    printf("  Result: %lld\n", result2);
    
    /* Test 3: Conditional caller-save */
    printf("Test 3: Conditional caller-save...\n");
    for (int i = 0; i < 4; i++) {
        double result3 = test3_conditional_caller_save(i);
        checksum += result3;
        printf("  Mode %d: %f\n", i, result3);
    }
    
    /* Test 4: setjmp/longjmp */
    printf("Test 4: setjmp/longjmp caller-save...\n");
    function_with_setjmp();
    longjmp(jump_buffer, 1);
    int_checksum += jmp_value;
    printf("  jmp_value: %d\n", jmp_value);
    
    /* Test 5: Chained calls */
    printf("Test 5: Chained calls...\n");
    double result5 = test5_chained_calls();
    checksum += result5;
    printf("  Result: %f\n", result5);
    
    /* Mixed operations test */
    printf("Test 6: Mixed operations...\n");
    long long result6 = mixed_operations(1, 2, 3.0f, 4.0, 5, 6, 7.0f, 8.0);
    int_checksum += result6;
    printf("  Result: %lld\n", result6);
    
    /* Final checksum */
    printf("\nFinal checksums:\n");
    printf("  Double checksum: %f\n", checksum);
    printf("  Integer checksum: %lld\n", int_checksum);
    
    /* Validate results are non-zero (prevent dead code elimination) */
    if (checksum == 0.0 && int_checksum == 0) {
        printf("WARNING: All results zero - possible optimization issue\n");
        return 1;
    }
    
    printf("All tests completed.\n");
    return 0;
}
