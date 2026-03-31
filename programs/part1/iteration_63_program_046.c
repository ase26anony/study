/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic
 * and trigger the specific instruction reordering code in caller-save.cc
 * (lines 905-913) that manipulates basic block instruction chains.
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
                  "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "xmm6", "xmm7", "memory");
}

/* Function that uses many registers, forcing spills */
__attribute__((noinline, optimize("O3")))
double compute_pressure(double a, double b, double c, double d,
                        double e, double f, double g, double h) {
    /* Complex computation using all parameters */
    double t1 = a * b + c * d;
    double t2 = e * f + g * h;
    double t3 = t1 * t2 - a * g;
    double t4 = b * h + c * e;
    double t5 = d * f - t3 * t4;
    
    /* Force register pressure with intermediate values */
    volatile double v1 = t1 + t2;
    volatile double v2 = t3 - t4;
    
    return t5 * v1 / v2;
}

/* Function with mixed float/int operations */
__attribute__((noinline))
long long mixed_ops(int a, int b, float c, double d, long long e) {
    /* Use all parameters in computation */
    double fd = (double)a * c + d;
    long long li = e * b + (long long)fd;
    float fc = c * b - a;
    
    /* Clobber registers between computations */
    asm volatile("" : : : "rax", "rdx", "xmm0", "xmm1", "xmm2");
    
    return li + (long long)(fc * fd);
}

/* ========== Test Functions for Different Patterns ========== */

/* Test 1: Many live variables across a function call */
__attribute__((noinline, optimize("O3")))
double test1_caller_save_chain() {
    /* Declare many variables to exceed callee-saved registers */
    double v1 = 1.1, v2 = 2.2, v3 = 3.3, v4 = 4.4;
    double v5 = 5.5, v6 = 6.6, v7 = 7.7, v8 = 8.8;
    double v9 = 9.9, v10 = 10.10, v11 = 11.11, v12 = 12.12;
    
    /* Use them in computation before call */
    double sum1 = v1 + v2 + v3 + v4;
    double sum2 = v5 + v6 + v7 + v8;
    double prod = v9 * v10 * v11 * v12;
    
    /* Function call that clobbers registers - forces saves */
    clobber_registers();
    
    /* Use the values after call - forces restores */
    double result = sum1 * sum2 / prod;
    
    /* More computations with same variables */
    v1 += 0.1; v2 -= 0.1; v3 *= 1.1; v4 /= 1.1;
    
    /* Another call creating new save/restore point */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                  "xmm0", "xmm1", "xmm2", "xmm3", "memory");
    
    return result + v1 + v2 + v3 + v4;
}

/* Test 2: Loop with function calls and register pressure */
__attribute__((noinline))
double test2_loop_caller_save() {
    double total = 0.0;
    
    for (int i = 0; i < 10; i++) {
        /* Loop-invariant calculations in registers */
        double a = i * 1.1;
        double b = i * 2.2;
        double c = i * 3.3;
        double d = i * 4.4;
        
        /* Function call inside loop - registers must be saved */
        double partial = compute_pressure(a, b, c, d, 
                                         a+1, b+1, c+1, d+1);
        
        /* Use result immediately after call */
        total += partial;
        
        /* More computations that reuse same registers */
        a = partial * 0.5;
        b = partial * 0.25;
        
        /* Another call with different arguments */
        asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "memory");
        
        total += a + b;
    }
    
    return total;
}

/* Test 3: Conditional branches with caller-save */
__attribute__((noinline, optimize("O3")))
long long test3_branch_caller_save(int mode) {
    /* Many live variables */
    long long a = 100, b = 200, c = 300, d = 400;
    double x = 1.5, y = 2.5, z = 3.5;
    
    switch (mode % 4) {
        case 0:
            /* Branch with computation then call */
            a = a * b + c;
            clobber_registers();
            x = x * y + z;
            break;
            
        case 1:
            /* Different computation pattern */
            b = c * d - a;
            asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "memory");
            y = x * z - y;
            break;
            
        case 2:
            /* Chain of calls */
            clobber_registers();
            c = mixed_ops(a, b, x, y, c);
            clobber_registers();
            break;
            
        case 3:
            /* Complex path with multiple saves */
            d = a + b + c;
            x = compute_pressure(x, y, z, 4.5, 5.5, 6.6, 7.7, 8.8);
            d += (long long)x;
            break;
    }
    
    /* Common continuation using all variables */
    return a + b + c + d + (long long)(x + y + z);
}

/* Test 4: setjmp/longjmp pattern */
static jmp_buf env;
__attribute__((noinline))
int test4_setjmp_caller_save() {
    /* Variables that must survive longjmp */
    volatile int a = 42;
    volatile double b = 3.14159;
    volatile long long c = 9999999999LL;
    
    if (setjmp(env) == 0) {
        /* First time through */
        a *= 2;
        b *= 2.0;
        c *= 2;
        
        /* Call that might trigger saves */
        clobber_registers();
        
        /* longjmp back */
        longjmp(env, 1);
        
        __builtin_unreachable();
    } else {
        /* After longjmp - values should be restored */
        return a + (int)b + (int)c;
    }
}

/* Test 5: Multiple predecessor blocks */
__attribute__((noinline, optimize("O3")))
double test5_multiple_predecessors(int n) {
    double result = 0.0;
    
    /* Create multiple basic blocks that merge */
    if (n > 0) {
        double a = n * 1.1;
        double b = n * 2.2;
        
        /* Call in one predecessor */
        result = compute_pressure(a, b, a+1, b+1, 
                                 a+2, b+2, a+3, b+3);
        
        /* Use result in more computations */
        a = result * 0.5;
        b = result * 0.25;
        
        asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
    } else {
        double c = -n * 3.3;
        double d = -n * 4.4;
        
        /* Different call in other predecessor */
        clobber_registers();
        
        result = c * d - c / d;
        
        /* Different computation pattern */
        c = result + 10.0;
        d = result - 10.0;
        
        asm volatile("" : : : "rax", "rbx", "xmm4", "xmm5", "memory");
    }
    
    /* Common successor block */
    result += sin(result) * cos(result);
    
    /* Another call at block end */
    mixed_ops((int)result, (int)(result*2), 
              (float)result, result*3, (long long)result);
    
    return result;
}

/* ========== Main Orchestrator ========== */

int main() {
    double total = 0.0;
    long long checksum = 0;
    
    printf("Starting caller-save stress tests...\n");
    
    /* Run all tests multiple times to increase coverage */
    for (int i = 0; i < 3; i++) {
        /* Test 1: Basic caller-save chain */
        double r1 = test1_caller_save_chain();
        total += r1;
        checksum ^= (long long)r1;
        printf("Test1 iteration %d: %f\n", i, r1);
        
        /* Test 2: Loop pattern */
        double r2 = test2_loop_caller_save();
        total += r2;
        checksum ^= (long long)r2;
        printf("Test2 iteration %d: %f\n", i, r2);
        
        /* Test 3: Branching pattern */
        long long r3 = test3_branch_caller_save(i);
        total += (double)r3;
        checksum ^= r3;
        printf("Test3 iteration %d: %lld\n", i, r3);
        
        /* Test 4: setjmp pattern */
        int r4 = test4_setjmp_caller_save();
        total += r4;
        checksum ^= r4;
        printf("Test4 iteration %d: %d\n", i, r4);
        
        /* Test 5: Multiple predecessors */
        double r5 = test5_multiple_predecessors(i - 1);
        total += r5;
        checksum ^= (long long)r5;
        printf("Test5 iteration %d: %f\n", i, r5);
    }
    
    /* Final validation to prevent dead code elimination */
    printf("\nFinal total: %f\n", total);
    printf("Checksum: %llx\n", (unsigned long long)checksum);
    
    /* Use results in volatile store */
    volatile double final_check = total;
    volatile long long final_cs = checksum;
    
    return (final_check > 0 && final_cs != 0) ? 0 : 1;
}
