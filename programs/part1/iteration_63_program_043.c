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

/* Force caller-save by clobbering many registers */
__attribute__((noinline))
void clobber_registers() {
    /* Clobber multiple caller-saved registers */
    asm volatile("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi",
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
    double result = (c * d) / (a + 1) + (b * g) - (e / 1000.0) + (f * h);
    
    /* Force register clobbering */
    clobber_registers();
    
    /* More computation that needs original values */
    result += (v1 * v2) - (a * b) + (c - d);
    
    return result;
}

/* ========== Test Functions Targeting Specific Patterns ========== */

/* Test 1: Many live variables across a function call */
__attribute__((noinline, optimize("O2")))
void test_many_live_vars() {
    /* Declare many variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    double da = 1.1, db = 2.2, dc = 3.3, dd = 4.4;
    float fa = 1.5f, fb = 2.5f, fc = 3.5f;
    long long la = 1000LL, lb = 2000LL;
    
    /* Use all variables before call */
    int sum1 = a + b + c + d;
    double prod1 = da * db * dc;
    float sumf = fa + fb + fc;
    long long diff = la - lb;
    
    /* Function call that clobbers registers */
    double result = compute_pressure(a, b, da, db, la, fa, c, dc);
    
    /* Use all variables after call - forces restore */
    int sum2 = e + f + g + h + sum1;
    double prod2 = dd * prod1 * result;
    float finalf = sumf * fb;
    long long final_ll = diff + la + lb;
    
    /* Prevent dead code elimination */
    volatile int sink = sum2;
    volatile double sink_d = prod2;
    (void)sink; (void)sink_d; (void)finalf; (void)final_ll;
}

/* Test 2: Loop with function calls and invariant calculations */
__attribute__((noinline))
void test_loop_with_calls() {
    double accumulator = 0.0;
    
    for (int i = 0; i < 10; i++) {
        /* Loop invariants that need to survive across call */
        double invariant1 = sin(i * 0.1);
        double invariant2 = cos(i * 0.2);
        int loop_var = i * 2;
        
        /* Function call that clobbers registers */
        clobber_registers();
        
        /* Use invariants after call - forces restore */
        accumulator += invariant1 * invariant2 + loop_var;
        
        /* Additional computation to create scheduling opportunities */
        for (int j = 0; j < 3; j++) {
            double temp = invariant1 * j;
            accumulator += temp;
            clobber_registers();  /* Another clobber inside nested loop */
            accumulator -= temp / 2.0;
        }
    }
    
    volatile double sink = accumulator;
    (void)sink;
}

/* Test 3: Conditional branches with different restore points */
__attribute__((noinline, optimize("O3")))
void test_conditional_restores(int mode) {
    /* Variables that will be live across multiple calls */
    int a = 100, b = 200, c = 300;
    double x = 1.234, y = 5.678;
    
    switch (mode % 4) {
        case 0: {
            /* Complex computation before call */
            double temp = x * y + a - b;
            clobber_registers();
            /* Restore needed here */
            c = (int)(temp + x + y);
            break;
        }
        case 1: {
            /* Different computation pattern */
            int temp = a * b + c;
            clobber_registers();
            /* Restore with different register usage */
            x = temp / (y + 1.0);
            break;
        }
        case 2: {
            /* Chain of computations with calls in between */
            a = a * 2;
            clobber_registers();
            b = b + a;
            clobber_registers();
            c = c * b;
            clobber_registers();
            x = x / c;
            break;
        }
        case 3: {
            /* Mix float/int operations */
            float fa = (float)a;
            float fb = (float)b;
            clobber_registers();
            /* Engage different register banks */
            asm volatile("" : : : "xmm8", "xmm9", "xmm10");
            c = (int)(fa * fb);
            break;
        }
    }
    
    /* Final use to ensure values are live */
    volatile int sink = a + b + c;
    volatile double sink_d = x + y;
    (void)sink; (void)sink_d;
}

/* Test 4: setjmp/longjmp with caller-save requirements */
jmp_buf env;
int jmp_val = 0;

__attribute__((noinline))
void function_with_setjmp() {
    int local1 = 42;
    double local2 = 3.14159;
    float local3 = 2.718f;
    
    if (setjmp(env) == 0) {
        /* First call - modify locals */
        local1 *= 2;
        local2 *= 2.0;
        local3 *= 2.0f;
        
        /* Call that might longjmp */
        if (jmp_val == 0) {
            jmp_val = 1;
            longjmp(env, 1);
        }
    } else {
        /* After longjmp - use locals (must be restored) */
        volatile int sink1 = local1;
        volatile double sink2 = local2;
        volatile float sink3 = local3;
        (void)sink1; (void)sink2; (void)sink3;
    }
}

/* Test 5: Multiple basic blocks with different predecessor edges */
__attribute__((noinline, optimize("O2")))
void test_multiple_predecessors(int n) {
    int a = 1, b = 2, c = 3;
    double x = 1.0, y = 2.0;
    
    /* Create multiple control flow paths */
    if (n > 0) {
        a = n * 10;
        clobber_registers();
        x = sin(a);
    } else {
        b = abs(n) * 5;
        clobber_registers();
        y = cos(b);
    }
    
    /* Common block with restore instructions */
    int result = a + b + c;
    double dresult = x + y;
    
    /* Another branch point */
    if (result > 50) {
        clobber_registers();
        result *= 2;
    } else {
        clobber_registers();
        dresult *= 2.0;
    }
    
    /* Use __builtin_unreachable to affect block analysis */
    if (result < 0) {
        __builtin_unreachable();
    }
    
    volatile int sink = result;
    volatile double sink_d = dresult;
    (void)sink; (void)sink_d;
}

/* ========== Main Orchestration Function ========== */

int main() {
    printf("Starting caller-save stress tests...\n");
    
    /* Run all tests multiple times with different parameters */
    for (int i = 0; i < 5; i++) {
        test_many_live_vars();
        test_loop_with_calls();
        test_conditional_restores(i);
        test_multiple_predecessors(i - 2);
    }
    
    function_with_setjmp();
    
    printf("All tests completed.\n");
    
    /* Final validation to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += i;
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
