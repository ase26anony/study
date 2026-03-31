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
                 "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
                 "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
                 "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
int pressure_function(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live variables */
    int v1 = a * b + c;
    int v2 = d * e - f;
    int v3 = g * h + a;
    int v4 = b * c - d;
    int v5 = e * f + g;
    int v6 = h * a - b;
    int v7 = c * d + e;
    int v8 = f * g - h;
    
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Force caller-save across this call */
    clobber_registers();
    
    /* Use all variables after the call - they must be restored */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + barrier;
}

/* Mixed float/int operations to engage different register banks */
__attribute__((noinline))
double mixed_operations(double a, double b, float c, float d, 
                        long long e, long long f, int g, int h) {
    double d1 = a * b + c * d;
    float f1 = c / d + a - b;
    long long ll1 = e * f + g * h;
    int i1 = g % (h + 1) + (int)a;
    
    clobber_registers();
    
    return d1 + f1 + ll1 + i1;
}

/* ========== Loop with Caller-Save Requirements ========== */

__attribute__((noinline, optimize("O3")))
long long loop_with_calls(int iterations) {
    long long sum = 0;
    double dsum = 0.0;
    float fsum = 0.0f;
    
    /* Loop creates multiple basic blocks with calls inside */
    for (int i = 0; i < iterations; i++) {
        /* Live variables that must survive across the call */
        int a = i * 2;
        int b = i * 3;
        double c = sin(i * 0.1);
        float d = cos(i * 0.1);
        long long e = i * 1000LL;
        
        /* Function call that clobbers registers */
        clobber_registers();
        
        /* Use variables after call - requires restoration */
        sum += a + b + (int)c + (int)d + (e % 100);
        dsum += c;
        fsum += d;
        
        /* Another call with different register usage */
        if (i % 3 == 0) {
            double temp = mixed_operations(dsum, fsum, fsum, dsum, 
                                          sum, sum, a, b);
            sum += (long long)temp;
        }
    }
    
    return sum + (long long)dsum + (long long)fsum;
}

/* ========== Conditional Control Flow ========== */

__attribute__((noinline))
int conditional_caller_save(int x) {
    int result = 0;
    
    /* Create multiple predecessor/successor blocks */
    if (x < 0) {
        int a = x * 2;
        int b = x * 3;
        clobber_registers();
        result = a + b;  /* Restore needed here */
    } else if (x < 100) {
        double a = sqrt(x);
        float b = log(x + 1);
        clobber_registers();
        result = (int)(a + b);  /* Restore needed here */
    } else {
        long long a = x * 1000LL;
        long long b = x * 2000LL;
        clobber_registers();
        result = (int)(a + b);  /* Restore needed here */
    }
    
    /* Common successor block */
    clobber_registers();
    return result * 2;
}

/* ========== Switch Statement with Multiple Edges ========== */

__attribute__((noinline, optimize("O2")))
int switch_caller_save(int mode) {
    int result = 0;
    
    switch (mode % 4) {
        case 0: {
            /* Use many registers */
            int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
            clobber_registers();
            result = a + b + c + d + e + f + g + h;
            break;
        }
        case 1: {
            /* Use floating point */
            double a = 1.1, b = 2.2, c = 3.3, d = 4.4;
            clobber_registers();
            result = (int)(a + b + c + d);
            break;
        }
        case 2: {
            /* Mix types */
            int a = 10;
            double b = 20.5;
            long long c = 30LL;
            clobber_registers();
            result = a + (int)b + (int)c;
            break;
        }
        case 3: {
            /* Maximum pressure */
            int v[8] = {1, 2, 3, 4, 5, 6, 7, 8};
            double d[4] = {1.1, 2.2, 3.3, 4.4};
            clobber_registers();
            for (int i = 0; i < 8; i++) result += v[i];
            for (int i = 0; i < 4; i++) result += (int)d[i];
            break;
        }
    }
    
    /* Common code after switch */
    clobber_registers();
    return result;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;

__attribute__((noinline))
int setjmp_test(int x) {
    int a = x * 2;
    int b = x * 3;
    double c = x * 0.5;
    
    if (setjmp(env) == 0) {
        /* First call - registers must be saved */
        clobber_registers();
        return a + b + (int)c;
    } else {
        /* After longjmp - registers restored */
        clobber_registers();
        return a * b + (int)c;
    }
}

/* ========== Complex Expression Chains ========== */

__attribute__((noinline, optimize("O3")))
double complex_chain(double x) {
    /* Create dependent chain of operations */
    double a = x * 2.0;
    double b = sin(a);
    double c = cos(b);
    double d = exp(c);
    double e = log(d + 1.0);
    double f = sqrt(e);
    double g = f * f * f;
    
    /* Call that forces saves/restores in the middle */
    clobber_registers();
    
    /* Continue the chain after restoration */
    double h = g / (x + 1.0);
    double i = tan(h);
    double j = atan(i);
    double k = j * j + 2.0 * j + 1.0;
    
    clobber_registers();
    
    return k;
}

/* ========== Main Test Orchestrator ========== */

int main() {
    long long total = 0;
    
    printf("Starting caller-save restoration tests...\n");
    
    /* Test 1: Register pressure across function call */
    total += pressure_function(1, 2, 3, 4, 5, 6, 7, 8);
    printf("Test 1 complete: %lld\n", total);
    
    /* Test 2: Loop with calls */
    total += loop_with_calls(10);
    printf("Test 2 complete: %lld\n", total);
    
    /* Test 3: Conditional control flow */
    for (int i = -5; i < 105; i += 10) {
        total += conditional_caller_save(i);
    }
    printf("Test 3 complete: %lld\n", total);
    
    /* Test 4: Switch statement */
    for (int i = 0; i < 8; i++) {
        total += switch_caller_save(i);
    }
    printf("Test 4 complete: %lld\n", total);
    
    /* Test 5: setjmp/longjmp */
    total += setjmp_test(42);
    longjmp(env, 1);  /* This will return to setjmp_test */
    total += setjmp_test(42);  /* Shouldn't reach here */
    
    /* Test 6: Complex chains */
    for (double x = 0.1; x < 2.0; x += 0.2) {
        total += (long long)complex_chain(x);
    }
    printf("Test 6 complete: %lld\n", total);
    
    /* Test 7: Mixed operations */
    total += (long long)mixed_operations(1.5, 2.5, 3.5f, 4.5f, 
                                        1000LL, 2000LL, 5, 6);
    printf("Test 7 complete: %lld\n", total);
    
    printf("Final checksum: %lld\n", total);
    
    /* Validate results aren't optimized away */
    if (total == 0) {
        printf("ERROR: All computations optimized away!\n");
        return 1;
    }
    
    return 0;
}
