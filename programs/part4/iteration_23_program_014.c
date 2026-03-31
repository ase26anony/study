/* caller-save-test.c - Test program to trigger uncovered code in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void opaque_func4(int, int, int, int, int, int);

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 42;
volatile double g_volatile_double = 3.14159;
volatile long g_volatile_long = 1234567890L;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
func_ptr_t volatile g_func_ptr = NULL;

/* Memory barrier */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test function 1: Many live variables across a call with register constraints */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Use explicit register variables to create pressure */
    register int r10_val asm("r10") = g_volatile_int + 1;
    register int r11_val asm("r11") = g_volatile_int * 2;
    register int r12_val asm("r12") = g_volatile_int / 3;
    volatile int stack_slot[10];
    
    /* Save values that must survive the call */
    stack_slot[0] = r10_val;
    stack_slot[1] = r11_val;
    stack_slot[2] = r12_val;
    
    /* Complex control flow with goto to create basic block boundaries */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    {
        /* Inline asm that clobbers call-clobbered registers */
        asm volatile(
            "movl $0x12345678, %%eax\n\t"
            "movl $0x87654321, %%ecx\n\t"
            "movl $0x55555555, %%edx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ecx", "edx", "memory"
        );
        
        /* Function call that clobbers registers */
        opaque_func1();
        COMPILER_BARRIER();
        
        /* Restore and use saved values - forces reload to insert restore code */
        r10_val = stack_slot[0] + 1;
        r11_val = stack_slot[1] * 2;
        r12_val = stack_slot[2] / 2;
        
        /* Use the values in a way that can't be optimized away */
        g_volatile_int = r10_val + r11_val + r12_val;
        goto end;
    }
    
label2:
    {
        /* Different path with another call */
        int result = opaque_func2(g_volatile_int);
        
        /* Complex expression requiring temporary registers */
        r10_val = (result * r10_val + r11_val - r12_val) / (result + 1);
        g_volatile_int = r10_val;
    }
    
end:
    /* Force register pressure with many live variables */
    asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r12_val));
}

/* Test function 2: Nested calls with floating point */
__attribute__((noinline, noclone))
void test2(double input) {
    volatile double saved[8];
    register double f1 asm("xmm0") = input;
    register double f2 asm("xmm1") = input * 2.0;
    register double f3 asm("xmm2") = input / 2.0;
    
    /* Save across call */
    saved[0] = f1;
    saved[1] = f2;
    saved[2] = f3;
    
    /* Switch statement to create complex CFG */
    int choice = g_volatile_int % 4;
    
    switch (choice) {
        case 0:
            opaque_func3(f1);
            break;
        case 1:
            opaque_func3(f2);
            break;
        case 2:
            opaque_func3(f3);
            break;
        default: {
            /* Default case with multiple calls in sequence */
            double temp = opaque_func3(f1);
            COMPILER_BARRIER();
            temp += opaque_func3(f2);
            COMPILER_BARRIER();
            temp += opaque_func3(f3);
            f1 = temp;
            
            /* This creates a basic block that ends with a call */
            if (temp > 0.0) {
                opaque_func1();
                /* Force insertion point after call */
                f2 = saved[1] + temp;
            }
            break;
        }
    }
    
    /* Restore and use values */
    f1 = saved[0] + 1.0;
    f2 = saved[1] * 2.0;
    f3 = saved[2] / 2.0;
    
    /* Use in computation */
    g_volatile_double = f1 + f2 + f3;
}

/* Test function 3: Many arguments forcing register spills */
__attribute__((noinline, noclone))
void test3(void) {
    /* Create many live values */
    int a = g_volatile_int + 1;
    int b = g_volatile_int * 2;
    int c = g_volatile_int / 3;
    int d = g_volatile_int - 4;
    int e = g_volatile_int + 5;
    int f = g_volatile_int * 6;
    
    volatile int saved[6];
    saved[0] = a;
    saved[1] = b;
    saved[2] = c;
    saved[3] = d;
    saved[4] = e;
    saved[5] = f;
    
    /* Loop with break/continue creating block boundaries */
    for (int i = 0; i < 10; i++) {
        if (i == g_volatile_int % 5) {
            /* Call with many arguments - some will be spilled */
            opaque_func4(a, b, c, d, e, f);
            COMPILER_BARRIER();
            continue;
        }
        
        if (i == 7) {
            /* Another call site */
            opaque_func1();
            break;
        }
        
        /* Modify values */
        a += i;
        b *= (i + 1);
    }
    
    /* Use saved values after loop */
    g_volatile_int = saved[0] + saved[1] + saved[2] + 
                     saved[3] + saved[4] + saved[5];
}

/* Test function 4: Irreducible control flow with gotos */
__attribute__((noinline, noclone))
void test4(int x) {
    volatile int saved[4];
    register int r8 asm("r8") = x;
    register int r9 asm("r9") = x * 2;
    
    saved[0] = r8;
    saved[1] = r9;
    
    /* Complex goto pattern */
    if (x > 100) goto block_a;
    if (x < 0) goto block_c;
    
block_b:
    opaque_func2(r8);
    COMPILER_BARRIER();
    r9 = saved[1];
    goto block_d;
    
block_a:
    opaque_func2(r9);
    COMPILER_BARRIER();
    r8 = saved[0];
    if (x > 200) goto block_b;
    else goto block_d;
    
block_c:
    opaque_func1();
    COMPILER_BARRIER();
    r8 = saved[0] + saved[1];
    goto block_b;
    
block_d:
    /* Use both register values */
    g_volatile_int = r8 + r9;
    
    /* Inline asm that looks like a call */
    asm volatile(
        "call *%0\n\t"
        : /* no outputs */
        : "r" (g_func_ptr)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
}

/* Helper function with nested call */
__attribute__((noinline, noclone))
int helper_with_nested_call(int x, int y) {
    volatile int saved[3];
    register int r10 asm("r10") = x;
    register int r11 asm("r11") = y;
    
    saved[0] = r10;
    saved[1] = r11;
    
    /* Nested call scenario */
    int result = opaque_func2(x);
    
    /* Use saved values after nested call */
    r10 = saved[0] + result;
    r11 = saved[1] * result;
    
    /* Another call */
    opaque_func2(r10);
    
    return r10 + r11;
}

/* Test function 5: Uses __builtin_apply for unusual calling convention */
__attribute__((noinline, noclone))
void test5(void) {
    /* Create argument buffer for __builtin_apply */
    void *args = __builtin_apply_args();
    
    /* Save volatile state */
    volatile long saved = g_volatile_long;
    register long rbx_save asm("rbx");
    
    /* Save rbx manually */
    asm volatile("mov %%rbx, %0" : "=r" (rbx_save));
    
    /* Complex expression that uses __builtin_apply */
    void *result = __builtin_apply((void (*)())opaque_func1, args, 64);
    
    /* Restore rbx */
    asm volatile("mov %0, %%rbx" : : "r" (rbx_save));
    
    /* Use saved value */
    g_volatile_long = saved + (long)result;
}

/* Main function with mode selection */
int main(int argc, char **argv) {
    int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Initialize function pointer (simulate dlsym-like behavior) */
    g_func_ptr = (func_ptr_t)((long)opaque_func1);
    
    /* Run all test functions in different orders based on mode */
    switch (mode % 5) {
        case 0:
            test1(mode);
            test2(g_volatile_double);
            test3();
            test4(g_volatile_int);
            test5();
            break;
        case 1:
            test4(g_volatile_int);
            test5();
            test1(mode);
            test2(g_volatile_double);
            test3();
            break;
        case 2:
            test2(g_volatile_double);
            test3();
            test4(g_volatile_int);
            test5();
            test1(mode);
            break;
        case 3:
            test3();
            test4(g_volatile_int);
            test5();
            test1(mode);
            test2(g_volatile_double);
            break;
        case 4:
            /* Test nested call helper */
            for (int i = 0; i < 3; i++) {
                int result = helper_with_nested_call(g_volatile_int + i, i * 10);
                g_volatile_int += result;
            }
            test1(mode);
            test2(g_volatile_double);
            test3();
            test4(g_volatile_int);
            test5();
            break;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = g_volatile_int + (long)g_volatile_double + g_volatile_long;
    
    /* Use checksum in a way that can't be optimized away */
    if (checksum != 0) {
        printf("Checksum: %ld\n", checksum);
    }
    
    return 0;
}

/* Dummy definitions to satisfy linker (in real test, these would be in a library) */
void opaque_func1(void) {
    /* Empty but marked noinline */
    asm volatile("");
}

int opaque_func2(int x) {
    return x + 1;
}

double opaque_func3(double x) {
    return x * 2.0;
}

void opaque_func4(int a, int b, int c, int d, int e, int f) {
    g_volatile_int = a + b + c + d + e + f;
}
