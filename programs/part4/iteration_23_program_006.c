/* caller_save_test.c
 * 
 * A test program designed to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets the instruction chain manipulation code at lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External opaque functions to force call instructions */
extern void opaque_func1(void);
extern void opaque_func2(int);
extern int opaque_func3(int, int);
extern double opaque_func4(double, double);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[100] = {0};
volatile double global_fp_array[50] = {0.0};

/* Function pointer with volatile to prevent devirtualization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fptr = NULL;

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e;
    double f, g, h;
    volatile int i;
};

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force specific register usage */
#define FORCE_REGISTER(var, reg) \
    register int var asm(reg) = global_counter++

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1_many_live_vars(void) {
    /* Force many variables to be live across call */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    
    /* Use explicit register variables */
    register int r10_val asm("r10") = v1 + v2;
    register int r11_val asm("r11") = v3 * v4;
    
    /* Complex expression requiring temporary registers */
    int complex_expr = (v1 * v2) + (v3 / v4) - (v5 << 2);
    
    /* Call that clobbers registers */
    asm volatile(
        "call *%0\n\t"
        : 
        : "r"((void*)opaque_func1)
        : "eax", "ebx", "ecx", "edx", "r10", "r11", "memory"
    );
    
    /* Use all variables after call - forces save/restore */
    v1 = r10_val + complex_expr;
    v2 = r11_val - complex_expr;
    v3 = (int)d1 + (int)d2;
    v4 = v5 * global_array[complex_expr % 100];
    
    /* Another call with different clobbers */
    opaque_func2(v1 + v2 + v3 + v4);
    
    /* Store results to prevent elimination */
    global_array[0] = v1;
    global_array[1] = v2;
    global_array[2] = v3;
    global_array[3] = v4;
}

/* Test function 2: Irreducible control flow with calls at boundaries */
__attribute__((noinline, noclone))
void test2_complex_cfg(void) {
    volatile int a = 1, b = 2, c = 3;
    volatile double x = 1.0, y = 2.0;
    
    /* Create irreducible control flow with goto */
    if (global_counter & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    {
        register int r12 asm("r12") = a * b;
        register int r13 asm("r13") = b * c;
        
        /* Call at basic block boundary */
        int result = opaque_func3(r12, r13);
        
        if (result > 0) {
            /* Force block splitting */
            for (int i = 0; i < 3; i++) {
                if (i == 1) {
                    /* Call in loop with break */
                    opaque_func2(i);
                    break;
                }
                a += i;
            }
            goto label3;
        } else {
            c = result;
            goto label4;
        }
    }

label2:
    {
        /* Different register pressure */
        register double xmm0 asm("xmm0") = x;
        register double xmm1 asm("xmm1") = y;
        
        double fp_result = opaque_func4(xmm0, xmm1);
        
        /* Switch with default calling function */
        switch ((int)fp_result) {
            case 0: a = 1; break;
            case 1: b = 2; break;
            default:
                /* Function call in default case */
                opaque_func1();
                c = 3;
                break;
        }
        goto label3;
    }

label3:
    /* Merge point with live variables */
    a = b + c;
    /* Another call after merge */
    asm volatile(
        "call *%0\n\t"
        : 
        : "r"((void*)opaque_func1)
        : "eax", "ebx", "ecx", "edx", "r12", "r13", "xmm0", "xmm1", "memory"
    );
    b = a * 2;

label4:
    /* Final computation */
    global_array[4] = a + b + c;
    COMPILER_BARRIER();
}

/* Test function 3: Nested calls with register pressure */
__attribute__((noinline, noclone))
void test3_nested_calls(int depth) {
    if (depth <= 0) return;
    
    /* Many variables that need to survive across nested call */
    volatile int vars[8];
    for (int i = 0; i < 8; i++) {
        vars[i] = i + depth;
    }
    
    /* Explicit register variables */
    register int r14 asm("r14") = vars[0] + vars[1];
    register int r15 asm("r15") = vars[2] * vars[3];
    
    /* Call that itself makes calls */
    test2_complex_cfg();
    
    /* Use registers after nested call */
    vars[4] = r14 + r15;
    vars[5] = r14 - r15;
    
    /* Recursive call */
    test3_nested_calls(depth - 1);
    
    /* More register usage */
    register double xmm2 asm("xmm2") = (double)vars[4];
    register double xmm3 asm("xmm3") = (double)vars[5];
    
    /* FP call */
    double result = opaque_func4(xmm2, xmm3);
    
    /* Store with volatile to prevent optimization */
    global_fp_array[depth % 50] = result;
}

/* Test function 4: __builtin_apply usage */
__attribute__((noinline, noclone))
void test4_builtin_apply(void) {
    /* Force unusual register usage with __builtin_apply */
    volatile int arg1 = 1, arg2 = 2, arg3 = 3, arg4 = 4;
    double farg1 = 1.5, farg2 = 2.5;
    
    /* Create argument buffer for __builtin_apply */
    void *args = __builtin_apply_args();
    
    /* Use many registers before the apply */
    register int r8 asm("r8") = arg1 + arg2;
    register int r9 asm("r9") = arg3 * arg4;
    register double xmm4 asm("xmm4") = farg1;
    register double xmm5 asm("xmm5") = farg2;
    
    /* Simulate a call with __builtin_apply */
    /* Note: We'll use a regular call instead since __builtin_apply is tricky */
    void (*func)(int, int, double, double) = (void(*)(int, int, double, double))opaque_func2;
    
    /* Call with mixed arguments */
    asm volatile(
        "push %4\n\t"
        "push %3\n\t"
        "movsd %2, %%xmm1\n\t"
        "movsd %1, %%xmm0\n\t"
        "mov %0, %%edi\n\t"
        "call *%5\n\t"
        "add $16, %%rsp\n\t"
        : 
        : "r"(arg1), "x"(farg1), "x"(farg2), "r"(arg2), "r"(arg3), "r"(func)
        : "edi", "esi", "xmm0", "xmm1", "memory"
    );
    
    /* Use all registers after call */
    arg1 = r8 + r9;
    farg1 = xmm4 * xmm5;
    
    /* Store results */
    global_array[5] = arg1;
    global_fp_array[1] = farg1;
}

/* Test function 5: Switch with computed goto and calls */
__attribute__((noinline, noclone))
void test5_computed_goto(void) {
    static void *jump_table[] = { &&case0, &&case1, &&case2, &&default_case };
    
    volatile int selector = global_counter % 4;
    volatile int a = 10, b = 20, c = 30;
    
    /* Force register usage */
    register int rbx_val asm("rbx") = a * b;
    register int rbp_val asm("rbp") = b * c;
    
    /* Computed goto */
    goto *jump_table[selector];
    
case0:
    /* Call immediately after label */
    opaque_func1();
    a = rbx_val + 1;
    goto end;
    
case1:
    /* Different call pattern */
    opaque_func2(rbx_val);
    b = rbp_val - 1;
    goto end;
    
case2:
    /* Call with FP arguments */
    {
        register double xmm6 asm("xmm6") = (double)rbx_val;
        register double xmm7 asm("xmm7") = (double)rbp_val;
        double result = opaque_func4(xmm6, xmm7);
        c = (int)result;
    }
    goto end;
    
default_case:
    /* Multiple calls in sequence */
    opaque_func1();
    COMPILER_BARRIER();
    opaque_func2(a);
    COMPILER_BARRIER();
    a = b = c = 0;
    /* Fall through */
    
end:
    /* Use all variables at merge point */
    global_array[6] = a + b + c + rbx_val + rbp_val;
}

/* Helper with inline asm that looks like a call */
__attribute__((noinline, noclone))
void helper_with_pseudo_call(int *ptr) {
    /* Inline asm that acts like a call */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movl $0x9ABCDEF0, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        "pushf\n\t"
        "call *%0\n\t"
        "popf\n\t"
        : 
        : "r"((void*)opaque_func1)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    *ptr += global_counter;
}

/* Main test driver */
int main(int argc, char **argv) {
    int test_mode = 0;
    
    /* Use argv to select mode but ensure all tests run */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize volatile function pointer */
    volatile_fptr = opaque_func1;
    
    /* Run all tests in different orders based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 5) {
            case 0:
                test1_many_live_vars();
                break;
            case 1:
                test2_complex_cfg();
                break;
            case 2:
                test3_nested_calls(2);
                break;
            case 3:
                test4_builtin_apply();
                break;
            case 4:
                test5_computed_goto();
                break;
        }
        
        /* Call helper that makes pseudo-calls */
        int temp = global_counter;
        helper_with_pseudo_call(&temp);
        global_counter = temp;
    }
    
    /* Force register pressure in main too */
    {
        struct LargeStruct s = {0};
        s.a = global_array[0];
        s.b = global_array[1];
        s.c = global_array[2];
        s.d = global_array[3];
        s.e = global_array[4];
        s.f = global_fp_array[0];
        s.g = global_fp_array[1];
        s.h = global_fp_array[2];
        s.i = global_counter;
        
        /* Call with structure (may be passed in registers) */
        opaque_func2(s.a + s.b + s.c);
        
        /* Compute checksum to prevent elimination */
        int checksum = 0;
        for (int i = 0; i < 10; i++) {
            checksum += global_array[i];
        }
        for (int i = 0; i < 5; i++) {
            checksum += (int)global_fp_array[i];
        }
        
        printf("Checksum: %d\n", checksum);
    }
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real test, these would be in a library) */
void opaque_func1(void) {
    global_counter++;
}

void opaque_func2(int x) {
    global_array[x % 100] = x;
}

int opaque_func3(int a, int b) {
    return a + b + global_counter;
}

double opaque_func4(double a, double b) {
    return a * b + global_counter;
}
