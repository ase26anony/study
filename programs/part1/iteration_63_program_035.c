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

/* ========== Helper Functions with Register Clobbering ========== */

/* Force caller-save by clobbering multiple registers */
__attribute__((noinline))
void clobber_registers(void) {
    /* Clobber multiple caller-saved registers */
    asm volatile("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi",
                 "r8", "r9", "r10", "r11",
                 "xmm0", "xmm1", "xmm2", "xmm3",
                 "xmm4", "xmm5", "xmm6", "xmm7",
                 "xmm8", "xmm9", "xmm10", "xmm11",
                 "xmm12", "xmm13", "xmm14", "xmm15");
}

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d,
                          double e, double f, double g, double h) {
    /* Create many intermediate values that must survive the clobber */
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = g / h + a;
    double t4 = sqrt(t1 * t1 + t2 * t2);
    
    /* Force caller-save by clobbering registers */
    clobber_registers();
    
    /* Use all intermediate values after the clobber */
    double t5 = t1 + t2 + t3;
    double t6 = t4 * t5;
    
    /* More clobbering */
    asm volatile("" : : : "memory", "rax", "rcx", "rdx", "xmm0", "xmm1");
    
    return t6 * (t1 - t2) / (t3 + t4);
}

/* ========== Functions Creating Basic Block Boundaries ========== */

/* Function with conditional branches around calls */
__attribute__((noinline, optimize("O3")))
int branchy_function(int x, int y, int z) {
    volatile int vx = x;  /* Prevent optimization */
    volatile int vy = y;
    volatile int vz = z;
    
    int result = 0;
    
    /* Create multiple basic blocks with calls in between */
    if (vx > 0) {
        /* Live variables across call */
        int a = vx * 2;
        int b = vy + 5;
        
        clobber_registers();  /* Call that clobbers registers */
        
        /* Use variables after call - must be restored */
        result = a + b;
        
        if (vz < 10) {
            int c = result * 3;
            clobber_registers();
            result = c - vz;
        } else {
            int d = result / 2;
            clobber_registers();
            result = d + vz;
        }
    } else {
        int e = vy * vz;
        clobber_registers();
        result = e - vx;
    }
    
    return result;
}

/* Function with switch creating multiple control flow edges */
__attribute__((noinline))
int switch_with_calls(int mode, int a, int b, int c, int d,
                     int e, int f, int g, int h) {
    /* Many live variables to increase register pressure */
    int result = 0;
    
    switch (mode % 4) {
        case 0: {
            int t1 = a + b;
            int t2 = c * d;
            clobber_registers();
            result = t1 * t2 - e;
            break;
        }
        case 1: {
            int t3 = e << 2;
            int t4 = f & g;
            clobber_registers();
            result = t3 | t4 + h;
            break;
        }
        case 2: {
            int t5 = (a ^ b) + c;
            int t6 = (d | e) & f;
            clobber_registers();
            clobber_registers();  /* Multiple calls in same block */
            result = t5 * t6 / g;
            break;
        }
        case 3: {
            int t7 = a * b * c;
            int t8 = d + e + f;
            clobber_registers();
            if (g > 0) {
                int t9 = t7 / g;
                clobber_registers();
                result = t9 + t8 + h;
            } else {
                result = t7 + t8;
            }
            break;
        }
    }
    
    return result;
}

/* ========== Loop with Register Pressure ========== */

__attribute__((noinline, optimize("O3")))
long long loop_with_calls(int iterations) {
    /* Many variables of different types to use different register banks */
    long long ll1 = 1, ll2 = 2, ll3 = 3, ll4 = 4;
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4;
    float f1 = 5.5f, f2 = 6.6f, f3 = 7.7f, f4 = 8.8f;
    int i1 = 9, i2 = 10, i3 = 11, i4 = 12;
    
    /* Loop creates multiple basic blocks with calls inside */
    for (int i = 0; i < iterations; i++) {
        /* Modify all variables - they must survive across calls */
        ll1 = ll1 * ll2 + i;
        ll2 = ll2 - ll3 * i;
        d1 = d1 * d2 + sin(d3);
        d2 = d2 / d4 - cos(d1);
        f1 = f1 + f2 * i;
        f2 = f2 - f3 / (i + 1);
        i1 = i1 ^ i2;
        i2 = i2 | i3;
        i3 = i3 & i4;
        i4 = i4 << 1;
        
        /* Function call that clobbers registers */
        clobber_registers();
        
        /* More computations with same variables */
        ll3 = ll3 + ll4 * ll1;
        ll4 = ll4 - ll2;
        d3 = d3 * d1 + d2;
        d4 = d4 / d3 - d1;
        f3 = f3 + f4 * f1;
        f4 = f4 - f2;
        
        /* Another call - different registers might need saving */
        asm volatile("" : : : "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                     "rax", "rbx", "rcx", "rdx");
    }
    
    /* Combine all results */
    return ll1 + ll2 + ll3 + ll4 + 
           (long long)(d1 + d2 + d3 + d4) +
           (long long)(f1 + f2 + f3 + f4) +
           i1 + i2 + i3 + i4;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;

__attribute__((noinline))
int setjmp_test(int x) {
    volatile int a = x * 2;
    volatile int b = x + 10;
    volatile int c = x - 5;
    
    if (setjmp(env) == 0) {
        /* First call - modify variables */
        a = a * 3;
        b = b / 2;
        c = c + 7;
        
        /* Call that might trigger caller-save */
        clobber_registers();
        
        return a + b + c;
    } else {
        /* After longjmp - variables should be restored */
        clobber_registers();
        return a - b - c;
    }
}

/* ========== Mixed Float/Int Operations ========== */

__attribute__((noinline, optimize("O3")))
float mixed_operations(int a, int b, float c, float d,
                      double e, double f, long long g) {
    /* Mix different types to use different register banks */
    float f1 = c * d + a;
    double d1 = e / f * b;
    int i1 = a ^ b + (int)c;
    long long ll1 = g * a;
    
    /* Force register pressure */
    float f2 = f1 * 2.0f;
    double d2 = d1 + 3.14;
    int i2 = i1 << 2;
    long long ll2 = ll1 / 4;
    
    /* Call clobbering both int and float registers */
    asm volatile("" : : : "memory", 
                 "rax", "rbx", "rcx", "rdx",
                 "xmm0", "xmm1", "xmm2", "xmm3",
                 "xmm4", "xmm5", "xmm6", "xmm7");
    
    /* Use all values after clobber */
    f2 = f2 + (float)d2;
    i2 = i2 + (int)(ll2 % 100);
    
    return f2 * i2;
}

/* ========== Main Test Orchestrator ========== */

int main(void) {
    printf("Starting caller-save stress test...\n");
    
    long long total_checksum = 0;
    
    /* Test 1: Complex floating-point calculations with clobbering */
    printf("Test 1: Complex floating-point... ");
    double result1 = complex_calculation(1.1, 2.2, 3.3, 4.4,
                                        5.5, 6.6, 7.7, 8.8);
    total_checksum += (long long)result1;
    printf("result = %f\n", result1);
    
    /* Test 2: Branchy function with calls in multiple blocks */
    printf("Test 2: Branchy function... ");
    int result2 = branchy_function(10, 20, 30);
    total_checksum += result2;
    printf("result = %d\n", result2);
    
    /* Test 3: Switch with multiple call sites */
    printf("Test 3: Switch with calls... ");
    int result3 = switch_with_calls(2, 1, 2, 3, 4, 5, 6, 7, 8);
    total_checksum += result3;
    printf("result = %d\n", result3);
    
    /* Test 4: Loop with high register pressure */
    printf("Test 4: Loop with calls... ");
    long long result4 = loop_with_calls(5);
    total_checksum += result4;
    printf("result = %lld\n", result4);
    
    /* Test 5: setjmp/longjmp */
    printf("Test 5: setjmp/longjmp... ");
    int result5 = setjmp_test(42);
    total_checksum += result5;
    
    /* Trigger longjmp to test restoration after jump */
    longjmp(env, 1);
    /* Unreachable, but compiler doesn't know */
    __builtin_unreachable();
    
    printf("result = %d\n", result5);
    
    /* Test 6: Mixed float/int operations */
    printf("Test 6: Mixed operations... ");
    float result6 = mixed_operations(10, 20, 3.14f, 2.71f,
                                    1.618, 2.718, 1000);
    total_checksum += (long long)result6;
    printf("result = %f\n", result6);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nTotal checksum: %lld\n", total_checksum);
    printf("All tests completed.\n");
    
    return (total_checksum != 0) ? 0 : 1;
}
