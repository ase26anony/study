/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc (lines 905-913). It creates scenarios where:
 * 1. Caller-saved registers must be saved/restored around function calls
 * 2. Basic block boundaries are manipulated with conditional control flow
 * 3. High register pressure forces spill decisions
 * 4. Instruction scheduling interacts with restore placement
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force no-inline to create clear call boundaries */
__attribute__((noinline, optimize("O3")))
double compute_pressure(int a, int b, double c, float d, long long e) {
    /* Complex expression using multiple registers */
    volatile int v1 = a * 3;  /* Prevent optimization */
    volatile float v2 = d * 2.5f;
    double result = c * (b + v1) / (v2 + 1.0);
    
    /* Clobber caller-saved registers explicitly */
    asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    return result + e;
}

__attribute__((noinline))
void clobber_registers(void) {
    /* Aggressive clobbering of both integer and floating-point registers */
    asm volatile("" : : : 
                 "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                 "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
}

/* External function declaration to prevent optimization */
extern void external_call(int, double, float);

/* ========== Test Case 1: Basic Block with Multiple Live Ranges ========== */

__attribute__((noinline, optimize("O3")))
int test_basic_block_restore(void) {
    /* Create many live variables that must survive across call */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    float fa = 1.1f, fb = 2.2f, fc = 3.3f, fd = 4.4f;
    double da = 1.11, db = 2.22, dc = 3.33, dd = 4.44;
    long long la = 1000, lb = 2000, lc = 3000, ld = 4000;
    
    /* Computation before call - uses all variables */
    int sum1 = a + b + c + d;
    float fsum1 = fa + fb + fc + fd;
    double dsum1 = da + db + dc + dd;
    long long lsum1 = la + lb + lc + ld;
    
    /* Function call that clobbers registers */
    clobber_registers();
    
    /* Computation after call - reuses same variables */
    /* This forces restore of original values */
    int sum2 = e + f + g + h + sum1;
    float fsum2 = fsum1 * fa * fb;
    double dsum2 = dsum1 / da * db;
    long long lsum2 = lsum1 - la + lb;
    
    /* Another call with different register usage */
    double pressure_result = compute_pressure(sum2, h, dsum2, fsum2, lsum2);
    
    /* Final computation mixing all types */
    return (int)(sum2 + fsum2 + dsum2 + lsum2 + pressure_result);
}

/* ========== Test Case 2: Loop with Caller-Save Across Iterations ========== */

__attribute__((noinline, optimize("O3")))
int test_loop_restore(void) {
    int result = 0;
    
    /* Loop with invariant calculations that need registers saved each iteration */
    for (int i = 0; i < 10; i++) {
        /* Live variables that must survive the external call */
        int a = i * 2, b = i * 3, c = i * 4;
        float fa = i * 1.5f, fb = i * 2.5f;
        double da = i * 1.25, db = i * 2.25;
        
        /* Pre-call computation */
        int pre_sum = a + b + c;
        float pre_fsum = fa * fb;
        double pre_dsum = da / db;
        
        /* External call clobbers registers */
        external_call(i, da, fa);
        
        /* Post-call computation needs original values restored */
        result += pre_sum + (int)pre_fsum + (int)pre_dsum;
        
        /* Additional computation to create more register pressure */
        result += compute_pressure(a, b, da, fa, pre_sum);
    }
    
    return result;
}

/* ========== Test Case 3: Conditional Blocks with Different Call Sites ========== */

__attribute__((noinline, optimize("O3")))
int test_conditional_restore(int mode) {
    /* Variables live across the switch */
    int a = 10, b = 20, c = 30, d = 40;
    float fa = 10.5f, fb = 20.5f;
    double da = 100.5, db = 200.5;
    
    /* Pre-switch computation */
    int base = a + b + c + d;
    float fbase = fa * fb;
    double dbase = da + db;
    
    switch (mode % 4) {
        case 0: {
            /* Each case calls different functions, creating different save/restore needs */
            int local = base * 2;
            clobber_registers();
            /* Need original a,b,c,d restored */
            return local + a + b;
        }
        case 1: {
            float local = fbase * 2.0f;
            double result = compute_pressure(base, c, dbase, local, d);
            /* Mixed type computation after call */
            return (int)(result + fa + fb);
        }
        case 2: {
            /* Multiple calls in sequence */
            clobber_registers();
            double temp = compute_pressure(a, b, da, fa, base);
            clobber_registers();
            return (int)(temp + c + d);
        }
        case 3: {
            /* Unreachable code hint might affect block analysis */
            external_call(base, dbase, fbase);
            if (mode < 0) {
                __builtin_unreachable();
            }
            return base + (int)fbase;
        }
    }
    
    return base;
}

/* ========== Test Case 4: setjmp/longjmp Caller-Save Requirements ========== */

static jmp_buf jump_buffer;
static int jmp_value = 0;

__attribute__((noinline, noipa))
void function_with_setjmp(int *result) {
    /* Variables that must be saved across longjmp */
    int a = 100, b = 200, c = 300;
    float fa = 100.5f, fb = 200.5f;
    double da = 1000.5, db = 2000.5;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        *result = a + b + c + (int)(fa + fb + da + db);
        
        /* Call that might trigger longjmp */
        clobber_registers();
        
        /* This code might not execute if longjmp is called */
        *result += compute_pressure(a, b, da, fa, c);
    } else {
        /* After longjmp - original values should be restored */
        *result += a * 2 + b * 3 + c * 4;
        *result += (int)(fa * fb + da / db);
    }
}

/* ========== Test Case 5: Mixed Float/Int Operations ========== */

__attribute__((noinline, optimize("O3")))
float test_mixed_operations(int iterations) {
    /* Create pressure on both integer and floating-point register banks */
    int int_acc = 0;
    float float_acc = 0.0f;
    double double_acc = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Integer computation */
        int a = i * 2, b = i * 3, c = i * 4;
        int_acc += a + b + c;
        
        /* Floating computation */
        float fa = i * 1.1f, fb = i * 2.2f;
        float_acc += fa * fb;
        
        /* Double computation */
        double da = i * 1.11, db = i * 2.22;
        double_acc += da - db;
        
        /* Function call clobbers registers */
        if (i % 3 == 0) {
            clobber_registers();
        } else if (i % 3 == 1) {
            double temp = compute_pressure(a, b, da, fa, int_acc);
            double_acc += temp;
        } else {
            external_call(a, da, fa);
        }
        
        /* Post-call computation needs all values restored */
        int_acc += (int)(float_acc + double_acc);
        float_acc += (float)(int_acc % 100);
        double_acc += int_acc / 100.0;
    }
    
    return float_acc + (float)double_acc + (float)int_acc;
}

/* ========== Main Orchestration Function ========== */

int main(void) {
    int total_checksum = 0;
    
    printf("Starting caller-save restoration stress test...\n");
    
    /* Test 1: Basic block restoration */
    printf("Test 1: Basic block with multiple live ranges\n");
    int result1 = test_basic_block_restore();
    total_checksum += result1;
    printf("  Result: %d\n", result1);
    
    /* Test 2: Loop restoration */
    printf("Test 2: Loop with caller-save across iterations\n");
    int result2 = test_loop_restore();
    total_checksum += result2;
    printf("  Result: %d\n", result2);
    
    /* Test 3: Conditional restoration */
    printf("Test 3: Conditional blocks with different call sites\n");
    for (int i = 0; i < 8; i++) {
        int result3 = test_conditional_restore(i);
        total_checksum += result3;
        if (i == 0) printf("  First result: %d\n", result3);
    }
    
    /* Test 4: setjmp/longjmp */
    printf("Test 4: setjmp/longjmp caller-save\n");
    int result4 = 0;
    function_with_setjmp(&result4);
    /* Trigger longjmp to test restoration */
    jmp_value = 1;
    longjmp(jump_buffer, 1);
    total_checksum += result4;
    printf("  Result: %d\n", result4);
    
    /* Test 5: Mixed operations */
    printf("Test 5: Mixed float/int operations\n");
    float result5 = test_mixed_operations(20);
    total_checksum += (int)result5;
    printf("  Result: %f\n", result5);
    
    /* Final validation */
    printf("\nTotal checksum: %d\n", total_checksum);
    printf("If this number is consistent across runs, computations survived caller-save.\n");
    
    return 0;
}

/* Dummy external function definition */
void external_call(int a, double b, float c) {
    /* Empty but with memory clobber to prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
}
