/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256];
volatile double global_farray[256];

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
func_ptr_t func_table[10];

/* Force register usage with explicit register variables */
register int reg_var_1 asm ("r12");
register int reg_var_2 asm ("r13");
register int reg_var_3 asm ("r14");
register double reg_fvar_1 asm ("xmm8");
register double reg_fvar_2 asm ("xmm9");

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test_function_1(int mode) {
    volatile int local_vars[20];
    volatile double f_local_vars[10];
    register int r1 asm ("r10");
    register int r2 asm ("r11");
    
    /* Force many live variables */
    for (int i = 0; i < 20; i++) {
        local_vars[i] = i + mode + global_counter;
    }
    
    /* Use explicit register variables */
    r1 = local_vars[0] * 2;
    r2 = local_vars[1] * 3;
    
    /* Inline asm to clobber call-clobbered registers */
    asm volatile ("# Clobber eax, r10, r11"
                  : 
                  : 
                  : "eax", "r10", "r11", "memory");
    
    /* Function call with many arguments - forces caller-save */
    int result = opaque_call_3(r1, r2);
    
    /* Complex use of saved values after call */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += local_vars[i] + result;
    }
    
    /* Another asm barrier */
    asm volatile ("# Another clobber"
                  :
                  :
                  : "rax", "r10", "r11", "r12", "memory");
    
    /* Indirect call through function pointer */
    if (mode & 1) {
        func_table[0]();
    }
    
    /* Use all variables in complex expression */
    global_array[mode % 256] = sum + r1 + r2 + local_vars[5];
}

/* Function with floating point and mixed usage */
__attribute__((noinline, noclone))
void test_function_2(double base) {
    volatile double d1, d2, d3, d4, d5;
    volatile int i1, i2, i3;
    register double fr1 asm ("xmm10");
    register double fr2 asm ("xmm11");
    
    /* Setup floating point values */
    d1 = base;
    d2 = base * 2.0;
    d3 = base * 3.0;
    d4 = base * 4.0;
    d5 = base * 5.0;
    
    fr1 = d1 + d2;
    fr2 = d3 + d4;
    
    /* Call that clobbers floating point registers */
    asm volatile ("# Clobber xmm0-xmm5"
                  :
                  :
                  : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory");
    
    /* Function call */
    double result = opaque_call_4(fr1, fr2);
    
    /* Complex control flow with calls at boundaries */
    for (int i = 0; i < 10; i++) {
        if (i & 1) {
            /* Call at loop body boundary */
            opaque_call_1();
            d1 += result;
        } else {
            d2 += result;
        }
        
        /* Nested condition with goto to create complex CFG */
        if (i == 5) {
            i3 = (int)d3;
            goto special_case;
        }
        
        continue;
        
    special_case:
        opaque_call_2(i3);
        break;
    }
    
    /* Use all saved values */
    global_farray[(int)base % 256] = d1 + d2 + d3 + d4 + d5 + fr1 + fr2;
}

/* Function with switch statement creating complex CFG */
__attribute__((noinline, noclone))
int test_function_3(int selector) {
    volatile int cases[10];
    int result = 0;
    
    /* Initialize cases */
    for (int i = 0; i < 10; i++) {
        cases[i] = i * selector + global_counter;
    }
    
    /* Complex switch with calls in multiple cases */
    switch (selector % 7) {
        case 0:
            result = cases[0] + cases[1];
            /* Call at case boundary */
            opaque_call_1();
            break;
        case 1:
            result = cases[2] - cases[3];
            opaque_call_2(result);
            /* Fall through */
        case 2:
            result += cases[4];
            asm volatile ("# Case 2 asm" : : : "rax", "rcx", "memory");
            break;
        case 3:
            /* Nested loop with call */
            for (int j = 0; j < 3; j++) {
                result += cases[j];
                if (j == 1) {
                    opaque_call_3(result, j);
                }
            }
            break;
        default:
            /* Default case with function call */
            result = opaque_call_3(cases[5], cases[6]);
            /* Use goto to create irreducible flow */
            if (result > 100) {
                goto default_exit;
            }
            break;
    }
    
    return result;

default_exit:
    /* Additional processing after goto */
    asm volatile ("# Default exit asm" : : : "r12", "r13", "memory");
    return result * 2;
}

/* Function with __builtin_apply to force unusual register usage */
__attribute__((noinline, noclone))
void test_function_4(void *args) {
    /* Simulate variable arguments */
    volatile long arg1 = 12345;
    volatile double arg2 = 67.89;
    volatile int arg3 = 101112;
    
    /* Use __builtin_apply_args to get argument pointer */
    void *argp = __builtin_apply_args();
    
    /* Force register pressure */
    register int r1 asm ("r15");
    register int r2 asm ("r14");
    register int r3 asm ("r13");
    
    r1 = (int)arg1;
    r2 = (int)arg2;
    r3 = arg3;
    
    /* Multiple asm statements to force save/restore insertion */
    asm volatile ("# Pre-call setup 1" : : : "rax", "rdx", "rcx", "memory");
    asm volatile ("# Pre-call setup 2" : : : "r8", "r9", "r10", "r11", "memory");
    
    /* Make a call */
    int temp = opaque_call_3(r1, r2);
    
    /* Immediately use result in complex expression requiring temps */
    int complex_result = (temp * r3) / (r1 + 1) - (r2 << 2);
    
    /* Another asm barrier */
    asm volatile ("# Post-call use" : : : "rax", "rdx", "memory");
    
    global_array[complex_result % 256] = complex_result;
}

/* Helper with nested calls to create outer/inner save scenarios */
__attribute__((noinline, noclone))
int nested_call_helper(int depth, int value) {
    volatile int saved = value;
    
    if (depth > 0) {
        /* Recursive call */
        int inner = nested_call_helper(depth - 1, value * 2);
        
        /* Use saved value after inner call */
        asm volatile ("# After nested call" : : : "rbx", "rbp", "r12", "memory");
        return saved + inner;
    }
    
    /* Leaf call */
    opaque_call_2(value);
    return value;
}

/* Main test orchestrator */
int main(int argc, char *argv[]) {
    int test_mode = 0;
    
    /* Use argv to select mode but ensure all paths are taken */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize function pointers */
    for (int i = 0; i < 10; i++) {
        func_table[i] = opaque_call_1;
    }
    
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
        global_farray[i] = i * 1.5;
    }
    
    /* Execute all test functions in different orders based on mode */
    int checksum = 0;
    
    /* Complex loop structure to force basic block splitting */
    for (int outer = 0; outer < 3; outer++) {
        switch ((test_mode + outer) % 4) {
            case 0:
                test_function_1(outer * 10);
                /* Call at loop boundary */
                if (outer == 1) {
                    opaque_call_1();
                }
                break;
            case 1:
                test_function_2(outer * 3.14159);
                /* Use goto to create irreducible flow */
                if (outer == 2) {
                    goto loop_merge;
                }
                break;
            case 2:
                checksum += test_function_3(outer * 7);
                break;
            case 3:
                test_function_4((void*)(long)outer);
                break;
        }
        
        /* Nested loop with function call */
        for (int inner = 0; inner < 2; inner++) {
            int nested_result = nested_call_helper(2, outer * 100 + inner);
            checksum += nested_result;
            
            /* Conditional break to force block boundary */
            if (nested_result > 1000 && inner == 1) {
                break;
            }
        }
        
    loop_merge:
        /* Merge point after goto */
        global_counter++;
    }
    
    /* Force use of all global state to prevent DCE */
    int final_checksum = checksum;
    for (int i = 0; i < 256; i++) {
        final_checksum += global_array[i];
        final_checksum += (int)global_farray[i];
    }
    final_checksum += global_counter;
    
    /* Use register variables in final computation */
    reg_var_1 = final_checksum % 1000;
    reg_var_2 = final_checksum / 1000;
    reg_var_3 = reg_var_1 * reg_var_2;
    
    reg_fvar_1 = final_checksum * 0.01;
    reg_fvar_2 = reg_fvar_1 * 2.0;
    
    /* Final asm to ensure all paths are used */
    asm volatile ("# Final computation"
                  : "+r" (reg_var_1), "+r" (reg_var_2)
                  : "r" (reg_var_3), "x" (reg_fvar_1), "x" (reg_fvar_2)
                  : "memory");
    
    printf("Result: %d (checksum: %d)\n", reg_var_1 + reg_var_2, final_checksum);
    
    return 0;
}

/* Dummy definitions for external functions to allow linking */
void opaque_call_1(void) {
    /* Empty but volatile to prevent optimization */
    asm volatile ("# opaque_call_1" : : : "memory");
}

void opaque_call_2(int x) {
    global_counter += x;
    asm volatile ("# opaque_call_2" : : : "memory");
}

int opaque_call_3(int a, int b) {
    int result = a + b;
    asm volatile ("# opaque_call_3" : "+r" (result) : : "memory");
    return result;
}

double opaque_call_4(double a, double b) {
    double result = a * b;
    asm volatile ("# opaque_call_4" : "+x" (result) : : "memory");
    return result;
}
