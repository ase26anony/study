/* caller-save-test.c
 * This program is designed to trigger specific uncovered code paths in GCC's
 * caller-save.cc, particularly the instruction chain manipulation logic
 * around lines 905-913 that handles insertion of save/restore instructions
 * in basic block boundaries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern void opaque_func2(int) __attribute__((noinline, noclone));
extern int opaque_func3(int, int) __attribute__((noinline, noclone));
extern double opaque_func4(double) __attribute__((noinline, noclone));

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32];
volatile double global_fp[16];

/* Function pointer with volatile to prevent constant propagation */
void (*volatile func_ptr)(void) = NULL;

/* Complex function with many live registers across calls */
__attribute__((noinline, noclone))
void test1(int a, int b, int c, int d, int e, int f) {
    /* Force register pressure with many live variables */
    volatile int v1 = a + 1;
    volatile int v2 = b + 2;
    volatile int v3 = c + 3;
    volatile int v4 = d + 4;
    volatile int v5 = e + 5;
    volatile int v6 = f + 6;
    
    /* Use explicit register variables to create conflicts */
    register int r12_val asm ("r12") = v1 * 2;
    register int r13_val asm ("r13") = v2 * 3;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile ("# Test1 asm clobber" 
                  : "+r" (r12_val), "+r" (r13_val)
                  : "r" (v1), "r" (v2)
                  : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
    
    /* Function call that clobbers registers */
    opaque_func2(v3);
    
    /* Use values after call - forces caller-save */
    int sum1 = r12_val + v4;
    int sum2 = r13_val + v5;
    
    /* Another asm barrier */
    asm volatile ("# After call use" 
                  : "+r" (sum1), "+r" (sum2)
                  : : "memory");
    
    /* Complex control flow with goto to create block boundaries */
    if (sum1 > sum2) {
        goto label1;
    } else {
        opaque_func1();
        goto label2;
    }
    
label1:
    /* This creates a merge point in CFG */
    v6 = sum1 * sum2;
    goto end;
    
label2:
    v6 = sum2 * sum1;
    /* Fall through */
    
end:
    /* Force use of all variables */
    global_array[0] = v1 + v2 + v3 + v4 + v5 + v6 + sum1 + sum2 + r12_val + r13_val;
}

/* Function with floating point and mixed register pressure */
__attribute__((noinline, noclone))
double test2(double x, double y, int n) {
    volatile double accum = 0.0;
    volatile int counter = n;
    
    /* Register variables for FP */
    register double fp1 asm ("xmm0") = x;
    register double fp2 asm ("xmm1") = y;
    
    /* Mixed integer registers */
    register int i asm ("r14") = 0;
    register int limit asm ("r15") = counter;
    
    /* Loop with function call inside - creates many save/restore points */
    for (i = 0; i < limit; i++) {
        /* Save FP values before call */
        double saved_fp1 = fp1;
        double saved_fp2 = fp2;
        
        /* Call that clobbers registers */
        double result = opaque_func4(saved_fp1);
        
        /* Restore and use values - forces caller-save insertion */
        fp1 = saved_fp1 + result;
        fp2 = saved_fp2 * result;
        
        /* Complex expression requiring temporaries */
        accum += fp1 * fp2 + i;
        
        /* Conditional break to create block splitting */
        if (accum > 1000.0) {
            /* Function call at block boundary */
            opaque_func1();
            break;
        }
        
        /* Continue creates another block boundary */
        if (i % 3 == 0) {
            continue;
        }
        
        /* Nested call in the middle of loop */
        int tmp = opaque_func3(i, (int)accum);
        accum += tmp;
    }
    
    /* Switch statement to create complex CFG */
    switch ((int)accum % 4) {
        case 0:
            opaque_func2(1);
            accum *= 2.0;
            break;
        case 1:
            /* Fall through to create merge point */
        case 2:
            opaque_func2(2);
            accum /= 2.0;
            break;
        default:
            /* This creates a call at block end */
            opaque_func2(3);
            accum = -accum;
            /* No break - falls through to return */
    }
    
    return accum + fp1 + fp2;
}

/* Function with vector-like operations and many arguments */
__attribute__((noinline, noclone))
long test3(long a, long b, long c, long d, long e, long f, long g, long h) {
    /* Many live variables across calls */
    volatile long vals[8] = {a, b, c, d, e, f, g, h};
    volatile long results[8];
    
    /* Unrolled loop with calls - creates many save/restore points */
    for (int i = 0; i < 8; i++) {
        register long temp asm ("r10") = vals[i];
        
        /* Inline asm that looks like a call */
        asm volatile ("# Pseudo-call %0" 
                      : "+r" (temp)
                      : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r11", "memory");
        
        /* Real function call */
        int call_result = opaque_func3((int)temp, i);
        
        /* Complex use after call */
        results[i] = temp * call_result + i;
        
        /* Conditional with goto to create irreducible flow */
        if (results[i] > 1000) {
            goto process_large;
        }
        
        /* Normal path */
        results[i] += 42;
        goto next_iter;
        
    process_large:
        /* Alternative path with function call */
        opaque_func2((int)results[i]);
        results[i] -= 100;
        
    next_iter:
        /* Compiler barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute final result */
    long total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i];
    }
    
    /* Function pointer call to create indirect jump */
    if (func_ptr) {
        func_ptr();
    }
    
    return total;
}

/* Helper with nested calls to create save/restore chains */
__attribute__((noinline, noclone))
int nested_calls(int depth, int value) {
    volatile int stack[10];
    
    if (depth <= 0) {
        return value;
    }
    
    /* Save value across call */
    stack[0] = value;
    
    /* First call */
    int r1 = opaque_func3(value, depth);
    
    /* Restore and use */
    value = stack[0] + r1;
    
    /* Recursive call */
    int r2 = nested_calls(depth - 1, value);
    
    /* Another call */
    opaque_func2(r2);
    
    return r2 + value;
}

/* Function using __builtin_apply for unusual calling convention */
__attribute__((noinline, noclone))
void test_builtin_apply(void) {
    volatile int args[3] = {1, 2, 3};
    volatile double dargs[2] = {1.5, 2.5};
    
    /* Simulate variable arguments */
    for (int i = 0; i < 3; i++) {
        register int arg asm ("edi") = args[i];
        register double darg asm ("xmm0") = dargs[i % 2];
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Call with mixed arguments */
        opaque_func2(arg);
        
        /* Use values after call */
        args[i] = arg + (int)darg;
        dargs[i % 2] = darg * 1.1;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use argv to create runtime variability */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize global state */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 2;
    }
    for (int i = 0; i < 16; i++) {
        global_fp[i] = i * 1.5;
    }
    
    /* Run all tests but with different orders based on mode */
    switch (test_mode) {
        case 0:
            test1(1, 2, 3, 4, 5, 6);
            test2(1.0, 2.0, 10);
            test3(1, 2, 3, 4, 5, 6, 7, 8);
            nested_calls(3, 42);
            test_builtin_apply();
            break;
        case 1:
            test2(3.0, 4.0, 5);
            test3(9, 8, 7, 6, 5, 4, 3, 2);
            test1(10, 20, 30, 40, 50, 60);
            break;
        case 2:
            for (int i = 0; i < 3; i++) {
                test1(i, i+1, i+2, i+3, i+4, i+5);
                nested_calls(2, i*10);
            }
            break;
        case 3:
            test3(100, 200, 300, 400, 500, 600, 700, 800);
            test_builtin_apply();
            break;
        default:
            test1(1000, 2000, 3000, 4000, 5000, 6000);
            test2(100.0, 200.0, 50);
            test3(1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000);
            nested_calls(4, 100);
            test_builtin_apply();
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += (long)global_fp[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum % 256);
}

/* Dummy definitions to satisfy linker (in real test, these would be
   in a separate compilation unit to prevent inlining) */
void opaque_func1(void) {
    global_counter++;
}

void opaque_func2(int x) {
    global_array[x % 32] += x;
}

int opaque_func3(int a, int b) {
    return a + b + global_counter;
}

double opaque_func4(double x) {
    global_fp[0] += x;
    return x * 1.5;
}
