/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets lines 905-913 which handle instruction chain manipulation
 * during register save/restore insertion around function calls.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Volatile globals to maintain live ranges across calls */
volatile int global_volatile_int = 0x12345678;
volatile double global_volatile_double = 3.141592653589793;
volatile void* global_volatile_ptr = NULL;

/* Function pointers to create indirect calls */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = NULL;

/* Force register pressure with explicit register variables */
register int reg_var1 asm ("r12") __attribute__((unused));
register int reg_var2 asm ("r13") __attribute__((unused));
register int reg_var3 asm ("r14") __attribute__((unused));

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Create many live variables to exceed call-saved registers */
    volatile int v1 = global_volatile_int;
    volatile int v2 = v1 * 2;
    volatile int v3 = v2 + mode;
    volatile double d1 = global_volatile_double;
    volatile double d2 = d1 * 2.0;
    
    /* Use inline asm to clobber specific registers */
    asm volatile ("# Clobber eax, r10" : : : "eax", "r10", "memory");
    
    /* Function call that clobbers registers */
    opaque_func1();
    
    /* Complex control flow to force basic block splitting */
    if (v1 > 0) {
        /* Another call with different register pressure */
        v2 = opaque_func2(v3);
        
        /* Use goto to create irreducible flow */
        if (v2 % 2) {
            goto label1;
        } else {
            goto label2;
        }
    }
    
    /* More register-intensive operations */
    asm volatile ("# Use many registers" 
                  : "=r" (v1), "=r" (v2), "=r" (v3)
                  : "0" (v1), "1" (v2), "2" (v3)
                  : "memory");
    
label1:
    /* Nested call in a loop to force save/restore insertion */
    for (int i = 0; i < 3; i++) {
        /* Call with live variables */
        d1 = opaque_func3(d2);
        
        /* Break in middle of loop to split basic block */
        if (i == 1 && v1 > 100) {
            /* Another call at block boundary */
            opaque_func1();
            break;
        }
        
        /* Continue creates another block boundary */
        if (d1 < 0.0) continue;
        
        v3 += (int)d1;
    }
    
label2:
    /* Switch statement to create complex CFG */
    switch (v3 & 3) {
        case 0:
            opaque_func1();
            break;
        case 1:
            v1 = opaque_func2(v2);
            /* Fall through */
        case 2:
            /* Call at switch case boundary */
            d2 = opaque_func3(d1);
            break;
        default:
            /* Call in default case - likely block end */
            opaque_func4(&v1);
            /* This should be at BB_END before insertion */
            v2 = v1 * v3;
    }
    
    /* Force use of all volatile variables to maintain live ranges */
    global_volatile_int = v1 + v2 + v3;
    global_volatile_double = d1 + d2;
}

/* Function with mixed register types and calling conventions */
__attribute__((noinline, noclone))
int test2(int a, double b, void* c) {
    /* Explicit register variables that need saving */
    register int r1 asm ("ebx");
    register int r2 asm ("esi");
    register int r3 asm ("edi");
    
    r1 = a;
    r2 = (int)b;
    r3 = (int)(long)c;
    
    /* Volatile array to force stack spills */
    volatile int stack_array[10];
    for (int i = 0; i < 10; i++) {
        stack_array[i] = r1 + i;
    }
    
    /* Function call that will clobber registers */
    int result = opaque_func2(r1);
    
    /* Use the stack array after call - forces reload */
    for (int i = 0; i < 10; i++) {
        r2 += stack_array[i];
    }
    
    /* Another call with different arguments */
    double d = opaque_func3(b);
    
    /* Complex expression requiring temporary registers */
    r3 = (int)(d * 100.0) + result + r2;
    
    /* Inline asm acting as pseudo-call */
    asm volatile ("# Pseudo-call with clobbers"
                  : "+r" (r1), "+r" (r2), "+r" (r3)
                  : "rm" (d)
                  : "rax", "rcx", "rdx", "memory");
    
    /* Indirect call through volatile pointer */
    if (volatile_func_ptr) {
        r1 = volatile_func_ptr(r3);
    }
    
    return r1 + r2 + r3;
}

/* Function with __builtin_apply to create unusual call sequences */
__attribute__((noinline, noclone))
void test3(void* arg) {
    /* Variable argument simulation */
    volatile int va1 = 1, va2 = 2, va3 = 3, va4 = 4, va5 = 5;
    
    /* Use __builtin_apply to force register pressure */
    void* args = __builtin_apply_args();
    
    /* Multiple calls in sequence with live variables */
    for (int i = 0; i < 2; i++) {
        /* Save volatile values before call */
        int saved1 = va1;
        int saved2 = va2;
        double saved3 = global_volatile_double;
        
        /* Call that clobbers registers */
        opaque_func1();
        
        /* Use saved values - forces save/restore around call */
        va1 = saved1 + i;
        va2 = saved2 * saved1;
        global_volatile_double = saved3 * 2.0;
        
        /* Break in middle to create block boundary */
        if (i == 0 && global_volatile_int > 0) {
            /* Another call at boundary */
            opaque_func4(arg);
            continue;
        }
    }
    
    /* Switch with calls in multiple cases */
    switch (global_volatile_int & 7) {
        case 0: case 1: case 2:
            opaque_func1();
            /* Fall through creates edge case for BB_END */
        case 3:
            va3 = opaque_func2(va4);
            if (va3 > 0) {
                /* Call in conditional block */
                opaque_func3((double)va3);
                va5 = va3 * 2;
            }
            break;
        case 4:
            /* Empty case to force jump table */
            break;
        default:
            /* Multiple calls in default */
            opaque_func1();
            opaque_func4(&va5);
            /* This should be BB_END before potential insertion */
            va4 = va5 + 1;
    }
}

/* Function with nested calls to force save/restore in outer call */
__attribute__((noinline, noclone))
int test4(int depth) {
    if (depth <= 0) {
        return opaque_func2(1);
    }
    
    /* Many live variables */
    volatile int l1 = depth;
    volatile int l2 = l1 * 2;
    volatile int l3 = l2 + global_volatile_int;
    
    /* Outer call */
    int result1 = opaque_func2(l1);
    
    /* Nested call with live variables from outer scope */
    int result2 = test4(depth - 1);
    
    /* Use variables that were live across both calls */
    l3 = result1 + result2 + l3;
    
    /* Another call after nested call */
    opaque_func1();
    
    /* Complex control flow with goto */
    if (l3 > 1000) {
        goto early_exit;
    }
    
    /* Loop with break/continue */
    for (int i = 0; i < l1; i++) {
        if (i == l1 / 2) {
            /* Call at loop mid-point */
            opaque_func3((double)i);
            break;
        }
        l2 += i;
        if (l2 % 3 == 0) continue;
        l3 -= i;
    }
    
early_exit:
    return l1 + l2 + l3;
}

/* Function with vector types for additional register pressure */
typedef int v4si __attribute__((vector_size(16)));
__attribute__((noinline, noclone))
void test5(void) {
    /* Vector operations use SSE/AVX registers */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* Scalar live variables */
    volatile int s1 = 100;
    volatile int s2 = 200;
    volatile double d1 = 3.14;
    
    /* Function call clobbers scalar registers */
    opaque_func1();
    
    /* Vector operation after call */
    v3 = v1 + v2;
    
    /* Another call */
    s1 = opaque_func2(s2);
    
    /* Use vector result */
    for (int i = 0; i < 4; i++) {
        s2 += v3[i];
    }
    
    /* Switch with vector size cases */
    switch (s2 % 4) {
        case 0:
            d1 = opaque_func3(d1);
            v1 = v1 * 2;
            break;
        case 1:
            opaque_func4(&s1);
            /* Fall through */
        case 2:
            v2 = v2 + v1;
            /* Call at potential BB_END */
            opaque_func1();
            s1 = s2 * 3;
            break;
        default:
            /* Multiple operations at block end */
            v3 = v1 - v2;
            s2 = s1 + 10;
    }
    
    global_volatile_int = s1 + s2 + v3[0];
}

/* Helper to create register pressure around a call */
__attribute__((noinline, noclone))
static void pressure_helper(int a, int b, int c, int d, int e, int f) {
    /* Use all arguments to keep them live */
    volatile int sum = a + b + c + d + e + f;
    
    /* Call with many live values */
    opaque_func1();
    
    /* Use values after call */
    global_volatile_int += sum;
}

/* Main function that orchestrates all tests */
int main(int argc, char** argv) {
    int test_mode = 0;
    
    /* Use argv to add runtime variability */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize volatile function pointer */
    volatile_func_ptr = (func_ptr_t)opaque_func2;
    
    /* Array to store checksums */
    int checksums[5] = {0};
    
    /* Execute all tests in different orders based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 5) {
            case 0:
                test1(cycle * 10);
                checksums[0] += global_volatile_int;
                break;
            case 1:
                checksums[1] += test2(cycle, (double)cycle * 1.5, &checksums[1]);
                break;
            case 2:
                test3(&checksums[2]);
                checksums[2] += global_volatile_int;
                break;
            case 3:
                checksums[3] += test4(3);
                break;
            case 4:
                test5();
                checksums[4] += global_volatile_int;
                break;
        }
        
        /* Create register pressure between test calls */
        pressure_helper(checksums[0], checksums[1], checksums[2],
                       checksums[3], checksums[4], cycle);
    }
    
    /* Final checksum to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < 5; i++) {
        final_checksum += checksums[i];
    }
    
    /* Use result to prevent optimization */
    printf("Final checksum: %d\n", final_checksum);
    
    /* Additional complex control flow in main */
    if (final_checksum > 1000) {
        /* Call at conditional block end */
        opaque_func4(&final_checksum);
        goto done;
    } else {
        /* Different path with call */
        opaque_func1();
        final_checksum = opaque_func2(final_checksum);
    }
    
done:
    return final_checksum & 0xFF;
}

/* Dummy definitions to satisfy linker (normally would be in separate file) */
void opaque_func1(void) {
    asm volatile ("# Opaque function 1" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("# Opaque function 2" : "+r" (x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile ("# Opaque function 3" : "+t" (x) : : "memory");
    return x * 2.0;
}

void* opaque_func4(void* x) {
    asm volatile ("# Opaque function 4" : "+r" (x) : : "memory");
    return x;
}
