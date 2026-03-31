/* caller-save-test.c
 * A comprehensive test to trigger caller-save restoration instruction
 * reordering within basic blocks, specifically targeting the uncovered
 * lines in caller-save.cc (lines 905-913).
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
                               "xmm4", "xmm5", "xmm6", "xmm7");
}

/* Function that uses many registers, forcing spills */
__attribute__((noinline, optimize("O3")))
double compute_pressure(int a, int b, double c, double d, 
                       long long e, float f, int g, double h) {
    volatile int v1 = a;  /* Prevent optimization */
    volatile double v2 = c;
    
    /* Complex computation using all parameters */
    double result = (a * b) + (c * d) + (e / 2.0) + (f * g) + h;
    
    /* Force register pressure with many temporaries */
    double t1 = result * 1.1;
    double t2 = t1 / 0.9;
    double t3 = t2 + sin(result);
    double t4 = t3 * cos(result);
    double t5 = t4 + tan(result);
    
    /* Clobber registers in the middle */
    clobber_registers();
    
    /* Continue using the temporaries after clobber */
    t1 = t5 * 2.0;
    t2 = t1 + v1;
    t3 = t2 * v2;
    
    return t3;
}

/* ========== Test Functions Targeting Specific Patterns ========== */

/* Test 1: Basic block with function call between computations */
__attribute__((noinline, optimize("O2")))
void test1_basic_block_reordering() {
    int a = 10, b = 20, c = 30, d = 40;
    double x = 1.5, y = 2.5, z = 3.5, w = 4.5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    long long l1 = 100, l2 = 200, l3 = 300, l4 = 400;
    
    /* First computation using all variables */
    int sum1 = a + b + c + d;
    double prod1 = x * y * z * w;
    float mix1 = f1 * f2 + f3 - f4;
    long long llsum1 = l1 + l2 + l3 + l4;
    
    /* Function call that clobbers caller-saved registers */
    clobber_registers();
    
    /* Second computation using same variables - forces restore */
    int sum2 = d + c + b + a;  /* Different order to prevent CSE */
    double prod2 = w * z * y * x;
    float mix2 = f4 * f3 + f2 - f1;
    long long llsum2 = l4 + l3 + l2 + l1;
    
    /* Use results to prevent dead code elimination */
    volatile int check = sum1 + sum2;
    volatile double check2 = prod1 + prod2;
    (void)check;
    (void)check2;
}

/* Test 2: Loop with function calls and register pressure */
__attribute__((noinline, optimize("O3")))
double test2_loop_caller_save(int iterations) {
    double accumulator = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Live variables across the function call */
        double a = i * 1.1;
        double b = sin(i * 0.1);
        double c = cos(i * 0.2);
        double d = tan(i * 0.3);
        int e = i * 2;
        int f = i * 3;
        float g = i * 0.5f;
        float h = i * 0.7f;
        
        /* Complex computation before call */
        double pre = (a * b) + (c * d) + (e * f) + (g * h);
        
        /* Function call - forces save/restore of all live vars */
        double result = compute_pressure(e, f, a, b, i, g, f, c);
        
        /* Computation after call using pre-call values */
        double post = pre * result;
        
        /* Mix int and float operations */
        if (i % 2 == 0) {
            accumulator += post * sin(a);
        } else {
            accumulator += post * cos(b);
        }
        
        /* Additional register pressure */
        volatile double temp = accumulator;
        accumulator = temp * 0.99;
    }
    
    return accumulator;
}

/* Test 3: Conditional branches with different caller-save patterns */
__attribute__((noinline, optimize("O2")))
int test3_conditional_blocks(int mode) {
    int a = 100, b = 200, c = 300, d = 400;
    double x = 10.5, y = 20.5, z = 30.5, w = 40.5;
    
    switch (mode % 4) {
        case 0: {
            /* Block with many live variables */
            int r1 = a + b;
            double r2 = x * y;
            
            clobber_registers();
            
            int r3 = c + d;
            double r4 = z * w;
            
            return r1 + r3 + (int)(r2 + r4);
        }
        case 1: {
            /* Different computation pattern */
            int r1 = a * b;
            double r2 = x / y;
            
            /* Inline asm with specific clobbers */
            asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1");
            
            int r3 = c * d;
            double r4 = z / w;
            
            return r1 - r3 + (int)(r2 - r4);
        }
        case 2: {
            /* Chain of computations with calls */
            int t1 = a + 1;
            double t2 = x + 1.0;
            
            clobber_registers();
            
            int t3 = t1 + b;
            double t4 = t2 + y;
            
            clobber_registers();
            
            int t5 = t3 + c;
            double t6 = t4 + z;
            
            return t5 + (int)t6;
        }
        default: {
            /* Mixed float/int operations */
            float f1 = (float)a;
            float f2 = (float)b;
            double d1 = (double)c;
            double d2 = (double)d;
            
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                                   "rax", "rbx");
            
            return (int)(f1 * f2 + d1 * d2);
        }
    }
}

/* Test 4: setjmp/longjmp caller-save requirements */
static jmp_buf env;
static volatile int setjmp_counter = 0;

__attribute__((noinline))
void setjmp_helper(int *ptr, double *dptr, float *fptr) {
    /* Modify values that might need restoration */
    *ptr += 100;
    *dptr *= 2.0;
    *fptr /= 2.0f;
    
    if (setjmp_counter++ < 3) {
        longjmp(env, 1);
    }
}

__attribute__((noinline))
int test4_setjmp_longjmp() {
    int local_int = 42;
    double local_double = 3.14159;
    float local_float = 2.71828f;
    
    if (setjmp(env) == 0) {
        /* First execution */
        local_int *= 2;
        local_double += 1.0;
        local_float -= 0.5f;
        
        /* Function call that does longjmp */
        setjmp_helper(&local_int, &local_double, &local_float);
        
        /* This point may be reached multiple times */
        __builtin_unreachable(); /* May affect block termination */
    } else {
        /* After longjmp - values might need restoration */
        local_int += 10;
        local_double -= 0.1;
        local_float += 0.2f;
    }
    
    return local_int + (int)local_double + (int)local_float;
}

/* Test 5: Multiple successive calls with overlapping live ranges */
__attribute__((noinline, optimize("O3")))
double test5_successive_calls() {
    /* Many variables with overlapping live ranges */
    double v1 = 1.0, v2 = 2.0, v3 = 3.0, v4 = 4.0;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    
    /* Chain of computations with intervening calls */
    double r1 = v1 * v2 + sin(v3);
    clobber_registers();
    
    double r2 = v3 * v4 + cos(v1);
    int r3 = i1 * i2 + i3 * i4;
    clobber_registers();
    
    float r4 = f1 * f2 + f3 * f4;
    double r5 = r1 * r2 + (double)r3;
    clobber_registers();
    
    double r6 = r5 * (double)r4 + v1 * v2 * v3 * v4;
    
    /* Force all results to be used */
    volatile double final = r1 + r2 + r5 + r6;
    return final;
}

/* ========== Main Orchestrator ========== */

int main() {
    printf("Starting caller-save restoration reordering tests...\n");
    
    /* Test 1: Basic block reordering */
    printf("Test 1: Basic block with function call...\n");
    for (int i = 0; i < 10; i++) {
        test1_basic_block_reordering();
    }
    
    /* Test 2: Loop with register pressure */
    printf("Test 2: Loop with caller-save across iterations...\n");
    double result2 = test2_loop_caller_save(50);
    printf("  Loop result: %f\n", result2);
    
    /* Test 3: Conditional blocks */
    printf("Test 3: Conditional branches with different patterns...\n");
    int sum3 = 0;
    for (int i = 0; i < 20; i++) {
        sum3 += test3_conditional_blocks(i);
    }
    printf("  Conditional sum: %d\n", sum3);
    
    /* Test 4: setjmp/longjmp */
    printf("Test 4: setjmp/longjmp caller-save...\n");
    int result4 = test4_setjmp_longjmp();
    printf("  setjmp result: %d\n", result4);
    
    /* Test 5: Successive calls */
    printf("Test 5: Multiple successive calls...\n");
    double result5 = test5_successive_calls();
    printf("  Successive calls result: %f\n", result5);
    
    /* Final validation */
    printf("\nAll tests completed.\n");
    printf("Total checksum: %f\n", 
           result2 + sum3 + result4 + result5);
    
    return 0;
}
