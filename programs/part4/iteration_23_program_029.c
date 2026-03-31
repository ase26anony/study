/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -o test-caller-save
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 0;
volatile double global_volatile_double = 0.0;
volatile long global_volatile_long = 0;

/* Function pointers for indirect calls */
typedef void (*func_ptr_t)(void);
typedef int (*func_ptr_int_t)(int, int, int, int, int, int);

/* Memory barrier */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with specific constraints */
#define FORCE_REGISTER(var, reg) \
    register int var asm(reg) = global_volatile_int

/* Complex function with many live variables across call */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many live variables in call-clobbered registers */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    
    /* Use explicit register variables */
    register int r10_val asm("r10") = v1 + v2;
    register int r11_val asm("r11") = v3 + v4;
    
    /* Array to save values across call */
    int saved[8];
    saved[0] = v1;
    saved[1] = v2;
    saved[2] = v3;
    saved[3] = v4;
    saved[4] = v5;
    saved[5] = v6;
    saved[6] = v7;
    saved[7] = v8;
    
    COMPILER_BARRIER();
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        /* Direct call */
        opaque_call_1();
        
        /* Inline asm that looks like a call */
        asm volatile(
            "movl $0x12345678, %%eax\n\t"
            "movl $0x87654321, %%ebx\n\t"
            "call *%%eax\n\t"
            : : : "eax", "ebx", "ecx", "edx", "memory"
        );
    } else {
        /* Indirect call */
        func_ptr_t fp = (func_ptr_t)opaque_call_1;
        fp();
    }
    
    COMPILER_BARRIER();
    
    /* Complex use of saved values requiring original registers */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += saved[i] * (i + 1);
    }
    
    /* Force register pressure */
    sum += r10_val * r11_val;
    
    /* Another call with live values */
    opaque_call_2(sum);
    
    global_volatile_int += sum;
}

/* Function with floating point and mixed types */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double d1 = base * 1.1;
    volatile double d2 = base * 2.2;
    volatile double d3 = base * 3.3;
    volatile double d4 = base * 4.4;
    
    volatile int i1 = (int)d1;
    volatile int i2 = (int)d2;
    volatile int i3 = (int)d3;
    volatile int i4 = (int)d4;
    
    /* Save across call */
    double saved_d[4] = {d1, d2, d3, d4};
    int saved_i[4] = {i1, i2, i3, i4};
    
    /* Create irreducible control flow with goto */
    int counter = 0;
    
start_label:
    if (counter++ > 3) goto end_label;
    
    /* Function call in the middle of complex control flow */
    double result = opaque_call_4(d1, d2);
    
    /* Use saved values */
    d3 = saved_d[2] + result;
    i3 = saved_i[2] + (int)result;
    
    /* Another call */
    opaque_call_2(i3);
    
    /* Jump creates basic block boundary */
    if (i3 & 1) {
        goto start_label;
    }
    
end_label:
    
    /* Force register usage after label */
    global_volatile_double += d1 + d2 + d3 + d4;
    global_volatile_int += i1 + i2 + i3 + i4;
}

/* Function with switch statement creating complex CFG */
__attribute__((noinline, noclone))
int test3(int selector) {
    volatile int a = 10, b = 20, c = 30, d = 40, e = 50, f = 60;
    int result = 0;
    
    /* Save values */
    int saved[6] = {a, b, c, d, e, f};
    
    /* Complex switch with calls in cases */
    switch (selector) {
        case 0:
            opaque_call_1();
            result = saved[0] + saved[1];
            break;
        case 1:
            opaque_call_2(saved[1]);
            result = saved[1] * saved[2];
            /* Fall through */
        case 2:
            opaque_call_2(saved[2]);
            result += saved[3] - saved[4];
            /* Create another basic block */
            if (result > 0) {
                opaque_call_1();
            }
            break;
        case 3:
            /* Nested call sequence */
            result = opaque_call_3(saved[3], saved[4]);
            opaque_call_2(result);
            result += saved[5];
            break;
        default:
            /* This creates a basic block with call at end */
            opaque_call_2(selector);
            result = saved[0] + saved[1] + saved[2] + 
                    saved[3] + saved[4] + saved[5];
            /* Force insertion after call */
            result *= 2;
    }
    
    /* Use all saved values in complex expression */
    for (int i = 0; i < 6; i++) {
        result += saved[i] * (i + 1);
    }
    
    return result;
}

/* Function with loop containing break/continue and calls */
__attribute__((noinline, noclone))
void test4(int iterations) {
    volatile int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
    }
    
    int sum = 0;
    int i = 0;
    
    /* Loop with complex control flow */
    while (1) {
        if (i >= iterations) break;
        
        volatile int temp = array[i];
        
        /* Call in loop */
        if (i & 1) {
            opaque_call_2(temp);
            
            /* Force register save around call */
            int saved_temp = temp;
            opaque_call_1();
            temp = saved_temp * 2;
        }
        
        /* Another conditional with call */
        if (temp > 10) {
            opaque_call_2(temp);
            continue;  /* Creates basic block boundary */
        }
        
        sum += temp;
        
        /* Nested loop with call */
        for (int j = 0; j < 2; j++) {
            int inner_temp = temp + j;
            opaque_call_2(inner_temp);
            sum += inner_temp;
        }
        
        i++;
    }
    
    global_volatile_int += sum;
}

/* Function using __builtin_apply for unusual calling convention */
__attribute__((noinline, noclone))
void test5(void) {
    /* Create variable argument list */
    volatile int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5, a6 = 6;
    
    /* Save values */
    int saved[6] = {a1, a2, a3, a4, a5, a6};
    
    /* Simulate __builtin_apply behavior */
    void* args = __builtin_apply_args();
    
    /* Force register pressure */
    register int r1 asm("eax") = saved[0];
    register int r2 asm("ebx") = saved[1];
    register int r3 asm("ecx") = saved[2];
    
    COMPILER_BARRIER();
    
    /* Multiple calls */
    opaque_call_3(r1, r2);
    
    /* Use registers immediately after call */
    int sum = r1 + r2 + r3;
    
    /* Another call */
    opaque_call_2(sum);
    
    /* Restore and use saved values */
    for (int i = 0; i < 6; i++) {
        sum += saved[i];
    }
    
    global_volatile_int = sum;
}

/* Helper with nested calls */
__attribute__((noinline, noclone))
int nested_helper(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    volatile int saved = value;
    
    /* Call that might need caller-save */
    int result = opaque_call_3(value, depth);
    
    /* Recursive call */
    int nested = nested_helper(depth - 1, result + saved);
    
    /* Use saved value after nested call */
    return nested + saved;
}

/* Main test driver */
int main(int argc, char** argv) {
    int test_mode = 0;
    
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 6;
    }
    
    /* Initialize with runtime values */
    global_volatile_int = argc;
    global_volatile_double = (double)argc / 2.0;
    
    /* Run all tests in different orders based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 6) {
            case 0:
                test1(cycle);
                test2(global_volatile_double);
                break;
            case 1:
                test2(global_volatile_double + 1.0);
                test3(cycle);
                break;
            case 2:
                test3(cycle + 1);
                test4(5 + cycle);
                break;
            case 3:
                test4(3 + cycle);
                test5();
                break;
            case 4:
                test5();
                test1(cycle ^ 1);
                break;
            case 5:
                /* Test nested calls */
                int result = nested_helper(3, cycle * 10);
                global_volatile_int += result;
                test3(result % 4);
                break;
        }
        
        /* Memory barrier between cycles */
        COMPILER_BARRIER();
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = global_volatile_int + (long)global_volatile_double + global_volatile_long;
    
    /* Use checksum in opaque way */
    asm volatile(
        "/* Checksum: %0 */\n\t"
        : : "r"(checksum)
    );
    
    printf("Result: %ld\n", checksum);
    
    return 0;
}

/* Dummy definitions for external functions to allow linking */
void opaque_call_1(void) {
    COMPILER_BARRIER();
}

void opaque_call_2(int x) {
    global_volatile_int ^= x;
    COMPILER_BARRIER();
}

int opaque_call_3(int a, int b) {
    COMPILER_BARRIER();
    return a + b + global_volatile_int;
}

double opaque_call_4(double a, double b) {
    COMPILER_BARRIER();
    return a * b + global_volatile_double;
}
