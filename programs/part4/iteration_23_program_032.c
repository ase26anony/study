/* test-caller-save.c - Complex program to trigger uncovered instruction chain manipulation in GCC's caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void* opaque_func4(void*);

/* Volatile globals to maintain live ranges across calls */
volatile int gv1 = 1, gv2 = 2, gv3 = 3, gv4 = 4;
volatile double gd1 = 1.0, gd2 = 2.0;
volatile void* gp1 = NULL;

/* Function pointer with volatile to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fp = NULL;

/* Complex control flow with register pressure */
__attribute__((noinline, noclone))
void test1(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Force many live variables across call */
    register int r1 asm ("r10") = a + b;
    register int r2 asm ("r11") = c + d;
    register int r3 asm ("r12") = e + f;
    register int r4 asm ("r13") = g + h;
    
    volatile int stack_save[8];
    stack_save[0] = r1;
    stack_save[1] = r2;
    stack_save[2] = r3;
    stack_save[3] = r4;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile ("# test1 asm clobber" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    /* Function call that forces caller-save */
    opaque_func1();
    
    /* Use saved values in complex expression */
    int result = (stack_save[0] * stack_save[1]) + 
                 (stack_save[2] / stack_save[3]) -
                 (r1 ^ r2 ^ r3 ^ r4);
    
    /* Force register pressure with another call */
    gv1 = opaque_func2(result);
    
    /* Compiler barrier */
    asm volatile ("" : : : "memory");
}

/* Floating point intensive with mixed calls */
__attribute__((noinline, noclone))
void test2(double a, double b, double c, double d) {
    volatile double saved[4];
    saved[0] = a;
    saved[1] = b;
    saved[2] = c;
    saved[3] = d;
    
    /* Complex floating point computation */
    double temp1 = a * b + c / d;
    double temp2 = b * c - a / d;
    
    /* Call that clobbers floating point registers */
    double result = opaque_func3(temp1);
    
    /* Use saved values after call */
    gd1 = saved[0] * result + saved[1];
    gd2 = saved[2] / result - saved[3];
    
    /* Inline asm acting as pseudo-call */
    asm volatile ("# FP clobber" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
}

/* Function with irreducible control flow */
__attribute__((noinline, noclone))
void test3(int n) {
    int i = 0;
    volatile int arr[10];
    
label1:
    if (i >= n) goto label4;
    
    arr[i] = i * gv1;
    
    if (i % 2 == 0) {
        /* Function call in one branch */
        opaque_func1();
        goto label2;
    } else {
        /* Different call in other branch */
        gv2 = opaque_func2(i);
        goto label3;
    }
    
label2:
    /* Use value saved before call */
    gv3 += arr[i];
    i++;
    goto label1;
    
label3:
    /* Another use path */
    gv4 ^= arr[i];
    i += 2;
    if (i < n) goto label1;
    
label4:
    /* Final call at block boundary */
    asm volatile ("# boundary call" : : : "memory");
    opaque_func1();
}

/* Function with switch and calls in default case */
__attribute__((noinline, noclone))
void test4(int mode) {
    volatile int saves[8];
    for (int i = 0; i < 8; i++) saves[i] = i + gv1;
    
    switch (mode) {
        case 1:
            gv1 = opaque_func2(1);
            break;
        case 2:
            gv2 = opaque_func2(2);
            break;
        case 3:
            gv3 = opaque_func2(3);
            break;
        default:
            /* Multiple calls in default case */
            opaque_func1();
            asm volatile ("# default case" : : : 
                "rax", "rbx", "rcx", "rdx");
            opaque_func1();
            /* Force register restore after calls */
            for (int i = 0; i < 8; i++) {
                gv4 += saves[i];
            }
            break;
    }
    
    /* Use saved values after switch */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += saves[i];
    }
    gv1 = sum;
}

/* Nested calls with register variables */
__attribute__((noinline, noclone))
void inner_nested(int x) {
    register int r asm ("r14") = x;
    asm volatile ("# inner" : "+r" (r) : : "memory");
    gv2 = r * 2;
}

__attribute__((noinline, noclone))
void outer_nested(int x) {
    register int a asm ("r15") = x;
    register int b asm ("r14") = x * 2;
    
    /* Save to volatile memory */
    volatile int save_a = a;
    volatile int save_b = b;
    
    /* Nested call */
    inner_nested(x);
    
    /* Use saved values after nested call */
    a = save_a + save_b;
    b = save_a - save_b;
    
    /* Another call */
    gv3 = opaque_func2(a + b);
    
    /* Restore and use */
    asm volatile ("# outer use %0, %1" : : "r" (a), "r" (b) : "memory");
}

/* Function using __builtin_apply for unusual calling convention */
__attribute__((noinline, noclone))
void test_builtin_apply(void) {
    /* Create artificial apply scenario */
    volatile int args[3] = {100, 200, 300};
    
    /* Simulate complex register usage around builtin */
    register int r1 asm ("r10") = args[0];
    register int r2 asm ("r11") = args[1];
    register int r3 asm ("r12") = args[2];
    
    asm volatile ("# before builtin" : : : "memory");
    
    /* Barrier to prevent optimization */
    void* dummy = __builtin_apply((void(*)(void))opaque_func1, 
                                  __builtin_apply_args(), 64);
    
    /* Use registers after builtin */
    args[0] = r1 + r2;
    args[1] = r2 + r3;
    args[2] = r3 + r1;
    
    gv4 = args[0] + args[1] + args[2];
}

/* Main test driver */
int main(int argc, char **argv) {
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize volatile function pointer */
    volatile_fp = opaque_func1;
    
    /* Force different execution paths based on input */
    switch (test_mode) {
        case 0:
            test1(1, 2, 3, 4, 5, 6, 7, 8);
            test2(1.1, 2.2, 3.3, 4.4);
            test3(10);
            break;
        case 1:
            test4(99);  /* Force default case */
            outer_nested(42);
            break;
        case 2:
            test_builtin_apply();
            for (int i = 0; i < 5; i++) {
                test1(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
            }
            break;
        case 3:
            /* Mix of all tests */
            test1(10, 20, 30, 40, 50, 60, 70, 80);
            test2(5.5, 6.6, 7.7, 8.8);
            test3(15);
            test4(100);
            break;
        case 4:
            /* Loop with nested calls */
            for (int i = 0; i < 3; i++) {
                outer_nested(i * 10);
                test4(i);
            }
            break;
    }
    
    /* Execute all tests to ensure coverage */
    if (test_mode != 0) {
        test1(1, 1, 1, 1, 1, 1, 1, 1);
        test2(1.0, 1.0, 1.0, 1.0);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = gv1 + gv2 + gv3 + gv4 + (int)gd1 + (int)gd2;
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_func1(void) {
    asm volatile ("# opaque1" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("# opaque2" : : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile ("# opaque3" : : : "memory");
    return x * 2.0;
}

void* opaque_func4(void* x) {
    asm volatile ("# opaque4" : : : "memory");
    return x;
}
