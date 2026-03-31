/* caller-save-test.c
 * A comprehensive test to trigger caller-save restoration insertion
 * and instruction reordering within basic blocks.
 * Compile with: gcc -O3 -fno-omit-frame-pointer -fno-inline -fno-inline-small-functions -fschedule-insns2 -fno-gcse caller-save-test.c -o caller-save-test
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
int pressure_caller_save(int a, int b, int c, int d, int e, int f) {
    /* Many local variables that must survive across the call */
    volatile int v1 = a * 2;
    volatile int v2 = b + c;
    volatile int v3 = d - e;
    volatile int v4 = f * 3;
    volatile long long v5 = (long long)a * b * c;
    volatile double v6 = (double)d / (e + 1);
    volatile float v7 = (float)f * 1.5f;
    
    /* Force register pressure with parallel computations */
    int t1 = v1 * v2 + v3;
    int t2 = v2 - v3 * v4;
    long long t3 = v5 + (long long)v1 * v2;
    double t4 = v6 * 2.0 + (double)v3;
    float t5 = v7 * 3.0f + (float)v4;
    
    /* Call that clobbers registers - forces caller-save */
    clobber_registers();
    
    /* Use all variables after call - requiring restoration */
    int result = t1 + t2 + (int)t3 + (int)t4 + (int)t5;
    result += v1 + v2 + v3 + v4;
    result += (int)v5 + (int)v6 + (int)v7;
    
    return result;
}

/* Function with mixed float/int operations */
__attribute__((noinline))
double mixed_operations(int iterations) {
    double sum = 0.0;
    float fsum = 0.0f;
    int isum = 0;
    long long lsum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple live values across the call */
        double dval = sin((double)i * 0.1);
        float fval = cosf((float)i * 0.2f);
        int ival = i * 3;
        long long llval = (long long)i * i;
        
        /* Force register pressure */
        sum += dval * 2.0;
        fsum += fval * 1.5f;
        isum += ival % 7;
        lsum += llval / 3;
        
        /* Inline asm that clobbers specific registers */
        asm volatile("" : : : "xmm0", "xmm1", "xmm2", "rax", "rbx", "rcx", "memory");
        
        /* Use values after clobber */
        sum += dval;
        fsum += fval;
        isum += ival;
        lsum += llval;
    }
    
    return sum + (double)fsum + (double)isum + (double)lsum;
}

/* ========== Control Flow Manipulation ========== */

/* Function with complex control flow */
__attribute__((noinline, optimize("O3")))
int control_flow_test(int x, int y) {
    int result = 0;
    volatile int a = x;
    volatile int b = y;
    volatile int c = x + y;
    volatile int d = x * y;
    
    /* Multiple basic blocks with calls */
    if (x > 0) {
        int t1 = a * b + c;
        int t2 = d - a;
        
        /* Call in one branch */
        clobber_registers();
        
        result = t1 + t2;
        
        if (y > 0) {
            int t3 = result * 2;
            int t4 = b + c;
            
            /* Another call */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
            
            result = t3 + t4 + a;
        } else {
            int t5 = result / 2;
            int t6 = d - c;
            
            /* Different clobber list */
            asm volatile("" : : : "rsi", "rdi", "r8", "r9", "memory");
            
            result = t5 + t6 + b;
        }
    } else {
        int t7 = b - a;
        int t8 = c * d;
        
        clobber_registers();
        
        result = t7 + t8;
        
        /* Switch to create more edges */
        switch (y % 4) {
            case 0:
                result += a * 2;
                asm volatile("" : : : "xmm0", "xmm1", "rax", "memory");
                break;
            case 1:
                result += b * 3;
                asm volatile("" : : : "xmm2", "xmm3", "rbx", "memory");
                break;
            case 2:
                result += c * 4;
                asm volatile("" : : : "xmm4", "xmm5", "rcx", "memory");
                break;
            default:
                result += d * 5;
                asm volatile("" : : : "xmm6", "xmm7", "rdx", "memory");
                if (result > 1000) {
                    /* Unreachable hint */
                    __builtin_unreachable();
                }
                break;
        }
    }
    
    return result;
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline))
long long loop_with_calls(int n) {
    long long total = 0;
    volatile int vars[10];
    
    /* Initialize many variables */
    for (int i = 0; i < 10; i++) {
        vars[i] = i * n;
    }
    
    for (int i = 0; i < n; i++) {
        /* Many live values in loop */
        int v1 = vars[0] + i;
        int v2 = vars[1] * i;
        int v3 = vars[2] - i;
        int v4 = vars[3] / (i + 1);
        long long v5 = (long long)vars[4] * i;
        double v6 = (double)vars[5] * 1.1;
        float v7 = (float)vars[6] * 2.2f;
        
        /* Computation before call */
        int t1 = v1 * v2 + v3;
        long long t2 = v5 + (long long)v4 * 3;
        double t3 = v6 * 2.5;
        float t4 = v7 * 1.8f;
        
        /* Function call in loop - forces caller-save each iteration */
        clobber_registers();
        
        /* Use values after call */
        total += t1 + (int)t2 + (int)t3 + (int)t4;
        total += v1 + v2 + v3 + v4;
        
        /* Update some variables */
        vars[i % 10] = total % 1000;
    }
    
    return total;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf env;

__attribute__((noinline))
int setjmp_test(int x) {
    volatile int a = x * 2;
    volatile int b = x + 10;
    volatile int c = x - 5;
    
    if (setjmp(env) == 0) {
        /* First execution */
        int t1 = a * b + c;
        int t2 = b - c;
        
        /* Clobber before longjmp */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        
        /* Simulate condition for longjmp */
        if (x > 100) {
            longjmp(env, 1);
        }
        
        return t1 + t2;
    } else {
        /* After longjmp - registers need restoration */
        int t3 = a + b * c;
        int t4 = c - a;
        
        /* Force more clobbering */
        asm volatile("" : : : "rsi", "rdi", "r8", "r9", "memory");
        
        return t3 + t4;
    }
}

/* ========== External Function Declaration ========== */

/* Simulate external function that compiler can't see */
extern void external_call(int, int, int, int, int, int);

__attribute__((noinline))
void call_external_with_pressure(void) {
    /* Many live variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int g = 7, h = 8, i = 9, j = 10;
    double k = 11.5;
    float l = 12.5f;
    
    /* Use variables before call */
    int t1 = a * b + c * d;
    int t2 = e + f + g + h;
    double t3 = k * 2.0;
    float t4 = l * 1.5f;
    
    /* External call - forces full caller-save */
    external_call(a, b, c, d, e, f);
    
    /* Use variables after call - requiring restoration */
    int result = t1 + t2 + (int)t3 + (int)t4;
    result += i + j;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
}

/* Dummy implementation to satisfy linker */
void external_call(int a, int b, int c, int d, int e, int f) {
    /* Do nothing but clobber registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2",
                  "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory");
}

/* ========== Main Test Orchestrator ========== */

int main(void) {
    int checksum = 0;
    
    printf("Starting caller-save restoration tests...\n");
    
    /* Test 1: Basic caller-save pressure */
    printf("Test 1: Pressure caller-save...\n");
    int r1 = pressure_caller_save(10, 20, 30, 40, 50, 60);
    checksum += r1;
    printf("  Result: %d\n", r1);
    
    /* Test 2: Mixed operations */
    printf("Test 2: Mixed float/int operations...\n");
    double r2 = mixed_operations(50);
    checksum += (int)r2;
    printf("  Result: %f\n", r2);
    
    /* Test 3: Complex control flow */
    printf("Test 3: Control flow with calls...\n");
    int r3 = control_flow_test(25, -15);
    checksum += r3;
    printf("  Result: %d\n", r3);
    
    /* Test 4: Loop with calls */
    printf("Test 4: Loop with caller-save pressure...\n");
    long long r4 = loop_with_calls(100);
    checksum += (int)(r4 % 1000000);
    printf("  Result: %lld\n", r4);
    
    /* Test 5: setjmp/longjmp */
    printf("Test 5: setjmp/longjmp test...\n");
    int r5 = setjmp_test(150);
    checksum += r5;
    printf("  Result: %d\n", r5);
    
    /* Test 6: External call */
    printf("Test 6: External call with pressure...\n");
    call_external_with_pressure();
    checksum += 42; /* Arbitrary */
    
    /* Final validation */
    printf("\nAll tests completed.\n");
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0) {
        printf("ERROR: All code eliminated!\n");
        return 1;
    }
    
    return 0;
}
