/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -O1 -fomit-frame-pointer -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to force calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void opaque_func4(int, int, int, int, int, int);

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile long global_accumulator = 0;
volatile double global_fp = 3.14159;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
func_ptr_t func_table[4];

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many live variables in call-clobbered registers */
    register int r10_val asm("r10") = mode * 2;
    register int r11_val asm("r11") = mode * 3;
    volatile int stack_slot1 = mode + 1;
    volatile int stack_slot2 = mode + 2;
    int local1, local2, local3, local4, local5;
    
    /* Use inline asm to clobber specific registers */
    asm volatile("" : : "r"(r10_val), "r"(r11_val) : "r10", "r11", "memory");
    
    /* Create register pressure */
    local1 = stack_slot1 * 2;
    local2 = stack_slot2 * 3;
    local3 = local1 + local2;
    
    /* Function call with many arguments - forces caller-save */
    opaque_func4(local1, local2, local3, r10_val, r11_val, mode);
    
    /* Use values after call - they need to be restored */
    local4 = local1 + r10_val;
    local5 = local2 + r11_val;
    
    /* Another asm barrier */
    asm volatile("" : : "r"(local4), "r"(local5) : "memory");
    
    /* Store to global to prevent elimination */
    global_counter += local4 + local5;
}

/* Function with floating point and mixed types */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double v1 = base;
    volatile double v2 = base * 2.0;
    volatile double v3 = base * 3.0;
    register double fp1 asm("xmm0") = v1;
    register double fp2 asm("xmm1") = v2;
    int int1, int2, int3;
    
    /* Mix FP and integer computations */
    int1 = (int)v1;
    int2 = (int)v2;
    int3 = (int)v3;
    
    /* Call that clobbers FP registers */
    double result = opaque_func3(fp1);
    
    /* Complex use of values across call */
    fp2 = fp2 + result;
    int1 = int1 + (int)fp2;
    
    /* Indirect call via function pointer */
    if (global_counter & 1) {
        func_table[0]();
    } else {
        func_table[1]();
    }
    
    /* More computations after call */
    v3 = v3 * fp2;
    global_fp += v3;
    
    /* Memory barrier */
    asm volatile("" : : "r"(int1), "x"(fp2) : "memory", "xmm0", "xmm1");
}

/* Function with irreducible control flow */
__attribute__((noinline, noclone))
void test3(int iterations) {
    int i = 0;
    volatile int arr[10] = {0};
    
    /* Complex loop with goto creating irreducible flow */
    start_loop:
    if (i >= iterations) goto end_loop;
    
    /* Function call inside loop */
    opaque_func1();
    
    /* Store to array - forces spills */
    arr[i % 10] = i * 2;
    
    /* Conditional goto creating merge point */
    if (i & 1) {
        goto odd_case;
    } else {
        goto even_case;
    }
    
    odd_case:
    {
        int temp = arr[(i + 1) % 10];
        /* Another call at merge point */
        arr[i % 10] = opaque_func2(temp);
        goto loop_continue;
    }
    
    even_case:
    {
        int temp = arr[(i + 2) % 10];
        /* Different call at merge point */
        arr[i % 10] = opaque_func2(temp * 2);
        goto loop_continue;
    }
    
    loop_continue:
    i++;
    
    /* Nested condition with call */
    if (i % 3 == 0) {
        opaque_func1();
        /* Force register pressure */
        register int eax_val asm("eax") = i;
        asm volatile("" : : "r"(eax_val) : "eax", "memory");
    }
    
    goto start_loop;
    
    end_loop:
    /* Compute checksum */
    for (int j = 0; j < 10; j++) {
        global_accumulator += arr[j];
    }
}

/* Function with switch statement and calls in cases */
__attribute__((noinline, noclone))
void test4(int selector) {
    volatile int case_var = selector;
    register int r12_val asm("r12") = selector * 100;
    
    /* Switch with calls in multiple cases */
    switch (case_var % 4) {
        case 0:
            opaque_func1();
            r12_val += 1;
            /* Fall through */
        case 1:
            opaque_func2(r12_val);
            r12_val += 2;
            break;
        case 2:
            opaque_func3(r12_val);
            r12_val += 3;
            /* Complex expression requiring temp register */
            {
                int temp = r12_val * 2;
                asm volatile("" : "+r"(temp) : : "memory");
                r12_val = temp + opaque_func2(temp);
            }
            break;
        default:
            /* Multiple calls in default case */
            opaque_func4(r12_val, r12_val+1, r12_val+2, 
                        r12_val+3, r12_val+4, r12_val+5);
            r12_val += 4;
            break;
    }
    
    /* Use value after switch */
    global_counter += r12_val;
    asm volatile("" : : "r"(r12_val) : "r12", "memory");
}

/* Helper with nested calls */
__attribute__((noinline, noclone))
int nested_helper(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    volatile int saved = value;
    register int rbx_val asm("rbx") = value * depth;
    
    /* Call within nested function */
    int result = opaque_func2(saved);
    
    /* Recursive call */
    int nested = nested_helper(depth - 1, result + rbx_val);
    
    /* Use both values after calls */
    return nested + result + saved;
}

/* Function that uses __builtin_apply */
__attribute__((noinline, noclone))
void test5_apply(void) {
    /* Create argument frame for __builtin_apply */
    void *args = __builtin_apply_args();
    
    /* Force register pressure around apply */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6;
    register int r13_val asm("r13") = v1 + v2 + v3;
    
    /* Call using apply - creates unusual register usage */
    void *ret = __builtin_apply((void (*)())opaque_func4, args, 48);
    
    /* Use values after apply */
    r13_val += v4 + v5 + v6;
    global_accumulator += r13_val;
    
    if (ret) {
        __builtin_return(ret);
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize function pointers to external functions */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
    
    /* Use argv to select mode but ensure all paths are taken */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Execute all test functions in different orders based on mode */
    switch (mode) {
        case 0:
            test1(1);
            test2(2.0);
            test3(3);
            test4(4);
            break;
        case 1:
            test4(1);
            test3(2);
            test2(3.0);
            test1(4);
            break;
        case 2:
            test2(1.0);
            test1(2);
            test4(3);
            test3(4);
            break;
        case 3:
            test3(1);
            test4(2);
            test1(3);
            test2(4.0);
            break;
        case 4:
            /* Include the apply test */
            test1(1);
            test5_apply();
            test3(2);
            test4(3);
            break;
    }
    
    /* Force execution of nested helper with register pressure */
    int nested_result = nested_helper(2, global_counter);
    global_accumulator += nested_result;
    
    /* Create irreducible control flow with labels and goto */
    volatile int flag = global_counter & 1;
    
    if (flag) {
        goto label_a;
    } else {
        goto label_b;
    }
    
    label_a:
    {
        /* Call at label with register usage */
        register int r14_val asm("r14") = 0x1234;
        opaque_func2(r14_val);
        asm volatile("" : : "r"(r14_val) : "r14", "memory");
        goto label_merge;
    }
    
    label_b:
    {
        /* Different call at other label */
        register int r15_val asm("r15") = 0x5678;
        opaque_func3(r15_val);
        asm volatile("" : : "r"(r15_val) : "r15", "memory");
        goto label_merge;
    }
    
    label_merge:
    /* Merge point - both paths join here */
    volatile int merge_var = global_accumulator;
    
    /* Final computation using all globals */
    long final_result = global_counter + global_accumulator + (long)global_fp + merge_var;
    
    /* Print to prevent elimination */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate library) */
void opaque_func1(void) {
    /* Empty but prevents optimization */
    asm volatile("" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile("" : "+r"(x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    double result;
    asm volatile("" : "=x"(result) : "x"(x) : "memory");
    return result * 2.0;
}

void opaque_func4(int a, int b, int c, int d, int e, int f) {
    /* Use all arguments to prevent optimization */
    volatile int sum = a + b + c + d + e + f;
    asm volatile("" : : "r"(sum) : "memory");
}
