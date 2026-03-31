/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern int opaque_call_2(int);
extern double opaque_call_3(double);
extern void opaque_call_4(int, ...);

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 12345;
volatile double global_volatile_double = 3.14159;
volatile long global_volatile_long = 999999999L;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile func_ptrs[4];

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with explicit register variables */
register int reg_var_1 asm("r10");
register int reg_var_2 asm("r11");
register int reg_var_3 asm("r12");
register int reg_var_4 asm("r13");
register int reg_var_5 asm("r14");
register int reg_var_6 asm("r15");

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
int test1(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live values that must be saved across calls */
    volatile int v1 = a + b;
    volatile int v2 = c + d;
    volatile int v3 = e + f;
    volatile int v4 = g + h;
    
    /* Use explicit register variables */
    reg_var_1 = v1 * 2;
    reg_var_2 = v2 * 3;
    reg_var_3 = v3 * 4;
    reg_var_4 = v4 * 5;
    
    /* Force register pressure */
    int r1 = reg_var_1;
    int r2 = reg_var_2;
    int r3 = reg_var_3;
    int r4 = reg_var_4;
    
    /* Complex expression requiring temporary registers */
    int complex = (r1 * r2) + (r3 * r4) - (r1 / r3) + (r2 % r4);
    
    /* Call that clobbers registers */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use values after call - forces caller-save */
    int result = complex + v1 + v2 + v3 + v4;
    
    /* Another call with different clobbers */
    opaque_call_1();
    COMPILER_BARRIER();
    
    return result + reg_var_1 + reg_var_2;
}

/* Test function 2: Complex control flow with calls at block boundaries */
__attribute__((noinline, noclone))
int test2(int x) {
    volatile int arr[10];
    int i, sum = 0;
    
    /* Initialize array with volatile writes */
    for (i = 0; i < 10; i++) {
        arr[i] = x + i;
        COMPILER_BARRIER();
    }
    
    /* Complex control flow with goto */
    if (x > 100) {
        goto label1;
    } else if (x > 50) {
        goto label2;
    }
    
    /* Call in the middle of a basic block */
    sum += opaque_call_2(x);
    
    /* Jump to different labels creating block boundaries */
    if (sum % 2 == 0) {
        goto label3;
    }
    
label1:
    /* Call at block start */
    opaque_call_1();
    
    /* Use many registers */
    reg_var_5 = arr[0] + arr[1];
    reg_var_6 = arr[2] + arr[3];
    
    /* Inline asm that looks like a call */
    asm volatile(
        "call *%0\n\t"
        : : "r" (func_ptrs[0]) : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "memory"
    );
    
    if (reg_var_5 > reg_var_6) {
        goto label4;
    }
    
label2:
    /* Another call */
    sum += opaque_call_2(x * 2);
    
    /* Force register spill */
    int temp1 = reg_var_5 * reg_var_6;
    int temp2 = temp1 + arr[4];
    int temp3 = temp2 * arr[5];
    
    /* Switch statement to create complex CFG */
    switch (x % 4) {
        case 0:
            opaque_call_1();
            sum += temp1;
            break;
        case 1:
            sum += temp2;
            opaque_call_1();
            break;
        case 2:
            sum += temp3;
            /* Fall through */
        default:
            opaque_call_1();
            sum += x;
            break;
    }
    
label3:
    /* Use values across another call */
    int saved = reg_var_5 + reg_var_6;
    opaque_call_1();
    sum += saved;
    
label4:
    /* Final computation using all values */
    for (i = 0; i < 10; i++) {
        sum += arr[i];
    }
    
    return sum;
}

/* Test function 3: Floating point and mixed mode */
__attribute__((noinline, noclone))
double test3(double a, double b, double c, double d) {
    volatile double vd1 = a;
    volatile double vd2 = b;
    volatile double vd3 = c;
    volatile double vd4 = d;
    
    /* Mix integer and floating point */
    int vi1 = (int)a;
    int vi2 = (int)b;
    
    /* Force FP register pressure */
    double t1 = vd1 * vd2;
    double t2 = vd3 * vd4;
    double t3 = t1 + t2;
    double t4 = t1 - t2;
    
    /* Call that might clobber FP registers */
    asm volatile(
        "fld1\n\t"
        "fldpi\n\t"
        "fldln2\n\t"
        : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    
    /* Use values after FP clobber */
    double result = t3 * t4 + vd1 - vd2;
    
    /* Another call with varargs */
    opaque_call_4(4, vi1, vi2, (int)vd3, (int)vd4);
    
    /* Complex expression requiring temporaries */
    result += (vi1 * vi2) / (vi1 + 1);
    
    return result;
}

/* Test function 4: Nested calls and __builtin_apply */
__attribute__((noinline, noclone))
int test4(int depth) {
    if (depth <= 0) {
        return opaque_call_2(42);
    }
    
    /* Save many values across recursive call */
    volatile int saves[8];
    for (int i = 0; i < 8; i++) {
        saves[i] = depth * 10 + i;
    }
    
    /* Use explicit registers */
    reg_var_1 = saves[0];
    reg_var_2 = saves[1];
    reg_var_3 = saves[2];
    reg_var_4 = saves[3];
    
    /* Nested call - forces caller-save in middle of outer call's live range */
    int nested_result = test4(depth - 1);
    
    /* Use saved values after nested call */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += saves[i];
    }
    
    /* Complex use of register variables */
    sum += reg_var_1 * reg_var_2 - reg_var_3 / reg_var_4;
    
    /* Another call via function pointer */
    if (func_ptrs[1]) {
        func_ptrs[1]();
    }
    
    return sum + nested_result;
}

/* Helper with loop containing calls at block boundaries */
__attribute__((noinline, noclone))
int helper_with_complex_flow(int iterations) {
    int result = 0;
    volatile int counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Call at loop start */
        if (i % 3 == 0) {
            opaque_call_1();
        }
        
        /* Complex computation using global volatiles */
        result += global_volatile_int % (i + 1);
        counter++;
        
        /* Call in middle of loop body */
        if (i % 2 == 0) {
            result += opaque_call_2(i);
            /* Break creates block boundary */
            if (result > 1000) {
                break;
            }
        }
        
        /* Another computation */
        result += (int)global_volatile_double;
        
        /* Continue creates another boundary */
        if (i % 5 == 0) {
            continue;
        }
        
        /* Call at potential block end */
        result -= opaque_call_2(result);
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize function pointers (some may be NULL) */
    func_ptrs[0] = (func_ptr_t)opaque_call_1;
    func_ptrs[1] = (func_ptr_t)opaque_call_1;
    func_ptrs[2] = NULL;
    func_ptrs[3] = NULL;
    
    int total_result = 0;
    
    /* Run all tests in different orders based on mode */
    switch (test_mode) {
        case 0:
            total_result += test1(1, 2, 3, 4, 5, 6, 7, 8);
            total_result += test2(75);
            total_result += (int)test3(1.1, 2.2, 3.3, 4.4);
            total_result += test4(3);
            break;
        case 1:
            total_result += test2(150);
            total_result += (int)test3(5.5, 6.6, 7.7, 8.8);
            total_result += test4(2);
            total_result += test1(9, 10, 11, 12, 13, 14, 15, 16);
            break;
        case 2:
            total_result += test4(4);
            total_result += test1(17, 18, 19, 20, 21, 22, 23, 24);
            total_result += test2(25);
            total_result += (int)test3(9.9, 10.1, 11.1, 12.1);
            break;
        case 3:
            total_result += (int)test3(13.1, 14.1, 15.1, 16.1);
            total_result += test4(1);
            total_result += test2(200);
            total_result += test1(25, 26, 27, 28, 29, 30, 31, 32);
            break;
    }
    
    /* Add helper with complex flow */
    total_result += helper_with_complex_flow(10);
    
    /* Use global volatiles to prevent dead code elimination */
    total_result += global_volatile_int;
    total_result += (int)global_volatile_double;
    total_result += (int)global_volatile_long;
    
    /* Use register variables */
    reg_var_1 = total_result % 100;
    reg_var_2 = total_result / 100;
    total_result += reg_var_1 * reg_var_2;
    
    /* Final opaque call */
    asm volatile(
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    printf("Result: %d\n", total_result);
    return total_result % 256;
}

/* Dummy implementations of opaque functions to satisfy linker */
void opaque_call_1(void) {
    COMPILER_BARRIER();
}

int opaque_call_2(int x) {
    COMPILER_BARRIER();
    return x + 1;
}

double opaque_call_3(double x) {
    COMPILER_BARRIER();
    return x * 2.0;
}

void opaque_call_4(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        int val = va_arg(args, int);
        COMPILER_BARRIER();
    }
    va_end(args);
}
