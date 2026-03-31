/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc (lines 905-913). It creates scenarios where:
 * 1. Caller-saved registers must be saved/restored around function calls
 * 2. Basic block boundaries are manipulated through control flow
 * 3. High register pressure forces spill decisions
 * 4. Instruction scheduling interacts with restore placement
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Function 1: Extensive FP calculations with inline asm clobbering */
__attribute__((noinline, optimize("O3")))
double fp_calculations(double a, double b, double c, double d, 
                       double e, double f, double g, double h) {
    /* Create many live FP values across asm statement */
    double t1 = a * b + c;
    double t2 = d / e - f;
    double t3 = g * h + a;
    double t4 = b - c * d;
    
    /* Clobber caller-saved FP registers */
    asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", 
                           "xmm4", "xmm5", "xmm6", "xmm7", "memory");
    
    /* Use all values after clobber */
    return t1 * t2 + t3 / t4;
}

/* Function 2: Integer calculations with mixed operations */
__attribute__((noinline))
long long int_calculations(long long a, long long b, long long c,
                           long long d, long long e, long long f) {
    volatile long long v1 = a;  /* Prevent optimization */
    long long t1 = v1 * b + c;
    long long t2 = d ^ e | f;
    long long t3 = (a << 3) | (b >> 2);
    
    /* Clobber general purpose registers */
    asm volatile("" ::: "rax", "rbx", "rcx", "rdx", 
                           "rsi", "rdi", "r8", "r9", "r10", "memory");
    
    return t1 + t2 * t3;
}

/* Function 3: Mixed int/float operations */
__attribute__((noinline, optimize("O3")))
float mixed_operations(int a, float b, double c, long long d) {
    float f1 = b * 2.0f;
    double d1 = c * 3.14159;
    int i1 = a * 2 + (int)d;
    
    /* Clobber mixed registers */
    asm volatile("" ::: "xmm0", "xmm1", "rax", "rbx", "rcx", "memory");
    
    return f1 + (float)d1 + i1;
}

/* External function declaration to prevent inlining analysis */
extern void external_call(int, double, long long);

/* ========== Test Functions with Different Control Flow ========== */

/* Test 1: Loop with function call and live variables */
__attribute__((noinline, optimize("O2")))
double test_loop_caller_save(int iterations) {
    double result = 0.0;
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4;
    double e = 5.5, f = 6.6, g = 7.7, h = 8.8;
    
    for (int i = 0; i < iterations; i++) {
        /* Live variables across function call */
        double temp = fp_calculations(a, b, c, d, e, f, g, h);
        
        /* Modify variables that must survive across call */
        a += 0.1;
        b *= 1.01;
        c = sin(c);
        d = cos(d);
        
        result += temp;
        
        /* Call external function with current values */
        external_call(i, result, (long long)(a * 1000));
    }
    
    return result;
}

/* Test 2: Complex conditional with multiple calls */
__attribute__((noinline))
long long test_conditional_calls(int mode, long long seed) {
    long long a = seed * 2;
    long long b = seed + 1000;
    long long c = seed ^ 0xABCDEF;
    long long d = seed >> 4;
    long long e = seed << 2;
    long long f = ~seed;
    
    long long result = 0;
    
    if (mode == 0) {
        result = int_calculations(a, b, c, d, e, f);
        /* Create another live range */
        a = result + 100;
        b = result - 50;
        result = int_calculations(b, a, c, d, e, f);
    } 
    else if (mode == 1) {
        /* Different call pattern */
        result = int_calculations(f, e, d, c, b, a);
        /* Force register pressure */
        volatile long long v1 = a + b;
        volatile long long v2 = c + d;
        result += v1 * v2;
    }
    else {
        /* Third path with mixed operations */
        float f1 = mixed_operations((int)a, (float)b, (double)c, d);
        double d1 = fp_calculations(f1, f1*2, f1*3, f1*4, 
                                   f1*5, f1*6, f1*7, f1*8);
        result = (long long)(d1 * 1000);
    }
    
    /* Use __builtin_unreachable to affect block analysis */
    if (mode > 100) {
        __builtin_unreachable();
    }
    
    return result;
}

/* Test 3: Switch statement with varying call patterns */
__attribute__((noinline, optimize("O3")))
double test_switch_calls(int case_id, double init) {
    double result = init;
    double a = init * 1.1;
    double b = init * 2.2;
    double c = init * 3.3;
    double d = init * 4.4;
    
    switch (case_id) {
        case 0:
            result = fp_calculations(a, b, c, d, a*2, b*2, c*2, d*2);
            /* Create multiple uses of result */
            a = result * 0.5;
            b = result * 0.25;
            result = fp_calculations(b, a, d, c, a*3, b*3, c*3, d*3);
            break;
            
        case 1:
            result = mixed_operations((int)a, (float)b, c, (long long)d);
            /* Chain computations */
            for (int i = 0; i < 3; i++) {
                result += fp_calculations(result, a, b, c, d, 
                                         a+i, b+i, c+i);
            }
            break;
            
        case 2:
            /* Nested calls */
            result = fp_calculations(
                fp_calculations(a, b, c, d, a, b, c, d),
                b, c, d, a, b, c, d
            );
            break;
            
        default:
            /* Default path with high register pressure */
            double e = a * 5.5;
            double f = b * 6.6;
            double g = c * 7.7;
            double h = d * 8.8;
            result = fp_calculations(a, b, c, d, e, f, g, h);
            result += fp_calculations(e, f, g, h, a, b, c, d);
            break;
    }
    
    return result;
}

/* Test 4: setjmp/longjmp test for special caller-save handling */
static jmp_buf env;
__attribute__((noinline))
double test_setjmp_calls(double init) {
    double a = init * 1.5;
    double b = init * 2.5;
    double c = init * 3.5;
    
    if (setjmp(env) == 0) {
        /* First call - registers must be saved */
        double result = fp_calculations(a, b, c, a*2, b*2, c*2, a*3, b*3);
        
        /* Modify variables */
        a += result * 0.1;
        b += result * 0.2;
        c += result * 0.3;
        
        /* Simulate longjmp - forces restore from save area */
        longjmp(env, 1);
        
        return result;  /* Never reached */
    } else {
        /* After longjmp - restored values should be used */
        return fp_calculations(a, b, c, a, b, c, a, b);
    }
}

/* Test 5: Instruction scheduling interaction */
__attribute__((optimize("O3"), noinline))
double test_scheduling(double a, double b, double c) {
    /* Create dependent chain where restore placement matters */
    double t1 = a * b;
    double t2 = t1 + c;
    double t3 = sin(t2);
    double t4 = cos(t3);
    
    /* Function call breaks the chain */
    double t5 = fp_calculations(t1, t2, t3, t4, a, b, c, t1);
    
    /* Continue chain after call */
    double t6 = t5 * t4;
    double t7 = t6 / t3;
    double t8 = exp(t7);
    
    /* Another call */
    t8 += mixed_operations((int)t8, (float)t7, t6, (long long)t5);
    
    return t8 * t1 + t2;
}

/* ========== External Function Implementation ========== */

/* Simple external function that clobbers registers */
void external_call(int x, double y, long long z) {
    /* Clobber important registers */
    asm volatile("" ::: "rax", "rbx", "xmm0", "xmm1", "xmm2", "memory");
    
    /* Prevent dead code elimination */
    volatile int dummy = x + (int)y + (int)z;
    (void)dummy;
}

/* ========== Main Orchestration ========== */

int main() {
    double total_checksum = 0.0;
    
    printf("Starting caller-save stress tests...\n");
    
    /* Test 1: Loop with calls */
    printf("Test 1: Loop caller-save...\n");
    double result1 = test_loop_caller_save(5);
    total_checksum += result1;
    printf("  Result: %f\n", result1);
    
    /* Test 2: Conditional calls */
    printf("Test 2: Conditional calls...\n");
    for (int i = 0; i < 3; i++) {
        long long result2 = test_conditional_calls(i, 12345 + i);
        total_checksum += (double)result2;
        printf("  Mode %d: %lld\n", i, result2);
    }
    
    /* Test 3: Switch calls */
    printf("Test 3: Switch calls...\n");
    for (int i = 0; i < 4; i++) {
        double result3 = test_switch_calls(i, 10.0 + i);
        total_checksum += result3;
        printf("  Case %d: %f\n", i, result3);
    }
    
    /* Test 4: setjmp/longjmp */
    printf("Test 4: setjmp/longjmp...\n");
    double result4 = test_setjmp_calls(5.0);
    total_checksum += result4;
    printf("  Result: %f\n", result4);
    
    /* Test 5: Scheduling */
    printf("Test 5: Instruction scheduling...\n");
    double result5 = test_scheduling(1.5, 2.5, 3.5);
    total_checksum += result5;
    printf("  Result: %f\n", result5);
    
    /* Final checksum */
    printf("\nTotal checksum: %f\n", total_checksum);
    printf("All tests completed.\n");
    
    return 0;
}
