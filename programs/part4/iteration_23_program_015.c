/* caller-save-test.c - Complex test to trigger uncovered instruction chain manipulation in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining and create call sites */
void __attribute__((noinline, noclone)) opaque_func1(int a, int b, int c, int d, int e, int f);
void __attribute__((noinline, noclone)) opaque_func2(double a, double b, double c, double d);
void __attribute__((noinline, noclone)) opaque_func3(void *ptr, size_t len);
int __attribute__((noinline, noclone)) opaque_func4(int a, int b, int c, int d, int e, int f, int g, int h);

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 42;
volatile double g_volatile_double = 3.14159;
volatile void *g_volatile_ptr = NULL;

/* Function pointer with volatile to force indirect calls */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_func_ptr = NULL;

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    void *ptr;
};

/* Test 1: Many integer arguments with live variables across call */
void __attribute__((noinline, noclone)) test1(int mode) {
    /* Force many live variables in call-clobbered registers */
    register int r10_val asm ("r10") = g_volatile_int + 1;
    register int r11_val asm ("r11") = g_volatile_int + 2;
    register int r12_val asm ("r12") = g_volatile_int + 3;
    register int r13_val asm ("r13") = g_volatile_int + 4;
    int local1 = g_volatile_int * 2;
    int local2 = g_volatile_int * 3;
    int local3 = g_volatile_int * 4;
    int local4 = g_volatile_int * 5;
    
    /* Volatile array to force stack usage */
    volatile int save_area[8];
    save_area[0] = r10_val;
    save_area[1] = r11_val;
    save_area[2] = r12_val;
    save_area[3] = r13_val;
    save_area[4] = local1;
    save_area[5] = local2;
    save_area[6] = local3;
    save_area[7] = local4;
    
    /* Compiler barrier */
    asm volatile ("" : : : "memory");
    
    /* Function call that clobbers registers */
    opaque_func1(local1, local2, local3, local4, r10_val, r11_val);
    
    /* Inline asm that clobbers specific registers */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%ecx\n\t"
        "movl $0x11111111, %%edx\n\t"
        : : : "eax", "ecx", "edx", "memory"
    );
    
    /* Use saved values after call - forces reload to insert restore code */
    int sum = save_area[0] + save_area[1] + save_area[2] + save_area[3] +
              save_area[4] + save_area[5] + save_area[6] + save_area[7];
    
    /* Complex control flow with goto to create basic block boundaries */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Another call site with different register usage */
    opaque_func2(g_volatile_double, g_volatile_double * 2, 
                 g_volatile_double * 3, g_volatile_double * 4);
    sum += 100;
    
label2:
    /* Use sum in volatile asm to prevent elimination */
    asm volatile ("" : "+r" (sum) : : "memory");
    g_volatile_int = sum;
}

/* Test 2: Floating point and mixed arguments */
void __attribute__((noinline, noclone)) test2(int mode) {
    volatile double d1 = g_volatile_double;
    volatile double d2 = d1 * 2.0;
    volatile double d3 = d1 * 3.0;
    volatile double d4 = d1 * 4.0;
    volatile double d5 = d1 * 5.0;
    
    int i1 = g_volatile_int;
    int i2 = i1 * 2;
    int i3 = i1 * 3;
    
    /* Save to volatile array across call */
    volatile double save_d[5];
    save_d[0] = d1;
    save_d[1] = d2;
    save_d[2] = d3;
    save_d[3] = d4;
    save_d[4] = d5;
    
    /* Switch statement to create complex CFG */
    switch (mode % 4) {
        case 0:
            opaque_func2(d1, d2, d3, d4);
            break;
        case 1:
            opaque_func1(i1, i2, i3, i1, i2, i3);
            break;
        case 2:
            /* Nested call scenario */
            {
                int temp = opaque_func4(i1, i2, i3, i1, i2, i3, i1, i2);
                asm volatile ("" : : "r" (temp) : "memory");
            }
            break;
        default:
            /* Call via volatile function pointer */
            if (volatile_func_ptr) {
                volatile_func_ptr();
            }
            break;
    }
    
    /* Use saved values - forces register restore */
    double result = save_d[0] + save_d[1] + save_d[2] + save_d[3] + save_d[4];
    
    /* Loop with break to create block boundaries */
    for (int i = 0; i < 10; i++) {
        if (i > mode) {
            /* Call inside loop with break */
            opaque_func3(&g_volatile_int, sizeof(g_volatile_int));
            break;
        }
        result += i;
    }
    
    g_volatile_double = result;
}

/* Test 3: Vector-like operations and many live ranges */
void __attribute__((noinline, noclone)) test3(int mode) {
    struct LargeStruct ls;
    ls.a = g_volatile_int;
    ls.b = ls.a + 1;
    ls.c = ls.b + 1;
    ls.d = ls.c + 1;
    ls.e = ls.d + 1;
    ls.f = ls.e + 1;
    ls.g = ls.f + 1;
    ls.h = ls.g + 1;
    ls.x = g_volatile_double;
    ls.y = ls.x * 2.0;
    ls.z = ls.x * 3.0;
    ls.ptr = &g_volatile_int;
    
    /* Many local variables to increase register pressure */
    int v1 = ls.a, v2 = ls.b, v3 = ls.c, v4 = ls.d;
    int v5 = ls.e, v6 = ls.f, v7 = ls.g, v8 = ls.h;
    double d1 = ls.x, d2 = ls.y, d3 = ls.z;
    
    /* Save all to volatile storage */
    volatile int save_int[8];
    volatile double save_dbl[3];
    save_int[0] = v1; save_int[1] = v2; save_int[2] = v3; save_int[3] = v4;
    save_int[4] = v5; save_int[5] = v6; save_int[6] = v7; save_int[7] = v8;
    save_dbl[0] = d1; save_dbl[1] = d2; save_dbl[2] = d3;
    
    /* Irreducible control flow with goto */
    if (mode & 1) {
        goto middle;
    }
    
    /* Function call that uses many arguments */
    int complex_result = opaque_func4(v1, v2, v3, v4, v5, v6, v7, v8);
    
    /* Inline asm with many clobbers */
    asm volatile (
        "movq $0x123456789ABCDEF0, %%rax\n\t"
        "movq $0xFEDCBA9876543210, %%r10\n\t"
        "movq $0x1111111111111111, %%r11\n\t"
        "movq $0x2222222222222222, %%r12\n\t"
        : : : "rax", "r10", "r11", "r12", "memory"
    );
    
middle:
    /* Another call site */
    opaque_func3(ls.ptr, sizeof(ls));
    
    /* Restore and use all saved values */
    int int_sum = 0;
    for (int i = 0; i < 8; i++) {
        int_sum += save_int[i];
    }
    
    double dbl_sum = 0.0;
    for (int i = 0; i < 3; i++) {
        dbl_sum += save_dbl[i];
    }
    
    /* Complex expression requiring temporary registers */
    g_volatile_int = int_sum + (int)dbl_sum + complex_result;
}

/* Test 4: __builtin_apply and variable arguments simulation */
void __attribute__((noinline, noclone)) test4(int mode) {
    /* Use __builtin_apply to create unusual call sequences */
    void *args;
    int arg_array[6] = {1, 2, 3, 4, 5, 6};
    
    /* Force register pressure before builtin */
    register int r10_temp asm ("r10") = g_volatile_int + 100;
    register int r11_temp asm ("r11") = g_volatile_int + 200;
    int local_vars[4] = {r10_temp, r11_temp, r10_temp * 2, r11_temp * 2};
    
    volatile int save_regs[4];
    for (int i = 0; i < 4; i++) {
        save_regs[i] = local_vars[i];
    }
    
    /* This creates complex call sequences */
    if (mode & 1) {
        /* Simulate variable argument call */
        args = __builtin_apply_args();
        /* Note: __builtin_apply would need function pointer,
           using opaque_func1 instead for simplicity */
        opaque_func1(arg_array[0], arg_array[1], arg_array[2], 
                     arg_array[3], arg_array[4], arg_array[5]);
    }
    
    /* Use saved registers after call */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += save_regs[i];
    }
    
    /* Nested loops with function calls at boundaries */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i + j > mode) {
                opaque_func2(g_volatile_double, g_volatile_double * i,
                            g_volatile_double * j, g_volatile_double * (i + j));
                goto loop_exit;
            }
        }
    }
loop_exit:
    
    g_volatile_int = sum;
}

/* Helper with nested calls */
void __attribute__((noinline, noclone)) nested_call_helper(int depth, int *result) {
    volatile int save = *result;
    
    if (depth > 0) {
        /* Recursive-like nested call */
        opaque_func1(save, save + 1, save + 2, save + 3, save + 4, save + 5);
        
        /* Inner call */
        int inner = opaque_func4(save, save + 1, save + 2, save + 3,
                                save + 4, save + 5, save + 6, save + 7);
        
        /* Use result in complex expression */
        *result = save + inner + depth;
        
        /* Another call at different depth */
        if (depth > 1) {
            opaque_func2(g_volatile_double, g_volatile_double * 2,
                        g_volatile_double * 3, g_volatile_double * 4);
        }
    }
}

/* Test 5: Nested calls and complex live ranges */
void __attribute__((noinline, noclone)) test5(int mode) {
    int result = g_volatile_int;
    volatile int intermediate[4];
    
    /* Multiple calls with live variables between them */
    intermediate[0] = result;
    opaque_func1(result, result + 1, result + 2, result + 3, result + 4, result + 5);
    
    intermediate[1] = result * 2;
    nested_call_helper(mode % 3, &result);
    
    intermediate[2] = result * 3;
    opaque_func3(&result, sizeof(result));
    
    intermediate[3] = result * 4;
    
    /* Use all intermediate values */
    int final = 0;
    for (int i = 0; i < 4; i++) {
        final += intermediate[i];
    }
    
    /* Complex control flow with switch and default label */
    switch (mode % 5) {
        case 0: final += 1; break;
        case 1: final += 2; break;
        case 2: final += 3; break;
        case 3: final += 4; break;
        default:
            /* Call in default case */
            opaque_func2(final, final * 1.1, final * 1.2, final * 1.3);
            final += 5;
            break;
    }
    
    g_volatile_int = final;
}

/* Main function with mode selection */
int main(int argc, char *argv[]) {
    int mode = 0;
    
    /* Use argv to determine mode, preventing constant propagation */
    if (argc > 1) {
        mode = atoi(argv[1]);
    } else {
        mode = 12345; /* Default mode */
    }
    
    /* Initialize volatile function pointer */
    volatile_func_ptr = (func_ptr_t)opaque_func1;
    
    /* Run all tests with different modes to exercise different paths */
    test1(mode);
    test2(mode + 1);
    test3(mode + 2);
    test4(mode + 3);
    test5(mode + 4);
    
    /* Compute checksum of global state to prevent elimination */
    int checksum = g_volatile_int;
    checksum += (int)g_volatile_double;
    checksum += (long)g_volatile_ptr;
    
    /* Final opaque call */
    opaque_func3(&checksum, sizeof(checksum));
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker (these won't be called in practice) */
void opaque_func1(int a, int b, int c, int d, int e, int f) {
    /* Empty - just for call sites */
}

void opaque_func2(double a, double b, double c, double d) {
    /* Empty - just for call sites */
}

void opaque_func3(void *ptr, size_t len) {
    /* Empty - just for call sites */
}

int opaque_func4(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a + b + c + d + e + f + g + h;
}
