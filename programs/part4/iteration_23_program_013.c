/* caller-save-test.c
 * Test program to trigger uncovered code in GCC's caller-save.cc
 * Specifically targets lines 905-913 that handle instruction chain manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void opaque_func4(int, ...);

/* Global volatile variables to prevent optimization */
volatile int gv1 = 12345;
volatile int gv2 = 67890;
volatile double gv3 = 3.14159;
volatile long gv4 = 999999999;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[4];

/* Complex structure to force register pressure */
struct RegPressure {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    long l1, l2;
};

/* Force non-inline, no cloning */
__attribute__((noinline, noclone))
void test1_many_registers(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int local1 = a;
    volatile int local2 = b;
    volatile int local3 = c;
    volatile int local4 = d;
    volatile int local5 = e;
    volatile int local6 = f;
    volatile int local7 = g;
    volatile int local8 = h;
    
    /* Use explicit register variables to create conflicts */
    register int r10_val asm ("r10") = local1 + local2;
    register int r11_val asm ("r11") = local3 + local4;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ecx\n\t"
        "addl %%ecx, %%eax\n\t"
        : /* no outputs */
        : "r" (r10_val), "r" (r11_val)
        : "eax", "ecx", "memory"
    );
    
    /* Function call that will clobber registers */
    opaque_func1();
    
    /* Use values after call - forces save/restore */
    int sum = r10_val + r11_val + local5 + local6 + local7 + local8;
    
    /* Another asm barrier */
    asm volatile ("" : : : "memory");
    
    /* Store result to global to prevent elimination */
    gv1 = sum;
}

__attribute__((noinline, noclone))
void test2_float_registers(float f1, float f2, double d1, double d2) {
    volatile float vf1 = f1;
    volatile float vf2 = f2;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    
    /* Mix integer and floating point */
    register double xmm0_val asm ("xmm0") = vd1 * 2.0;
    register double xmm1_val asm ("xmm1") = vd2 * 3.0;
    
    /* Complex expression requiring multiple registers */
    double result = (xmm0_val + xmm1_val) * (vf1 + vf2);
    
    /* Call that clobbers floating point registers */
    double ret = opaque_func3(result);
    
    /* Use values across call boundary */
    result = xmm0_val + xmm1_val + ret;
    
    /* Force spill/reload with inline asm */
    asm volatile (
        "movsd %0, %%xmm2\n\t"
        "addsd %1, %%xmm2\n\t"
        : /* no outputs */
        : "x" (xmm0_val), "x" (xmm1_val)
        : "xmm2", "memory"
    );
    
    gv3 = result;
}

__attribute__((noinline, noclone))
int test3_control_flow(int n) {
    volatile int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * n;
    }
    
    /* Complex control flow with calls at boundaries */
    int sum = 0;
    
    switch (n % 4) {
        case 0:
            sum += arr[0];
            opaque_func1();
            /* Fall through - creates edge case for BB_END update */
        case 1: {
            register int eax_val asm ("eax") = arr[1] + arr[2];
            int temp = opaque_func2(eax_val);
            sum += temp;
            if (temp > 0) {
                goto label1;
            }
            break;
        }
        case 2:
            sum += arr[3];
            /* Nested call in loop */
            for (int j = 0; j < 3; j++) {
                if (j == 1) {
                    opaque_func1();
                    continue;
                }
                sum += arr[j];
            }
            break;
        case 3:
        default:
            sum += arr[4];
            opaque_func4(1, 2, 3, 4);
            break;
    }
    
label1:
    /* Use goto to create irreducible flow */
    if (sum < 0) {
        goto early_exit;
    }
    
    /* Another call after label */
    opaque_func2(sum);
    
    return sum;

early_exit:
    return -1;
}

__attribute__((noinline, noclone))
void test4_va_args_and_builtins(int count, ...) {
    va_list args;
    va_start(args, count);
    
    volatile int vals[8];
    for (int i = 0; i < count && i < 8; i++) {
        vals[i] = va_arg(args, int);
    }
    va_end(args);
    
    /* Use __builtin_apply to force unusual register usage */
    void* arg_buf = __builtin_apply_args();
    
    /* Register pressure with many live variables */
    register int r8_val asm ("r8") = vals[0];
    register int r9_val asm ("r9") = vals[1];
    register int r10_val asm ("r10") = vals[2];
    register int r11_val asm ("r11") = vals[3];
    register int r12_val asm ("r12") = vals[4];
    register int r13_val asm ("r13") = vals[5];
    
    /* Call with register arguments */
    asm volatile (
        "call *%0\n\t"
        : /* no outputs */
        : "r" (func_table[0])
        : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
    
    /* Use all register variables after call */
    int total = r8_val + r9_val + r10_val + r11_val + r12_val + r13_val;
    
    /* Force another call with different clobbers */
    opaque_func4(total, vals[6], vals[7]);
    
    gv2 = total;
}

__attribute__((noinline, noclone))
void test5_nested_calls_and_loops(int iterations) {
    struct RegPressure pressure;
    pressure.a = 1; pressure.b = 2; pressure.c = 3; pressure.d = 4;
    pressure.e = 5; pressure.f = 6; pressure.g = 7; pressure.h = 8;
    pressure.x = 1.1; pressure.y = 2.2; pressure.z = 3.3;
    pressure.l1 = 100; pressure.l2 = 200;
    
    volatile int counter = 0;
    
    /* Loop with nested calls - creates complex BB structure */
    for (int i = 0; i < iterations; i++) {
        /* Save live values before call */
        int saved_a = pressure.a;
        int saved_b = pressure.b;
        double saved_x = pressure.x;
        
        /* Call that clobbers registers */
        if (i % 2 == 0) {
            opaque_func1();
        } else {
            opaque_func2(i);
        }
        
        /* Restore and use saved values */
        pressure.a = saved_a + 1;
        pressure.b = saved_b + pressure.a;
        pressure.x = saved_x * 2.0;
        
        /* Break in middle of loop with call */
        if (i == iterations / 2) {
            opaque_func3(pressure.x);
            break;
        }
        
        /* Continue with another call */
        if (i % 3 == 0) {
            continue;
        }
        
        counter++;
    }
    
    /* Final computation using all struct members */
    long result = pressure.a + pressure.b + pressure.c + pressure.d +
                  pressure.e + pressure.f + pressure.g + pressure.h +
                  (long)pressure.x + (long)pressure.y + (long)pressure.z +
                  pressure.l1 + pressure.l2;
    
    gv4 = result;
}

/* Helper with nested call to force instruction insertion at BB boundaries */
__attribute__((noinline, noclone))
int helper_with_inner_call(int x) {
    volatile int a = x;
    volatile int b = x * 2;
    
    /* Inner call */
    int inner = opaque_func2(a);
    
    /* Complex expression requiring temporary */
    int result = (a + b) * inner - (a * b) / inner + (inner << 2);
    
    /* Jump to label creating BB boundary */
    if (result > 1000) {
        goto large_result;
    }
    
    return result;
    
large_result:
    /* Another call at BB end */
    opaque_func1();
    return result / 2;
}

__attribute__((noinline, noclone))
void test6_bb_boundary_manipulation(int val) {
    /* This test specifically aims to trigger BB_END updates */
    volatile int x = val;
    volatile int y = val + 1;
    volatile int z = val + 2;
    
    /* Multiple basic blocks with calls at ends */
    if (x > 0) {
        register int rbx_val asm ("rbx") = x + y;
        opaque_func1();
        x = rbx_val;  /* Use after call */
        
        if (y > 10) {
            opaque_func2(y);
            goto merge_point;
        } else {
            opaque_func3((double)y);
        }
    } else {
        opaque_func4(x, y, z);
    }
    
merge_point:
    /* Instruction that should be at BB_END before potential insertion */
    z = x + y;
    
    /* Call exactly at what might be BB_END */
    int ret = opaque_func2(z);
    
    /* Force instruction insertion after call */
    x = ret + helper_with_inner_call(ret);
    
    gv1 = x + y + z;
}

int main(int argc, char *argv[]) {
    /* Initialize function pointers */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)(void*)opaque_func2;
    func_table[2] = (func_ptr_t)(void*)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
    
    /* Use argv to select mode but run all tests */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Run all test functions with different parameters */
    test1_many_registers(1, 2, 3, 4, 5, 6, 7, 8);
    
    test2_float_registers(1.0f, 2.0f, 3.0, 4.0);
    
    int cf_result = test3_control_flow(mode + 1);
    
    test4_va_args_and_builtins(6, 10, 20, 30, 40, 50, 60);
    
    test5_nested_calls_and_loops(5 + mode);
    
    test6_bb_boundary_manipulation(cf_result);
    
    /* Compute checksum to prevent elimination */
    long checksum = gv1 + gv2 + (long)gv3 + gv4 + cf_result;
    
    /* Use asm to prevent reordering */
    asm volatile ("" : : "r" (checksum) : "memory");
    
    printf("Result: %ld\n", checksum);
    
    return (int)(checksum % 256);
}

/* Dummy definitions to satisfy linker if not linking with external lib */
#ifdef COMPILE_STANDALONE
void opaque_func1(void) {
    asm volatile ("" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("" : "+r" (x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile ("" : "+x" (x) : : "memory");
    return x * 2.0;
}

void opaque_func4(int a, ...) {
    volatile int dummy = a;
    asm volatile ("" : : "r" (dummy) : "memory");
}
#endif
