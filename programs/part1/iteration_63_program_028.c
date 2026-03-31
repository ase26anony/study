/* caller-save-test.c
 * 
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
void clobber_registers(void) {
    /* Clobber multiple caller-saved registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                               "r8", "r9", "r10", "r11", "xmm0", "xmm1",
                               "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                               "xmm8", "xmm9", "xmm10", "xmm11", "xmm12",
                               "xmm13", "xmm14", "xmm15", "memory");
}

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d,
                           double e, double f, double g, double h) {
    /* Create many intermediate values that must survive the clobber call */
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = g / h + a;
    double t4 = sqrt(t1 * t1 + t2 * t2);
    double t5 = sin(t3) * cos(t4);
    
    /* Force caller-save by clobbering registers */
    clobber_registers();
    
    /* Use all intermediate values after the call */
    double result = t1 + t2 + t3 + t4 + t5;
    
    /* More computations to increase register pressure */
    result += a * c * e * g;
    result -= b * d * f * h;
    
    return result;
}

/* Function with mixed int/float operations */
__attribute__((noinline))
long long mixed_operations(int a, int b, float c, float d,
                           double e, double f, long long g) {
    volatile int vi = a;  /* Prevent optimization */
    volatile float vf = c;
    
    int i1 = a * b + vi;
    int i2 = (a + b) * (a - b);
    float f1 = c * d + vf;
    float f2 = c / d - vf;
    double d1 = e * f;
    double d2 = e / f;
    long long ll1 = g * 2;
    long long ll2 = g / 3;
    
    /* Clobber registers between computations */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory");
    
    /* Use all values after clobber */
    long long result = i1 + i2 + (long long)(f1 + f2) + 
                       (long long)(d1 + d2) + ll1 + ll2;
    
    return result;
}

/* ========== Functions Creating Complex Control Flow ========== */

/* Function with conditional branches and calls */
__attribute__((noinline, optimize("O3")))
int conditional_caller_save(int x, int y, int z) {
    int result = 0;
    
    /* Many live variables */
    int a = x * 2;
    int b = y + 3;
    int c = z - 4;
    int d = x * y;
    int e = y * z;
    int f = z * x;
    
    if (x > 0) {
        /* Function call in one branch */
        clobber_registers();
        result += a + b;
    } else {
        /* Different computation in other branch */
        result += c + d;
    }
    
    /* More variables that must survive across potential calls */
    int g = a * b * c;
    int h = d * e * f;
    
    if (y < 0) {
        /* Another call site */
        asm volatile("" : : : "rax", "rbx", "rcx", "memory");
        result += g - h;
    } else {
        result += g + h;
    }
    
    /* Use all variables at the end */
    return result + a + b + c + d + e + f + g + h;
}

/* Function with switch statement creating multiple edges */
__attribute__((noinline))
int switch_caller_save(int mode, double base) {
    double result = base;
    
    /* Create many live floating-point values */
    double d1 = base * 1.1;
    double d2 = base * 1.2;
    double d3 = base * 1.3;
    double d4 = base * 1.4;
    double d5 = base * 1.5;
    
    switch (mode % 4) {
        case 0:
            clobber_registers();
            result += d1 + d2;
            break;
        case 1:
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
            result += d2 + d3;
            break;
        case 2:
            result += d3 + d4;
            clobber_registers();
            break;
        case 3:
            result += d4 + d5;
            /* Fall through to unreachable */
            __builtin_unreachable();
            break;
        default:
            /* Should never happen */
            break;
    }
    
    /* Use all doubles after switch */
    return (int)(result + d1 + d2 + d3 + d4 + d5);
}

/* ========== Loop-Based Caller-Save Testing ========== */

__attribute__((noinline, optimize("O3")))
long long loop_caller_save(int iterations) {
    long long acc = 0;
    volatile int counter = 0;  /* Prevent loop optimizations */
    
    /* Loop with many live variables across each iteration */
    for (int i = 0; i < iterations; i++) {
        /* Variables that must survive the clobber call */
        int a = i * 2;
        int b = i + 3;
        int c = i - 4;
        long long d = i * 5LL;
        double e = i * 1.5;
        float f = i * 0.5f;
        
        /* Force caller-save in loop body */
        if (i % 3 == 0) {
            clobber_registers();
        } else if (i % 3 == 1) {
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                   "xmm0", "xmm1", "memory");
        }
        
        /* Use all variables after potential clobber */
        acc += a + b + c + d + (long long)e + (long long)f;
        counter++;
    }
    
    return acc;
}

/* ========== setjmp/longjmp Testing ========== */

static jmp_buf env;
static int jmp_val = 0;

__attribute__((noinline))
void function_with_setjmp(int *result) {
    /* Many variables that need saving */
    int a = 10, b = 20, c = 30, d = 40;
    double e = 1.1, f = 2.2, g = 3.3, h = 4.4;
    
    if (setjmp(env) == 0) {
        /* First call - modify variables */
        a++; b++; c++; d++;
        e *= 1.1; f *= 1.2; g *= 1.3; h *= 1.4;
        
        /* Force register spilling */
        clobber_registers();
        
        *result = a + b + c + d + (int)(e + f + g + h);
    } else {
        /* After longjmp - variables should be restored */
        *result += a + b + c + d + (int)(e + f + g + h) + jmp_val;
    }
}

/* ========== Main Test Orchestrator ========== */

int main(void) {
    long long total_checksum = 0;
    
    printf("Starting caller-save restoration test...\n");
    
    /* Test 1: Complex floating-point calculations with clobber */
    printf("Test 1: Complex floating-point calculations...\n");
    double r1 = complex_calculation(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8);
    total_checksum += (long long)r1;
    printf("  Result 1: %f\n", r1);
    
    /* Test 2: Mixed int/float operations */
    printf("Test 2: Mixed int/float operations...\n");
    long long r2 = mixed_operations(10, 20, 3.14f, 2.71f, 1.618, 3.14159, 1000);
    total_checksum += r2;
    printf("  Result 2: %lld\n", r2);
    
    /* Test 3: Conditional caller-save */
    printf("Test 3: Conditional branches with caller-save...\n");
    int r3 = conditional_caller_save(5, -3, 10);
    total_checksum += r3;
    printf("  Result 3: %d\n", r3);
    
    /* Test 4: Switch statement with multiple edges */
    printf("Test 4: Switch statement with multiple control edges...\n");
    int r4 = switch_caller_save(2, 10.5);
    total_checksum += r4;
    printf("  Result 4: %d\n", r4);
    
    /* Test 5: Loop with caller-save */
    printf("Test 5: Loop with caller-save in body...\n");
    long long r5 = loop_caller_save(10);
    total_checksum += r5;
    printf("  Result 5: %lld\n", r5);
    
    /* Test 6: setjmp/longjmp */
    printf("Test 6: setjmp/longjmp with caller-save...\n");
    int r6 = 0;
    function_with_setjmp(&r6);
    jmp_val = 100;
    longjmp(env, 1);
    total_checksum += r6;
    printf("  Result 6: %d\n", r6);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nTotal checksum: %lld\n", total_checksum);
    printf("All tests completed.\n");
    
    return (total_checksum != 0) ? 0 : 1;
}
