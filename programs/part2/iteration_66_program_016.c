/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer test_caller_save.c -o test_caller_save
 * Or for coverage: gcc -O2 -fno-omit-frame-pointer -c test_caller_save.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs(void);
extern int __attribute__((noinline)) use_some_args(int, int, int, int, int);
extern void* __attribute__((noinline)) clobber_pointer_regs(void*);

/* Prevent constant propagation and dead code elimination */
volatile int global_seed = 0;

/* Function 1: High register pressure with multiple calls */
int __attribute__((noinline)) test_high_pressure(int x) {
    /* Many local variables that must live across calls */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    int i = x + 9;
    int j = x + 10;
    int k = x + 11;
    int l = x + 12;
    int m = x + 13;
    int n = x + 14;
    int o = x + 15;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs();
    
    /* Use variables to keep them live */
    int sum1 = a + b + c + d + e;
    
    /* Second call with different clobber pattern */
    int temp = use_some_args(f, g, h, i, j);
    
    /* More computations keeping many variables live */
    int sum2 = k + l + m + n + o + temp;
    
    /* Third call */
    clobber_many_regs();
    
    /* Final use of all variables */
    return sum1 + sum2 + a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
}

/* Function 2: Mix of caller-saved and callee-saved usage with control flow */
int __attribute__((noinline)) test_mixed_save(int x, int y) {
    /* Variables that might go in callee-saved registers */
    register long r1 asm("rbx") = x * 2;
    register long r2 asm("r12") = y * 3;
    register long r3 asm("r13") = x + y;
    register long r4 asm("r14") = x - y;
    
    /* Many temporary variables for caller-saved pressure */
    int t1 = x + 1, t2 = x + 2, t3 = x + 3, t4 = x + 4;
    int t5 = x + 5, t6 = x + 6, t7 = x + 7, t8 = x + 8;
    
    /* Conditional with calls in both branches */
    if (x > y) {
        clobber_many_regs();
        /* Use both register and stack variables */
        t1 = t1 + (int)r1;
        t2 = t2 + (int)r2;
        t3 = t3 + (int)r3;
        t4 = t4 + (int)r4;
    } else {
        int result = use_some_args(t1, t2, t3, t4);
        t5 = t5 + result;
        t6 = t6 + result;
    }
    
    /* Another call after the conditional */
    clobber_many_regs();
    
    /* Use all variables to keep them live */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + (int)(r1 + r2 + r3 + r4);
}

/* Function 3: Pointer-heavy with calls that clobber pointer registers */
void* __attribute__((noinline)) test_pointer_pressure(void* base, int n) {
    /* Many pointer variables */
    char* p1 = (char*)base + 1;
    char* p2 = (char*)base + 2;
    char* p3 = (char*)base + 3;
    char* p4 = (char*)base + 4;
    char* p5 = (char*)base + 5;
    char* p6 = (char*)base + 6;
    
    /* Integer variables mixed in */
    int i1 = n + 1, i2 = n + 2, i3 = n + 3;
    int i4 = n + 4, i5 = n + 5, i6 = n + 6;
    
    /* Call that clobbers pointer registers */
    void* result = clobber_pointer_regs(base);
    
    /* Use pointers and integers */
    *p1 = (char)i1;
    *p2 = (char)i2;
    *p3 = (char)i3;
    
    /* Another call */
    clobber_many_regs();
    
    /* More uses */
    *p4 = (char)i4;
    *p5 = (char)i5;
    *p6 = (char)i6;
    
    return result;
}

/* Function 4: Loop with calls inside */
int __attribute__((noinline)) test_loop_calls(int iterations) {
    int sum = 0;
    
    /* Many live variables across loop iterations */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int n = 0; n < iterations; n++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs();
        
        /* Use all variables */
        sum += a + b + c + d + e + f + g + h + i + j + n;
        
        /* Modify some variables */
        a = (a * 3) % 100;
        b = (b * 5) % 100;
        c = (c * 7) % 100;
    }
    
    /* Final call */
    use_some_args(a, b, c, d, e);
    
    return sum;
}

/* Function 5: Nested calls with register pressure */
int __attribute__((noinline)) test_nested_pressure(int x) {
    /* Deep expression with many temporaries */
    int a = x * 2;
    int b = x * 3;
    int c = x * 4;
    int d = x * 5;
    int e = x * 6;
    int f = x * 7;
    int g = x * 8;
    int h = x * 9;
    
    /* Call between computations */
    int t1 = use_some_args(a, b, c, d, e);
    
    /* More computations */
    int t2 = f + g + h + t1;
    
    /* Another call */
    clobber_many_regs();
    
    /* Complex expression forcing many temporaries */
    return ((a * b) + (c * d) + (e * f) + (g * h)) / (t1 + t2 + 1);
}

/* Main function that runs all tests */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use argc to vary execution paths */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Allocate memory for pointer test */
    char buffer[100];
    
    /* Run test 1 */
    result += test_high_pressure(seed);
    
    /* Run test 2 */
    result += test_mixed_save(seed, seed * 2);
    
    /* Run test 3 */
    result += (int)(long)test_pointer_pressure(buffer, seed);
    
    /* Run test 4 */
    result += test_loop_calls(seed % 10 + 1);
    
    /* Run test 5 */
    result += test_nested_pressure(seed);
    
    /* Add some inline asm to clobber caller-saved regs directly */
    asm volatile (
        "mov $0, %%rax\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Definitions of external functions (in same file to avoid linking issues,
   but marked noinline to prevent inlining) */
void __attribute__((noinline)) clobber_many_regs(void) {
    /* Inline asm that clobbers many caller-saved registers */
    asm volatile (
        "nop\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory", "cc"
    );
}

int __attribute__((noinline)) use_some_args(int a, int b, int c, int d, int e) {
    /* Function that uses arguments and returns a value */
    return (a + b) * (c + d) + e;
}

void* __attribute__((noinline)) clobber_pointer_regs(void* p) {
    /* Clobber pointer-related registers */
    asm volatile (
        "nop\n"
        : /* no outputs */
        : "r"(p)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    return p;
}
