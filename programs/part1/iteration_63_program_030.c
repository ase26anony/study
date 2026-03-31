/* caller-save-test.c
 * 
 * A comprehensive test program designed to trigger the specific instruction
 * reordering logic in GCC's caller-save pass (lines 905-913 of caller-save.cc).
 * This code creates scenarios where caller-save restoration instructions
 * are inserted and then repositioned within basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force register pressure by using many live variables */
__attribute__((noinline, optimize("O3")))
int pressure_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many intermediate values that must be kept alive */
    int t1 = a * b + c;
    int t2 = d * e - f;
    int t3 = g * h / 2;
    int t4 = t1 ^ t2;
    int t5 = t2 | t3;
    int t6 = t3 & t1;
    int t7 = t4 + t5;
    int t8 = t6 * t7;
    int t9 = t8 - t4;
    int t10 = t9 / (t5 + 1);
    
    /* Clobber caller-saved registers explicitly */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15", "memory");
    
    return t10 + t1 + t2 + t3;
}

/* Function that uses floating point registers extensively */
__attribute__((noinline))
double fp_pressure(double a, double b, double c, double d, 
                   double e, double f, double g, double h) {
    volatile double v1 = a;  /* Prevent optimization */
    volatile double v2 = b;
    double r1 = v1 * v2 + c;
    double r2 = d * e - f;
    double r3 = g * h / 2.0;
    double r4 = sin(r1) * cos(r2);
    double r5 = exp(r3) * log(fabs(r4) + 1.0);
    double r6 = r1 * r2 * r3 * r4 * r5;
    
    /* Clobber FP/SIMD registers */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15", "memory");
    
    return r6 + a + b;
}

/* External function declaration to force call */
extern int external_helper(int x, int y);

/* ========== Test Case 1: Basic Block with Multiple Live Ranges ========== */

__attribute__((noinline, optimize("O3")))
int test_basic_block_reordering(int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed / 2;
    int d = seed - 11;
    int e = seed ^ 0x55;
    int f = seed | 0xAA;
    int g = seed & 0xF0;
    int h = seed << 2;
    
    /* These values must survive the function call */
    int sum1 = a + b + c + d;
    int sum2 = e + f + g + h;
    int prod1 = a * b * c;
    int prod2 = d * e * f;
    
    /* Function call that clobbers registers */
    int temp = pressure_function(a, b, c, d, e, f, g, h);
    
    /* Use all the pre-call values after the call */
    int result = sum1 * temp + sum2;
    result += prod1 - prod2;
    result ^= (a ^ b ^ c ^ d);
    result |= (e & f & g & h);
    
    /* Another call to create more restore placement opportunities */
    temp = pressure_function(result, sum1, sum2, prod1, 
                            prod2, a, b, c);
    
    return result + temp;
}

/* ========== Test Case 2: Loop with Caller-Save Across Iterations ========== */

__attribute__((noinline))
int test_loop_caller_save(int iterations) {
    int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = i * iterations;
    }
    
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Loop-invariant computations that need registers */
        int t1 = array[0] * i + array[1];
        int t2 = array[2] - i * array[3];
        int t3 = array[4] ^ (i & array[5]);
        int t4 = array[6] | (i + array[7]);
        
        /* These must survive the function call */
        acc1 += t1;
        acc2 += t2;
        acc3 += t3;
        acc4 += t4;
        
        /* Function call inside loop - requires saving registers each iteration */
        int call_result = external_helper(t1, t2);
        
        /* More computations with pre-call values */
        acc5 += t1 * call_result;
        acc6 += t2 - call_result;
        acc7 += t3 ^ call_result;
        acc8 += t4 | call_result;
        
        /* Modify array elements to create varying live ranges */
        array[i % 8] = call_result;
    }
    
    /* Final computation using all accumulators */
    int final = acc1 + acc2 + acc3 + acc4;
    final *= (acc5 - acc6);
    final ^= (acc7 | acc8);
    
    return final;
}

/* ========== Test Case 3: Conditional Blocks with Different Call Sites ========== */

__attribute__((noinline, optimize("O3")))
int test_conditional_blocks(int mode, int x) {
    int a = x * 2;
    int b = x + 100;
    int c = x / 3;
    int d = x - 50;
    
    int result = 0;
    
    switch (mode % 4) {
        case 0: {
            /* Block with floating point operations */
            double fa = a, fb = b, fc = c, fd = d;
            double fres = fp_pressure(fa, fb, fc, fd, fa*2, fb/2, fc+1, fd-1);
            
            /* Integer computations that must survive FP call */
            int t1 = a * b;
            int t2 = c + d;
            result = (int)fres + t1 - t2;
            
            /* Another call in the same block */
            result = pressure_function(result, t1, t2, a, b, c, d, x);
            break;
        }
        case 1: {
            /* Different register usage pattern */
            long long la = a, lb = b, lc = c, ld = d;
            long long lres = la * lb + lc - ld;
            
            /* Call with different argument types */
            int t3 = pressure_function(a, b, (int)lres, d, 
                                      (int)(la >> 32), (int)lb, 
                                      (int)lc, (int)ld);
            
            result = t3 + (int)lres;
            break;
        }
        case 2: {
            /* Chain of dependent computations with calls in between */
            int t4 = a * 3;
            int t5 = b + t4;
            
            /* First call */
            int tmp1 = pressure_function(t4, t5, c, d, a, b, x, mode);
            
            int t6 = tmp1 ^ t4;
            int t7 = t5 | t6;
            
            /* Second call */
            int tmp2 = pressure_function(t6, t7, tmp1, t4, t5, a, b, c);
            
            result = t6 * t7 + tmp1 - tmp2;
            break;
        }
        case 3: {
            /* Mixed float/int operations */
            double da = a * 1.5;
            double db = b / 2.0;
            int t8 = c * d;
            
            /* Call that clobbers both int and FP regs */
            double dres = fp_pressure(da, db, da+1, db-1, 
                                     da*2, db/2, da*db, da/db);
            
            int t9 = pressure_function(t8, a, b, c, d, (int)da, (int)db, x);
            
            result = t8 + t9 + (int)dres;
            break;
        }
    }
    
    /* Common tail with another call */
    if (result > 0) {
        result = pressure_function(result, a, b, c, d, x, mode, result % 100);
    } else {
        result = external_helper(result, a + b + c + d);
    }
    
    return result;
}

/* ========== Test Case 4: setjmp/longjmp Pattern ========== */

static jmp_buf env;
static volatile int jmp_value = 0;

__attribute__((noinline))
int test_setjmp_pattern(int val) {
    int a = val * 11;
    int b = val + 22;
    int c = val / 3;
    int d = val - 44;
    
    /* Values that must survive longjmp */
    volatile int save1 = a;
    volatile int save2 = b;
    volatile int save3 = c;
    volatile int save4 = d;
    
    if (setjmp(env) == 0) {
        /* First execution path */
        int t1 = save1 * save2;
        int t2 = save3 + save4;
        
        /* Call that might trigger longjmp */
        if (val % 7 == 0) {
            jmp_value = t1 + t2;
            longjmp(env, 1);
        }
        
        return pressure_function(t1, t2, save1, save2, save3, save4, val, 0);
    } else {
        /* After longjmp - must restore registers */
        return pressure_function(jmp_value, save1, save2, save3, save4, 
                                val, jmp_value % 100, 1);
    }
}

/* ========== Test Case 5: Unreachable Code After Call ========== */

__attribute__((noinline, optimize("O3")))
int test_unreachable_pattern(int x) {
    int a = x * 3;
    int b = x + 7;
    int c = x / 2;
    
    /* Call with side effects */
    int res = pressure_function(a, b, c, x, a^b, b|c, c&a, x%10);
    
    if (res < 0) {
        /* This creates interesting control flow */
        return res;
    }
    
    /* Compiler might think this is unreachable in some paths */
    if (x > 1000) {
        external_helper(res, x);
        __builtin_unreachable();
    }
    
    /* More computations requiring register restoration */
    int d = a * b - c;
    int e = pressure_function(d, res, a, b, c, x, d%100, e);
    
    return e + res;
}

/* ========== Main Test Orchestrator ========== */

int main() {
    int total_result = 0;
    
    printf("Starting caller-save test patterns...\n");
    
    /* Run test 1: Basic block reordering */
    printf("Test 1: Basic block reordering...\n");
    for (int i = 0; i < 10; i++) {
        total_result ^= test_basic_block_reordering(i + 100);
    }
    
    /* Run test 2: Loop with caller-save */
    printf("Test 2: Loop with caller-save...\n");
    total_result += test_loop_caller_save(5);
    
    /* Run test 3: Conditional blocks */
    printf("Test 3: Conditional blocks...\n");
    for (int i = 0; i < 8; i++) {
        total_result += test_conditional_blocks(i, 50 + i);
    }
    
    /* Run test 4: setjmp/longjmp */
    printf("Test 4: setjmp/longjmp pattern...\n");
    for (int i = 0; i < 5; i++) {
        total_result ^= test_setjmp_pattern(20 + i * 3);
    }
    
    /* Run test 5: Unreachable code */
    printf("Test 5: Unreachable code pattern...\n");
    total_result += test_unreachable_pattern(500);
    
    printf("Final checksum: %d\n", total_result);
    
    /* Validate results to prevent dead code elimination */
    if (total_result == 0) {
        printf("WARNING: All computations eliminated by optimizer!\n");
        return 1;
    }
    
    return 0;
}

/* External function definition */
int external_helper(int x, int y) {
    /* Simple function that gets called externally */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "memory");
    return x * y + (x ^ y) - (x & y);
}
