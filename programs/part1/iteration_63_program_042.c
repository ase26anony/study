/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction chain manipulation code in
 * caller-save.cc (lines 905-913). It creates scenarios where:
 * 1. Caller-saved registers must be saved/restored around function calls
 * 2. Basic block boundaries are manipulated with conditional control flow
 * 3. High register pressure forces spill/fill decisions
 * 4. Instruction scheduling interacts with restore placement
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller to save many registers before this call */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d,
                          double e, double f, double g, double h) {
    /* Use all parameters in parallel computations */
    double t1 = a * b + c * d;
    double t2 = e * f - g * h;
    double t3 = a * c + b * d;
    double t4 = e * g - f * h;
    
    /* Clobber caller-saved registers with inline asm */
    asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx",
                               "xmm0", "xmm1", "xmm2", "xmm3",
                               "xmm4", "xmm5", "xmm6", "xmm7");
    
    return t1 * t2 + t3 * t4;
}

/* Function that uses mixed float/int operations */
__attribute__((noinline))
long long mixed_operations(int a, float b, double c, long long d) {
    volatile int vi = a;  /* Prevent optimization */
    volatile float vf = b;
    
    /* Force use of different register banks */
    double d1 = c * vi + vf;
    long long ll1 = d + (long long)(d1 * 100.0);
    
    /* Clobber specific registers */
    asm volatile("" ::: "rax", "rdx", "xmm0", "xmm1", "xmm2");
    
    int i2 = vi * 3;
    float f2 = vf * 2.5f;
    return ll1 + i2 + (long long)f2;
}

/* ========== Functions with Complex Control Flow ========== */

__attribute__((noinline, optimize("O3")))
int branching_function(int mode, int x, int y, int z) {
    int result = 0;
    
    /* Create multiple basic blocks with calls in between */
    if (mode & 1) {
        /* Live variables across call */
        int a = x * y;
        int b = y * z;
        int c = z * x;
        
        /* Call clobbers registers */
        asm volatile("" ::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory");
        
        result += a + b + c;
    }
    
    if (mode & 2) {
        /* Different set of live variables */
        float f1 = x * 1.5f;
        float f2 = y * 2.5f;
        double d1 = z * 3.14159;
        
        /* Another register-clobbering call */
        double temp = complex_calculation(f1, f2, d1, 1.0, 2.0, 3.0, 4.0, 5.0);
        
        result += (int)(temp * 100);
    }
    
    if (mode & 4) {
        /* More live variables */
        long long ll1 = x * 100LL;
        long long ll2 = y * 200LL;
        long long ll3 = z * 300LL;
        
        asm volatile("" ::: "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "memory");
        
        result += (int)((ll1 + ll2 + ll3) % 1000);
    }
    
    return result;
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((optimize("O3")))
void loop_with_calls(int iterations) {
    /* Many live variables that must survive across loop iterations */
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f, f4 = 4.5f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    long long ll1 = 100, ll2 = 200, ll3 = 300, ll4 = 400;
    
    for (int i = 0; i < iterations; i++) {
        /* Modify variables before call */
        d1 += i * 0.1;
        d2 += i * 0.2;
        f1 += i * 0.05f;
        f2 += i * 0.15f;
        i1 += i;
        i2 += i * 2;
        ll1 += i * 10;
        ll2 += i * 20;
        
        /* Function call clobbers caller-saved registers */
        double result = complex_calculation(d1, d2, d3, d4, f1, f2, f3, f4);
        
        /* Use results after call - forces restore */
        i3 += (int)(result * 100);
        i4 += i1 + i2;
        ll3 += (long long)(d1 * 1000);
        ll4 += ll1 + ll2;
        
        /* More register pressure */
        asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3",
                               "xmm4", "xmm5", "xmm6", "xmm7",
                               "rax", "rbx", "rcx", "rdx", "memory");
    }
    
    /* Prevent dead code elimination */
    volatile double sink = d1 + d2 + d3 + d4;
    volatile int isink = i1 + i2 + i3 + i4;
    (void)sink;
    (void)isink;
}

/* ========== Switch Statement with Multiple Edges ========== */

__attribute__((noinline))
int switch_test(int value) {
    int result = 0;
    
    /* Many live variables across switch */
    double d1 = 1.234, d2 = 5.678, d3 = 9.012;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    int i1 = 10, i2 = 20, i3 = 30;
    
    switch (value % 5) {
        case 0:
            /* Call with current live values */
            result = branching_function(1, i1, i2, i3);
            d1 += result * 0.01;
            break;
            
        case 1:
            /* Different computation path */
            asm volatile("" ::: "rax", "rbx", "rcx", "xmm0", "xmm1", "memory");
            result = (int)(d1 * d2 * 100);
            f1 += result * 0.1f;
            break;
            
        case 2:
            /* Use mixed operations */
            long long ll = mixed_operations(i1, f1, d1, i2);
            result = (int)(ll % 1000);
            d3 += result * 0.001;
            break;
            
        case 3:
            /* More complex path */
            result = i1 * i2 * i3;
            asm volatile("" ::: "rax", "rdx", "xmm2", "xmm3", "xmm4", "memory");
            f2 = f3 * 2.0f + result * 0.01f;
            break;
            
        case 4:
            /* Path with unreachable code hint */
            result = -1;
            asm volatile("" ::: "memory");
            if (result < 0) {
                __builtin_unreachable();
            }
            break;
    }
    
    /* All paths merge here with different live values */
    return result + (int)d1 + (int)f1 + i1;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;
static volatile int jmp_value = 0;

__attribute__((noinline))
void function_with_setjmp(int *ptr) {
    /* Variables that must be saved across longjmp */
    double d1 = 3.14159, d2 = 2.71828;
    float f1 = 1.618f, f2 = 0.577f;
    int i1 = 42, i2 = 99;
    
    if (setjmp(env) == 0) {
        /* First call - modify variables */
        d1 *= 2.0;
        f1 *= 1.5f;
        i1 += 10;
        
        /* Call that might longjmp */
        if (*ptr > 100) {
            longjmp(env, 1);
        }
        
        /* More computations */
        d2 = complex_calculation(d1, d2, f1, f2, 1.0, 2.0, 3.0, 4.0);
        i2 = branching_function(3, i1, i2, *ptr);
    } else {
        /* After longjmp - use original values */
        *ptr = (int)(d1 + d2 + f1 + f2) + i1 + i2;
    }
}

/* ========== Main Test Orchestrator ========== */

int main() {
    int checksum = 0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Complex calculation with many live variables */
    printf("Test 1: Complex calculation...\n");
    double result1 = complex_calculation(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8);
    checksum += (int)(result1 * 1000);
    
    /* Test 2: Mixed operations */
    printf("Test 2: Mixed operations...\n");
    long long result2 = mixed_operations(10, 20.5f, 30.75, 400LL);
    checksum += (int)(result2 % 10000);
    
    /* Test 3: Branching function */
    printf("Test 3: Branching function...\n");
    int result3 = branching_function(7, 1, 2, 3);
    checksum += result3;
    
    /* Test 4: Loop with calls */
    printf("Test 4: Loop with calls...\n");
    loop_with_calls(10);
    checksum += 1000;  /* Fixed contribution */
    
    /* Test 5: Switch test */
    printf("Test 5: Switch test...\n");
    for (int i = 0; i < 10; i++) {
        checksum += switch_test(i);
    }
    
    /* Test 6: setjmp/longjmp */
    printf("Test 6: setjmp/longjmp...\n");
    int jmp_test = 150;
    function_with_setjmp(&jmp_test);
    checksum += jmp_test;
    
    /* Final validation */
    printf("Final checksum: %d\n", checksum);
    printf("Expected range: 15000-25000 (compiler dependent)\n");
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0) {
        printf("ERROR: All computations eliminated!\n");
        return 1;
    }
    
    return 0;
}
