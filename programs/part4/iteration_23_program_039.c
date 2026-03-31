/* caller-save-test.c
 * Test program to trigger uncovered code in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* External opaque functions to prevent inlining */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void opaque_func4(volatile int*) __attribute__((noinline, noclone));

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256] = {0};
volatile double global_farray[256] = {0.0};

/* Function pointer with volatile assignment to prevent optimization */
void (*volatile func_ptr)(void) = opaque_func1;

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force register pressure with many local variables */
    register int r1 asm ("r12") = mode + 1;
    register int r2 asm ("r13") = mode + 2;
    volatile int v1 = mode * 3;
    volatile int v2 = mode * 4;
    volatile int v3 = mode * 5;
    volatile int v4 = mode * 6;
    volatile int v5 = mode * 7;
    
    /* Array to force stack usage */
    int stack_array[16];
    for (int i = 0; i < 16; i++) {
        stack_array[i] = mode + i;
    }
    
    /* Complex control flow with goto to create irreducible CFG */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Use all volatile variables before call */
    asm volatile ("" : : "r" (r1), "r" (r2), "m" (v1), "m" (v2) : "memory");
    
    /* Function call that clobbers registers */
    opaque_func1();
    
    /* Inline asm that acts like a call */
    asm volatile (
        "movl $0, %%eax\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        : : : "eax", "ecx", "edx", "memory"
    );
    
    /* Use variables after call - forces caller-save */
    v3 = r1 + r2 + v1 + v2;
    asm volatile ("" : : "r" (v3), "m" (stack_array[0]) : "memory");
    
    if (v3 > 100) {
        goto label3;
    }
    
label2:
    /* Another call site with different register pressure */
    volatile double d1 = mode * 1.5;
    volatile double d2 = mode * 2.5;
    
    /* Call with floating point */
    double result = opaque_func3(d1);
    
    /* Complex expression requiring temporary registers */
    d2 = result + d1 + (double)r1 + (double)r2;
    
    /* Force register spill/reload */
    asm volatile ("" : : "r" (r1), "r" (r2), "m" (d1), "m" (d2) : "memory");
    
    /* Call via function pointer */
    func_ptr();
    
    /* Use all variables again */
    v4 = (int)d1 + (int)d2 + r1 + r2;
    
label3:
    /* Switch statement to create complex CFG */
    switch (mode % 4) {
        case 0:
            opaque_func2(v4);
            break;
        case 1:
            opaque_func2(v3);
            /* Fall through */
        case 2:
            v5 = opaque_func2(v4) + v3;
            break;
        default:
            /* Call in default case - may create block boundary */
            opaque_func4(&global_counter);
            v5 = global_counter;
            break;
    }
    
    /* Final use to prevent DCE */
    global_array[mode % 256] = v3 + v4 + v5 + r1 + r2;
}

/* Function with mixed argument types and varargs */
__attribute__((noinline, noclone))
void test2(int a, double b, int c, ...) {
    va_list args;
    va_start(args, c);
    
    /* Force many values into registers */
    register int r3 asm ("r14") = a;
    register int r4 asm ("r15") = c;
    volatile float f1 = (float)b;
    volatile float f2 = f1 * 2.0f;
    
    /* Loop with break/continue creating block splits */
    for (int i = 0; i < 10; i++) {
        if (i == a % 5) {
            /* Call inside loop with break */
            opaque_func2(i);
            if (i > c) break;
            continue;
        }
        
        if (i == c % 3) {
            /* Another call site */
            opaque_func3((double)i);
            continue;
        }
        
        /* Use volatile variables */
        f2 += (float)r3 + (float)r4;
        
        /* Inline asm with clobbers */
        asm volatile (
            "movq $0, %%r10\n\t"
            "movq $0, %%r11\n\t"
            : : : "r10", "r11", "memory"
        );
    }
    
    /* Varargs usage */
    int varg1 = va_arg(args, int);
    double varg2 = va_arg(args, double);
    
    /* Complex expression across call */
    f1 = (float)(b + varg2) * (float)(a + varg1);
    
    /* Call with result used immediately */
    int temp = opaque_func2((int)f1) + r3 + r4;
    
    /* Force save/restore around this call */
    asm volatile ("" : : "r" (r3), "r" (r4), "m" (f1), "m" (f2) : "memory");
    
    /* Another call */
    opaque_func1();
    
    /* Use all values */
    global_farray[a % 256] = f1 + f2 + (double)temp;
    
    va_end(args);
}

/* Function with nested calls and register variables */
__attribute__((noinline, noclone))
int test3(volatile int *ptr) {
    /* Explicit register variables */
    register long r5 asm ("rbx") = (long)ptr;
    register long r6 asm ("rbp") = global_counter;
    
    /* Volatile array */
    volatile long values[8];
    for (int i = 0; i < 8; i++) {
        values[i] = r5 + r6 + i;
    }
    
    /* Outer call */
    int result1 = opaque_func2(*ptr);
    
    /* Complex control flow */
    switch (result1 % 3) {
        case 0: {
            /* Inner function with its own complexity */
            auto int inner_func(int x) {
                volatile int inner = x;
                /* Call within inner function */
                opaque_func4(&inner);
                return inner + 1;
            }
            
            int inner_result = inner_func(result1);
            
            /* Use register variables after inner call */
            asm volatile ("" : : "r" (r5), "r" (r6) : "memory");
            
            result1 += inner_result;
            break;
        }
        case 1:
            /* Direct call sequence */
            opaque_func1();
            asm volatile ("" : : "r" (r5), "r" (r6) : "memory");
            opaque_func1();
            break;
        default:
            /* Loop with call at end */
            for (int j = 0; j < 3; j++) {
                values[j % 8] += j;
                if (j == 1) {
                    opaque_func2(j);
                }
            }
            break;
    }
    
    /* Final computation using all registers */
    long final = r5 + r6;
    for (int i = 0; i < 8; i++) {
        final += values[i];
    }
    
    return (int)final + result1;
}

/* Function using __builtin_apply */
__attribute__((noinline, noclone))
void test4(void (*func)(int, double), int x, double y) {
    /* Create argument frame */
    __builtin_apply((void (*)(void))func, __builtin_apply_args(), 32);
    
    /* Use many volatiles to force spills */
    volatile double darray[4];
    volatile int iarray[4];
    
    for (int i = 0; i < 4; i++) {
        darray[i] = y * i;
        iarray[i] = x + i;
    }
    
    /* Multiple asm barriers */
    asm volatile ("" : : : "memory");
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
    asm volatile ("" : : : "memory");
    
    /* Call again */
    func(x + 1, y * 2.0);
    
    /* Complex use of arrays */
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += darray[i] + iarray[i];
    }
    
    global_farray[x % 256] = sum;
}

/* Vector type function */
typedef int v4si __attribute__((vector_size(16)));
__attribute__((noinline, noclone))
void test5(v4si vec) {
    volatile v4si v1 = vec;
    volatile v4si v2 = vec + (v4si){1, 2, 3, 4};
    
    /* Call between vector operations */
    opaque_func2(v1[0]);
    
    /* Vector operation */
    v4si v3 = v1 + v2;
    
    /* Another call */
    opaque_func3((double)v2[1]);
    
    /* Use result */
    volatile int sum = v3[0] + v3[1] + v3[2] + v3[3];
    
    /* Force register pressure with scalar variables too */
    register int s1 asm ("r12") = sum;
    register int s2 asm ("r13") = sum * 2;
    volatile int s3 = sum * 3;
    volatile int s4 = sum * 4;
    
    /* Multiple calls in sequence */
    for (int i = 0; i < 2; i++) {
        opaque_func2(s1 + i);
        s3 += s2;
        opaque_func1();
        s4 += s3;
    }
    
    global_array[sum % 256] = s1 + s2 + s3 + s4;
}

/* Helper for nested call scenario */
__attribute__((noinline, noclone))
int helper_func(int depth, volatile int *ptr) {
    if (depth <= 0) {
        opaque_func4(ptr);
        return *ptr;
    }
    
    /* Recursive call */
    int result = helper_func(depth - 1, ptr);
    
    /* Use result in complex expression */
    register int r asm ("r14") = result;
    asm volatile ("" : : "r" (r) : "memory");
    
    /* Call after recursive call */
    opaque_func2(r);
    
    return r + depth;
}

/* Main test driver */
int main(int argc, char **argv) {
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize global state */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
        global_farray[i] = i * 1.5;
    }
    
    /* Run all tests in different orders based on mode */
    void (*test_funcs[])(void) = {
        [0] = () => test1(mode),
        [1] = () => test2(mode, mode * 1.5, mode + 1, mode + 2, mode * 2.5),
        [2] = () => { volatile int x = mode; test3(&x); },
        [3] = () => test4((void (*)(int, double))opaque_func2, mode, mode * 1.5),
        [4] = () => test5((v4si){mode, mode+1, mode+2, mode+3}),
    };
    
    /* Execute tests with complex ordering */
    for (int i = 0; i < 5; i++) {
        int idx = (mode + i) % 5;
        test_funcs[idx]();
        
        /* Call helper with nested calls */
        volatile int val = global_counter + i;
        int helper_result = helper_func(2, &val);
        global_counter += helper_result;
    }
    
    /* Compute checksum to prevent DCE */
    unsigned long long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
        checksum += (unsigned long long)global_farray[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum % 256);
}

/* Dummy implementations of opaque functions */
void opaque_func1(void) {
    asm volatile ("" : : : "memory");
}

int opaque_func2(int x) {
    volatile int y = x;
    asm volatile ("" : "+r" (y) : : "memory");
    return y + 1;
}

double opaque_func3(double x) {
    volatile double y = x;
    asm volatile ("" : "+x" (y) : : "memory");
    return y * 1.1;
}

void opaque_func4(volatile int *ptr) {
    if (ptr) {
        *ptr += 1;
    }
    asm volatile ("" : : : "memory");
}
