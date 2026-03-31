/* caller-save-test.c
 * A comprehensive test to trigger caller-save restoration instruction
 * reordering within basic blocks (targeting caller-save.cc lines 905-913).
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
    return t1 + t2 + t3 + t4 + t5;
}

/* Function with mixed int/float operations */
__attribute__((noinline))
long long mixed_operations(int a, int b, float c, double d,
                          long long e, int f, float g, double h) {
    volatile int v1 = a;  /* Prevent optimization */
    volatile float v2 = c;
    
    /* Multiple parallel computations */
    int i1 = a * b + f;
    float f1 = c * g * 2.0f;
    double d1 = d * h * 3.14159;
    long long ll1 = e * 7;
    
    /* Clobber registers between computations */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory");
    
    int i2 = i1 + v1 * 3;
    float f2 = f1 + v2 * 4.0f;
    double d2 = d1 / 2.0;
    long long ll2 = ll1 - 100;
    
    /* Another clobber */
    clobber_registers();
    
    return i2 + (long long)f2 + (long long)d2 + ll2;
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline, optimize("O3")))
double loop_with_calls(int iterations) {
    double result = 0.0;
    
    /* Multiple loop-carried variables */
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    float e = 5.0f, f = 6.0f, g = 7.0f, h = 8.0f;
    int i = 9, j = 10, k = 11, l = 12;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Compute values that must survive the function call */
        double temp1 = a * sin(b) + cos(c);
        float temp2 = e * f * g / h;
        int temp3 = i * j + k * l;
        
        /* External function call - forces caller-save */
        result += complex_calculation(temp1, b, c, d, e, f, g, h);
        
        /* Modify loop variables after call (register pressure) */
        a += 0.1;
        b = sin(b + 0.2);
        c = cos(c * 1.1);
        d = d * 0.95;
        e = e * 1.05f;
        f = f + 0.3f;
        g = g - 0.1f;
        h = h / 1.01f;
        i = (i + 3) % 100;
        j = (j * 2) % 97;
        k = k + iter;
        l = l - 1;
        
        /* Use all variables to keep them live */
        result += temp2 + temp3;
    }
    
    return result;
}

/* ========== Conditional Control Flow ========== */

__attribute__((noinline))
double conditional_caller_save(int mode, double x, double y) {
    double result = 0.0;
    
    /* Many live variables */
    double a = x, b = y, c = x * y, d = x / y;
    float e = (float)x, f = (float)y, g = (float)(x + y), h = (float)(x - y);
    int i = (int)x, j = (int)y, k = i * j, l = i + j;
    
    switch (mode % 4) {
        case 0:
            /* Call with some arguments, keep others live */
            result = complex_calculation(a, b, c, d, e, f, g, h);
            /* Use remaining variables */
            result += i + j + k + l;
            break;
            
        case 1:
            /* Different call pattern */
            asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "memory");
            result = a + b + c + d + e + f + g + h + i + j + k + l;
            clobber_registers();
            break;
            
        case 2:
            /* Nested computations with clobber */
            for (int m = 0; m < 3; m++) {
                double t = complex_calculation(a+m, b, c, d, e, f, g, h);
                result += t + i + j;
                clobber_registers();
            }
            break;
            
        case 3:
            /* Multiple calls in sequence */
            result = complex_calculation(a, b, c, d, e, f, g, h);
            result += complex_calculation(b, c, d, a, f, g, h, e);
            result += complex_calculation(c, d, a, b, g, h, e, f);
            break;
    }
    
    /* All variables must be used to ensure they're live across calls */
    return result + a - b + c * d + e * f + g / h + i * j - k + l;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;
static volatile int jmp_flag = 0;

__attribute__((noinline))
void function_with_setjmp(double* results, int size) {
    /* Multiple variables that need saving */
    double a = 1.0, b = 2.0, c = 3.0;
    float d = 4.0f, e = 5.0f;
    int f = 6, g = 7, h = 8;
    
    if (setjmp(env) == 0) {
        /* First call - compute values */
        for (int i = 0; i < size; i++) {
            results[i] = complex_calculation(a+i, b, c, d, e, f, g, h);
            a += 0.5;
            b = sin(b);
            c = cos(c);
            
            /* Force register pressure */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory");
        }
    } else {
        /* After longjmp - use all variables */
        results[0] = a + b + c + d + e + f + g + h;
    }
}

/* ========== Main Test Orchestrator ========== */

__attribute__((optimize("O3")))
int main() {
    printf("Starting caller-save restoration test...\n");
    
    double total = 0.0;
    long long checksum = 0;
    
    /* Test 1: Complex calculation with register pressure */
    printf("Test 1: Complex calculation...\n");
    for (int i = 0; i < 10; i++) {
        double result = complex_calculation(
            1.0 + i, 2.0 + i*0.1, 3.0 + i*0.2, 4.0 + i*0.3,
            5.0 + i*0.4, 6.0 + i*0.5, 7.0 + i*0.6, 8.0 + i*0.7
        );
        total += result;
        checksum += (long long)result;
    }
    
    /* Test 2: Mixed operations */
    printf("Test 2: Mixed int/float operations...\n");
    for (int i = 0; i < 5; i++) {
        long long result = mixed_operations(
            100 + i, 200 + i*2, 3.14f + i, 2.71828 + i*0.1,
            1000LL + i*100, 50 + i, 1.414f + i*0.01, 1.732 + i*0.05
        );
        checksum += result;
    }
    
    /* Test 3: Loop with calls */
    printf("Test 3: Loop with function calls...\n");
    double loop_result = loop_with_calls(20);
    total += loop_result;
    checksum += (long long)loop_result;
    
    /* Test 4: Conditional control flow */
    printf("Test 4: Conditional caller-save...\n");
    for (int i = 0; i < 8; i++) {
        double result = conditional_caller_save(
            i, 10.0 + i*0.5, 20.0 - i*0.3
        );
        total += result;
        checksum += (long long)result;
    }
    
    /* Test 5: setjmp/longjmp */
    printf("Test 5: setjmp/longjmp test...\n");
    double jmp_results[5];
    function_with_setjmp(jmp_results, 5);
    
    /* Force longjmp to test restoration */
    jmp_flag = 1;
    if (jmp_flag) {
        longjmp(env, 1);
    }
    
    for (int i = 0; i < 5; i++) {
        total += jmp_results[i];
        checksum += (long long)jmp_results[i];
    }
    
    /* Final validation */
    printf("\nTest Results:\n");
    printf("Total: %f\n", total);
    printf("Checksum: %lld\n", checksum);
    
    /* Use results to prevent dead code elimination */
    if (total != 0.0 || checksum != 0) {
        printf("All tests completed successfully.\n");
    }
    
    return 0;
}
