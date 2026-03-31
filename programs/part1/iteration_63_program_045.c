/* caller-save-test.c
 * 
 * A comprehensive test to trigger caller-save restoration instruction
 * reordering within basic blocks, specifically targeting the uncovered
 * lines in caller-save.cc (lines 905-913).
 *
 * Compilation recommendations:
 *   gcc -O3 -fschedule-insns2 -fno-gcse -fno-strict-aliasing \
 *       -fno-omit-frame-pointer -fno-inline -fno-inline-small-functions \
 *       -march=native -fno-rename-registers -fno-sched-interblock \
 *       caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller-save by clobbering many registers */
__attribute__((noinline))
void clobber_registers() {
    /* Clobber multiple caller-saved registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                               "r8", "r9", "r10", "r11",
                               "xmm0", "xmm1", "xmm2", "xmm3",
                               "xmm4", "xmm5", "xmm6", "xmm7",
                               "xmm8", "xmm9", "xmm10", "xmm11",
                               "xmm12", "xmm13", "xmm14", "xmm15");
}

/* Complex floating-point computation that uses many registers */
__attribute__((noinline, optimize("O3")))
double fp_computation(double a, double b, double c, double d,
                      double e, double f, double g, double h) {
    /* Chain of dependent FP operations to create long live ranges */
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = g / h + t1;
    double t4 = sin(t2) * cos(t3);
    double t5 = t4 * t1 / t2;
    
    /* Force register pressure with many intermediate values */
    volatile double v1 = t1 + t2;
    volatile double v2 = t3 * t4;
    volatile double v3 = t5 - v1;
    
    return t5 + v2 * v3;
}

/* Integer computation with many live variables across call */
__attribute__((noinline))
long long int_computation(long long a, long long b, long long c,
                          long long d, long long e, long long f) {
    /* Many parallel computations to use registers */
    long long r1 = a * b;
    long long r2 = c * d;
    long long r3 = e * f;
    long long r4 = r1 ^ r2;
    long long r5 = r3 & r4;
    long long r6 = r1 | r2;
    long long r7 = r5 + r6;
    long long r8 = r7 * r3;
    long long r9 = r8 - r4;
    long long r10 = r9 / (r1 + 1);
    
    /* Call that clobbers caller-saved registers */
    clobber_registers();
    
    /* Continue using all the computed values */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Mixed float/int operations to engage different register banks */
__attribute__((noinline, optimize("O3")))
double mixed_operations(int a, float b, double c, long long d) {
    double result = 0.0;
    
    /* Use all parameters in computations */
    double da = (double)a;
    float fb = b * 2.0f;
    double dc = c * 3.0;
    long long ld = d / 4;
    
    /* Chain of mixed operations */
    result = da + (double)fb;
    result *= dc;
    result += (double)ld;
    
    /* Inline asm that clobbers specific registers */
    asm volatile("" : : : "rax", "rcx", "xmm0", "xmm1", "xmm2");
    
    /* More computations after clobber */
    result = sin(result) * cos((double)a);
    
    return result;
}

/* ========== Test Functions with Different Control Flow ========== */

/* Test 1: Loop with function call inside */
__attribute__((noinline, optimize("O3")))
double test_loop_calls(int iterations) {
    double acc = 1.0;
    volatile double v = 2.5; /* Prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables across the call */
        double a = acc * i;
        double b = sin(acc);
        double c = cos(v);
        double d = a * b + c;
        
        /* Function call that forces caller-save */
        double e = fp_computation(a, b, c, d, acc, v, (double)i, 1.0);
        
        /* Continue using all variables */
        acc = a + b + c + d + e;
        v = v * 1.1;
    }
    
    return acc;
}

/* Test 2: Conditional branches with calls in each path */
__attribute__((noinline))
long long test_conditional_calls(int mode, long long x) {
    long long result = x;
    
    if (mode == 0) {
        /* Path 1: Many live variables */
        long long a = x * 2;
        long long b = x + 100;
        long long c = x ^ 0xFFFF;
        
        clobber_registers();
        
        result = int_computation(a, b, c, x, a+b, c^x);
    } 
    else if (mode == 1) {
        /* Path 2: Different set of live variables */
        long long d = x / 3;
        long long e = x << 2;
        long long f = x >> 1;
        long long g = x | 0xFF;
        
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
        
        result = d + e + f + g;
        
        /* Another call to force restore placement */
        clobber_registers();
        
        result *= 2;
    }
    else {
        /* Path 3: Mixed operations */
        double r = mixed_operations((int)x, (float)x, (double)x, x);
        result = (long long)(r * 1000.0);
    }
    
    return result;
}

/* Test 3: Switch statement with calls in multiple cases */
__attribute__((noinline))
double test_switch_calls(int case_id, double base) {
    double result = base;
    
    switch (case_id) {
        case 0: {
            /* Case with FP computation and call */
            double a = sin(base);
            double b = cos(base);
            clobber_registers();
            result = fp_computation(a, b, base, 1.0, 2.0, 3.0, 4.0, 5.0);
            break;
        }
        case 1: {
            /* Case with integer computation */
            long long ll = (long long)base;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            result = (double)int_computation(ll, ll+1, ll+2, ll+3, ll+4, ll+5);
            break;
        }
        case 2: {
            /* Case with mixed operations */
            int i = (int)base;
            float f = (float)base;
            result = mixed_operations(i, f, base, (long long)base);
            
            /* Additional call to create more restore points */
            clobber_registers();
            result *= 2.0;
            break;
        }
        default: {
            /* Default case with unreachable hint */
            result = base * 10.0;
            clobber_registers();
            if (case_id > 100) {
                __builtin_unreachable();
            }
            break;
        }
    }
    
    return result;
}

/* Test 4: setjmp/longjmp with caller-save requirements */
static jmp_buf env;
__attribute__((noinline))
double test_setjmp_calls(double input) {
    double a = input * 2.0;
    double b = sin(input);
    double c = cos(input);
    
    if (setjmp(env) == 0) {
        /* First call: do computation and longjmp */
        double d = fp_computation(a, b, c, input, 1.0, 2.0, 3.0, 4.0);
        
        /* This should force caller-save around setjmp */
        volatile double keeper = d;
        
        /* Simulate longjmp - in real use would call longjmp */
        /* For test purposes, we'll just continue */
        return d + keeper;
    } else {
        /* After longjmp - registers need restoring */
        return a + b + c;
    }
}

/* Test 5: Nested calls with register pressure */
__attribute__((noinline))
double test_nested_calls(double x) {
    /* Outer function with many live variables */
    double a = x * 1.1;
    double b = x * 2.2;
    double c = x * 3.3;
    
    /* Call that itself has caller-save requirements */
    double inner = fp_computation(a, b, c, x, a+1, b+2, c+3, x/2);
    
    /* More live variables */
    double d = inner * 4.4;
    double e = inner * 5.5;
    
    /* Another call */
    clobber_registers();
    
    return a + b + c + d + e + inner;
}

/* ========== Main Orchestrator ========== */

int main() {
    double checksum = 0.0;
    
    printf("Starting caller-save restoration test...\n");
    
    /* Test 1: Loop with calls */
    printf("Test 1: Loop with function calls...\n");
    double loop_result = test_loop_calls(10);
    checksum += loop_result;
    printf("  Loop result: %f\n", loop_result);
    
    /* Test 2: Conditional calls */
    printf("Test 2: Conditional branches with calls...\n");
    for (int i = 0; i < 3; i++) {
        long long cond_result = test_conditional_calls(i, 1000 + i);
        checksum += (double)cond_result;
        printf("  Mode %d: %lld\n", i, cond_result);
    }
    
    /* Test 3: Switch with calls */
    printf("Test 3: Switch statement with calls...\n");
    for (int i = 0; i < 4; i++) {
        double switch_result = test_switch_calls(i, 100.0 + i);
        checksum += switch_result;
        printf("  Case %d: %f\n", i, switch_result);
    }
    
    /* Test 4: setjmp test */
    printf("Test 4: setjmp/longjmp pattern...\n");
    double setjmp_result = test_setjmp_calls(50.0);
    checksum += setjmp_result;
    printf("  setjmp result: %f\n", setjmp_result);
    
    /* Test 5: Nested calls */
    printf("Test 5: Nested calls with register pressure...\n");
    double nested_result = test_nested_calls(25.0);
    checksum += nested_result;
    printf("  Nested result: %f\n", nested_result);
    
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
