/* test-caller-save.c
 * A program designed to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets the instruction insertion logic at lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;
volatile void* global_pointer = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[10];

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e, f;
    double x, y, z;
    void* p;
};

/* Force many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Declare many register variables to increase pressure */
    register int r0 asm ("r10") = mode + 1;
    register int r1 asm ("r11") = mode + 2;
    register int r2 asm ("r12") = mode + 3;
    register int r3 asm ("r13") = mode + 4;
    register int r4 asm ("r14") = mode + 5;
    register int r5 asm ("r15") = mode + 6;
    
    volatile int stack_vars[10];
    for (int i = 0; i < 10; i++) {
        stack_vars[i] = i + mode;
    }
    
    /* Use inline asm to clobber call-clobbered registers */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Save register values to volatile memory */
    int saved_r0 = r0;
    int saved_r1 = r1;
    int saved_r2 = r2;
    
    /* Complex control flow with goto to create block boundaries */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Function call that clobbers registers */
    opaque_func1();
    
    /* Use saved values immediately after call */
    r0 = saved_r0 + saved_r1;
    r1 = saved_r2 * 2;
    
    /* Another asm barrier */
    asm volatile ("" : : : "memory");
    
    goto label3;
    
label2:
    /* Different path with another call */
    int result = opaque_func2(mode);
    
    /* Complex expression using multiple registers */
    r2 = (result * saved_r0) / (saved_r1 + 1);
    r3 = saved_r2 ^ result;
    
    /* Force register spill by using all variables */
    stack_vars[0] = r0 + r1 + r2 + r3 + r4 + r5;
    
    goto label3;
    
label3:
    /* Merge point - creates basic block boundary */
    volatile int merge_var = r0 + r1 + r2;
    
    /* Another call at block boundary */
    if (merge_var > 100) {
        opaque_func1();
    }
    
    /* Use all variables to keep them live */
    global_counter += r0 + r1 + r2 + r3 + r4 + r5;
    for (int i = 0; i < 10; i++) {
        global_counter += stack_vars[i];
    }
}

/* Test with floating point and mixed types */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double d1 = base;
    volatile double d2 = base * 2.0;
    volatile double d3 = base * 3.0;
    volatile double d4 = base * 4.0;
    volatile double d5 = base * 5.0;
    
    register double fp1 asm ("xmm0") = d1;
    register double fp2 asm ("xmm1") = d2;
    register double fp3 asm ("xmm3") = d3;
    register double fp4 asm ("xmm4") = d4;
    register double fp5 asm ("xmm5") = d5;
    
    /* Save FP values */
    double saved_fp1 = fp1;
    double saved_fp2 = fp2;
    double saved_fp3 = fp3;
    
    /* Complex switch statement to create CFG complexity */
    int choice = (int)base % 5;
    
    switch (choice) {
        case 0:
            fp1 = opaque_func3(fp1);
            /* Fall through */
        case 1:
            fp2 = opaque_func3(fp2);
            break;
        case 2:
            /* Nested call in default case */
            fp3 = opaque_func3(fp3);
            fp4 = opaque_func3(fp4);
            break;
        default:
            /* Multiple calls in default branch */
            for (int i = 0; i < 3; i++) {
                fp1 = opaque_func3(fp1 + i);
                if (i == 1) {
                    fp2 = opaque_func3(fp2);
                }
            }
            break;
    }
    
    /* Use saved values */
    fp4 = saved_fp1 + saved_fp2;
    fp5 = saved_fp3 * saved_fp1;
    
    /* Loop with break that creates block boundaries */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            /* Call at loop break point */
            opaque_func1();
            break;
        }
        fp1 += 0.1;
    }
    
    global_accumulator += fp1 + fp2 + fp3 + fp4 + fp5;
}

/* Test with pointer manipulation and __builtin_apply */
__attribute__((noinline, noclone))
void test3(void* ptr, int count) {
    volatile int* arr = (volatile int*)ptr;
    register void* rptr asm ("rbx") = ptr;
    
    /* Use __builtin_apply to create unusual call sequences */
    void* args[3];
    args[0] = ptr;
    args[1] = (void*)(long)count;
    args[2] = NULL;
    
    /* Save register value */
    void* saved_ptr = rptr;
    
    /* Indirect call through function pointer */
    if (func_table[0]) {
        func_table[0]();
    }
    
    /* Complex expression using saved pointer */
    rptr = (void*)((long)saved_ptr + count * sizeof(int));
    
    /* Nested loops with calls at boundaries */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                arr[i] = i + j;
                /* Call in inner loop */
                if (i % 3 == 0) {
                    opaque_func1();
                }
            } else {
                arr[i] *= 2;
            }
            
            /* Continue creates new block */
            if (arr[i] > 100) continue;
            
            /* Another call */
            int val = opaque_func2(arr[i]);
            arr[i] = val;
        }
    }
    
    /* Use __builtin_va_arg style pattern */
    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        int val = va_arg(ap, int);
        arr[i] += val;
    }
    va_end(ap);
    
    global_pointer = rptr;
}

/* Helper with nested calls to force save/restore insertion */
__attribute__((noinline, noclone))
int helper_nested(int depth, int value) {
    volatile int stack[5] = {value, value+1, value+2, value+3, value+4};
    
    if (depth <= 0) {
        return value;
    }
    
    /* Recursive call */
    int result = helper_nested(depth - 1, value * 2);
    
    /* Use stack variables after call */
    stack[0] = result;
    
    /* Call another function */
    result = opaque_func2(result);
    
    /* Complex control flow */
    switch (result % 4) {
        case 0:
            result += stack[1];
            break;
        case 1:
            result += stack[2];
            /* Fall through */
        case 2:
            result += stack[3];
            opaque_func1();
            break;
        default:
            result += stack[4];
            break;
    }
    
    return result;
}

/* Test with nested calls and irreducible control flow */
__attribute__((noinline, noclone))
void test4(int iterations) {
    register int accum asm ("r10") = 0;
    register int temp asm ("r11") = iterations;
    
    volatile int values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = i * iterations;
    }
    
    /* Create irreducible CFG with goto */
    int i = 0;
loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Save register before call */
    int saved_accum = accum;
    int saved_temp = temp;
    
    /* Nested call */
    int result = helper_nested(2, i);
    
    /* Restore and use saved values */
    accum = saved_accum + result;
    temp = saved_temp * result;
    
    /* Complex condition with call */
    if (i % 3 == 0) {
        opaque_func1();
        goto special_case;
    } else if (i % 3 == 1) {
        values[i] = opaque_func2(values[i]);
        goto normal_case;
    } else {
        goto another_case;
    }
    
special_case:
    values[i] += accum;
    i++;
    goto loop_start;
    
normal_case:
    values[i] += temp;
    i++;
    goto loop_start;
    
another_case:
    /* Call at block end */
    values[i] = opaque_func2(values[i] + accum);
    i++;
    if (i < iterations) {
        goto loop_start;
    }
    
loop_end:
    /* Final computation */
    for (int j = 0; j < 20; j++) {
        global_counter += values[j];
    }
}

/* Test with mixed types and many arguments */
__attribute__((noinline, noclone))
double test5(int a, double b, void* c, int d, double e, int f) {
    /* Many arguments force register/stack pressure */
    register int r1 asm ("r10") = a;
    register double r2 asm ("xmm0") = b;
    register void* r3 asm ("r12") = c;
    register int r4 asm ("r13") = d;
    register double r5 asm ("xmm1") = e;
    register int r6 asm ("r14") = f;
    
    /* Save all to volatile memory */
    volatile int saved_ints[6] = {r1, r4, r6, 0, 0, 0};
    volatile double saved_fps[3] = {r2, r5, 0.0};
    
    /* Multiple calls with different conventions */
    opaque_func1();
    
    /* Use saved values */
    r1 = saved_ints[0] + saved_ints[1];
    r2 = saved_fps[0] * saved_fps[1];
    
    /* Another call */
    r2 = opaque_func3(r2);
    
    /* Complex expression requiring temporary */
    double result = r2 + (double)r1 + (double)r4 + (double)r6;
    
    /* Call via function pointer */
    if (func_table[1]) {
        func_table[1]();
    }
    
    /* Use inline asm that looks like a call */
    asm volatile (
        "call dummy_label\n\t"
        "dummy_label:\n\t"
        "pop %%rax\n\t"
        : : : "rax", "memory"
    );
    
    return result;
}

/* Main function that orchestrates all tests */
int main(int argc, char** argv) {
    /* Initialize function pointers */
    for (int i = 0; i < 10; i++) {
        func_table[i] = opaque_func1;
    }
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize volatile data */
    volatile int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * mode;
    }
    
    /* Run all tests in sequence, but with mode-dependent order */
    switch (mode) {
        case 0:
            test1(mode);
            test2(3.14159 * mode);
            test3((void*)data, 50);
            test4(10 + mode);
            test5(mode, 2.71828 * mode, (void*)data, mode+1, 1.414 * mode, mode+2);
            break;
        case 1:
            test4(5 + mode);
            test5(mode, 1.234 * mode, NULL, mode, 5.678 * mode, mode*2);
            test1(mode * 2);
            test2(9.876 * mode);
            test3((void*)&global_counter, 20);
            break;
        case 2:
            test2(1.0 / (mode + 1));
            test3((void*)data, 30);
            test1(mode * 3);
            test4(15);
            test5(mode, 3.333 * mode, (void*)&global_accumulator, mode, 6.666 * mode, mode);
            break;
        case 3:
            test5(0, 0.0, NULL, 0, 0.0, 0);
            for (int i = 0; i < 3; i++) {
                test1(mode + i);
                test2(i * 1.5);
            }
            test3((void*)data, 40);
            test4(8);
            break;
        default:
            /* Run all tests multiple times */
            for (int i = 0; i < 2; i++) {
                test1(mode + i);
                test2(i * 2.5);
                test3((void*)data, 25);
                test4(12);
                test5(i, i * 1.1, (void*)data, i+1, i * 2.2, i+2);
            }
            break;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_counter;
    checksum += (int)global_accumulator;
    checksum += (long)global_pointer % 1000;
    
    for (int i = 0; i < 100; i++) {
        checksum += data[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* Dummy definitions for external functions */
void opaque_func1(void) {
    asm volatile ("" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("" : "+r" (x) : : "memory");
    return x ^ 0x55AA55AA;
}

double opaque_func3(double x) {
    asm volatile ("" : "+x" (x) : : "memory");
    return x * 1.5;
}

void* opaque_func4(void* x) {
    asm volatile ("" : "+r" (x) : : "memory");
    return (void*)((long)x + 1);
}
