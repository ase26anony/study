/* caller-save-test.c - Test program to trigger uncovered lines in GCC's caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double, double);
extern void* opaque_func4(void*, void*);

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 0;
volatile double global_volatile_double = 0.0;
volatile void* global_volatile_ptr = NULL;

/* Function pointer with volatile to prevent constant propagation */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = NULL;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with explicit register variables */
#ifdef __x86_64__
register long r12_val asm("r12");
register long r13_val asm("r13");
register long r14_val asm("r14");
register long r15_val asm("r15");
#endif

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
int test1(int a, int b, int c, int d, int e, int f) {
    /* Force many values to be live across call */
    volatile int v1 = a + b;
    volatile int v2 = c + d;
    volatile int v3 = e + f;
    volatile int v4 = a * b;
    volatile int v5 = c * d;
    volatile int v6 = e * f;
    
    /* Array to force stack usage */
    int stack_array[16];
    for (int i = 0; i < 16; i++) {
        stack_array[i] = i + a + b + c;
    }
    
    /* Call that clobbers registers */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movl $0x9ABCDEF0, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use all volatile values after call */
    int result = v1 + v2 + v3 + v4 + v5 + v6;
    for (int i = 0; i < 16; i++) {
        result += stack_array[i];
    }
    
    /* Another call with different clobbers */
    opaque_func1();
    COMPILER_BARRIER();
    
    return result + global_volatile_int;
}

/* Function with irreducible control flow */
__attribute__((noinline, noclone))
int test2(int x) {
    int result = 0;
    volatile int a = x;
    volatile int b = x * 2;
    volatile int c = x * 3;
    
    /* Create complex CFG with goto */
    if (x > 100) {
        goto label1;
    } else if (x > 50) {
        goto label2;
    }
    
    /* Call in one path */
    result = opaque_func2(x);
    COMPILER_BARRIER();
    
    if (result > 0) {
        goto label3;
    }
    
label1:
    /* Use volatile values */
    result += a + b;
    
    /* Another call */
    asm volatile(
        "movq $0xAAAAAAAA, %%r10\n\t"
        "movq $0xBBBBBBBB, %%r11\n\t"
        : : : "r10", "r11", "memory"
    );
    
    if (x % 2 == 0) {
        goto label4;
    }
    
label2:
    result += c * 2;
    
    /* Call that might split basic block */
    void* ptr = opaque_func4(&result, &global_volatile_int);
    COMPILER_BARRIER();
    
    if (ptr) {
        goto label1;
    }
    
label3:
    result += b * 3;
    
    /* Force register pressure with many temporaries */
    {
        register int t1 asm("r12") = result;
        register int t2 asm("r13") = a;
        register int t3 asm("r14") = b;
        register int t4 asm("r15") = c;
        
        asm volatile(
            "addl %%r12d, %%eax\n\t"
            "addl %%r13d, %%eax\n\t"
            "addl %%r14d, %%eax\n\t"
            "addl %%r15d, %%eax\n\t"
            : "+a"(result)
            : "r"(t1), "r"(t2), "r"(t3), "r"(t4)
            : "cc", "memory"
        );
    }
    
label4:
    return result;
}

/* Function with nested calls and loops */
__attribute__((noinline, noclone))
double test3(double base) {
    volatile double accum = base;
    volatile double temp[8];
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        temp[i] = base * i;
    }
    
    /* Loop with call inside */
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) {
            /* Call that clobbers floating point registers */
            double r = opaque_func3(accum, temp[i % 8]);
            COMPILER_BARRIER();
            accum += r;
            
            /* Force spill with many live values */
            asm volatile(
                "fld1\n\t"
                "fldz\n\t"
                "fldpi\n\t"
                "fldln2\n\t"
                : : : "st", "st(1)", "st(2)", "st(3)", "memory"
            );
        }
        
        if (i == 50) {
            /* Break creates block boundary */
            break;
        }
        
        accum += temp[i % 8];
    }
    
    /* Switch statement with calls */
    switch ((int)accum % 4) {
        case 0:
            opaque_func1();
            accum *= 2.0;
            break;
        case 1:
            accum = opaque_func3(accum, 3.14159);
            /* fall through */
        case 2:
            asm volatile("" : "+m"(accum));
            opaque_func2((int)accum);
            break;
        default:
            /* This should trigger block end updates */
            void* p = opaque_func4(&accum, &global_volatile_double);
            if (p) {
                accum += *(double*)p;
            }
            break;
    }
    
    return accum;
}

/* Function using __builtin_apply */
__attribute__((noinline, noclone))
int test4(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Force register pressure with many args */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    volatile int v7 = g;
    volatile int v8 = h;
    
    /* Simulate variable arguments */
    int (*func)(int, ...) = (int (*)(int, ...))opaque_func2;
    
    /* Use __builtin_apply to create complex call sequence */
    void* args = __builtin_apply_args();
    int result;
    
    /* This creates unusual register pressure */
    asm volatile(
        "pushq %%rbp\n\t"
        "movq %%rsp, %%rbp\n\t"
        "andq $-16, %%rsp\n\t"
        : : : "memory"
    );
    
    result = func(a, b, c, d, e, f, g, h);
    COMPILER_BARRIER();
    
    /* Use all volatile values after call */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    
    /* Nested call scenario */
    {
        int inner_result = test2(result);
        result += inner_result;
        
        /* Call via volatile function pointer */
        if (volatile_func_ptr) {
            result += volatile_func_ptr(result);
        }
    }
    
    return result;
}

/* Function with computed goto */
__attribute__((noinline, noclone))
int test5(int selector) {
    static void* jump_table[] = {
        &&case0, &&case1, &&case2, &&case3,
        &&case4, &&case5, &&default_case
    };
    
    volatile int values[10];
    for (int i = 0; i < 10; i++) {
        values[i] = i * selector;
    }
    
    int idx = selector % 7;
    goto *jump_table[idx];
    
case0:
    opaque_func1();
    return values[0] + values[1];
    
case1:
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
    return values[2] + opaque_func2(values[3]);
    
case2:
    /* Force block splitting */
    if (selector > 0) {
        opaque_func3(values[4], values[5]);
        COMPILER_BARRIER();
    }
    return values[6];
    
case3:
    /* Complex expression requiring temporaries */
    {
        int t1 = values[7];
        int t2 = values[8];
        int t3 = values[9];
        
        /* Call in middle of computation */
        int r = opaque_func2(t1);
        COMPILER_BARRIER();
        
        return r + t2 * t3;
    }
    
case4:
    /* Loop with break that creates block boundary */
    for (int i = 0; i < 100; i++) {
        if (i == selector) {
            opaque_func1();
            break;
        }
        values[i % 10] += i;
    }
    return values[0];
    
case5:
    /* Nested conditionals with calls */
    if (selector % 2) {
        if (selector % 3) {
            opaque_func2(values[1]);
        } else {
            opaque_func3(values[2], values[3]);
        }
    }
    return values[4];
    
default_case:
    /* Default case with call at block end */
    void* p = opaque_func4(values, &global_volatile_int);
    return p ? *(int*)p : values[5];
}

/* Helper with nested call */
__attribute__((noinline, noclone))
int nested_helper(int depth, int val) {
    volatile int stack[8];
    for (int i = 0; i < 8; i++) {
        stack[i] = val + i;
    }
    
    if (depth > 0) {
        /* Recursive call */
        int r = nested_helper(depth - 1, val * 2);
        
        /* Use stack values after call */
        int sum = 0;
        for (int i = 0; i < 8; i++) {
            sum += stack[i];
        }
        return r + sum;
    }
    
    /* Leaf call that clobbers registers */
    asm volatile(
        "movl $0xDEADBEEF, %%eax\n\t"
        "movl $0xCAFEBABE, %%ebx\n\t"
        "movl $0xFEEDFACE, %%ecx\n\t"
        : : : "eax", "ebx", "ecx", "memory"
    );
    
    return val;
}

/* Main test driver */
int main(int argc, char** argv) {
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 6;
    }
    
    /* Initialize volatile function pointer */
    volatile_func_ptr = (func_ptr_t)opaque_func2;
    
    int result = 0;
    double fp_result = 0.0;
    
    /* Execute all tests but with different order based on mode */
    switch (test_mode) {
        case 0:
            result += test1(1, 2, 3, 4, 5, 6);
            fp_result += test3(1.5);
            result += test2(42);
            result += test4(1, 2, 3, 4, 5, 6, 7, 8);
            result += test5(99);
            break;
        case 1:
            result += test2(100);
            fp_result += test3(2.5);
            result += test1(10, 20, 30, 40, 50, 60);
            result += test5(50);
            result += test4(2, 4, 6, 8, 10, 12, 14, 16);
            break;
        case 2:
            result += test4(3, 6, 9, 12, 15, 18, 21, 24);
            fp_result += test3(3.14159);
            result += test5(25);
            result += test1(100, 200, 300, 400, 500, 600);
            result += test2(1000);
            break;
        default:
            /* Execute all in different order */
            for (int i = 0; i < 3; i++) {
                result += test1(i, i*2, i*3, i*4, i*5, i*6);
                fp_result += test3(i * 1.1);
                result += test2(i * 10);
                result += nested_helper(2, i * 100);
            }
            result += test4(1, 3, 5, 7, 9, 11, 13, 15);
            result += test5(33);
            break;
    }
    
    /* Use explicit register variables */
    #ifdef __x86_64__
    r12_val = result;
    r13_val = result * 2;
    r14_val = result * 3;
    r15_val = result * 4;
    
    asm volatile(
        "addq %%r12, %%rax\n\t"
        "addq %%r13, %%rax\n\t"
        "addq %%r14, %%rax\n\t"
        "addq %%r15, %%rax\n\t"
        : "+a"(result)
        : "r"(r12_val), "r"(r13_val), "r"(r14_val), "r"(r15_val)
        : "cc"
    );
    #endif
    
    /* Final call that might trigger prologue/epilogue complexity */
    global_volatile_int = result;
    global_volatile_double = fp_result;
    
    /* Compute checksum to prevent elimination */
    int checksum = result + (int)fp_result + global_volatile_int;
    printf("Result: %d, FP: %f, Checksum: %d\n", 
           result, fp_result, checksum);
    
    return checksum != 0 ? 0 : 1;
}
