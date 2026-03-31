/* caller-save-test.c
 * 
 * This program is designed to stress GCC's caller-save restoration logic,
 * specifically targeting the instruction reordering code in caller-save.cc
 * that manipulates the instruction chain pointers (SET_NEXT_INSN, SET_PREV_INSN).
 * 
 * Compilation recommendations:
 *   gcc -O3 -fno-inline -fno-inline-small-functions -fno-omit-frame-pointer \
 *       -fschedule-insns2 -fno-gcse -march=native -fno-rename-registers \
 *       -fno-sched-interblock caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

/* Global volatile variables to prevent dead code elimination */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* ========== Helper Functions with Register Pressure ========== */

/* Force caller-save by clobbering many registers */
__attribute__((noinline))
void clobber_registers(void) {
    /* Clobber multiple caller-saved registers */
    asm volatile(
        "# Clobber caller-saved registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
}

/* Function with many live variables across a call */
__attribute__((noinline, optimize("O3")))
double complex_calculation(double a, double b, double c, double d,
                           double e, double f, double g, double h) {
    /* Create many intermediate values that must survive the clobber call */
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = g * h / 2.0;
    double t4 = t1 * t2 + t3;
    double t5 = sin(t1) + cos(t2);
    double t6 = t4 * t5;
    
    /* Force caller-save by clobbering registers */
    clobber_registers();
    
    /* Use all the temporaries after the call - they must be restored */
    double result = t1 + t2 + t3 + t4 + t5 + t6;
    
    /* More computations to create scheduling opportunities */
    result = result * a / b + c * d - e / f;
    
    /* Another clobber to force more save/restore */
    asm volatile("# Another clobber" ::: "rax", "rcx", "rdx", "memory");
    
    return result + g + h;
}

/* Function with mixed float/int operations */
__attribute__((noinline))
long long mixed_operations(int a, int b, float c, float d,
                           double e, double f, long long g) {
    /* Use all parameters in computations */
    int i1 = a * b + (int)c;
    float f1 = c * d + (float)a;
    double d1 = e * f + (double)b;
    long long ll1 = g + (long long)i1;
    
    /* Force caller-save */
    clobber_registers();
    
    /* More mixed computations */
    i1 = i1 + (int)(f1 * 2.0f);
    f1 = f1 + (float)(d1 / 3.0);
    d1 = d1 + (double)ll1;
    ll1 = ll1 * 2 - (long long)i1;
    
    /* Another function call to create basic block boundaries */
    global_counter++;
    
    return ll1 + (long long)d1 + (long long)f1 + i1;
}

/* ========== Loop with Caller-Save Pressure ========== */

__attribute__((noinline, optimize("O3")))
double loop_with_calls(int iterations) {
    double sum = 0.0;
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4;
    double e = 5.5, f = 6.6, g = 7.7, h = 8.8;
    
    for (int i = 0; i < iterations; i++) {
        /* Loop-invariant computations that need registers */
        double inv1 = a * i + b;
        double inv2 = c * i - d;
        double inv3 = e / (i + 1) + f;
        double inv4 = g * h * i;
        
        /* Function call that clobbers caller-saved registers */
        double temp = complex_calculation(inv1, inv2, inv3, inv4,
                                         a, b, c, d);
        
        /* Use invariants after the call - they must be restored */
        sum += temp + inv1 + inv2 + inv3 + inv4;
        
        /* Modify variables to create live ranges */
        a += 0.1;
        b -= 0.1;
        c *= 1.01;
        d /= 1.01;
    }
    
    return sum;
}

/* ========== Conditional Control Flow ========== */

__attribute__((noinline))
double conditional_caller_save(int mode, double x, double y) {
    double result = 0.0;
    
    /* Create multiple basic blocks with different caller-save patterns */
    if (mode == 0) {
        double a = x * y;
        double b = x / y;
        double c = x + y;
        double d = x - y;
        
        clobber_registers();
        
        result = a + b + c + d;
    } 
    else if (mode == 1) {
        float f1 = (float)x * 2.0f;
        float f2 = (float)y * 3.0f;
        int i1 = (int)x * 4;
        int i2 = (int)y * 5;
        
        clobber_registers();
        
        result = (double)(f1 + f2) + (double)(i1 * i2);
    }
    else if (mode == 2) {
        /* Chain of computations with intermediate calls */
        double t1 = sin(x);
        double t2 = cos(y);
        
        clobber_registers();
        
        double t3 = t1 * t2;
        
        clobber_registers();
        
        double t4 = exp(t3);
        
        clobber_registers();
        
        result = t1 + t2 + t3 + t4;
    }
    else {
        /* Default case with many live variables */
        double v1 = x, v2 = y, v3 = x*y, v4 = x/y;
        double v5 = x*x, v6 = y*y, v7 = x+y, v8 = x-y;
        double v9 = sqrt(x), v10 = sqrt(y);
        
        clobber_registers();
        
        result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    return result;
}

/* ========== Switch Statement with Multiple Edges ========== */

__attribute__((noinline, optimize("O3")))
double switch_caller_save(int option, double base) {
    double result = base;
    
    switch (option % 4) {
        case 0: {
            /* Many live variables */
            double a = result * 1.1;
            double b = result * 2.2;
            double c = result * 3.3;
            
            clobber_registers();
            
            double temp = complex_calculation(a, b, c, result,
                                             a+1, b+2, c+3, result+4);
            
            /* Use all variables after call */
            result = a + b + c + temp;
            break;
        }
            
        case 1: {
            /* Different register types */
            int i1 = (int)result * 2;
            int i2 = (int)result * 3;
            float f1 = (float)result * 4.0f;
            float f2 = (float)result * 5.0f;
            
            clobber_registers();
            
            result = (double)(i1 + i2) * (double)(f1 + f2);
            break;
        }
            
        case 2: {
            /* Nested computations */
            double t1 = result;
            for (int i = 0; i < 3; i++) {
                t1 = t1 * 1.5 + (double)i;
                clobber_registers();
            }
            result = t1;
            break;
        }
            
        case 3: {
            /* Chain of function calls */
            result = conditional_caller_save(0, result, result*2);
            clobber_registers();
            result = conditional_caller_save(1, result, result/2);
            clobber_registers();
            result = conditional_caller_save(2, result, sqrt(result));
            break;
        }
    }
    
    return result;
}

/* ========== setjmp/longjmp Test ========== */

static jmp_buf jump_buffer;

__attribute__((noinline))
int setjmp_test(double *values, int count) {
    /* Create many live variables before setjmp */
    double sum = 0.0;
    double prod = 1.0;
    double max = values[0];
    double min = values[0];
    
    for (int i = 0; i < count; i++) {
        sum += values[i];
        prod *= values[i];
        if (values[i] > max) max = values[i];
        if (values[i] < min) min = values[i];
    }
    
    int ret = setjmp(jump_buffer);
    
    if (ret == 0) {
        /* Force caller-save of all computed values */
        clobber_registers();
        
        /* Use the variables after setjmp */
        double result = (sum + prod) / (max - min + 1.0);
        global_accumulator += result;
        
        /* Simulate error - jump back */
        if (global_counter++ < 3) {
            longjmp(jump_buffer, 1);
        }
    }
    
    return ret;
}

/* ========== Main Test Orchestrator ========== */

int main(void) {
    double total_checksum = 0.0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Test 1: Complex calculation with many live variables */
    printf("Test 1: Complex calculation...\n");
    double r1 = complex_calculation(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8);
    total_checksum += r1;
    printf("  Result: %f\n", r1);
    
    /* Test 2: Mixed operations */
    printf("Test 2: Mixed int/float operations...\n");
    long long r2 = mixed_operations(10, 20, 3.14f, 2.71f, 1.618, 2.718, 1000LL);
    total_checksum += (double)r2;
    printf("  Result: %lld\n", r2);
    
    /* Test 3: Loop with calls */
    printf("Test 3: Loop with function calls...\n");
    double r3 = loop_with_calls(10);
    total_checksum += r3;
    printf("  Result: %f\n", r3);
    
    /* Test 4: Conditional control flow */
    printf("Test 4: Conditional caller-save...\n");
    for (int i = 0; i < 4; i++) {
        double r4 = conditional_caller_save(i, 10.0 + i, 20.0 - i);
        total_checksum += r4;
        printf("  Mode %d: %f\n", i, r4);
    }
    
    /* Test 5: Switch statement */
    printf("Test 5: Switch statement with multiple edges...\n");
    for (int i = 0; i < 8; i++) {
        double r5 = switch_caller_save(i, 5.0 * (i + 1));
        total_checksum += r5;
        printf("  Option %d: %f\n", i, r5);
    }
    
    /* Test 6: setjmp/longjmp */
    printf("Test 6: setjmp/longjmp test...\n");
    double values[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    int r6 = setjmp_test(values, 5);
    total_checksum += (double)r6;
    printf("  setjmp returned: %d\n", r6);
    
    /* Test 7: __builtin_unreachable test */
    printf("Test 7: __builtin_unreachable test...\n");
    {
        double a = 100.0, b = 200.0, c = 300.0;
        double temp = a * b + c;
        
        clobber_registers();
        
        double result = temp / (a + b + c);
        total_checksum += result;
        
        if (result > 1000.0) {
            __builtin_unreachable();  /* May affect block termination */
        }
        printf("  Result: %f\n", result);
    }
    
    /* Final checksum */
    printf("\nTotal checksum: %f\n", total_checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Validate that computations were performed */
    if (total_checksum != 0.0 && global_counter > 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Error: Tests may have been optimized away.\n");
        return 1;
    }
}
