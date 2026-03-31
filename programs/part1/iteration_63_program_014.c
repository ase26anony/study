/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc lines 905-913.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Function that uses many caller-saved registers */
__attribute__((noinline, optimize("O3")))
double compute_pressure(double a, double b, double c, double d,
                       double e, double f, double g, double h) {
    /* Complex floating-point chain to use FP registers */
    double t1 = a * b + c * d;
    double t2 = e * f + g * h;
    double t3 = t1 * t2 - a * g;
    double t4 = b * h + c * e;
    
    /* Inline assembly that clobbers multiple caller-saved registers */
    asm volatile("" 
                 : 
                 : "r"(t1), "r"(t2), "r"(t3), "r"(t4)
                 : "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                   "xmm4", "xmm5", "xmm6", "xmm7",
                   "rax", "rbx", "rcx", "rdx");
    
    return t3 / (t4 + 1.0);
}

/* Function with mixed int/float operations */
__attribute__((noinline))
long long mixed_operations(int a, int b, float c, double d,
                          long long e, int f, float g, double h) {
    volatile int v1 = a;  /* Prevent optimization */
    volatile float v2 = c;
    
    /* Operations using different register banks */
    long long ll_result = e * (v1 + b);
    double fp_result = (d * h) + (c * g);
    
    /* Clobber both integer and floating-point registers */
    asm volatile("# Mixed clobber"
                 :
                 : "r"(ll_result), "r"(fp_result)
                 : "memory", "rax", "rbx", "xmm0", "xmm1");
    
    return ll_result + (long long)fp_result;
}

/* External function declaration to force caller-save */
extern void external_clobber(void);

/* Function that must preserve many live values across call */
__attribute__((noinline, optimize("O2")))
int preserve_across_call(int a, int b, int c, int d,
                        int e, int f, int g, int h,
                        int i, int j, int k, int l) {
    /* Many live variables exceeding callee-saved registers */
    int r1 = a * b + c;
    int r2 = d * e - f;
    int r3 = g * h / (i + 1);
    int r4 = j * k * l;
    int r5 = a + b + c + d;
    int r6 = e * f * g;
    int r7 = h * i * j;
    int r8 = k * l * a;
    int r9 = b * c * d;
    int r10 = e * f * g;
    
    /* Call that clobbers caller-saved registers */
    external_clobber();
    
    /* Use all variables after call - must be restored */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* ========== Control Flow Structures ========== */

/* Function with complex control flow */
__attribute__((noinline, optimize("O3")))
double control_flow_test(double base, int iterations) {
    double result = base;
    double accumulator = 0.0;
    
    /* Loop with function call inside */
    for (int i = 0; i < iterations; i++) {
        double a = result * i;
        double b = sin(result);
        double c = cos(accumulator);
        
        /* Call that uses FP registers */
        double temp = compute_pressure(a, b, c, result,
                                      accumulator, 1.0, 2.0, 3.0);
        
        /* Live values across call must be preserved */
        accumulator += temp;
        result = result * 0.99 + accumulator * 0.01;
        
        /* Conditional that creates basic block boundaries */
        if (i % 100 == 0) {
            /* Another call in conditional block */
            asm volatile("# Conditional clobber"
                         :
                         :
                         : "memory", "xmm0", "xmm1", "xmm2", "xmm3");
            accumulator *= 0.5;
        }
    }
    
    return result + accumulator;
}

/* Switch statement creating multiple control flow edges */
__attribute__((noinline))
int switch_test(int mode, int x, int y, int z) {
    int result = 0;
    
    switch (mode) {
        case 0: {
            /* Case with many live variables */
            int a = x * y;
            int b = y * z;
            int c = z * x;
            external_clobber();
            result = a + b + c;  /* Must restore a, b, c */
            break;
        }
        case 1: {
            /* Different register usage pattern */
            float f1 = x * 1.5f;
            float f2 = y * 2.5f;
            asm volatile("# Case 1 clobber"
                         :
                         :
                         : "memory", "xmm0", "xmm1", "rax", "rbx");
            result = (int)(f1 + f2);
            break;
        }
        case 2: {
            /* Chain of operations */
            long long ll = (long long)x * y * z;
            int i = x + y + z;
            double d = (double)i / 3.0;
            external_clobber();
            result = (int)(ll % 1000) + (int)d;
            break;
        }
        default:
            /* Unreachable to test edge case */
            __builtin_unreachable();
    }
    
    return result;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;

__attribute__((noinline))
int setjmp_test(int value) {
    volatile int preserved = value * 2;
    volatile double fp_preserved = sin(value * 3.14);
    
    if (setjmp(env) == 0) {
        /* Clobber registers before longjmp */
        asm volatile("# Before longjmp clobber"
                     :
                     :
                     : "memory", "rax", "rbx", "rcx", "rdx",
                       "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* Call that might trigger longjmp */
        if (value > 1000) {
            longjmp(env, 1);
        }
        
        return preserved + (int)fp_preserved;
    } else {
        /* After longjmp - registers must be restored */
        return preserved * 2 + (int)(fp_preserved * 3);
    }
}

/* ========== Main Test Orchestrator ========== */

/* External function definition */
void external_clobber(void) {
    /* Clobber many registers */
    asm volatile("# External clobber"
                 :
                 :
                 : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                   "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                   "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                   "xmm12", "xmm13", "xmm14", "xmm15");
}

int main(void) {
    int checksum = 0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Floating-point register pressure */
    double fp_result = control_flow_test(1.0, 1000);
    checksum += (int)fp_result;
    printf("Test 1 FP result: %f\n", fp_result);
    
    /* Test 2: Mixed int/float operations */
    long long mixed_result = mixed_operations(1, 2, 3.0f, 4.0,
                                            5, 6, 7.0f, 8.0);
    checksum += (int)(mixed_result % 1000000);
    printf("Test 2 mixed result: %lld\n", mixed_result);
    
    /* Test 3: Many live variables across call */
    int preserved_result = preserve_across_call(1, 2, 3, 4, 5, 6, 7, 8,
                                               9, 10, 11, 12);
    checksum += preserved_result;
    printf("Test 3 preserved result: %d\n", preserved_result);
    
    /* Test 4: Switch with different register usage patterns */
    for (int i = 0; i < 3; i++) {
        int switch_result = switch_test(i, i+1, i+2, i+3);
        checksum += switch_result;
        printf("Test 4 switch case %d: %d\n", i, switch_result);
    }
    
    /* Test 5: setjmp/longjmp */
    int setjmp_result = setjmp_test(500);
    checksum += setjmp_result;
    printf("Test 5 setjmp (no jump): %d\n", setjmp_result);
    
    setjmp_result = setjmp_test(1500);
    checksum += setjmp_result;
    printf("Test 5 setjmp (with jump): %d\n", setjmp_result);
    
    /* Final validation */
    printf("\nFinal checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return 0;
}
