/* caller-save-test.c
 * A comprehensive test to trigger caller-save restoration instruction
 * reordering within basic blocks, specifically targeting the uncovered
 * instruction chain manipulation logic in caller-save.cc lines 905-913.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller-save by clobbering many registers */
__attribute__((noinline))
void clobber_registers() {
    /* Clobber multiple caller-saved registers across architectures */
    asm volatile("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi",
                                 "r8", "r9", "r10", "r11",
                                 "xmm0", "xmm1", "xmm2", "xmm3",
                                 "xmm4", "xmm5", "xmm6", "xmm7");
}

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, int c, long d) {
    volatile double v1 = a * 2.0;  /* Prevent optimization */
    volatile int v2 = c + 5;
    volatile long v3 = d << 2;
    
    /* Force register pressure with many temporaries */
    double t1 = v1 * 3.14159;
    double t2 = t1 / (b + 1.0);
    double t3 = t2 * t2;
    double t4 = sin(t3);
    double t5 = cos(t4);
    double t6 = t5 * exp(t4);
    
    /* Call that clobbers registers */
    clobber_registers();
    
    /* Use all variables after call - must be restored */
    return (t6 * v1) + (v2 * 0.01) + (v3 * 0.001);
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline))
void process_chunk(int* data, int size, int multiplier) {
    for (int i = 0; i < size; i++) {
        data[i] *= multiplier;
    }
    clobber_registers();
}

__attribute__((optimize("O3")))
void loop_with_calls() {
    /* Many live variables across loop iterations */
    int arr1[16], arr2[16], arr3[16];
    double accum1 = 0.0, accum2 = 0.0;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    long long ll1 = 1000, ll2 = 2000, ll3 = 3000;
    
    /* Initialize arrays with different patterns */
    for (int i = 0; i < 16; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
        arr3[i] = i * 5;
    }
    
    /* Loop with function call - variables must survive */
    for (int iter = 0; iter < 10; iter++) {
        /* Use all variables before call */
        accum1 += arr1[iter % 16] * f1 + ll1;
        accum2 += arr2[iter % 16] * f2 + ll2;
        
        /* Function call clobbers registers */
        process_chunk(arr3, 16, iter + 1);
        
        /* Use variables after call - must be restored */
        f1 = f1 * 1.1f + accum1 * 0.01f;
        f2 = f2 * 1.2f + accum2 * 0.02f;
        ll1 += arr3[iter % 16];
        ll2 += arr3[(iter + 1) % 16];
        
        /* Another clobber */
        asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx",
                                     "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Final computation using all live variables */
    volatile double result = accum1 + accum2 + f1 + f2 + ll1 + ll2;
    (void)result;  /* Use to prevent elimination */
}

/* ========== Conditional Control Flow ========== */

__attribute__((noinline, optimize("O2")))
int branch_helper(int x, double y, float z) {
    /* Complex expression using all parameters */
    return (int)(x * y * z + sin(y) * cos(z));
}

__attribute__((optimize("O3")))
void conditional_calls(int mode) {
    /* Many live variables that span across conditional calls */
    int a = 100, b = 200, c = 300;
    double d = 1.234, e = 5.678, f = 9.012;
    float g = 3.14f, h = 2.71f, i = 1.41f;
    
    switch (mode % 4) {
        case 0:
            /* Use variables before call */
            a = a * 2 + (int)(d * 10);
            b = b * 3 + (int)(e * 20);
            
            /* Call that clobbers */
            c = branch_helper(a, d, g);
            
            /* Restore and continue */
            d = d * 1.5 + c * 0.01;
            e = e * 2.0 + b * 0.02;
            break;
            
        case 1:
            /* Different computation pattern */
            f = f * 3.0 + a * 0.001;
            g = g * 1.1f + b * 0.01f;
            
            /* Another clobbering call */
            asm volatile("" : : : "memory", "r12", "r13", "r14", "r15",
                                         "xmm8", "xmm9", "xmm10", "xmm11");
            
            h = h * 1.2f + c * 0.02f;
            i = i * 1.3f + (int)f * 0.03f;
            break;
            
        case 2:
            /* Chain of calls with live variables */
            a = branch_helper(b, e, h);
            clobber_registers();
            b = branch_helper(c, f, i);
            clobber_registers();
            c = branch_helper(a, d, g);
            break;
            
        case 3:
            /* Mixed float/int operations */
            d = (double)a * g + (double)b * h;
            e = (double)c * i + sin(d);
            
            /* Force register bank switching */
            asm volatile("" : : : "memory", "rax", "rcx", "xmm0", "xmm1");
            
            f = cos(e) * tan(d);
            a = (int)(d * e * f);
            break;
    }
    
    /* Final use of all variables */
    volatile int check = a + b + c + (int)d + (int)e + (int)f;
    (void)check;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;
static int jmp_val = 0;

__attribute__((noinline))
void function_with_setjmp() {
    /* Variables that must be saved across longjmp */
    int local1 = 100;
    double local2 = 3.14159;
    float local3 = 2.71828f;
    long long local4 = 123456789;
    
    if (setjmp(env) == 0) {
        /* First time through */
        local1 *= 2;
        local2 *= 1.5;
        local3 *= 1.1f;
        local4 <<= 1;
        
        /* Call that might trigger longjmp */
        if (jmp_val == 0) {
            longjmp(env, 1);
        }
    } else {
        /* After longjmp - variables should be restored */
        local1 += 50;
        local2 += 0.5;
        local3 += 0.1f;
        local4 += 1000;
    }
    
    volatile double result = local1 + local2 + local3 + local4;
    (void)result;
}

/* ========== Unreachable Code Pattern ========== */

__attribute__((noinline, optimize("O2")))
int unreachable_helper(int x) {
    return x * 2;
}

__attribute__((optimize("O3")))
void test_unreachable() {
    int a = 10, b = 20, c = 30;
    double d = 1.1, e = 2.2, f = 3.3;
    
    /* Computation before call */
    a = a * 3 + b;
    d = d * e + f;
    
    /* Call with side effects */
    c = unreachable_helper(a);
    
    /* Use __builtin_unreachable to affect block analysis */
    if (c > 1000) {
        __builtin_unreachable();
    }
    
    /* More computations that must be scheduled around restores */
    b = b * 2 + c;
    e = e * 3.0 + d;
    f = sqrt(e) * cos(d);
    
    volatile int check = a + b + c + (int)(d + e + f);
    (void)check;
}

/* ========== Main Orchestrator ========== */

int main() {
    printf("Starting caller-save restoration test...\n");
    
    /* Test 1: Complex calculation with register pressure */
    double r1 = complex_calculation(1.0, 2.0, 3, 4L);
    printf("Test 1 result: %f\n", r1);
    
    /* Test 2: Loop with multiple calls */
    loop_with_calls();
    printf("Test 2 completed\n");
    
    /* Test 3: Conditional control flow */
    for (int i = 0; i < 8; i++) {
        conditional_calls(i);
    }
    printf("Test 3 completed\n");
    
    /* Test 4: setjmp/longjmp */
    function_with_setjmp();
    printf("Test 4 completed\n");
    
    /* Test 5: Unreachable code pattern */
    test_unreachable();
    printf("Test 5 completed\n");
    
    /* Test 6: Mixed operations with high register pressure */
    {
        /* Maximum register pressure test */
        int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
        double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4, d5 = 5.5;
        float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
        long long ll1 = 100, ll2 = 200, ll3 = 300;
        
        /* Use all variables */
        v1 = v2 * v3 + v4;
        d1 = d2 * d3 + d4;
        f1 = f2 * f3 + f4;
        ll1 = ll2 + ll3 * v5;
        
        /* Clobber all registers */
        asm volatile("" : : : 
            "memory",
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Use all variables again */
        v6 = v7 * v8 + v1;
        d5 = d1 * d2 + d3;
        f4 = f1 * f2 + f3;
        ll3 = ll1 + ll2 * v2;
        
        volatile long long final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                                  (long long)(d1 + d2 + d3 + d4 + d5) +
                                  (long long)(f1 + f2 + f3 + f4) +
                                  ll1 + ll2 + ll3;
        (void)final;
    }
    printf("Test 6 completed\n");
    
    printf("All tests completed successfully!\n");
    return 0;
}
