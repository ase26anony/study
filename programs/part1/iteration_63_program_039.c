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
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                 "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
                 "xmm4", "xmm5", "xmm6", "xmm7", "memory");
}

/* Function that uses many registers, forcing spills */
__attribute__((noinline, optimize("O3")))
double compute_pressure(int a, int b, int c, int d, 
                       double e, double f, double g, double h) {
    /* Complex computation using all inputs */
    double t1 = e * f + g / h;
    double t2 = sin(e) * cos(f) + tan(g);
    int t3 = (a * b) + (c << 3) - (d / 2);
    double t4 = t1 * t2 * t3;
    
    /* Force register pressure with many temporaries */
    volatile double v1 = t4;
    volatile double v2 = t4 * 2.0;
    volatile double v3 = t4 * 3.0;
    volatile double v4 = t4 * 4.0;
    
    return v1 + v2 + v3 + v4 + a + b + c + d;
}

/* ========== Test Case 1: Basic Block with Caller-Save Restoration ========== */

__attribute__((noinline, optimize("O2")))
double test1_basic_block_reordering() {
    /* Create many live variables across a function call */
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    int i = 9, j = 10, k = 11, l = 12;
    
    /* Computation before call - uses many registers */
    double pre1 = a * b + c * d;
    double pre2 = e / f - g * h;
    int pre3 = i ^ j | k & l;
    
    /* Function call that clobbers caller-saved registers */
    clobber_registers();
    
    /* Computation after call - uses same variables */
    /* This should trigger caller-save restoration */
    double post1 = pre1 * 2.0 + a;
    double post2 = pre2 / 2.0 + b;
    int post3 = pre3 << 2 + i;
    
    /* More computations to create scheduling opportunities */
    volatile double v = post1;
    for (int m = 0; m < 3; m++) {
        v += post2 * m;
    }
    
    return v + post3 + pre1 + pre2 + pre3;
}

/* ========== Test Case 2: Loop with Caller-Save in Multiple Blocks ========== */

__attribute__((noinline, optimize("O3")))
double test2_loop_caller_save() {
    double result = 0.0;
    
    /* Loop creates multiple basic blocks */
    for (int i = 0; i < 10; i++) {
        /* Live variables that must survive across call */
        double a = i * 1.1;
        double b = i * 2.2;
        double c = i * 3.3;
        int d = i * 4;
        int e = i * 5;
        
        /* Computation using all variables */
        double temp = a * b + c;
        int itemp = d * e + i;
        
        /* Function call that clobbers registers */
        /* Different calls based on condition to create control flow */
        if (i % 2 == 0) {
            clobber_registers();
        } else {
            /* Alternative computation path */
            asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "memory");
        }
        
        /* Use variables after call - requires restoration */
        result += temp * itemp + a + b + c + d + e;
        
        /* Additional computation to create instruction scheduling opportunities */
        volatile double check = result;
        if (check > 100.0) {
            result = sqrt(check);
        }
    }
    
    return result;
}

/* ========== Test Case 3: Conditional Blocks with Different Call Sites ========== */

__attribute__((noinline, optimize("O2")))
double test3_conditional_blocks(int mode) {
    double a = 1.234, b = 5.678, c = 9.012;
    double d = 3.456, e = 7.890, f = 2.468;
    int x = 123, y = 456, z = 789;
    
    /* Complex pre-call computation */
    double base = a * b + c * d - e / f;
    int ibase = (x & y) | (z ^ x);
    
    /* Switch creates multiple basic blocks */
    double result = 0.0;
    switch (mode % 4) {
        case 0: {
            /* Block with function call */
            double t1 = base * 2.0;
            int t2 = ibase << 1;
            
            clobber_registers();
            
            /* Post-call computation */
            result = t1 + t2 + sin(a) + cos(b);
            break;
        }
        case 1: {
            /* Different computation pattern */
            double t1 = base / 2.0;
            int t2 = ibase >> 1;
            
            /* Inline asm with different clobbers */
            asm volatile("" : : : "rax", "rdx", "xmm2", "xmm3", "xmm4", "memory");
            
            result = t1 - t2 + tan(c) + atan(d);
            break;
        }
        case 2: {
            /* Path with multiple calls */
            double t1 = sqrt(base);
            int t2 = ibase * 2;
            
            clobber_registers();
            
            /* Intermediate computation */
            t1 = t1 * 3.14159;
            
            /* Another register-clobbering operation */
            asm volatile("" : : : "rsi", "rdi", "r8", "r9", "xmm5", "xmm6", "memory");
            
            result = t1 + t2 + exp(e) + log(f);
            break;
        }
        default: {
            /* Default path with heavy computation */
            result = compute_pressure(x, y, z, mode, a, b, c, d);
            
            /* Unreachable hint might affect block analysis */
            if (result < 0) {
                __builtin_unreachable();
            }
            break;
        }
    }
    
    /* Common post-switch computation */
    volatile double verify = result;
    for (int i = 0; i < 2; i++) {
        verify += i * 0.5;
    }
    
    return verify + base + ibase;
}

/* ========== Test Case 4: setjmp/longjmp Caller-Save Stress ========== */

static jmp_buf env;
static volatile int jmp_flag = 0;

__attribute__((noinline))
void jump_function(int *counter) {
    (*counter)++;
    if (*counter < 3) {
        longjmp(env, *counter);
    }
}

__attribute__((noinline, optimize("O2")))
double test4_setjmp_caller_save() {
    double a = 10.0, b = 20.0, c = 30.0, d = 40.0;
    int x = 100, y = 200, z = 300;
    int counter = 0;
    
    /* Variables that must survive longjmp */
    volatile double preserved = a + b + c + d;
    volatile int ipreserved = x + y + z;
    
    if (setjmp(env) == 0) {
        /* First execution path */
        double t1 = a * b - c / d;
        int t2 = x ^ y & z;
        
        /* Function call that might longjmp */
        jump_function(&counter);
        
        /* This code might not be reached, but compiler must prepare for it */
        double t3 = t1 * 2.0 + t2;
        preserved += t3;
    } else {
        /* After longjmp - different computation */
        double t1 = sqrt(a) + pow(b, 2.0);
        int t2 = (x | y) ^ z;
        
        /* More register pressure */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "memory");
        
        preserved = t1 + t2;
    }
    
    /* Final computation that uses all variables */
    ipreserved = counter * 100;
    return preserved + ipreserved + a + b + x + y;
}

/* ========== Test Case 5: Mixed Float/Int Register Bank Pressure ========== */

__attribute__((noinline, optimize("O3")))
double test5_mixed_register_banks() {
    /* Mix float and int operations to engage different register banks */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    double d1 = 5.5, d2 = 6.6, d3 = 7.7, d4 = 8.8;
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400;
    long long l1 = 1000, l2 = 2000, l3 = 3000, l4 = 4000;
    
    /* Pre-call computation using all register types */
    float fsum = f1 * f2 + f3 / f4;
    double dsum = d1 * d2 - d3 * d4;
    int isum = i1 * i2 + i3 * i4;
    long long lsum = l1 * l2 - l3 * l4;
    
    /* Force register spilling */
    volatile float vf = fsum;
    volatile double vd = dsum;
    volatile int vi = isum;
    volatile long long vl = lsum;
    
    /* Call that clobbers both integer and floating-point registers */
    asm volatile("" : : : 
                 "rax", "rbx", "rcx", "rdx",     /* Integer registers */
                 "xmm0", "xmm1", "xmm2", "xmm3", /* SSE registers */
                 "xmm4", "xmm5", "xmm6", "xmm7",
                 "memory");
    
    /* Post-call computation requiring restoration */
    float fresult = vf * 2.0f + f1 + f2;
    double dresult = vd / 2.0 + d1 - d2;
    int iresult = vi << 1 | i1 ^ i2;
    long long lresult = vl + l1 * l2;
    
    /* Mixed type computation */
    double final_result = fresult + dresult + iresult + lresult;
    
    /* Loop to create scheduling opportunities */
    for (int i = 0; i < 4; i++) {
        final_result += sin(final_result) * 0.1;
        asm volatile("" : : : "xmm15", "memory"); /* Clobber high SSE register */
    }
    
    return final_result;
}

/* ========== Main Function Orchestrating All Tests ========== */

int main() {
    double total = 0.0;
    double checksum = 0.0;
    
    printf("Starting caller-save restoration stress tests...\n");
    
    /* Run test 1 multiple times with different parameters */
    for (int i = 0; i < 5; i++) {
        double result = test1_basic_block_reordering();
        total += result;
        checksum += result * (i + 1);
        printf("Test1 iteration %d: %f\n", i, result);
    }
    
    /* Test 2 with loop-based caller-save */
    double result2 = test2_loop_caller_save();
    total += result2;
    checksum += result2 * 6;
    printf("Test2 result: %f\n", result2);
    
    /* Test 3 with conditional blocks */
    for (int i = 0; i < 8; i++) {
        double result = test3_conditional_blocks(i);
        total += result;
        checksum += result * (i + 7);
        printf("Test3 mode %d: %f\n", i, result);
    }
    
    /* Test 4 with setjmp/longjmp */
    double result4 = test4_setjmp_caller_save();
    total += result4;
    checksum += result4 * 15;
    printf("Test4 result: %f\n", result4);
    
    /* Test 5 with mixed register banks */
    double result5 = test5_mixed_register_banks();
    total += result5;
    checksum += result5 * 16;
    printf("Test5 result: %f\n", result5);
    
    /* Final validation to prevent dead code elimination */
    printf("\nFinal total: %f\n", total);
    printf("Checksum: %f\n", checksum);
    
    /* Use results to affect return value */
    if (total > 1000000.0 || checksum > 2000000.0) {
        return 0; /* Normal execution */
    }
    
    return 1; /* Should not happen with proper execution */
}
