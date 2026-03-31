/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc (lines 905-913). It creates scenarios where:
 * 1. Caller-saved registers must be saved/restored around function calls
 * 2. Basic block boundaries force interesting placement of restore instructions
 * 3. High register pressure causes complex spill/fill decisions
 * 4. Instruction scheduling interacts with caller-save placement
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

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d) {
    volatile double v1 = a * b;  /* Prevent optimization */
    volatile double v2 = c * d;
    volatile double v3 = a + c;
    volatile double v4 = b + d;
    
    /* Force caller-save of all these volatile values */
    clobber_registers();
    
    /* Use all values after call - forces restore */
    return (v1 * v2) + (v3 * v4) + (a * c) + (b * d);
}

/* Function that uses mixed float/int operations */
__attribute__((noinline))
long long mixed_operations(int a, float b, double c, long long d) {
    volatile float f1 = b * 2.0f;
    volatile double d1 = c * 3.0;
    volatile int i1 = a * 4;
    volatile long long ll1 = d * 5;
    
    /* Clobber both general and vector registers */
    asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "memory");
    
    /* Complex expression using all types */
    return (long long)((i1 * f1) + (ll1 * d1)) + a + (long long)b;
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline, optimize("O3")))
double loop_with_calls(int iterations) {
    double sum = 0.0;
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4;
    volatile double preserve[8];  /* Array to force spills */
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values across the call */
        preserve[0] = a * i;
        preserve[1] = b * i;
        preserve[2] = c * i;
        preserve[3] = d * i;
        preserve[4] = a + b;
        preserve[5] = c + d;
        preserve[6] = a * c;
        preserve[7] = b * d;
        
        /* Function call clobbers registers */
        clobber_registers();
        
        /* Use all preserved values - forces restores */
        sum += preserve[0] + preserve[1] + preserve[2] + preserve[3] +
               preserve[4] + preserve[5] + preserve[6] + preserve[7];
        
        /* Modify variables for next iteration */
        a += 0.1;
        b += 0.2;
        c += 0.3;
        d += 0.4;
    }
    
    return sum;
}

/* ========== Conditional Blocks with Caller-Save ========== */

__attribute__((noinline, optimize("O3")))
double conditional_caller_save(int mode) {
    double result = 0.0;
    
    /* Many live variables */
    volatile double v1 = 1.0, v2 = 2.0, v3 = 3.0, v4 = 4.0;
    volatile double v5 = 5.0, v6 = 6.0, v7 = 7.0, v8 = 8.0;
    
    switch (mode % 4) {
        case 0:
            /* Call in each case with different live values */
            clobber_registers();
            result = v1 + v2 + v3 + v4;
            if (result > 10.0) {
                /* Nested call with different live set */
                asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "memory");
                result += v5;
            }
            break;
            
        case 1:
            clobber_registers();
            result = v2 * v3 * v4 * v5;
            /* Force another basic block boundary */
            if (result < 100.0) {
                asm volatile("" : : : "rdx", "rsi", "rdi", "xmm2", "xmm3", "memory");
                result -= v6;
            }
            break;
            
        case 2:
            clobber_registers();
            result = v3 / v4 + v5 / v6;
            /* Unreachable code hint might affect block layout */
            if (result < 0) {
                __builtin_unreachable();
            }
            break;
            
        case 3:
            clobber_registers();
            result = v7 * v8 - v1 * v2;
            /* Empty else to create another block */
            if (result > 50.0) {
                /* Do nothing */
            } else {
                asm volatile("" : : : "r8", "r9", "r10", "xmm4", "xmm5", "memory");
            }
            break;
    }
    
    /* Use all variables one more time */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;
static volatile int jmp_val = 0;

__attribute__((noinline))
void function_with_setjmp(int *counter) {
    if (setjmp(env) == 0) {
        /* First call - modify many registers */
        volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;
        volatile int i1 = 10, i2 = 20, i3 = 30;
        
        /* Call that clobbers registers */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                      "xmm0", "xmm1", "xmm2", "memory");
        
        (*counter)++;
        
        /* Use values after asm - forces restore if not optimized away */
        jmp_val = (int)(d1 + d2 + d3) + i1 + i2 + i3;
    } else {
        /* After longjmp - different register usage */
        volatile float f1 = 1.5f, f2 = 2.5f;
        volatile long long ll1 = 100, ll2 = 200;
        
        asm volatile("" : : : "rsi", "rdi", "r8", "r9",
                      "xmm3", "xmm4", "xmm5", "memory");
        
        jmp_val += (int)(f1 * f2) + (int)(ll1 + ll2);
    }
}

/* ========== Main Test Orchestrator ========== */

int main() {
    double total = 0.0;
    long long checksum = 0;
    int counter = 0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Complex calculation with many live values */
    total += complex_calculation(1.0, 2.0, 3.0, 4.0);
    printf("Test 1 complete: total = %f\n", total);
    
    /* Test 2: Mixed type operations */
    checksum += mixed_operations(10, 20.5f, 30.5, 40);
    printf("Test 2 complete: checksum = %lld\n", checksum);
    
    /* Test 3: Loop with high register pressure */
    total += loop_with_calls(5);
    printf("Test 3 complete: total = %f\n", total);
    
    /* Test 4: Conditional blocks */
    for (int i = 0; i < 8; i++) {
        total += conditional_caller_save(i);
    }
    printf("Test 4 complete: total = %f\n", total);
    
    /* Test 5: setjmp/longjmp */
    function_with_setjmp(&counter);
    longjmp(env, 1);  /* Jump back */
    function_with_setjmp(&counter);  /* Shouldn't reach here after longjmp */
    
    /* Final validation */
    printf("Final total: %f\n", total);
    printf("Final checksum: %lld\n", checksum);
    printf("Counter: %d\n", counter);
    printf("Jmp_val: %d\n", jmp_val);
    
    /* Create a checksum of all results to prevent dead code elimination */
    long long final_check = (long long)total + checksum + counter + jmp_val;
    printf("Final combined check: %lld\n", final_check);
    
    return (final_check != 0) ? 0 : 1;
}
